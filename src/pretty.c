
#include <stdbool.h>
#include <string.h>
#include <ctype.h> /* isdigit */

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "include/pretty.h"
#include "include/request.h"
#include "include/util.h"
#include "include/main.h"

/*
 * Structure representing an HTML entity string, along with its corresponding
 * ASCII character.
 */
typedef struct {
    const char* html_str;
    size_t html_str_len;
    char character;
} HtmlEntityPair;

/*
 * Helper macro to create an 'HtmlEntityPair' structure from a string literal
 * and an ASCII character at compile-time.
 */
#define HTML_ENTITY_PAIR(HTML, CH)                                             \
    { .html_str = HTML, .html_str_len = STRLEN(HTML), .character = CH }

/*
 * Replace each HTML entity in the input string with its corresponding ASCII
 * character.
 */
static char* replace_html_entities(char* str) {
    static const HtmlEntityPair entity_map[] = {
        HTML_ENTITY_PAIR("&quot;", '\"'), HTML_ENTITY_PAIR("&apos;", '\''),
        HTML_ENTITY_PAIR("&amp;", '&'),   HTML_ENTITY_PAIR("&lt;", '<'),
        HTML_ENTITY_PAIR("&gt;", '>'),    HTML_ENTITY_PAIR("&#034;", '\"'),
        HTML_ENTITY_PAIR("&#039", '\''),  HTML_ENTITY_PAIR("&#038;", '&'),
        HTML_ENTITY_PAIR("&#060;", '<'),  HTML_ENTITY_PAIR("&#062;", '>'),
    };

    /*
     * For each entity in the map, iterate all occurrences of that entity in the
     * target string. For each occurrence, insert the replacement character and
     * shift the rest of the input.
     */
    for (size_t i = 0; i < ARRLEN(entity_map); i++) {
        char* cur_ent;
        while ((cur_ent = strstr(str, entity_map[i].html_str)) != NULL) {
            *cur_ent = entity_map[i].character;
            my_strcpy(cur_ent + 1, cur_ent + entity_map[i].html_str_len);
        }
    }

    return str;
}

static char* html2txt(char* str) {
    char* const start = str;

    char* label_start = str;
    bool in_br        = false;

    while (*str != '\0') {
        switch (*str) {
            case '<': {
                label_start = str;
                str++;

                /* If label is <br>, store */
                if (*str++ == 'b' && *str++ == 'r')
                    in_br = true;
            } break;

            case '>': {
                /* If we are closing a <br> string, insert '\n' */
                if (in_br) {
                    *label_start++ = '\n';
                    in_br          = false;
                }

                /* Shift rest of string */
                my_strcpy(label_start, str + 1);
                str = label_start;
            } break;

            default: {
                str++;
            } break;
        }
    }

    return start;
}

static inline void print_pad(void) {
    const int pad = 6;

    printf("%s", COL_NORM);
    for (int i = 0; i < pad; i++)
        putchar(' ');
}

static inline bool iswhitespace(char c) {
    return c == ' ' || c == '\n';
}

static void print_post(const char* str, bool use_pad) {
    bool first_of_line = true;
    bool in_quote      = false; /* >text */

    printf("%s", COL_POST);

    for (size_t i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
            case '>':
                if (first_of_line && use_pad)
                    print_pad();

                if (str[i + 1] == '>') { /* >>... */
                    const char* last_col = (in_quote) ? COL_QUOTE : COL_POST;

                    if (str[i + 2] == '>') { /* >>>... */
                        printf(COL_XPOST ">>>");

                        for (i += 3; !iswhitespace(str[i]); i++)
                            putchar(str[i]);
                        i--;
                    } else if (isdigit(str[i + 2])) { /* >>1231231 */
                        printf(COL_XPOST ">>");

                        for (i += 2; isdigit(str[i]); i++)
                            putchar(str[i]);
                        i--;
                    } else {                  /* >>text */
                        last_col = COL_QUOTE; /* Force override to quote */
                        in_quote = true;
                    }

                    /* Reset color after >>... */
                    printf("%s", last_col);
                } else if (!in_quote && first_of_line) { /* >... */
                    printf(COL_QUOTE ">");
                    in_quote = true;
                }

                first_of_line = false;
                break;
            case '\n':
                /* Reset quotes on newline */
                printf("%s", COL_POST);
                in_quote = false;

                first_of_line = true;
                putchar('\n');
                break;
            default:
                if (first_of_line && use_pad)
                    print_pad();

                first_of_line = false;
                putchar(str[i]);
                break;
        }
    }

    printf("%s", COL_NORM);
}

/* Print thread information */
bool print_thread_info(CURL* curl, ThreadId id) {
    static char url[255] = { '\0' };
    snprintf(url, 255, "https://a.4cdn.org/" BOARD "/thread/%lu.json", id);

    cJSON* thread = request_json_from_url(curl, url);
    if (!thread) {
        PANIC("json_from_url returned NULL (%lu)", id);
        return false;
    }

    cJSON* posts = cJSON_GetObjectItemCaseSensitive(thread, "posts");
    if (!posts || !cJSON_IsArray(posts)) {
        PANIC("Can't find \"posts\" array in thread JSON (%lu)", id);
        return false;
    }

    /* Is this the first post? */
    int post_count = 0;

    cJSON* p;
    cJSON_ArrayForEach(p, posts) {
        cJSON* post_replies  = cJSON_GetObjectItemCaseSensitive(p, "replies");
        cJSON* post_images   = cJSON_GetObjectItemCaseSensitive(p, "images");
        cJSON* post_title    = cJSON_GetObjectItemCaseSensitive(p, "sub");
        cJSON* post_filename = cJSON_GetObjectItemCaseSensitive(p, "filename");
        cJSON* post_ext      = cJSON_GetObjectItemCaseSensitive(p, "ext");
        cJSON* post_img_url  = cJSON_GetObjectItemCaseSensitive(p, "tim");
        cJSON* post_content  = cJSON_GetObjectItemCaseSensitive(p, "com");

        putchar('\n');
        if (post_count > 0)
            print_pad();

        /* Post ID and title */
        printf(COL_INFO "[%lu] " COL_NORM, id);

        if (is_cjson_str(post_title))
            printf(COL_TITLE "%s" COL_NORM,
                   replace_html_entities(post_title->valuestring));
        else
            printf(COL_TITLE "Anonymous" COL_NORM);

        if (is_cjson_int(post_replies)) {
            printf(COL_REPLIES " (%d replies", post_replies->valueint);

            if (is_cjson_int(post_images))
                printf(", %d images", post_images->valueint);

            printf(")" COL_NORM);
        }

        /* Image URL and filename. Need to use valuedouble because of size */
        if (is_cjson_int(post_img_url) && is_cjson_str(post_ext)) {
            putchar('\n');
            if (post_count > 0)
                print_pad();

            printf(COL_URL "https://i.4cdn.org/" BOARD "/%.0f%s" COL_NORM,
                   post_img_url->valuedouble,
                   post_ext->valuestring);

            if (is_cjson_str(post_filename))
                printf(" (" COL_FILENAME "%s%s" COL_NORM ")",
                       post_filename->valuestring,
                       post_ext->valuestring);
        }

        /* Post content */
        if (is_cjson_str(post_content)) {
            const char* converted =
              replace_html_entities(html2txt(post_content->valuestring));

            putchar('\n');
            print_post(converted, post_count > 0);
        }

        putchar('\n');
        post_count++;
    }

    cJSON_Delete(thread);
    return true;
}

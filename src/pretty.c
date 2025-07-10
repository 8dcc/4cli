
#include <stdbool.h>
#include <string.h>
#include <ctype.h> /* isdigit */

#include <curl/curl.h>
#include "dependencies/cJSON/cJSON.h"

#include "include/pretty.h"
#include "include/util.h"
#include "include/main.h"

typedef struct {
    int strlen;
    const char* str;
    char c;
} HtmlEntityPair;

static char* replace_html_entities(char* str) {
    static const HtmlEntityPair ents[] = {
        { 6, "&quot;", '\"' }, /**/
        { 6, "&apos;", '\'' }, /**/
        { 5, "&amp;", '&' },   /**/
        { 4, "&lt;", '<' },    /**/
        { 4, "&gt;", '>' },    /**/
        { 6, "&#034;", '\"' }, /**/
        { 6, "&#039", '\'' },  /**/
        { 6, "&#038;", '&' },  /**/
        { 6, "&#060;", '<' },  /**/
        { 6, "&#062;", '>' },  /**/
    };

    /* Each HTML entity */
    for (int i = 0; i < LENGTH(ents); i++) {
        char* cur_ent;

        /* Each ocurrence in the target string */
        while ((cur_ent = strstr(str, ents[i].str)) != NULL) {
            /* Replace with real char */
            *cur_ent = ents[i].c;

            /* Shift the rest of the string */
            my_strcpy(cur_ent + 1, cur_ent + ents[i].strlen);
        }
    }

    return str;
}

static char* html2txt(char* str) {
    char* ret = str;

    char* label_start = str;
    bool in_br        = false;

    while (*str != '\0') {
        switch (*str) {
            case '<':
                label_start = str;
                str++;

                /* If label is <br>, store */
                if (*str++ == 'b' && *str++ == 'r')
                    in_br = true;
                break;
            case '>':
                /* We are closing a <br> string, place '\n' before shifting */
                if (in_br) {
                    *label_start = '\n';
                    label_start++;
                    in_br = false;
                }

                /* Shift rest of string */
                my_strcpy(label_start, str + 1);
                str = label_start;
                break;
            default:
                str++;
                break;
        }
    }

    return ret;
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
bool print_thread_info(Thread id) {
    static char url[255] = { '\0' };
    snprintf(url, 255, "https://a.4cdn.org/" BOARD "/thread/%d.json", id);

    cJSON* thread = json_from_url(url);
    if (!thread) {
        PANIC("json_from_url returned NULL (%d)", id);
        return false;
    }

    cJSON* posts = cJSON_GetObjectItemCaseSensitive(thread, "posts");
    if (!posts || !cJSON_IsArray(posts)) {
        PANIC("Can't find \"posts\" array in thread JSON (%d)", id);
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
        printf(COL_INFO "[%d] " COL_NORM, id);

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

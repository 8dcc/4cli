
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> /* isdigit */

#include <curl/curl.h>
#include "dependencies/cJSON/cJSON.h"

#include "include/util.h"

typedef struct {
    char* data;
    size_t sz;
} MemChunk;

typedef struct {
    int strlen;
    const char* str;
    char c;
} HtmlEntityPair;

/*----------------------------------------------------------------------------*/

/* string.h function does weird stuff when strings overlap (our case) */
static char* my_strcpy(char* dest, const char* src) {
    size_t i;

    for (i = 0; src[i] != '\0'; i++)
        dest[i] = src[i];

    dest[i] = '\0';

    return dest;
}

/* Curl callback */
static size_t parse_curl_response(char* response, size_t item_sz,
                                  size_t item_num, void* user_data) {
    /* Total bytes. This value should be returned */
    size_t real_sz = item_sz * item_num;

    /* Set by CURLOPT_WRITEDATA in main */
    MemChunk* mem = (MemChunk*)user_data;

    char* ptr = realloc(mem->data, mem->sz + real_sz + 1);
    if (!ptr) {
        PANIC("Couldn't realloc from %ld to %ld bytes", mem->sz,
              mem->sz + real_sz + 1);
        return 0;
    }

    mem->data = ptr;
    memcpy(&mem->data[mem->sz], response, real_sz);
    mem->sz += real_sz;
    mem->data[mem->sz] = '\0';

    return real_sz;
}

/* Initialize cJSON object from parsed URL response */
cJSON* json_from_url(const char* url) {
    /* Main memory chunk used for curl responses. The data attribute will get
     * reallocated in parse_curl_response() */
    MemChunk mem = {
        .data = malloc(1),
        .sz   = 0,
    };

    /* Set target URL, callback function and user_data parameter for callback */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parse_curl_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);

    /* Make request to get the JSON string */
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        PANIC("curl_easy_perform failed for URL \"%s\": %s", url,
              curl_easy_strerror(res));
        free(mem.data);
        return NULL;
    }

    /* Fill JSON object parameter with parsed response */
    cJSON* ret = cJSON_Parse(mem.data);
    if (!ret) {
        PANIC("cJSON_Parse returned NULL");
        free(mem.data);
        return NULL;
    }

    free(mem.data);
    return ret;
}

/* Fill thread ID list from threads.json object */
bool threads_from_json(Thread* out, cJSON* in) {
    int i = 0;

    cJSON* cur_page;
    cJSON_ArrayForEach(cur_page, in) {
        cJSON* json_threads = cJSON_GetObjectItemCaseSensitive(cur_page, "threa"
                                                                         "ds");
        if (!json_threads) {
            PANIC("Couldn't get thread list cJSON object from page");
            return false;
        }

        cJSON* cur_thread;
        cJSON_ArrayForEach(cur_thread, json_threads) {
            cJSON* cur_thread_no =
              cJSON_GetObjectItemCaseSensitive(cur_thread, "no");
            if (!cJSON_IsNumber(cur_thread_no)) {
                PANIC("Thread number was not an integer");
                return false;
            }

            out[i++] = cur_thread_no->valueint;

            if (i >= MAX_THREADS) {
                PANIC("Reached MAX_THREADS");
                return false;
            }
        }
    }

    return true;
}

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

static inline bool iswhitespace(char c) {
    return c == ' ' || c == '\n';
}

static void print_post(const char* str) {
    bool first_of_line = true;
    bool in_quote      = false; /* >text */

    printf("%s", COL_POST);

    for (size_t i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
            case '>':
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
                first_of_line = false;
                putchar(str[i]);
                break;
        }
    }

    printf("%s", COL_NORM);
}

static inline bool is_cjson_int(cJSON* p) {
    return p && cJSON_IsNumber(p);
}

static inline bool is_cjson_str(cJSON* p) {
    return p && cJSON_IsString(p);
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

    /* TODO: Iterate posts of thread. If post 0, print normally, otherwise, add
     * '|' and indentation. */
    cJSON* fp = cJSON_GetArrayItem(posts, 0);
    if (!fp) {
        PANIC("Can't get first post from array in thread JSON (%d)", id);
        return false;
    }

    /* TODO: Rename to post_* */
    cJSON* thread_replies  = cJSON_GetObjectItemCaseSensitive(fp, "replies");
    cJSON* thread_images   = cJSON_GetObjectItemCaseSensitive(fp, "images");
    cJSON* thread_title    = cJSON_GetObjectItemCaseSensitive(fp, "sub");
    cJSON* thread_filename = cJSON_GetObjectItemCaseSensitive(fp, "filename");
    cJSON* thread_ext      = cJSON_GetObjectItemCaseSensitive(fp, "ext");
    cJSON* thread_img_url  = cJSON_GetObjectItemCaseSensitive(fp, "tim");
    cJSON* thread_content  = cJSON_GetObjectItemCaseSensitive(fp, "com");

    /* Post ID and title */
    printf(COL_INFO "[%d] " COL_NORM, id);

    if (is_cjson_str(thread_title))
        printf(COL_TITLE "%s" COL_NORM,
               replace_html_entities(thread_title->valuestring));
    else
        printf(COL_TITLE "Anonymous" COL_NORM);

    if (is_cjson_int(thread_replies)) {
        printf(COL_REPLIES " (%d replies", thread_replies->valueint);

        if (is_cjson_int(thread_images))
            printf(", %d images", thread_images->valueint);

        printf(")" COL_NORM);
    }

    putchar('\n');

    /* Image URL and filename. Need to use valuedouble because it's a big int */
    if (is_cjson_int(thread_img_url) && is_cjson_str(thread_ext)) {
        printf(COL_URL "https://i.4cdn.org/" BOARD "/%.0f%s" COL_NORM,
               thread_img_url->valuedouble, thread_ext->valuestring);

        if (is_cjson_str(thread_filename))
            printf(" (" COL_FILENAME "%s%s" COL_NORM ")",
                   thread_filename->valuestring, thread_ext->valuestring);

        putchar('\n');
    }

    if (is_cjson_str(thread_content)) {
        const char* converted =
          replace_html_entities(html2txt(thread_content->valuestring));

        putchar('\n');
        print_post(converted);
        putchar('\n');
    }

    putchar('\n');

    cJSON_Delete(thread);
    return true;
}

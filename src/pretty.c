/*
 * Copyright 2025 8dcc
 *
 * This file is part of 4cli.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <string.h>
#include <ctype.h> /* isdigit */

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
        HTML_ENTITY_PAIR("&#039;", '\''), HTML_ENTITY_PAIR("&#038;", '&'),
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
            STRMOVE(cur_ent + 1, cur_ent + entity_map[i].html_str_len);
        }
    }

    return str;
}

/*
 * Perform a basic conversion between HTML and plain text. The conversion is
 * done in place, and the input buffer is returned.
 */
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
                STRMOVE(label_start, str + 1);
                str = label_start;
            } break;

            default: {
                str++;
            } break;
        }
    }

    return start;
}

/*
 * Check if the specified cJSON pointer is non-null number.
 */
static inline bool is_cjson_num(cJSON* p) {
    return p != NULL && cJSON_IsNumber(p);
}

/*
 * Check if the specified cJSON pointer is non-null string.
 */
static inline bool is_cjson_str(cJSON* p) {
    return p != NULL && cJSON_IsString(p);
}

/*
 * Print a constant ammount of padding.
 */
static inline void print_pad(void) {
    const int pad = 6;
    for (int i = 0; i < pad; i++)
        putchar(' ');
}

/*
 * Print the specified string as if they were the contents of a 4chan post.
 */
static void print_post_contents(const char* str, bool use_pad) {
    bool in_quote = false; /* >text */

    for (size_t i = 0; str[i] != '\0'; i++) {
        const bool first_of_line = (i == 0 || str[i - 1] == '\n');

        /* Whenever we change line, reset color and quote state */
        if (first_of_line) {
            printf(COL_POST);
            in_quote = false;
        }

        /* If we reached this point, we are not changing line */
        if (first_of_line && use_pad)
            print_pad();

        /*
         * The current character doesn't start a quote-like block, print it
         * normally.
         */
        if (str[i] != '>') {
            putchar(str[i]);
            continue;
        }

        /*
         * If we reached this point, the character is a quote arrow. If it's
         * just one and it's at the start of a line, it's a simple quote like:
         *   >text
         */
        if (str[i + 1] != '>') {
            if (first_of_line && !in_quote) {
                printf(COL_QUOTE);
                in_quote = true;
            }
            putchar('>');
            continue;
        }

        /*
         * If we reached this point, there are 2 or more consecutive quote
         * characters. First, store current color, so we can reset it later.
         */
        const char* last_col = (in_quote) ? COL_QUOTE : COL_POST;

        /*
         * Check if there are only two consecutive arrows, like:
         *   >>123456789
         *   >>text
         *
         * If the character after them is a digit, it's a post ID. Just print
         * the highlighted number now.
         *
         * If the character after them is not a digit, just print the arrows
         * normally, and the text will be printed in future iterations.
         */
        if (str[i + 2] != '>') {
            if (isdigit(str[i + 2])) {
                printf(COL_XPOST ">>");
                i += 1; /* Skip arrow */
                while (isdigit(str[i + 1]))
                    putchar(str[++i]);
                printf("%s", last_col);
            } else {
                printf(">>");
                i++; /* Skip second arrow, so next iteration prints text */
            }
            continue;
        }

        /*
         * If we reached this point, there are 3 (or more) consecutive quote
         * characters. Just print them, and highlight up to the next space.
         */
        printf(COL_XPOST ">>>");
        i += 2; /* Skip arrows */
        while (!isspace(str[i + 1]))
            putchar(str[++i]);
        printf("%s", last_col);
    }

    printf("%s", COL_NORM);
}

bool pretty_print_thread(cJSON* thread_json) {
    cJSON* posts = cJSON_GetObjectItemCaseSensitive(thread_json, "posts");
    if (!posts || !cJSON_IsArray(posts))
        return false;

    /* Is this the first post? */
    int post_count = 0;

    cJSON* p;
    cJSON_ArrayForEach(p, posts) {
        cJSON* post_no       = cJSON_GetObjectItemCaseSensitive(p, "no");
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

        /* Post ID */
        if (is_cjson_num(post_no))
            printf(COL_INFO "[%d] " COL_NORM, post_no->valueint);
        else
            printf(COL_INFO "[???] " COL_NORM);

        /* Title */
        if (is_cjson_str(post_title))
            printf(COL_TITLE "%s" COL_NORM,
                   replace_html_entities(post_title->valuestring));
        else
            printf(COL_TITLE "Anonymous" COL_NORM);

        /* Reply and image count */
        if (is_cjson_num(post_replies)) {
            printf(COL_REPLIES " (%d replies", post_replies->valueint);
            if (is_cjson_num(post_images))
                printf(", %d images", post_images->valueint);
            printf(")" COL_NORM);
        }

        /* Image URL and filename */
        if (is_cjson_num(post_img_url) && is_cjson_str(post_ext)) {
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

        /* Post contents */
        if (is_cjson_str(post_content)) {
            const char* converted =
              replace_html_entities(html2txt(post_content->valuestring));

            putchar('\n');
            print_post_contents(converted, post_count > 0);
        }

        putchar('\n');
        post_count++;
    }

    return true;
}

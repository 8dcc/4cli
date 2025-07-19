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
 * Number of spaces used for indenting post replies.
 */
#define POST_PAD 6

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
 * Print a constant amount of padding.
 */
static inline void print_pad(FILE* fp, int amount) {
    for (int i = 0; i < amount; i++)
        fputc(' ', fp);
}

/*
 * Print the specified string as if they were the contents of a 4chan post.
 */
static void print_post_contents(FILE* fp, const char* str, bool use_pad) {
    bool in_quote = false; /* >foo */
    enum {
        XPOST_NONE,
        XPOST_DIGITS, /* >>123456789 */
        XPOST_TEXT,   /* >>>/foo/ */
    } xpost_state = XPOST_NONE;

    for (size_t i = 0; str[i] != '\0'; i++) {
        const bool first_of_line = (i == 0 || str[i - 1] == '\n');

        /* Whenever we change line, reset color and quote state */
        if (first_of_line) {
            fprintf(fp, COL_POST);
            in_quote = false;
            if (use_pad)
                print_pad(fp, POST_PAD);
        }

        /* Check if this character starts a quote, and what kind */
        if (str[i] == '>') {
            if (str[i + 1] == '>') {
                if (isdigit(str[i + 2])) {
                    xpost_state = XPOST_DIGITS; /* >>123456789 */
                    fprintf(fp, COL_XPOST);
                } else if (str[i + 2] == '>') {
                    xpost_state = XPOST_TEXT; /* >>>/foo/ */
                    fprintf(fp, COL_XPOST);
                }
            } else if (first_of_line && !in_quote) {
                in_quote = true; /* >foo */
                fprintf(fp, COL_QUOTE);
            }
        } else if ((xpost_state == XPOST_DIGITS && !isdigit(str[i])) ||
                   (xpost_state == XPOST_TEXT && isspace(str[i]))) {
            const char* old_color = (in_quote) ? COL_QUOTE : COL_POST;
            fprintf(fp, "%s", old_color);
            xpost_state = XPOST_NONE;
        }

        fputc(str[i], fp);
    }

    fprintf(fp, "%s", COL_NORM);
}

bool pretty_print_thread(FILE* fp, cJSON* thread_json) {
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

        fputc('\n', fp);
        if (post_count > 0)
            print_pad(fp, POST_PAD);

        /* Post ID */
        if (is_cjson_num(post_no))
            fprintf(fp, COL_INFO "[%d] " COL_NORM, post_no->valueint);
        else
            fprintf(fp, COL_INFO "[???] " COL_NORM);

        /* Title */
        if (is_cjson_str(post_title))
            fprintf(fp,
                    COL_TITLE "%s" COL_NORM,
                    replace_html_entities(post_title->valuestring));
        else
            fprintf(fp, COL_TITLE "Anonymous" COL_NORM);

        /* Reply and image count */
        if (is_cjson_num(post_replies)) {
            fprintf(fp, COL_REPLIES " (%d replies", post_replies->valueint);
            if (is_cjson_num(post_images))
                fprintf(fp, ", %d images", post_images->valueint);
            fprintf(fp, ")" COL_NORM);
        }

        /* Image URL and filename */
        if (is_cjson_num(post_img_url) && is_cjson_str(post_ext)) {
            fputc('\n', fp);
            if (post_count > 0)
                print_pad(fp, POST_PAD);

            fprintf(fp,
                    COL_URL "https://i.4cdn.org/" BOARD "/%.0f%s" COL_NORM,
                    post_img_url->valuedouble,
                    post_ext->valuestring);

            if (is_cjson_str(post_filename))
                fprintf(fp,
                        " (" COL_FILENAME "%s%s" COL_NORM ")",
                        post_filename->valuestring,
                        post_ext->valuestring);
        }

        /* Post contents */
        if (is_cjson_str(post_content)) {
            const char* converted =
              replace_html_entities(html2txt(post_content->valuestring));

            fputc('\n', fp);
            print_post_contents(fp, converted, post_count > 0);
        }

        fputc('\n', fp);
        post_count++;
    }

    return true;
}

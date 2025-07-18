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

#ifndef REQUEST_H_
#define REQUEST_H_ 1

#include <curl/curl.h>
#include <cjson/cJSON.h>

/*
 * Request the contents of the specified URL, and parse them as JSON. The 'curl'
 * argument should have been initialized through 'curl_easy_init'.
 */
cJSON* request_json_from_url(CURL* curl, const char* url);

#endif /* REQUEST_H_ */

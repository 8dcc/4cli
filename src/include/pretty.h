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

#ifndef PRETTY_H_
#define PRETTY_H_ 1

#include <stdbool.h>

#include <cjson/cJSON.h>

/*
 * Print the contents of a specific thread JSON to the standard output.
 *
 * TODO: Print to specificed file.
 */
bool pretty_print_thread(cJSON* thread_json);

#endif /* PRETTY_H_ */

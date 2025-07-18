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

#ifndef MAIN_H_
#define MAIN_H_ 1

/*
 * Compile-time configuration.
 */
#define BOARD       "g"
#define THREADS_URL "https://a.4cdn.org/" BOARD "/threads.json"

/*
 * Maximum number of threads (not posts) to parse and print.
 */
#define MAX_THREADS 255

/*
 * Entry point of the program.
 */
int main(void);

#endif /* MAIN_H_ */

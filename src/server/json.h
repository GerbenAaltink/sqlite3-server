#ifndef SQLITE3_SERVER_JSON_H
#define SQLITE3_SERVER_JSON_H

#include "string.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool json_validate(char *content) {
    if (!content)
        return false;
    if (strlen(content) < 2)
        return false;
    char cleaned[strlen(content)];
    stripwhitespace(content, cleaned);
    if (strlen(cleaned) < 2)
        return false;
    if (cleaned[0] != '{' || cleaned[strlen(cleaned) - 1] != '}')
        return false;
    int braces_open = 0;
    int squares_open = 0;
    bool escape = false;
    bool in_string = false;
    char expected = 0;
    for (size_t i = 0; i < strlen(cleaned); i++) {
        if (cleaned[i] == '\'')
            cleaned[i] = '"';

        if (!in_string && strchr("\r\n\t \f", cleaned[i]))
            continue;

        if (in_string && cleaned[i] == '\\') {
            escape = true;
            continue;
        }

        if (expected == '"' || expected == 1) {
            if (cleaned[i] == '"' && !escape) {
                if (in_string == true) {
                    in_string = false;
                    expected = 4;
                } else {
                    in_string = true;
                }
                continue;
            } else if (in_string) {
                continue;
            }
        }

        if (expected == '"' && cleaned[i] != expected)
            return false;

        if (expected == 1) {
            if (isdigit(cleaned[i])) {
                expected = 3;
                continue;
            }

            if (strlen(cleaned + i) >= 4 && strncmp(cleaned + i, "true", 4) == 0) {
                expected = 4;
                i += 3;
                continue;
            } else if (strlen(cleaned + i) >= 5 && strncmp(cleaned + i, "false", 5) == 0) {
                expected = 4;
                i += 4;
                continue;
            }
            return false;
        }

        if (expected == 3) {
            if (cleaned[i] == ',') {
                expected = 0;
            } else if (cleaned[i] == '}') {
                expected = 0;
            } else if (cleaned[i] == ']') {
                expected = 0;
            } else if (isdigit(cleaned[i]) || cleaned[i] == '.') {
                continue;
            } else {
                printf("HIERSOO\n");
                return false;
            }
        }
        if (expected == 4) {
            if (!strchr("}],:", cleaned[i])) {
                return false;
            } else {
                printf("VIER!!:%c %d\n", cleaned[i], in_string);

                expected = 0;
            }
        }

        escape = false;
        if (!in_string) {
            if (cleaned[i] == '{') {
                printf("Start object %c\n", cleaned[i]);
                braces_open++;
                expected = '"';
            } else if (cleaned[i] == ':') {
                printf("Start value %c\n", cleaned[i]);
                expected = 1;
            } else if (cleaned[i] == '}') {
                printf("Exit object %c\n", cleaned[i]);
                braces_open--;
            } else if (cleaned[i] == '[') {
                printf("Start array %c\n", cleaned[i]);
                squares_open++;
            } else if (cleaned[i] == ']') {

                printf("Exit array %c\n", cleaned[i]);
                squares_open--;
            } else if (cleaned[i] == ',') {
                printf("Expected to 1\n");
                expected = 1;
            }
            if (braces_open < 0 || squares_open < 0)
                return false;
        }
    }
    return braces_open == 0 && squares_open == 0 && !in_string;
}

#endif
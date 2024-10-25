#ifndef SQLITE3_SERVER_STRING_H
#define SQLITE3_SERVER_STRING_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stripslashes(const char *content, char *result) {
  size_t content_length = strlen((char *)content);
  unsigned int index = 0;
  for (unsigned int i = 0; i < content_length; i++) {
    char c = content[i];
    if (c == '\\') {
      i++;
      c = content[i];
      if (c == 'r') {
        c = '\r';
      } else if (c == 't') {
        c = '\t';
      } else if (c == 'b') {
        c = '\b';
      } else if (c == 'n') {
        c = '\n';
      } else if (c == 'f') {
        c = '\f';
      } else if (c == '\\') {
        // No need tbh
        c = '\\';
      }
    }
    result[index] = c;
    index++;
  }
  result[index] = 0;
}

void addslashes(const char *content, char *result) {
  size_t content_length = strlen((char *)content);
  unsigned int index = 0;
  for (unsigned int i = 0; i < content_length; i++) {
    if (content[i] == '\r') {
      result[index] = '\\';
      index++;
      result[index] = 'r';
      index++;
      continue;
    } else if (content[i] == '\t') {
      result[index] = '\\';
      index++;
      result[index] = 't';
      index++;
      continue;
    } else if (content[i] == '\n') {
      result[index] = '\\';
      index++;
      result[index] = 'n';
      index++;
      continue;
    } else if (content[i] == '\\') {
      result[index] = '\\';
      index++;
      result[index] = '\\';
      index++;
      continue;
    } else if (content[i] == '\b') {
      result[index] = '\\';
      index++;
      result[index] = 'b';
      index++;
      continue;
    } else if (content[i] == '\f') {
      result[index] = '\\';
      index++;
      result[index] = 'f';
      index++;
      continue;
    }
    result[index] = content[i];
    index++;
  }
  result[index] = 0;
}

int stripwhitespace(char *input, char *output) {
  output[0] = 0;
  int count = 0;
  size_t len = strlen(input);
  for (size_t i = 0; i < len; i++) {
    if (input[i] == '\t' || input[i] == ' ' || input[i] == '\n') {
      continue;
    }
    count = i;
    size_t j;
    for (j = 0; j < len - count; j++) {
      output[j] = input[j + count];
    }
    output[j] = '\0';
    break;
  }
  return count;
}
#endif

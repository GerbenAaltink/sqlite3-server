#include "json.h"
#include "rlib.h"
#include <ctype.h>
#include <signal.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *path = 0;
sqlite3 *db = 0;
char *errmsg = 0;
int rc = 0;
unsigned int log_line_length = 120;
bool verbose = false;

/*************  ✨ Codeium Command ⭐  *************/
/**
 * Attempt to connect to the database stored in the global variable
 * `path`. If we are unable to connect, print an error message and
 * exit. Otherwise, print a success message.
 */
/******  35b1a65d-4aae-4bef-bd98-5c53c0fe29c2  *******/
void connect_db() {
  if (sqlite3_open(path, &db)) {
    printf("Can't open database: %s\n", sqlite3_errmsg(db));
    exit(1);
  } else {
    printf("Attached to db \"%s\".\n", path);
  }
}
void close_db() {
  if (sqlite3_close(db) != SQLITE_OK) {
    printf("Can't close database: %s\n", sqlite3_errmsg(db));
  } else {
    printf("Database \"%s\" closed.\n", path);
  }
}

void exit_application() {
  close_db();
  printf("Shutting down server.\n");
  exit(0);
}

void handle_sigint(int sig) {
  if (sig == SIGINT) {
    exit_application();
  }
  printf("\nUncaught signal %d (SIGINT). Exiting gracefully...\n", sig);
  exit(1);
}

rliza_t *db_execute(char *query, rliza_t *params) {
  sqlite3_stmt *stmt = 0;

  if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
    rliza_t *error = rliza_new(RLIZA_OBJECT);
    error->set_string(error, "error", (char *)sqlite3_errmsg(db));
    error->set_boolean(error, "success", false);

    printf("Error: %s\n", (char *)sqlite3_errmsg(db));
    return error;
  } else /* if(sqlite3_step(stmt) != 1377){
       rliza_t * error = rliza_new(RLIZA_OBJECT);
       error->set_string(error, "error", (char *)sqlite3_errmsg(db));
       error->set_boolean(error, "success",false);
       return error;
   }else*/
  {
    for (unsigned col = 0; col < params->count; col++) {
      if (params->content.map[col]->type == RLIZA_INTEGER) {
        sqlite3_bind_int(stmt, col + 1,
                         params->content.map[col]->content.integer);
      } else if (params->content.map[col]->type == RLIZA_STRING) {
        if (params->content.map[col]->content.string &&
            *params->content.map[col]->content.string) {
          sqlite3_bind_text(stmt, col + 1,
                            (char *)params->content.map[col]->content.string,
                            -1, SQLITE_STATIC);
        } else {
          printf("BINDED NULL\n");
          sqlite3_bind_null(stmt, col + 1);
        }
      } else if (params->content.map[col]->type == RLIZA_BOOLEAN) {
        printf("BINDED BOOL %s\n", params->content.map[col]->content.boolean);
        sqlite3_bind_int(stmt, col + 1,
                         params->content.map[col]->content.boolean);
      } else if (params->content.map[col]->type == RLIZA_NULL) {
        printf("BINDED EXPLICIT_ NULL\n");
        sqlite3_bind_null(stmt, col + 1);
      } else {
        printf("Unknown parameter type: %s\n", params->content.map[col]->key);
      }
    }

    rliza_t *result = rliza_new(RLIZA_OBJECT);
    long long count = 0;
    rliza_t *rows = rliza_new(RLIZA_ARRAY);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
      rliza_t *row = rliza_new(RLIZA_ARRAY);
      for (int col = 0; col < sqlite3_column_count(stmt); col++) {
        int type = sqlite3_column_type(stmt, col);
        switch (type) {
        case SQLITE_INTEGER:
          // printf("Column %d is an INTEGER with value %d\n", col,
          // sqlite3_column_int(stmt, col));

          rliza_push(
              row, rliza_new_integer((long long)sqlite3_column_int(stmt, col)));
          break;
        case SQLITE_FLOAT:
          // printf("Column %d is a FLOAT with value %f\n", col,
          // sqlite3_column_double(stmt, col));
          break;
        case SQLITE_TEXT:
          // printf("Column %d is TEXT with value %s\n", col,
          // sqlite3_column_text(stmt, col));

          rliza_push(row,
                     rliza_new_string((char *)sqlite3_column_text(stmt, col)));
          break;
        case SQLITE_BLOB:
          // printf("Column %d is a BLOB\n", col);
          break;
        case SQLITE_NULL:
          // printf("Column %d is NULL\n", col);
          break;
        default:
          printf("Unknown column type\n");
        }
      }

      count++;
      rliza_push(rows, row);
    }
    result->set_array(result, "rows", rows);
    result->set_integer(result, "count", (long long)count);
    result->set_boolean(result, "success", true);
    return result;
  }
  return NULL;
}

void print_partial(char *content, unsigned int length) {
  if (strlen(content) >= length) {
    char c = content[length - 2];
    content[length - 2] = '\0';
    printf("%s..\n", content);
    content[length - 2] = c;
  } else {
    printf("%s\n", content);
  }
}

int request_handler(rhttp_request_t *r) {
  long request_content_length = rhttp_header_get_long(r, "Content-Length");

  char *json = (char *)malloc(request_content_length + 1);
  json[request_content_length] = 0;
  long total_size = 0;
  while (total_size != request_content_length) {
    long chunk =
        read(r->c, json + total_size, request_content_length - total_size);
    if (chunk <= 0) {
      close(r->c);
      return 1;
    }
    total_size += chunk;
  }
  if (true) {
    char *jsonp = json;
    if (verbose) {
      printf("req:");
      print_partial(json, log_line_length);
    }
    rliza_t *obj = rliza_object_from_string(&jsonp);

    char *query = (char *)obj->get_string(obj, "query");
    rliza_t *params = obj->get_object(obj, "params");
    rliza_t *response_obj = db_execute(query, params);
    char *response_json = (char *)rliza_object_to_string(response_obj);
    if (verbose) {
      printf("res:");
      print_partial(response_json, log_line_length);
    }
    long response_content_length = strlen(response_json);
    char *response_headers = (char *)calloc(1024, sizeof(char));
    sprintf(response_headers,
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: %ld\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n\r\n",
            response_content_length);
    rhttp_send_drain(r->c, response_headers, 0);
    free(response_headers);
    rhttp_send_drain(r->c, response_json, response_content_length);
    rliza_free(obj);
    free(json);
    rliza_free(response_obj);
    free(response_json);
  }
  close(r->c);
  return 1;
}

rhttp_request_handler_t handler = request_handler;

int main(int argc, char *argv[]) {

  if (atexit(exit_application) != 0) {
    printf(
        "Couldn't register atexit handler. Database may close incorrectly.\n");
  }
  signal(SIGINT, handle_sigint);

  if (argc < 3 || !isdigit(argv[1][0])) {
    printf("Usage: %s <port> <file path>\n", argv[0]);
    exit(1);
  }

  unsigned int port = atoi(argv[1]);
  (void)port;
  path = argv[2];

  connect_db();

  rhttp_serve("0.0.0.0", port, 8096, 1, 1, handler, NULL);

  while (true) {
    printf("Type 'q' to exit.\n");
    if (fgetc(stdin) == 'q') {
      break;
    }
  }
  exit_application();
  return 0;
}
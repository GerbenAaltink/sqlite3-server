#include "json.h"
#include "rlib.h"
#include "sqlite.h"
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
bool verbose = true;

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

char *hexsize(size_t i) {
    char *hex = (char *)malloc(4);
    sprintf(hex, "%X", (unsigned int)i);
    return hex;
}

void connect_db() {
    if (sqlite3_open(path, &db)) {
        printf("Can't open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    } else {
        printf("Attached to db \"%s\".\n", path);
        sqlite3_exec(db, "PRAGMA synchronous = OFF;", NULL, NULL, NULL);
        sqlite3_exec(db, "PRAGMA journal_mode = MEMORY;", NULL, NULL, NULL);
        sqlite3_exec(db, "PRAGMA cache_size = -2000;", NULL, NULL, NULL);
        sqlite3_exec(db, "PRAGMA temp_store = MEMORY;", NULL, NULL, NULL);
    }
    if (sqlite_register_bool_function(db) != SQLITE_OK) {
        printf("Can't register function: %s\n", sqlite3_errmsg(db));
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
    exit(1);
}

size_t write_http_chunk(rhttp_request_t *request, char *response, bool end) {
    size_t response_length = strlen(response);
    char *hex = hexsize(response_length);
    char *chunk_response = (char *)malloc(response_length + 64);
    chunk_response[0] = 0;
    sprintf(chunk_response, "%s\r\n%s\r\n", hex, response);
    free(hex);
    if (end) {
        strcat(chunk_response, "0\r\n\r\n");
    }
    size_t bytes_sent = rhttp_send_drain(request->c, chunk_response, 0);
    if (verbose) {
        if (end) {
            printf("(last)");
        }
        printf("chunk:");
        print_partial(chunk_response, log_line_length);
    }
    free(chunk_response);
    return bytes_sent;
}

size_t db_execute(rhttp_request_t *request, char *query, rliza_t *params) {
    sqlite3_stmt *stmt = 0;
    char * escaped_query = (char *)malloc(strlen(query) + 1);
    rstrstripslashes(query,escaped_query);
    if (sqlite3_prepare_v2(db, escaped_query, -1, &stmt, 0) != SQLITE_OK) {
        free(escaped_query);
        rliza_t *error = rliza_new(RLIZA_OBJECT);
        error->set_string(error, "error", (char *)sqlite3_errmsg(db));
        error->set_boolean(error, "success", false);
        char *json_response = (char *)rliza_object_to_string(error);

        size_t bytes_sent = write_http_chunk(request, json_response, true);
        free(json_response);
        rliza_free(error);

        return bytes_sent;
    } else {
        free(escaped_query);
        for (unsigned col = 0; col < params->count; col++) {
            if (params->content.map[col]->type == RLIZA_INTEGER) {
                sqlite3_bind_int(stmt, col + 1, params->content.map[col]->content.integer);
            } else if (params->content.map[col]->type == RLIZA_STRING) {
                if (params->content.map[col]->content.string && *params->content.map[col]->content.string) {
                    sqlite3_bind_text(stmt, col + 1, (char *)params->content.map[col]->content.string, -1, SQLITE_STATIC);
                } else {
                    sqlite3_bind_null(stmt, col + 1);
                }
            } else if (params->content.map[col]->type == RLIZA_BOOLEAN) {
                sqlite3_bind_int(stmt, col + 1, params->content.map[col]->content.boolean);
            } else if (params->content.map[col]->type == RLIZA_NULL) {
                sqlite3_bind_null(stmt, col + 1);
            } else if (params->content.map[col]->type == RLIZA_NUMBER) {
                sqlite3_bind_double(stmt, col + 1, params->content.map[col]->content.number);
            } else {
                printf("Unknown parameter type: %s\n", params->content.map[col]->key);
            }
        }

        rliza_t *result = rliza_new(RLIZA_OBJECT);
        result->set_boolean(result, "success", true);
        if(!sqlite3_strnicmp(query, "UPDATE", 6) || !sqlite3_strnicmp(query, "DELETE", 6) || !sqlite3_strnicmp(query, "DROP", 6)) {
            int rows_affected = sqlite3_changes(db);
            result->set_integer(result, "rows_affected", rows_affected);
        }else if(!sqlite3_strnicmp(query, "INSERT", 6)) {
            sqlite3_int64 last_insert_id = sqlite3_last_insert_rowid(db);
            if(last_insert_id)
                result->set_integer(result, "last_insert_id",last_insert_id);
        }
        char *json_response = (char *)rliza_object_to_string(result);
        json_response[strlen(json_response) - 1] = '\0';
        size_t bytes_sent = write_http_chunk(request, json_response, false);
        free(json_response);
        if (!bytes_sent)
            return 0;
        long long count = 0;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            rliza_t *row = rliza_new(RLIZA_ARRAY);
            if (count == 0) {
                json_response = (char *)malloc(1024 * 1024);
                json_response[0] = 0;
                strcpy(json_response, ",\"columns\":[");
                for (int col = 0; col < sqlite3_column_count(stmt); col++) {
                    char * column_name = (char *)sqlite3_column_name(stmt, col);
                    char * escaped_column_name = (char *)malloc(strlen(column_name) * 2 + 1);
                    rstraddslashes(column_name,escaped_column_name); 

                    strcat(json_response, "\"");
                    strcat(json_response, escaped_column_name);
                    free(escaped_column_name);
                    strcat(json_response, "\",");
                }
                if (json_response[strlen(json_response) - 1] == ',')
                    json_response[strlen(json_response) - 1] = ']';
                strcat(json_response, ",\"rows\":[");
                bytes_sent = write_http_chunk(request, json_response, 0);
                free(json_response);
                rliza_free(result);
            }

            for (int col = 0; col < sqlite3_column_count(stmt); col++) {
                int type = sqlite3_column_type(stmt, col);
                switch (type) {
                case SQLITE_INTEGER:
                    rliza_push(row, rliza_new_integer((long long)sqlite3_column_int(stmt, col)));
                    break;

                case SQLITE_FLOAT:
                    rliza_push(row, rliza_new_number(sqlite3_column_double(stmt, col)));
                    break;
                case SQLITE_TEXT:
                    char *string = (char *)sqlite3_column_text(stmt, col);
                    char *escaped_string = (char *)malloc(strlen(string) * 2 + 1);
                    rstraddslashes(string, escaped_string);
                    if (string && !strcmp(string, "true")) {
                        rliza_push(row, rliza_new_boolean(true));
                    } else if (string && !strcmp(string, "false")) {
                        rliza_push(row, rliza_new_boolean(false));
                    } else {
                        rliza_push(row, rliza_new_string(escaped_string));
                    }
                    free(escaped_string);
                    break;
                case SQLITE_BLOB:
                    break;
                case SQLITE_NULL:
                    break;
                default:
                    printf("Unknown column type\n");
                }
            }
            if (rc == SQLITE_DONE) {
                printf("DONEEE\n");
            }

            count++;

            json_response = (char *)rliza_object_to_string(row);
            char *prefixed_json_response = (char *)malloc(strlen(json_response) + 1024);
            prefixed_json_response[0] = 0;
            if (count > 1)
                strcpy(prefixed_json_response, ",");

            strcat(prefixed_json_response, json_response);
            free(json_response);
            size_t bytes_sent_chunk = write_http_chunk(request, prefixed_json_response, false);
            if (!bytes_sent_chunk)
                return 0;
            bytes_sent += bytes_sent_chunk;
            free(prefixed_json_response);
            rliza_free(row);
        }
        sqlite3_finalize(stmt);
        char *rows_end = strdup(count ? "]" : "");
        char *footer_end = (char *)malloc(512);
        sprintf(footer_end, ",\"count\":%lld}", count);
        json_response = (char *)malloc(1024);
        sprintf(json_response, "%s,%s\r\n", rows_end, footer_end + 1);
        free(rows_end);
        free(footer_end);
        size_t bytes_sent_chunk = write_http_chunk(request, json_response, true);
        if (bytes_sent_chunk == 0)
            return 0;
        bytes_sent += bytes_sent_chunk;
        free(json_response);

        return bytes_sent;
    }
    return 0;
}

int request_handler(rhttp_request_t *r) {
    if (verbose) {
        rhttp_print_request(r);
    }
    if(strcmp(r->method,"POST")){
        return 0;
    }
    long request_content_length = rhttp_header_get_long(r, "Content-Length");
    if(request_content_length == 0){
        return 0;
    }
    char *json = (char *)malloc(request_content_length + 1);
    json[request_content_length] = 0;
    long total_size = 0;
    while (total_size != request_content_length) {
        long chunk = read(r->c, json + total_size, request_content_length - total_size);
        if (chunk <= 0) {
            close(r->c);
            return 1;
        }
        total_size += chunk;
    }
    size_t bytes_sent = 0;
    if (verbose) {
        printf("req:");
        print_partial(json, log_line_length);
    }
    char *jsonp = json;
    rliza_t *obj = rliza_object_from_string(&jsonp);
    char *query = (char *)obj->get_string(obj, "query");
    rliza_t *params = obj->get_object(obj, "params");
    char *response_headers = (char *)calloc(1024, sizeof(char));
    sprintf(response_headers,
            "HTTP/1.1 200 OK\r\n"
            //"Content-Length: %ld\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Content-Type: application/json\r\n"
            "%s\r\n",
            r->keep_alive ? "Connection: close\r\n" : "Connection: close\r\n");
    if (verbose) {
        printf(response_headers);
    }
    bytes_sent = rhttp_send_drain(r->c, response_headers, 0);
    free(response_headers);
    if (bytes_sent == 0)
        return 0;
    size_t bytes_sent_chunks = db_execute(r, query, params);
    rliza_free(obj);
    free(json);
    if (verbose)
        printf("%s\n", rmalloc_stats());
    if (bytes_sent_chunks == 0)
        return 0;
    bytes_sent += bytes_sent_chunks;

    return bytes_sent > 0;
}

rhttp_request_handler_t handler = request_handler;

int main(int argc, char *argv[]) {

    if (atexit(exit_application) != 0) {
        printf("Couldn't register atexit handler. Database may close "
               "incorrectly.\n");
    }
    signal(SIGINT, handle_sigint);

    if (argc < 3 || !isdigit(argv[1][0])) {
        printf("Usage: %s <port> <file path>\n", argv[0]);
        exit(1);
    }

    unsigned int port = atoi(argv[1]);

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

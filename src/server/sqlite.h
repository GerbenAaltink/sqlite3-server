#ifndef SQLITE3_SERVER_SQLITE_H
#define SQLITE3_SERVER_SQLITE_H
#include <sqlite3.h>
#include <stdlib.h>

// Function to calculate factorial
static void sqlite_bool_function(sqlite3_context *context, int argc,
                                 sqlite3_value **argv) {
    if (argc == 1) {
        int value_type = sqlite3_value_type(argv[0]);
        if ((value_type == SQLITE_INTEGER)) {
            int n = sqlite3_value_int(argv[0]);
            if (n != 0)
                sqlite3_result_text(context, "true", -1,
                                    SQLITE_TRANSIENT); // Set the result
            else
                sqlite3_result_text(context, "false", -1,
                                    SQLITE_TRANSIENT); // Set the result
        } else if (value_type == SQLITE_TEXT) {
            char *str = (char *)sqlite3_value_text(argv[0]);
            if (!str || *str == 0)
                sqlite3_result_text(context, "false", -1, SQLITE_TRANSIENT);
            else
                sqlite3_result_text(context, "true", -1, SQLITE_TRANSIENT);
        } else if (value_type == SQLITE_FLOAT) {
            double n = sqlite3_value_double(argv[0]);
            if (n > 0)
                sqlite3_result_text(context, "true", -1,
                                    SQLITE_TRANSIENT); // Set the result
            else
                sqlite3_result_text(context, "false", -1,
                                    SQLITE_TRANSIENT); // Set the result
        } else if (value_type == SQLITE_NULL) {
            sqlite3_result_text(context, "false", -1, SQLITE_TRANSIENT);
        }
    } else {
        sqlite3_result_null(context); // Return NULL if arguments are invalid
    }
}

int sqlite_register_bool_function(sqlite3 *db) {
    return sqlite3_create_function(
        db,                   // SQLite database connection
        "bool",               // Name of the function as it will appear in SQL
        1,                    // Number of arguments the function takes
        SQLITE_UTF8,          // Text encoding
        NULL,                 // User data (not used here)
        sqlite_bool_function, // Pointer to the function implementation
        NULL,                 // Step function (for aggregates, not used here)
        NULL                  // Final function (for aggregates, not used here)
    );
}
#endif
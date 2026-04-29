#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

#define COMMANDS_COUNT 3
#define NOTIFICATION_FILENAME "notifications.txt"
#define DATABASE_FILENAME "notifications.db"

int migrate_db() {
    sqlite3 *db;
    char *err_msg = 0;

    sqlite3_open(DATABASE_FILENAME, &db);

    char *notifications_table_sql =
        "CREATE TABLE IF NOT EXISTS notifications ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "notification TEXT"
        ");";

    int result = sqlite3_exec(db, notifications_table_sql, NULL, NULL, &err_msg);

    if (result != SQLITE_OK) {
        printf("failed to create notifications table: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }

    sqlite3_close(db);
    return 0;
}

typedef int (*CommandFunc)(char *);

typedef struct {
    const char* name;
    CommandFunc func;
} CommandMap;

/**
 * Adds a notification to the notifications file
 * @param arg the notification to add
 * @return 0 if successful, 1 if not
 */
int add(char *arg) {
    FILE *file = fopen(NOTIFICATION_FILENAME, "a");
    if (file == NULL) {
        printf("failed to open file\n");
        return 1;
    }

    fprintf(file, "%s\n", arg);
    fclose(file);
    return 0;
}

/**
 * Clears the notifications file
 * @return 0 if successful, 1 if not
 */
int clear() {
    FILE *file = fopen(NOTIFICATION_FILENAME, "w");
    if (file == NULL) {
        printf("failed to open file\n");
        return 1;
    }

    fclose(file);
    return 0;
}

/**
 * Lists all notifications in the notifications file
 * @return 0 if successful, 1 if not
 */
int list_all() {
    FILE *file = fopen(NOTIFICATION_FILENAME, "r");
    if (file == NULL) {
        printf("failed to open file\n");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }
    fclose(file);
    return 0;
}

CommandMap map[] = {
    {
        "add",
        add
    },
    {
        "clear",
        clear
    },
    {
        "list",
        list_all
    },
};

CommandFunc get_command_by_name(const char *name) {
    for (int i = 0; i < COMMANDS_COUNT; i++) {
        if (strcmp(map[i].name, name) == 0) {
            return map[i].func;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    migrate_db();

    if (argc < 2) {
        printf("usage: %s <command>\n", argv[0]);
        return 1;
    }

    CommandFunc func_to_run = get_command_by_name(argv[1]);

    if (func_to_run == NULL) {
        printf("unknown command: %s\n", argv[1]);
        return 1;
    }

    int result = func_to_run(argv[2]);

    if (result != 0) {
        return 1;
    }

    return 0;
}

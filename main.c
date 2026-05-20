#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

#define COMMANDS_COUNT 5
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

    char *project_table_sql =
        "CREATE TABLE IF NOT EXISTS projects ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "name TEXT"
        ");";

    int result_projects = sqlite3_exec(db, project_table_sql, NULL, NULL, &err_msg);

    if (result_projects != SQLITE_OK) {
        printf("failed to create projects table: %s\n", err_msg);
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
    sqlite3 *db;

    sqlite3_open(DATABASE_FILENAME, &db);

    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "INSERT INTO notifications (notification) VALUES (?);";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        printf("failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, arg, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        printf("failed to insert notification: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}

/**
 * Clears the notifications file
 * @return 0 if successful, 1 if not
 */
int clear() {
    sqlite3 *db;

    sqlite3_open(DATABASE_FILENAME, &db);

    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "DELETE FROM notifications;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        printf("failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        printf("failed to clear notifications: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}

/**
 * Lists all notifications in the notifications file
 * @return 0 if successful, 1 if not
 */
int list_all() {
    sqlite3 *db;

    sqlite3_open(DATABASE_FILENAME, &db);

    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "SELECT * FROM notifications;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        printf("failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *message = (const char *)sqlite3_column_text(stmt, 1);
        printf("%s\n", message);
    }

    if (rc != SQLITE_DONE) {
        printf("failed to list notifications: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}

int add_project(char *arg) {
    sqlite3 *db;

    sqlite3_open(DATABASE_FILENAME, &db);

    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "INSERT INTO projects (name) VALUES (?);";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        printf("failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_bind_text(stmt, 1, arg, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        printf("failed to insert project: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}

int list_projects() {
    sqlite3 *db;

    sqlite3_open(DATABASE_FILENAME, &db);

    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "SELECT * FROM projects;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        printf("failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *message = (const char *)sqlite3_column_text(stmt, 1);
        printf("%s\n", message);
    }

    if (rc != SQLITE_DONE) {
        printf("failed to list projects: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

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
    {
        "addp",
        add_project
    },
    {
        "listp",
        list_projects
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

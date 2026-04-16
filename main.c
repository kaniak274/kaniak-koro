#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMANDS_COUNT 1

typedef void (*CommandFunc)(char *);

typedef struct {
    const char* name;
    CommandFunc func;
} CommandMap;

void add(char *arg) {
    printf("add %s\n", arg);
}

CommandMap map[] = {
    {
        "add",
        add
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
    if (argc < 2) {
        printf("usage: %s <command>\n", argv[0]);
        return 1;
    }

    CommandFunc func_to_run = get_command_by_name(argv[1]);

    if (func_to_run == NULL) {
        printf("unknown command: %s\n", argv[1]);
        return 1;
    }

    func_to_run(argv[2]);

    return 0;
}

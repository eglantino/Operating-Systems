#include "cli.h"

//removes trailing newline from string(no need for /n at end)
static void trim_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

//creates new dialog in shared memory
static void cmd_create(int dialog_id) {
    int idx = create_dialog(dialog_id);
    if (idx < 0) {
        printf("[error] cannot create dialog %d (no free slot)\n", dialog_id);
    } else {
        printf("[ok] dialog %d created at slot %d\n", dialog_id, idx);
    }
}

//addds current pid to the dialog participant list
static void cmd_join(int dialog_id) {
    pid_t me = getpid();
    int slot = dialog_join(dialog_id, me);
    if (slot < 0) {
        printf("[error] cannot join dialog %d\n", dialog_id);
    } else {
        printf("[ok] pid %d joined dialog %d at slot %d\n", me, dialog_id, slot);
    }
}

//removes current pid from dialog participant list
static void cmd_leave(int dialog_id) {
    pid_t me = getpid();
    if (dialog_leave(dialog_id, me) < 0) {
        printf("[warn] pid %d was not in dialog %d\n", me, dialog_id);
    } else {
        printf("[ok] pid %d left dialog %d\n", me, dialog_id);
    }
}

//enqueues a message to the dialog queue
static void cmd_send(int dialog_id, const char *msg) {
    pid_t me = getpid();
    if (q_enqueue(dialog_id, me, msg) < 0) {
        printf("[error] send failed for dialog %d\n", dialog_id);
    } else {
        printf("[ok] sent to dialog %d: %s\n", dialog_id, msg);
    }
}

//bocking receive from dialog queue
static void cmd_recv(int dialog_id) {
    pid_t me = getpid();
    char buf[MAX_TXT];

    int r = q_recv_block(dialog_id, me, buf);
    if (r < 0) {
        printf("[error] recv failed for dialog %d\n", dialog_id);
        return;
    }

    printf("[recv] dialog %d: %s\n", dialog_id, buf);

    if (strcmp(buf, "TERMINATE") == 0) {
        printf("[info] received TERMINATE for dialog %d\n", dialog_id);
    }
}

static void cmd_terminate(int dialog_id) {
    cmd_send(dialog_id, "TERMINATE");
}

void cli_loop(void) {
    char line[512];

    printf("[cli] pid=%d ready. Type 'help' for commands.\n", getpid());

    for (;;) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n[cli] EOF, exiting.\n");
            break;
        }
        trim_newline(line);

        if (line[0] == '\0')    //ignore empty lines
            continue;

        char cmd[32];   //parse first token and dialog id(if any)
        int id;
        int n = sscanf(line, "%31s %d", cmd, &id);

        if (n >= 1 && strcmp(cmd, "help") == 0) {
            printf("Commands:\n");
            printf("  create <id>\n");
            printf("  join <id>\n");
            printf("  leave <id>\n");
            printf("  send <id> <text>\n");
            printf("  recv <id>\n");
            printf("  terminate <id>\n");
            printf("  quit\n");
        }
        else if (n >= 1 && strcmp(cmd, "quit") == 0) {
            printf("[cli] quitting.\n");
            break;
        }
        else if (n == 2 && strcmp(cmd, "create") == 0) {
            cmd_create(id);
        }
        else if (n == 2 && strcmp(cmd, "join") == 0) {
            cmd_join(id);
        }
        else if (n == 2 && strcmp(cmd, "leave") == 0) {
            cmd_leave(id);
        }
        else if (n == 2 && strcmp(cmd, "recv") == 0) {
            cmd_recv(id);
        }
        else if (n == 2 && strcmp(cmd, "terminate") == 0) {
            cmd_terminate(id);
        }
        else if (n >= 2 && strcmp(cmd, "send") == 0) {
            //find start of text message
            char *p = strchr(line, ' ');
            if (!p) {
                printf("[error] usage: send <id> <text>\n");
                continue;
            }
            p++; //move past first space
            p = strchr(p, ' ');
            if (!p) {
                printf("[error] usage: send <id> <text>\n");
                continue;
            }
            p++; //start of text

            if (*p == '\0') {
                printf("[error] empty message.\n");
                continue;
            }
            cmd_send(id, p);
        }
        else {
            printf("[error] unknown or invalid command. Type 'help'.\n");
        }
    }
}

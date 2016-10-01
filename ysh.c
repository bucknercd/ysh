#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define SZ 1024
char buf[SZ];

typedef enum {
    READ,
    EXEC1,
    EXEC2,
    EXIT
} State;

typedef struct cmd
  {
    int redirect_in;     /* Any stdin redirection?         */
    int redirect_out;    /* Any stdout redirection?        */
    int redirect_append; /* Append stdout redirection?     */
    int background;      /* Put process in background?     */
    int piping;          /* Pipe prog1 into prog2?         */
    char *infile;        /* Name of stdin redirect file    */
    char *outfile;       /* Name of stdout redirect file   */
    char *argv1[10];     /* First program to execute       */
    char *argv2[10];     /* Second program in pipe         */
  } Cmd;

State parse_args(Cmd *cmd, const char *cwd);
int cmdscan(char *cmdbuf, Cmd *com);
State parse_args(Cmd *cmd, const char *cwd);
void create_exec(const State state, const Cmd *cmd);
void execute(const Cmd *cmd, int pipe[2], int id);
void done(Cmd *cmd, char *cwd);

State parse_args(Cmd *cmd, const char *cwd) {
    State state;
    memset(cmd, 0, sizeof(Cmd));
    memset(buf, 0, SZ);
    printf("%s$ ", cwd);
    char * qval;
    qval = fgets(buf, sizeof(buf), stdin);
    if ((qval == NULL) || (strcmp(qval, "exit\n") == 0)) {
        return EXIT;
    } else if (cmdscan(qval, cmd) == -1) {
        return READ;
    }
    if (cmd->argv2[0] != NULL) {
        state = EXEC1;
    } else {
        state = EXEC2;
    }
    return state;
}

void create_exec(const State state, const Cmd *cmd) {
    int i, parent, id = 0, waitarg=0, fdp[2];
    pid_t pids[3];
    pipe(fdp);

    if (cmd->background)
        waitarg = WNOHANG;

    for (i=1; i <= state; i++) {
        switch(parent = fork()) {
            case -1:
                perror("fork error");
                exit(1);
            case 0:
                id = i;
            default:
                pids[i] = parent;
        }
        if (!parent)
            break;
    }
    if (parent) {
        close(fdp[0]);
        close(fdp[1]);
        waitpid(pids[1], NULL, waitarg);
        if (state == EXEC2)
            waitpid(pids[2], NULL, waitarg);
    } else {
       execute(cmd, fdp, id);
    }

}

void execute(const Cmd *cmd, int pipe[2], int id) {
    if (cmd->redirect_in) {
        int fdin = open(cmd->infile, O_RDONLY);
        dup2(fdin, STDIN_FILENO);
        close(fdin);
    }
    if (cmd->redirect_out) {
        int fdout = open(cmd->outfile, O_WRONLY | O_CREAT, 0644);
        dup2(fdout, STDOUT_FILENO);
        close(fdout);
    }
    if (cmd->redirect_out && cmd->redirect_append) {
        int fdout = open(cmd->outfile, O_WRONLY | O_APPEND | O_CREAT, 0644);
        dup2(fdout, STDOUT_FILENO);
        close(fdout);
    }
    if (id == 1) {
        if (cmd->piping) {
            close(pipe[0]);
            dup2(pipe[1], STDOUT_FILENO);
            close(pipe[1]);
        }
        execvp(cmd->argv1[0], cmd->argv1);
        perror("");
        exit(1);
    }
    if (id == 2) {
        if (cmd->piping) {
            close(pipe[1]);
            dup2(pipe[0], STDIN_FILENO);
            close(pipe[0]);
        }
        execvp(cmd->argv2[0], cmd->argv2);
        perror("");
        exit(1);
    }
}

void done(Cmd *cmd, char *cwd) {
    printf("\n");
    free(cwd);
    free(cmd);
    exit(0);
}

int main(void) {
    State state = READ;
    Cmd *cmd = malloc(sizeof(Cmd));
    char * cwd = getcwd(0,0);
    while(1) {

        if (state == READ) {
            state = parse_args(cmd, cwd);
        } else if ((state == EXEC1) || (state == EXEC2)) {
            create_exec(state, cmd);
            state = READ;
        } else {
            done(cmd, cwd);
        }
    }
}

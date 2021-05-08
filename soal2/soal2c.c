#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {

    int pid;
    int pipe0[2];
    int pipe1[2];

    // create pipe0
    if (pipe(pipe0) == -1) {
        perror("bad pipe0");
        exit(1);
    }

    // fork (ps aux)
    if ((pid = fork()) == -1) {
        perror("bad fork0");
        exit(1);
    } 
    else if (pid == 0) {
        // stdin --> ls --> pipe0
        // input from stdin (already done), output to pipe0
        dup2(pipe0[1], 1);
        // close fds
        close(pipe0[0]);
        close(pipe0[1]);

        char *arg[]={"ps", "aux",NULL};
        execv("/bin/ps", arg);
        // exec didn't work, exit
        perror("bad exec ps");
        _exit(1);
    }
    // parent

    // create pipe1
    if (pipe(pipe1) == -1) {
        perror("bad pipe1");
        exit(1);
    }

    // fork (sort -nrk 3,3)
    if ((pid = fork()) == -1) {
        perror("bad fork1");
        exit(1);
    } 
    else if (pid == 0) {
        // pipe0--> grep --> pipe1
        // input from pipe0
        dup2(pipe0[0], 0);
        // output to pipe1
        dup2(pipe1[1], 1);
        // close fds
        close(pipe0[0]);
        close(pipe0[1]);
        close(pipe1[0]);
        close(pipe1[1]);

        char *arg[]={"sort", "-nrk","3,3",NULL};
        execv("/bin/sort", arg);
        // exec didn't work, exit
        perror("bad exec sort");
        _exit(1);
    }
    // parent

    // close unused fds
    close(pipe0[0]);
    close(pipe0[1]);

    // pipe1--> tail --> stdout
    // input from pipe1
    dup2(pipe1[0], 0);
    // output to stdout (already done). Close fds
    close(pipe1[0]);
    close(pipe1[1]);

    char *arg[]={"head", "-5",NULL};
    execv("/bin/head", arg);
    // exec didn't work, exit
    perror("bad exec head");
    _exit(1);

    return 0;
}
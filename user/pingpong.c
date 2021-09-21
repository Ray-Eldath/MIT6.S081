#include "kernel/types.h"
#include "user.h"

int main(int argc, char **argvv)
{
    int p1[2], p2[2];
    int pid;
    char c;

    pipe(p1);
    pipe(p2);
    pid = fork();

    if (pid > 0)
    {
        write(p1[1], &c, 1);
        read(p2[0], &c, 1);
        printf("%d: received pong\n", getpid());
        wait((int *)0);
    }
    else if (pid == 0)
    {
        read(p1[0], &c, 1);
        printf("%d: received ping\n", getpid());
        write(p2[1], &c, 1);
    }
    else
    {
        fprintf(2, "fork error");
        exit(1);
    }

    exit(0);
}
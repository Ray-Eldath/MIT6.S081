#include "kernel/types.h"
#include "user.h"

void nextstage(int rx)
{
    // printf("ns %d\n", rx);
    int p, n, pid;
    int pp[2];

    if (read(rx, &p, 4) != 0)
    {
        printf("prime %d\n", p);
    }

    if (read(rx, &n, 4) != 0)
    {
        // printf("before pipe\n");
        pipe(pp);
        // printf("piped\n");
        pid = fork();
        if (pid == 0)
        {
            // printf("forked\n");
            if (n % p != 0)
            {
                write(pp[1], &n, 4);
            }

            while (read(rx, &n, 4) != 0)
            {
                if (n % p != 0)
                {
                    write(pp[1], &n, 4);
                }
            }
            // close(rx);
            close(pp[1]);
            exit(0);
        }
        else
        {
            close(pp[1]);
            nextstage(pp[0]);
        }
    }

    exit(0);
}

int main(int argc, char **argv)
{
    int i;
    int p[2];
    pipe(p);

    for (i = 2; i <= 35; i++)
    {
        write(p[1], &i, 4);
    }
    close(p[1]);

    nextstage(p[0]);
    exit(0);
}

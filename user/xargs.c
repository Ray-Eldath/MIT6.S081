#include "kernel/types.h"
#include "kernel/param.h"
#include "user.h"

int main(int argc, char **argv)
{
    char *buf[5];
    char next;
    int bp, i;
    int pid;

    for (i = 0; i < 5; i++)
    {
        buf[i] = malloc(sizeof buf);
        memset(buf[i], 0, sizeof buf[i]);
    }

    for (i = 1; i < argc; i++)
    {
        strcpy(buf[i - 1], argv[i]);
    }
    bp = 0;
    while (read(0, &next, 1) != 0)
    {
        if (next != '\n')
        {
            *(buf[argc - 1] + (bp++)) = next;
        }
        else
        {
            *(buf[argc - 1] + (bp++)) = '\0';
            // printf("debug: ");
            // for (i = 0; i < argc; i++)
            // {
            //     printf("%d=%s\n", i, buf[i]);
            // }

            pid = fork();
            if (pid == 0)
            {
                if (exec(buf[0], buf) < 0)
                {
                    fprintf(2, "exec failed\n");
                }
            }
            else
            {
                wait((int *)0);
            }
        }
    }

    exit(0);
}
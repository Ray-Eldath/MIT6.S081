#include "kernel/types.h"
#include "user.h"

int main(int argc, char **argv)
{
    int s;

    if (argc < 2)
    {
        fprintf(2, "usage: sleep ticks\n");
        exit(1);
    }
    if ((s = sleep(atoi(argv[0]))) < 0)
    {
        fprintf(2, "sleep returned with error %d", s);
        exit(1);
    }
    exit(0);
}
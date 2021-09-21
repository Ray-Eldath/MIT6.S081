#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user.h"

char *
fmtname(char *path)
{
    char *p;

    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    return p;
}

void find(char *path, const char *name)
{
    struct dirent de;
    struct stat st;
    char buf[512], *b;
    int fd;

    fd = open(path, O_RDONLY);
    if (stat(path, &st) < 0)
    {
        fprintf(2, "stat failed for %s\n", path);
        exit(1);
    }

    switch (st.type)
    {
    case T_FILE:
        b = fmtname(path);
        // printf("%s,%s,%s\n", name, b, path);
        if (strcmp(b, name) == 0)
        {
            printf("%s\n", path);
        }
        break;
    case T_DIR:
        strcpy(buf, path);
        b = buf + strlen(buf);
        *b++ = '/';
        // printf("b1 = %s\n", buf);
        while (read(fd, &de, sizeof de) == sizeof de)
        {
            if (de.inum == 0)
                continue;

            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            memmove(b, de.name, DIRSIZ);
            b[DIRSIZ] = 0;
            // printf("b2 = %s\n", buf);
            find(buf, name);
        }
        break;
    }
    close(fd);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(2, "usage: find [path] [pattern]\n");
        exit(1);
    }
    find(argv[1], argv[2]);

    exit(0);
}
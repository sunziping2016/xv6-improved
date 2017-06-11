//
// Created by parallels on 5/5/17.
//

#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"
#include "xv6/fs.h"

int
get_inum(char *path)
{
    struct stat st;
    int fd;

    if ((fd = open(path, 0)) < 0)
    {
        printf(2, "pwd: cannot open %s\n", path);
        exit();
    }

    if ((fstat(fd, &st)) < 0)
    {
        printf(2, "pwd: cannot stat %s\n", path);
        exit();
    }

    close(fd);

    return st.ino;
}

void
get_dirent(char *path, int inum, struct dirent *cur_de)
{
    struct dirent tmp_de;
    int dir;

    if ((dir = open(path, 0)) < 0)
    {
        printf(2, "pwd: cannot open %s\n", path);
        exit();
    }

    while(read(dir, &tmp_de, sizeof(tmp_de)) == sizeof(tmp_de))
    {
        if(tmp_de.inum == inum)
            break;
    }
    close(dir);
    *cur_de = tmp_de;
}

void
prepend(char *str, char *prefix, int max_size)
{
    char buf[1024];
    strcpy(buf, str);
    if((max_size - strlen(prefix)) < strlen(buf))
    {
        printf(2, "pwd: cannot prepend prefix %s to %s, length of path is not enough\n", prefix, buf);
        exit();
    }
    memmove(str + strlen(prefix), buf, max_size - strlen(prefix));
    memmove(str, prefix, strlen(prefix));
}

int
main(int argc, char *argv[])
{
    char cur_path[256] = ".";
    char parent_path[256] = "..";
    char working_directory[256] = "";
    struct dirent cur_de;

    while(get_inum(cur_path) != get_inum(parent_path))
    {
        get_dirent(parent_path, get_inum(cur_path), &cur_de);
        prepend(working_directory, cur_de.name, sizeof(working_directory));
        prepend(working_directory, "/", sizeof(working_directory));
        strcpy(cur_path, parent_path);
        prepend(parent_path, "../", sizeof(parent_path));
    }

    if(working_directory[0] == '\0')
    {
        prepend(working_directory, "/", sizeof(working_directory));
    }
    printf(2, "%s\n", working_directory);

    exit();
}


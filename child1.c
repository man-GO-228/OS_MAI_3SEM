#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

int main(void)
{
    char *name_file = malloc(128);
    if (name_file == NULL)
        return 1;
    char c;
    size_t cap_name = 0;
    while (cap_name < 127)
    {
        ssize_t n = read(0, &c, 1);
        if (n == 0)
        {
            free(name_file);
            return 1;
        }
        if (n < 0)
        {
            free(name_file);
            return 1;
        }
        if (c == '\n')
            break;

        name_file[cap_name++] = c;
    }

    name_file[cap_name] = '\0';

    int fd = open(name_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    free(name_file);
    if (fd < 0)
    {
        write(2, "open failed\n", 12);
        return 1;
    }

    while (1)
    {
        size_t cap = 512;
        size_t len = 0;

        char *line = malloc(cap);

        if (line == NULL)
        {
            close(fd);
            return 1;
        }

        while (1)
        {
            ssize_t n = read(0, &c, 1);

            if (n == 0)
            {
                free(line);
                close(fd);
                return 0;
            }

            if (n < 0)
            {
                free(line);
                close(fd);
                return 1;
            }
            if (c == '\n')
                break;
            if (len + 1 >= cap)
            {
                cap *= 2;
                char *tmp = realloc(line, cap);
                if (tmp == NULL)
                {
                    free(line);
                    close(fd);
                    return 1;
                }
                line = tmp;
            }

            line[len++] = c;
        }
        size_t j = 0;

        for (size_t i = 0; i < len; i++)
        {
            char ch = line[i];

            if (ch != 'a' && ch != 'e' &&
                ch != 'i' && ch != 'o' &&
                ch != 'u' &&
                ch != 'A' && ch != 'E' &&
                ch != 'I' && ch != 'O' &&
                ch != 'U')
            {
                line[j++] = ch;
            }
        }


        len = j;

        size_t written = 0;
        while (written < len)
        {
            ssize_t w = write(fd, line + written, len - written);
            if (w < 0)
            {
                free(line);
                close(fd);
                return 1;
            }

            written += (size_t)w;
        }
        write(fd, "\n", 1);
        free(line);
    }
}

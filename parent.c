#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#define STDIN_FILENO 0


int main(void)
{
    int p[2];
    if (pipe(p) < 0)
    {
        write(2, "pipe err\n", 9);
        return 1;
    }


    pid_t pid = fork();
    if (pid < 0)
    {
        write(2, "fork err\n", 9);
        return 1;
    }
    if (pid == 0)
    {
        close(p[1]);
        dup2(p[0], STDIN_FILENO);
        close(p[0]);
        char *const argv[] = {"./child1", NULL};
        char *const envp[] = {NULL};
        execve("./child1", argv, envp);

        write(2, "execve failed\n", 14);
        _exit(1);
    }

    close(p[0]);


    int p2[2];
    if(pipe(p2) < 0) 
    {
        write(2, "pipe err\n", 9);
        return 1;
    }
    pid_t pid2 = fork();
    if (pid2 < 0)
    {
        write(2, "fork err\n", 9);
        return 1;
    }
    else if (pid2 == 0)
    {
        close(p2[1]);
        dup2(p2[0], STDIN_FILENO);
        close(p2[0]);
        close(p[1]); 
        char *const argv[] = {"./child1", NULL};
        char *const envp[] = {NULL};
        execve("./child1", argv, envp);

        write(2, "execve failed\n", 14);
        _exit(1);
    }
    close(p2[0]);



    char name[128];
    size_t name_len = 0;
    char c;

    while (name_len < sizeof(name) - 1)
    {
        ssize_t n = read(STDIN_FILENO, &c, 1);

        if (n <= 0)
        {
            close(p[1]);
            close(p2[1]);
            waitpid(pid, NULL, 0);
            waitpid(pid2, NULL, 0);
            return 1;
        }

        if (c == '\n')
            break;

        name[name_len++] = c;
    }

    name[name_len] = '\0';


    /*
     * Передаём имя файла дочернему.
     */

    write(p[1], name, name_len);
    write(p[1], "\n", 1);


    size_t name_len2 = 0;
    char name2[128];
    while (name_len2 < sizeof(name2) - 1)
    {
        ssize_t n = read(STDIN_FILENO, &c, 1);

        if (n <= 0)
        {
            close(p[1]);
            close(p2[1]);
            waitpid(pid, NULL, 0);
            waitpid(pid2, NULL, 0);
            return 1;
        }

        if (c == '\n')
            break;

        name2[name_len2++] = c;
    }

    name2[name_len2] = '\0';

    write(p2[1], name2, name_len2);
    write(p2[1], "\n", 1);

    /*
     * Теперь читаем остальные строки.
     */

    char chunk[4096];
    char *line = NULL;

    size_t cap = 0;
    size_t len = 0;

    ssize_t n;
    int chet_nechet = 1;
    while ((n = read(STDIN_FILENO, chunk, sizeof(chunk))) > 0)
    {
        for (ssize_t i = 0; i < n; i++)
        {
            if (chunk[i] == '\n')
            {
                /*
                 * Строка полностью получена.
                 */
                if (len > 0)
                {
                    if (chet_nechet == 1)
                    {
                        write(p[1], line, len);
                        write(p[1], "\n", 1);
                        chet_nechet = 2;
                    }
                    else if(chet_nechet == 2)
                    {
                        write(p2[1], line, len);
                        write(p2[1], "\n", 1);
                        chet_nechet = 1;
                    }    
                }
                len = 0;
            }
            else
            {
                /*
                 * Увеличиваем буфер,
                 * если места не хватает.
                 */

                if (len + 1 >= cap)
                {
                    size_t new_cap = cap ? cap * 2 : 256;

                    char *tmp = realloc(line, new_cap);

                    if (tmp == NULL)
                    {
                        free(line);
                        close(p[1]);
                        close(p2[1]);
                        waitpid(pid, NULL, 0);
                        waitpid(pid2, NULL, 0);

                        return 1;
                    }

                    line = tmp;
                    cap = new_cap;
                }

                line[len++] = chunk[i];
            }
        }
    }


    free(line);

    close(p[1]);

    waitpid(pid, NULL, 0);

    close(p2[1]);

    waitpid(pid2, NULL, 0);

    return 0;
}


/*
gcc -Wall -Wextra -O2 parent.c -o parent
gcc -Wall -Wextra -O2 child1.c  -o child1
./parent
*/
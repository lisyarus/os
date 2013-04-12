#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

int main (int argc, char ** argv)
{
    char delimiter = '\n';
    int buffer_size = 4 * 1024;
    char option;

    while ((option = getopt(argc, argv, "znb:")) != -1)
        switch (option)
        {
            case 'n':
                delimiter = '\n';
                break;
            case 'z':
                delimiter = '\0';
                break;
            case 'b':
                buffer_size = atoi(optarg);
                break;
        }

    int last_arg;
    for (last_arg = optind; argv[last_arg] != 0; ++last_arg);

    char ** args = (char **)malloc(sizeof(char *) * (last_arg - optind + 2));
    int a;
    for (a = 0; a < last_arg - optind; ++a)
        args[a] = argv[optind + a];
    args[last_arg - optind + 1] = 0;

    char * buffer = malloc(buffer_size + 1);

    int buffer_pos = 0; 
    int read_length = 0;
    int i;
    int quit = 0;

    while (quit != 1)
    { 
        read_length = read(0, buffer + buffer_pos, buffer_size + 1 - buffer_pos);

        if (read_length == 0)
        {
            if (buffer_pos >= buffer_size)
                return 1;
            buffer[buffer_pos] = delimiter;
            read_length = 1;
            quit = 1;
        }

        for (i = buffer_pos; i < buffer_pos + read_length; ++i)
        {
            // printf("[%i:%i]\n", i, buffer_pos);
            if (buffer[i] == delimiter)
            {
                // execute
                buffer[i] = 0;
                args[last_arg - optind] = buffer;

                if (fork() == 0)
                {
                    int fd = open("/dev/null", O_WRONLY);
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                    execvp(argv[optind], args); 
                    return 0;
                }
                int status;
                wait(&status);

                // print
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                {
                    int written = 0;
                    buffer[i] = delimiter;
                    while ((written += write(1, buffer + written, i + 1 - written)) < i + 1);
                }

                // move
                memmove(buffer, buffer + i + 1, buffer_pos + read_length - i);
                read_length -= i + 1;
                i = -1;
            }
        }
        buffer_pos += read_length;
    }

    free(buffer);
    return 0;
    
}

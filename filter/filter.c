#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

void full_write (int fd, const char * buffer, int len)
{
    while (len != 0)
    {
        int k = write(fd, buffer, len);
        if (k == 0) exit(1);
        len -= k;
        buffer += k;
    }
}

int safe_read (int fd, void * buf, int count)
{
    const char * error_msg = "Error while reading\n";
    int result = read(fd, buf, count);
    if (result == -1)
    {
        full_write(1, error_msg, strlen(error_msg));
        exit(1);
    }
    return result;
}

void * safe_malloc (int size)
{
    const char * error_msg = "Error while allocating memory\n";
    void * result = malloc(size);
    if (result == NULL)
    {
        full_write(1, error_msg, strlen(error_msg));
        exit(1);
    }
    return result;
}

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

    char ** args = (char **)safe_malloc(sizeof(char *) * (last_arg - optind + 2));
    int a;
    for (a = 0; a < last_arg - optind; ++a)
        args[a] = argv[optind + a];
    args[last_arg - optind + 1] = 0;

    char * buffer = safe_malloc(buffer_size + 1);

    int buffer_pos = 0; 
    int read_length = 0;
    int i;
    int quit = 0;

    while (quit != 1)
    { 
        read_length = safe_read(0, buffer + buffer_pos, buffer_size + 1 - buffer_pos);

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
            if (buffer[i] == delimiter)
            {
                // execute
                buffer[i] = 0;
                args[last_arg - optind] = buffer;

                if (fork() == 0)
                {
                    int fd = open("/dev/null", O_WRONLY);
                    if (fd == -1)
                    {
                        const char * error_msg = "Could not open /dev/null\n";
                        full_write(1, error_msg, strlen(error_msg));
                        exit(1);
                    }
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
                    full_write(1, buffer, i + 1);

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

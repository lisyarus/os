#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char * safe_malloc (int size)
{
    const char * error_msg = "Error while allocation memory\n";
    char * result = (char *)malloc(size);
    if (result == NULL)
    {
        full_write(1, error_msg, strlen(error_msg));
        exit(1);
    }
    return result;
}

int main (int argc, char ** argv)
{
    const char * usage_str = "Usage: ./main <buffer_size>\n";
    if (argc != 2)
    {
        full_write(1, usage_str, strlen(usage_str));
        return -1;
    }
    
    const char * wrong_argument = "Second argument should be a positive number\n";
    int buffer_size = atoi(argv[1]);
    if (buffer_size <= 0)
    {
        full_write(1, wrong_argument, strlen(wrong_argument));
        return -1;
    }
    
    ++buffer_size;
    char * buffer = safe_malloc(buffer_size);
    int length = 0;
    int begin = 0;
    int end = 0;

    typedef enum {
        normal, skip
    } state_t;
    state_t state = normal;
    
    int quit = 0;

    while (!quit)
    {
        if (length == buffer_size)
        {
            state = skip;
            end = 0;
        }
        int block;
        if (state == normal)
            if (end >= begin)
                block = safe_read(0, buffer + end, buffer_size - end);
            else
                block = safe_read(0, buffer + end, buffer_size - length);
        else
            block = safe_read(0, buffer, buffer_size);
        if (block == 0)
        {
            if (length == buffer_size || state == 1)
                break;
            // fake to process the end
            buffer[end % buffer_size] = '\n';
            block = 1;
            quit = 1;
        }

        int i;
        for (i = 0; i < block; ++i)
        {
            if (buffer[end + i] == '\n')
            {
                if (state == normal)
                {
                    // write, including current '\n' symbol
                    int count = 0;
                    for (; count < 2; ++count)
                    {
                        if (begin <= end + i)
                            full_write(1, buffer + begin, length + i + 1);
                        else
                        {
                            full_write(1, buffer + begin, buffer_size - begin);
                            full_write(1, buffer, end + i + 1);
                        }
                    }
                    length = - i - 1;
                    begin = (end + i + 1) % buffer_size; 
                }
                else
                {
                    begin = i + 1;
                    length = - i - 1;
                    state = normal;
                }
            }
        }
        end = (end + block) % buffer_size;        
        length += block;
    }

    free(buffer);
    return 0;
}

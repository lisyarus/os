#include <unistd.h>
#include <stdio.h> // debug
#include <stdlib.h>

int _strlen (char * str)
{
    char * temp = str;
    while (*temp++);
    return temp - str;
}

int _is_number (char c)
{
    return (c >= '0') && (c <= '9');
}

int _get_int (char * str)
{
    int result = 0;
    while (*str)
        if (_is_number(*str))
            result = result * 10 + ((*str++) - '0');
        else
        {
            return -1;
        }
    return result;
}

void _full_write (int fd, char * buffer, int len)
{
    while (len != 0)
    {
        int k = write(fd, buffer, len);
        len -= k;
        buffer += k;
    }
}

int main (int argc, char ** argv)
{
    char * usage_str = "Usage: ./main <buffer_size>\n";
    if (argc != 2)
    {
        _full_write(1, usage_str, _strlen(usage_str));
        return -1;
    }
    
    char * wrong_argument = "Second argument should be a positive number\n";
    int buffer_size = _get_int(argv[1]);
    if (buffer_size <= 0)
    {
        _full_write(1, wrong_argument, _strlen(wrong_argument));
        return -1;
    }
    
    ++buffer_size;
    char * buffer = malloc(buffer_size);
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
                block = read(0, buffer + end, buffer_size - end);
            else
                block = read(0, buffer + end, buffer_size - length);
        else
            block = read(0, buffer, buffer_size);
        if (block == 0)
        {
            if (length == buffer_size || state == 1)
                break;
            // fake to process the end
            buffer[end % buffer_size] = '\n';
            block = 1;
            quit = 1;
        }
        if (block == -1)
        {
            char * read_error = "An error occured while reading\n";
            _full_write(1, read_error, _strlen(read_error));
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
                            _full_write(1, buffer + begin, length + i + 1);
                        else
                        {
                            _full_write(1, buffer + begin, buffer_size - begin);
                            _full_write(1, buffer, end + i + 1);
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

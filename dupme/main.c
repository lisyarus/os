#include <unistd.h>
#include <stdio.h> // debug

int _strlen (char * str)
{
    char * temp = str;
    while (*temp++);
    return (int)(temp - str);
}

int main (int argc, char ** argv)
{
    char * usage_str = "Usage: ./main <buffer_size>\n";
    if (argc != 2)
    {
        write(1, usage_str, _strlen(usage_str));
    }
    return 0;
}

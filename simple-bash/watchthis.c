#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

void full_write (int fd, char * buffer, int size)
{
    //printf("Need to write %i\n", size);
    int written;
    for (written = 0; (written += write(fd, buffer + written, size - written)) < size;)
    {
        //printf("[%i]: %i written\n", fd, written);
    }
}

void write_to_file (const char * filename, char * buffer, int size)
{
    //printf("Opening '%s'...\n", filename);
    int fd = open(filename, O_WRONLY);
    //printf("Openned. Writing...\n");
    full_write(fd, buffer, size);
    close(fd);
}

const char * fifo_old = "/tmp/watchthis-fifo-old";
const char * fifo_new = "/tmp/watchthis-fifo-new";

void remove_fifos ( )
{
    remove(fifo_old);
    remove(fifo_new);
}

void exit_function ( )
{
    remove_fifos();
    exit(0);
}

int main (int argc, char ** argv)
{
    signal(SIGTERM, exit_function);
    signal(SIGINT, exit_function);
    
    if (argc < 3)
    {
        full_write(1, "Usage: watchthis interval command [args...]\n", 45);
        return 1;
    }
    
    int sleep_interval = atoi(argv[1]);
    if (sleep_interval < 0)
    {
        full_write(1, "Sleep interval must not be below zero\n", 39);
        return 1;
    }
    
    int fifo_old_status = mkfifo(fifo_old, S_IRUSR | S_IWUSR);
    int fifo_new_status = mkfifo(fifo_new, S_IRUSR | S_IWUSR);
    if (fifo_old_status || fifo_new_status)
    {
        full_write(1, "Unable to create fifos\n", 24);
        remove_fifos();
        return 1;
    }
    
    int pipefd[2];
    
    int buffer_size = 16;
    int old_buffer_pos = 0;
    char * old_buffer = (char *) malloc(buffer_size);
    char * new_buffer = (char *) malloc(buffer_size);
    
    while (1)
    {
        pipe(pipefd);
        if (fork())
        {
            close(pipefd[1]);
            wait(0);
            
            //printf("Command finished\n");
            
            int new_buffer_pos = 0;
            int read_count;
            while ((read_count = read(pipefd[0], new_buffer + new_buffer_pos, buffer_size - new_buffer_pos)) > 0)
            {
                new_buffer_pos += read_count;
                if (new_buffer_pos == buffer_size)
                {
                    buffer_size *= 2;
                    old_buffer = (char *) realloc(old_buffer, buffer_size);
                    new_buffer = (char *) realloc(new_buffer, buffer_size);
                }
            }
            close(pipefd[0]);
            
            /*printf("Command output:\n");
            printf("================\n");
            full_write(1, new_buffer, new_buffer_pos);
            printf("================\n");*/
            
            if (old_buffer_pos > 0)
            {
            
                //printf("Forking...\n");
                if (fork())
                {
                    //printf("Begin writing...\n");
                    write_to_file(fifo_old, old_buffer, old_buffer_pos);
                    write_to_file(fifo_new, new_buffer, new_buffer_pos);
                    //printf("Written\n");
                    wait(0);
                    //printf("diff finished\n");
                }
                else
                {
                    execlp("diff", "diff", "-u", fifo_old, fifo_new, NULL);
                }
            }
            
            char * temp = old_buffer;
            old_buffer = new_buffer;
            new_buffer = temp;
            old_buffer_pos = new_buffer_pos;
            
            sleep(sleep_interval);
        }
        else
        {
            dup2(pipefd[1], 1);
            close(pipefd[0]);
            close(pipefd[1]);
            execvp(argv[2], argv + 2);
        }
    }
    
    remove_fifos();
    return 0;
}

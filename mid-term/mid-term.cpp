#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

bool debug = false;

using command_t = std::vector<std::string>;
using pipe_t = std::vector<command_t>;

struct pipefd_t
{
    int pipefd[2];

    int operator [] (int i)
    {
        return pipefd[i];
    }
};

int run_pipe (pipe_t const & p)
{
    std::vector<pipefd_t> pipefds(p.size() - 1);
    for (pipefd_t & pfd : pipefds)
        pipe(pfd.pipefd);

    for (int c = 0; c < p.size(); ++c)
    {
        if (debug) std::cout << "Running process number " << c << "\n";
        if (fork())
        {
            if (c > 0) close(pipefds[c - 1][0]);
            if (c < p.size() - 1) close(pipefds[c][1]);
            int return_status;
            wait(&return_status);


            if (!(WIFEXITED(return_status) && WEXITSTATUS(return_status) == 0))
                return 1;
        }
        else
        {
            if (c > 0)
            {
                dup2(pipefds[c - 1][0], 0);
                close(pipefds[c - 1][0]);
            }
            if (c < p.size() - 1)
            {
                dup2(pipefds[c][1], 1);
                close(pipefds[c][1]);
            }
            std::vector<char *> args(p[c].size());
            for (int i = 0; i < p[c].size(); ++i)
                args[i] = const_cast<char *>(p[c][i].c_str());
            args.push_back(nullptr);
            execvp(args[0], args.data());
        }
    }

    return 0;
}

int main ( )
{
    const int buffer_size = 1024;
    char buffer[buffer_size];
    int buffer_pos = 0;
    int read_length = 0;

    bool success = true;

    bool end = false;
    while (!end)
    {
        pipe_t pipe;

        bool need_flag = false;
        bool flag;

        int line_end;
        bool found_endline = false;
        while (!found_endline)
        {
            if (read_length == 0)
                read_length = read(0, buffer + buffer_pos, buffer_size - buffer_pos);
            if (buffer_pos + read_length >= buffer_size)
            {
                std::cout << "Too long input, aborting.\n";
                return 1;
            }
            for (int i = buffer_pos; i < buffer_pos + read_length; ++i)
            {
                if (buffer[i] == '\n')
                {
                    line_end = i;
                    found_endline = true;
                    break;
                }
            }
            buffer_pos += read_length;
            if (!found_endline && read_length == 0)
            {
                line_end = buffer_pos;
                found_endline = true;
            }
        }
        if (read_length == 0)
            end = true;

        if (debug)
        {
            std::cout << "Buffer: [";
            for (int i = 0; i < buffer_pos; ++i)
                std::cout << buffer[i];
            std::cout << "]\n";
        }

        if (debug) std::cout << "Line end: " << line_end << "\n";
         
        auto put_char = [&] (char c)
        {
            if (pipe.empty())
                pipe.emplace_back();
            if (pipe.back().empty())
                pipe.back().emplace_back();
            pipe.back().back().push_back(c); 
        };

        for (int i = 0; i < line_end; ++i)
        {
            if (debug) std::cout << "Symbol " << i << "\n";
            if (i == 0 && buffer[i] == '+')
            {
                need_flag = true;
                flag = true;
                if (debug) std::cout << "Run only if previous succeeded\n";
            }
            else if (i == 0 && buffer[i] == '-')
            {
                need_flag = true;
                flag = false;
                if (debug) std::cout << "Run only if previous failed\n";
            }
            else if (buffer[i] == '\\')
            {
                ++i;
                if (i >= line_end)
                    std::cout << "Unexpected end. Aborting.\n";
                else if (buffer[i] == ' ')
                    put_char(' ');
                else if (buffer[i] == '\\')
                    put_char('\\');
                else 
                    std::cout << "Error: unknown sequence '\\" << buffer[i] <<"'. Aborting.\n";
            }
            else if (buffer[i] == ' ')
            {
                while (i < line_end && buffer[i] == ' ') ++i;
                if (debug) std::cout << "Found [" << buffer[i] << "] after spaces\n";
                if (buffer[i] == '|')
                {
                    pipe.push_back(command_t());
                }
                else
                {
                    --i;
                    if (pipe.empty())
                        pipe.emplace_back();
                    pipe.back().emplace_back();
                }
            }
            else
                put_char(buffer[i]);
        }

        if (debug) 
        {
            std::cout << "Pipe: ";
            for (int i = 0; i < pipe.size(); ++i)
            {
                if (i != 0) std::cout << " >---> ";
                for (int j = 0; j < pipe[i].size(); ++j)
                    std::cout << '"' << pipe[i][j] << "\" ";
            }
            std::cout << '\n';
        }
        
        if (buffer_pos > line_end)
        {
            memmove(buffer, buffer + line_end + 1, buffer_pos - line_end - 1);
            read_length = buffer_pos - line_end - 1;
            buffer_pos = 0;
        }

        if (debug)
        {
            std::cout << "Buffer after moving: [";
            for (int i = 0; i < buffer_pos; ++i)
                std::cout << buffer[i];
            std::cout << "]\n";
        }

        if (!pipe.empty() && !pipe.back().empty())
            if (!need_flag || (flag == success))
                success = (run_pipe(pipe) == 0); 
    }
}

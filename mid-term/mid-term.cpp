#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
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
}

int main ( )
{
    pipe_t p;
    run_pipe(p);
}

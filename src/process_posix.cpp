#include "internal/process_posix.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace codegrep::internal {


ProcessResult run_capture(const std::filesystem::path &cwd,
                          const std::vector<std::string> &argv) 
{
    if (argv.empty()) {
        throw std::runtime_error("run_capture: argv must not be empty");
    }

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        throw std::runtime_error(std::string("pipe failed: ") + std::strerror(errno));
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        throw std::runtime_error(std::string("fork failed: ") + std::strerror(errno));
    }

    if (pid == 0) {
        if (chdir(cwd.c_str()) != 0) {
            _exit(127);
        }

        if (dup2(pipefd[1], STDOUT_FILENO) < 0 || dup2(pipefd[1], STDERR_FILENO) < 0) {
            _exit(127);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        std::vector<char*> exec_argv;
        exec_argv.reserve(argv.size() + 1);
        for (const auto& arg : argv) {
            exec_argv.push_back(const_cast<char*>(arg.c_str()));
        }
        exec_argv.push_back(nullptr);

        execvp(exec_argv[0], exec_argv.data());
        _exit(127);
    }

    close(pipefd[1]);

    std::string output;
    char buffer[4096];
    while (true) {
        ssize_t n = read(pipefd[0], buffer, sizeof(buffer));
        if (n > 0) {
            output.append(buffer, static_cast<std::size_t>(n));
            continue;
        }
        if (n == 0) {
            break;
        }
        if (errno == EINTR) {
            continue;
        }
        break;
    }
    close(pipefd[0]);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        throw std::runtime_error(std::string("waitpid failed: ") + std::strerror(errno));
    }

    int exit_code = -1;
    if (WIFEXITED(status)) {
        exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        exit_code = 128 + WTERMSIG(status);
    }

    return ProcessResult{.exit_code = exit_code, .output = std::move(output)};
}

}  // namespace codegrep::internal

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace codegrep::internal {

struct ProcessResult 
{
    int exit_code{-1};
    std::string output;
};

ProcessResult run_capture(const std::filesystem::path &cwd,
                          const std::vector<std::string> &argv);

}  // namespace codegrep::internal

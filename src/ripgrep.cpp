#include "internal/ripgrep.h"

#include "codegrep/searcher.h"
#include "internal/process_posix.h"
#include "internal/rg_command.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace codegrep::internal {

namespace {


// edge case handling - append \0 and split on it later
// posix allows \n in file name
std::vector<std::string> split_nul(const std::string &text) 
{
    std::vector<std::string> out;
    std::string current;
    for (unsigned char ch : text) {
        if (ch == '\0') {
            if (!current.empty()) {
                out.push_back(std::move(current));
                // only sets the size, allocation not freed
                current.clear();
            }
            continue;
        }
        current.push_back(static_cast<char>(ch));
    }
    if (!current.empty()) {
        out.push_back(std::move(current));
    }
    return out;
}


std::vector<std::filesystem::path> parse_paths(const std::filesystem::path &repo_root,
                                               const std::string &data) 
{
    std::vector<std::filesystem::path> out;
    for (const std::string &rel : split_nul(data)) {
        if (rel.empty()) {
            continue;
        }
        out.push_back(std::filesystem::absolute(repo_root / rel).lexically_normal());
    }
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

}  // namespace


std::vector<std::filesystem::path> rg_list_files(const std::filesystem::path &repo_root,
                                                 const SearcherOptions &options,
                                                 const SearchRequest &request) 
{
    const auto argv = build_list_files_argv(options, request);
    const ProcessResult result = run_capture(repo_root, argv);
    if (result.exit_code != 0) {
        throw Error("codegrep: rg --files failed: " + result.output);
    }
    return parse_paths(repo_root, result.output);
}


std::vector<std::filesystem::path> rg_files_with_matches(const std::filesystem::path &repo_root,
                                                         const SearcherOptions &options,
                                                         const SearchRequest &request,
                                                         const QueryTerm &term) 
{
    if (term.kind == QueryKind::Path) {
        return {};
    }

    const auto argv = build_files_with_matches_argv(options, request, term);
    const ProcessResult result = run_capture(repo_root, argv);
    if (result.exit_code == 1) {
        return {};
    }
    if (result.exit_code != 0) {
        throw Error("codegrep: rg search failed for term '" + term.value + "': " + result.output);
    }
    return parse_paths(repo_root, result.output);
}


}  // namespace codegrep::internal




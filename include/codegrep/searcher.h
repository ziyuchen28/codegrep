#pragma once

#include "codegrep/query.h"
#include "codegrep/result.h"

#include <filesystem>
#include <stdexcept>
#include <vector>

namespace codegrep {


class Error : public std::runtime_error 
{
public:
    using std::runtime_error::runtime_error;
};


struct SearcherOptions 
{
    // optional
    std::filesystem::path rg_path{};
    bool honor_gitignore{true};
    bool include_hidden{false};
    std::vector<std::string> default_include_globs;
    std::vector<std::string> default_exclude_globs;
};


class Searcher 
{
public:
    explicit Searcher(SearcherOptions options = {});
    SearchResult search(const std::filesystem::path &repo_root,
                        const SearchRequest &request) const;

private:
    SearcherOptions options_;
};

}  // namespace codegrep

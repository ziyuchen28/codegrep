#pragma once

#include <optional>
#include <string>
#include <vector>

namespace codegrep {

enum class QueryKind 
{
    Identifier,
    Literal,
    Path,
    Text,
};

struct QueryTerm 
{
    QueryKind kind{};
    std::string value;
    double weight{1.0};
    bool required{false};
    std::optional<bool> case_sensitive{};
};

struct SearchLimits 
{
    std::size_t max_files{20};
    std::size_t max_hits_per_file{20};
    std::size_t max_total_hits{200};
};

struct SnippetOptions 
{
    std::size_t context_before{2};
    std::size_t context_after{2};
    std::size_t max_snippets_per_file{3};
    bool merge_overlapping{true};
};

struct SearchRequest 
{
    std::vector<QueryTerm> terms;
    std::vector<std::string> include_globs;
    std::vector<std::string> exclude_globs;
    SearchLimits limits{};
    SnippetOptions snippets{};
};

}  // namespace codegrep

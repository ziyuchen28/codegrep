#pragma once

#include "codegrep/query.h"

#include <filesystem>
#include <string>
#include <vector>

namespace codegrep {


enum class HitKind 
{
    Path,
    Content,
};


enum class ScoreComponentKind 
{
    PathHit,
    IdentifierHit,
    LiteralHit,
    TextHit,
    RequiredTermSatisfied,
    DistinctTermsBonus,
    MultiHitBonus,
};


struct ScoreComponent 
{
    ScoreComponentKind kind{};
    std::string term;
    double delta{0.0};
};


struct Hit 
{
    HitKind kind{HitKind::Content};
    QueryKind query_kind{};
    std::string matched_term;

    std::size_t line{0};
    std::size_t column{0};
    std::size_t length{0};

    double score{0.0};
    std::string line_text;
};


struct Snippet 
{
    std::size_t start_line{0};
    std::size_t end_line{0};
    double score{0.0};
    std::string text;
    std::vector<std::string> matched_terms;
};


struct FileResult 
{
    std::filesystem::path path;
    double score{0.0};

    std::vector<std::string> matched_terms;
    std::vector<ScoreComponent> score_breakdown;
    std::vector<Hit> hits;
    std::vector<Snippet> snippets;
};

struct SearchStats 
{
    std::size_t files_considered{0};
    std::size_t files_with_matches{0};
    std::size_t total_hits{0};
};


struct SearchResult 
{
    std::vector<FileResult> files;
    SearchStats stats{};
};

const char *to_string(QueryKind kind);
const char *to_string(HitKind kind);
const char *to_string(ScoreComponentKind kind);

}  // namespace codegrep



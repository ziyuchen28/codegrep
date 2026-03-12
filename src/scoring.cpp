#include "internal/scoring.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include <string_view>

namespace codegrep::internal {


namespace 
{

    std::string to_lower(std::string_view input) 
    {
        std::string out;
        out.reserve(input.size());
        for (unsigned char ch : input) {
            out.push_back(static_cast<char>(std::tolower(ch)));
        }
        return out;
    }

    std::string relative_string(const std::filesystem::path &repo_root,
                                const std::filesystem::path &path) 
    {
        try {
            return std::filesystem::relative(path, repo_root).generic_string();
        } catch (...) {
            return path.generic_string();
        }
    }

    std::string stem_string(const std::filesystem::path &path) 
    {
        return path.stem().string();
    }

}  // namespace


bool default_case_sensitive(QueryKind kind) 
{
    switch (kind) {
        case QueryKind::Identifier:
        case QueryKind::Literal:
            return true;
        case QueryKind::Path:
        case QueryKind::Text:
            return false;
    }
    return true;
}


ScoreComponentKind content_component_kind(QueryKind kind) 
{
    switch (kind) {
        case QueryKind::Identifier:
            return ScoreComponentKind::IdentifierHit;
        case QueryKind::Literal:
            return ScoreComponentKind::LiteralHit;
        case QueryKind::Path:
            return ScoreComponentKind::PathHit;
        case QueryKind::Text:
            return ScoreComponentKind::TextHit;
    }
    return ScoreComponentKind::TextHit;
}


double path_hit_score(const std::filesystem::path &repo_root,
                      const std::filesystem::path &path,
                      const QueryTerm &term) 
{
    if (term.value.empty()) {
        return 0.0;
    }

    const bool case_sensitive = term.case_sensitive.value_or(default_case_sensitive(term.kind));
    const auto norm = [&](std::string_view s) {
        return case_sensitive ? std::string(s) : to_lower(s);
    };

    const std::string rel = norm(relative_string(repo_root, path));
    const std::string file = norm(path.filename().string());
    const std::string stem = norm(stem_string(path));
    const std::string needle = norm(term.value);
    const double weight = std::max(0.0, term.weight);

    // to do tuning
    if (file == needle || rel == needle) {
        return 8.0 * weight;
    }
    if (stem == needle) {
        return 7.0 * weight;
    }
    if (file.find(needle) != std::string::npos) {
        return 5.5 * weight;
    }
    if (rel.find(needle) != std::string::npos) {
        return 3.0 * weight;
    }
    return 0.0;
}


double content_hit_score(const QueryTerm &term) 
{
    const double weight = std::max(0.0, term.weight);
    switch (term.kind) {
        case QueryKind::Identifier:
            return 10.0 * weight;
        case QueryKind::Literal:
            return 8.5 * weight;
        case QueryKind::Path:
            return 0.0;
        case QueryKind::Text:
            return 3.5 * weight;
    }
    return 0.0;
}


double required_term_bonus(const QueryTerm &term) 
{
    if (!term.required) {
        return 0.0;
    }
    return 4.0 * std::max(0.0, term.weight);
}


double distinct_terms_bonus(std::size_t distinct_terms) 
{
    if (distinct_terms <= 1) {
        return 0.0;
    }
    return static_cast<double>(distinct_terms - 1) * 1.5;
}


double multi_hit_bonus(std::size_t total_content_hits) 
{
    if (total_content_hits <= 1) {
        return 0.0;
    }
    return std::min(3.0, static_cast<double>(total_content_hits - 1) * 0.35);
}

}  // namespace codegrep::internal




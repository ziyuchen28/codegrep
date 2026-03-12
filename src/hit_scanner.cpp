#include "internal/hit_scanner.h"

#include "internal/scoring.h"

#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace codegrep::internal {


namespace {


bool is_identifier_char(char ch) 
{
    const unsigned char u = static_cast<unsigned char>(ch);
    return std::isalnum(u) || ch == '_';
}


std::string to_lower(std::string_view input) {
    std::string out;
    out.reserve(input.size());
    for (unsigned char ch : input) {
        out.push_back(static_cast<char>(std::tolower(ch)));
    }
    return out;
}

bool effective_case_sensitive(const QueryTerm& term) {
    return term.case_sensitive.value_or(default_case_sensitive(term.kind));
}


std::vector<std::size_t> find_identifier_occurrences(std::string_view line,
                                                     std::string_view term,
                                                     bool case_sensitive) 
{
    std::vector<std::size_t> offsets;
    if (term.empty() || line.size() < term.size()) {
        return offsets;
    }

    const std::string haystack = case_sensitive ? std::string(line) : to_lower(line);
    const std::string needle = case_sensitive ? std::string(term) : to_lower(term);

    std::size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        const bool left_ok = (pos == 0) || !is_identifier_char(line[pos - 1]);
        const std::size_t end = pos + term.size();
        const bool right_ok = (end >= line.size()) || !is_identifier_char(line[end]);
        if (left_ok && right_ok) {
            offsets.push_back(pos);
        }
        ++pos;
        // to do - pos += term.size()
    }

    return offsets;
}


std::vector<std::size_t> find_substring_occurrences(std::string_view line,
                                                    std::string_view term,
                                                    bool case_sensitive) 
{
    std::vector<std::size_t> offsets;
    if (term.empty() || line.size() < term.size()) {
        return offsets;
    }

    const std::string haystack = case_sensitive ? std::string(line) : to_lower(line);
    const std::string needle = case_sensitive ? std::string(term) : to_lower(term);

    std::size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        offsets.push_back(pos);
        ++pos;
        // to do - pos += term.size()
    }

    return offsets;
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

}  // namespace



std::optional<Hit> build_path_hit(const std::filesystem::path &repo_root,
                                  const std::filesystem::path &path,
                                  const QueryTerm &term,
                                  double score) 
{
    if (score <= 0.0) {
        return std::nullopt;
    }

    return Hit{
        .kind = HitKind::Path,
        .query_kind = term.kind,
        .matched_term = term.value,
        .line = 0,
        .column = 0,
        .length = term.value.size(),
        .score = score,
        .line_text = relative_string(repo_root, path),
    };
}


std::vector<Hit> scan_content_hits(const std::vector<std::string> &lines,
                                   const QueryTerm &term,
                                   double hit_score) 
{
    std::vector<Hit> hits;
    if (term.kind == QueryKind::Path) {
        return hits;
    }

    const bool case_sensitive = effective_case_sensitive(term);

    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto &line = lines[i];
        std::vector<std::size_t> offsets;

        switch (term.kind) {
            case QueryKind::Identifier:
                offsets = find_identifier_occurrences(line, term.value, case_sensitive);
                break;
            case QueryKind::Literal:
            case QueryKind::Text:
                offsets = find_substring_occurrences(line, term.value, case_sensitive);
                break;
            case QueryKind::Path:
                break;
        }

        for (std::size_t offset : offsets) {
            hits.push_back(Hit{
                .kind = HitKind::Content,
                .query_kind = term.kind,
                .matched_term = term.value,
                .line = i + 1,
                .column = offset + 1,
                .length = term.value.size(),
                .score = hit_score,
                .line_text = line,
            });
        }
    }

    return hits;
}


}  // namespace codegrep::internal



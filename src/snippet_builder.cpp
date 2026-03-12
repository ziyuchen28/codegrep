#include "internal/snippet_builder.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>

namespace codegrep::internal {


namespace {


struct Window 
{
    std::size_t start{0};
    std::size_t end{0};
    double score{0.0};
    std::set<std::string> matched_terms;
};


std::string build_text(const std::vector<std::string> &lines,
                       std::size_t start,
                       std::size_t end) 
{
    std::ostringstream out;
    for (std::size_t line_no = start; line_no <= end; ++line_no) {
        out << line_no << ": " << lines[line_no - 1];
        if (line_no != end) {
            out << '\n';
        }
    }
    return out.str();
}


}  // namespace



std::vector<Snippet> build_snippets(const std::vector<std::string> &lines,
                                    const std::vector<Hit> &hits,
                                    const SnippetOptions &options) 
{
    std::vector<Window> windows;

    for (const Hit &hit : hits) {
        if (hit.kind != HitKind::Content || hit.line == 0 || hit.line > lines.size()) {
            continue;
        }

        const std::size_t start = (hit.line > options.context_before)
                                   ? hit.line - options.context_before
                                   : 1;
        const std::size_t end = std::min(lines.size(), hit.line + options.context_after);

        Window window;
        window.start = start;
        window.end = end;
        window.score = hit.score;
        window.matched_terms.insert(hit.matched_term);
        windows.push_back(std::move(window));
    }

    std::sort(windows.begin(), windows.end(), [](const Window &lhs, const Window &rhs) {
        if (lhs.start != rhs.start) {
            return lhs.start < rhs.start;
        }
        return lhs.end < rhs.end;
    });

    // merger intervals
    std::vector<Window> merged;
    for (const Window &window : windows) {
        if (!options.merge_overlapping || merged.empty() || window.start > merged.back().end + 1) {
            merged.push_back(window);
            continue;
        }

        Window &back = merged.back();
        back.end = std::max(back.end, window.end);
        back.score += window.score;
        back.matched_terms.insert(window.matched_terms.begin(), window.matched_terms.end());
    }

    std::vector<Snippet> snippets;
    snippets.reserve(merged.size());
    for (const Window &window : merged) {
        Snippet snippet;
        snippet.start_line = window.start;
        snippet.end_line = window.end;
        snippet.score = window.score;
        snippet.text = build_text(lines, window.start, window.end);
        snippet.matched_terms.assign(window.matched_terms.begin(), window.matched_terms.end());
        snippets.push_back(std::move(snippet));
    }

    std::sort(snippets.begin(), snippets.end(), [](const Snippet& lhs, const Snippet& rhs) {
        if (lhs.score != rhs.score) {
            return lhs.score > rhs.score;
        }
        if (lhs.start_line != rhs.start_line) {
            return lhs.start_line < rhs.start_line;
        }
        return lhs.end_line < rhs.end_line;
    });

    if (snippets.size() > options.max_snippets_per_file) {
        snippets.resize(options.max_snippets_per_file);
    }

    std::sort(snippets.begin(), snippets.end(), [](const Snippet& lhs, const Snippet& rhs) {
        if (lhs.start_line != rhs.start_line) {
            return lhs.start_line < rhs.start_line;
        }
        return lhs.end_line < rhs.end_line;
    });

    return snippets;
}

}  // namespace codegrep::internal




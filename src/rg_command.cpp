#include "internal/rg_command.h"

#include <string>
#include <string_view>
#include <vector>

namespace codegrep::internal {


namespace {


bool default_case_sensitive_for_kind(QueryKind kind) 
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


bool effective_case_sensitive(const QueryTerm &term) 
{
    return term.case_sensitive.value_or(default_case_sensitive_for_kind(term.kind));
}


std::string escape_regex(std::string_view text) 
{
    static constexpr std::string_view meta = R"(\.^$|()[]{}*+?-)";
    std::string out;
    out.reserve(text.size() * 2);
    for (char ch : text) {
        if (meta.find(ch) != std::string_view::npos) {
            out.push_back('\\');
        }
        out.push_back(ch);
    }
    return out;
}


std::vector<std::string> merged_includes(const SearcherOptions &options,
                                         const SearchRequest &request) 
{
    std::vector<std::string> out = options.default_include_globs;
    out.insert(out.end(), request.include_globs.begin(), request.include_globs.end());
    return out;
}


std::vector<std::string> merged_excludes(const SearcherOptions &options,
                                         const SearchRequest &request) 
{
    std::vector<std::string> out = options.default_exclude_globs;
    out.insert(out.end(), request.exclude_globs.begin(), request.exclude_globs.end());
    return out;
}


void append_common_args(std::vector<std::string> &argv,
                        const SearcherOptions &options,
                        const SearchRequest &request) 
{
    argv.push_back("--no-config");
    argv.push_back("--color=never");

    if (options.include_hidden) {
        argv.push_back("--hidden");
    }
    if (!options.honor_gitignore) {
        argv.push_back("--no-ignore");
    }

    for (const std::string &glob : merged_includes(options, request)) {
        argv.push_back("-g");
        argv.push_back(glob);
    }
    for (const std::string &glob : merged_excludes(options, request)) {
        argv.push_back("-g");
        argv.push_back("!" + glob);
    }
}


}  // namespace


std::string resolve_rg_program(const SearcherOptions &options) 
{
    if (!options.rg_path.empty()) {
        return options.rg_path.string();
    }
#ifdef CODEGREP_DEFAULT_RG_PATH
    return CODEGREP_DEFAULT_RG_PATH;
#else
    return "rg";
#endif
}


std::vector<std::string> build_list_files_argv(const SearcherOptions &options,
                                               const SearchRequest &request) 
{
    std::vector<std::string> argv;
    argv.push_back(resolve_rg_program(options));
    argv.push_back("--files");
    // edge case handling - append \0 and split on it later
    // posix allows \n in file name
    argv.push_back("-0");
    append_common_args(argv, options, request);
    return argv;
}


std::vector<std::string> build_files_with_matches_argv(const SearcherOptions &options,
                                                       const SearchRequest &request,
                                                       const QueryTerm &term) 
{
    std::vector<std::string> argv;
    argv.push_back(resolve_rg_program(options));
    // -l will shortcut the search if one match found
    argv.push_back("-l");
    // edge case handling - append \0 and split on it later
    // posix allows \n in file name
    argv.push_back("-0");
    append_common_args(argv, options, request);

    switch (term.kind) {
        case QueryKind::Identifier:
            if (!effective_case_sensitive(term)) {
                argv.push_back("-i");
            }
            argv.push_back("-e");
            argv.push_back("\\b" + escape_regex(term.value) + "\\b");
            break;

        case QueryKind::Literal:
            argv.push_back("-F");
            if (!effective_case_sensitive(term)) {
                argv.push_back("-i");
            }
            argv.push_back("-e");
            argv.push_back(term.value);
            break;

        case QueryKind::Text:
            argv.push_back("-F");
            if (!effective_case_sensitive(term)) {
                argv.push_back("-i");
            }
            argv.push_back("-e");
            argv.push_back(term.value);
            break;

        case QueryKind::Path:
            break;
    }

    argv.push_back(".");
    return argv;
}


}  // namespace codegrep::internal





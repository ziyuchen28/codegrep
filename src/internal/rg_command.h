#pragma once

#include "codegrep/searcher.h"

#include <string>
#include <vector>

namespace codegrep::internal {


std::string resolve_rg_program(const SearcherOptions &options);


std::vector<std::string> build_list_files_argv(const SearcherOptions &options,
                                               const SearchRequest &request);


std::vector<std::string> build_files_with_matches_argv(const SearcherOptions &options,
                                                       const SearchRequest &request,
                                                       const QueryTerm &term);


}  // namespace codegrep::internal

#pragma once

#include "codegrep/searcher.h"

#include <filesystem>
#include <vector>

namespace codegrep::internal {


std::vector<std::filesystem::path> rg_list_files(const std::filesystem::path &repo_root,
                                                 const SearcherOptions &options,
                                                 const SearchRequest &request);

std::vector<std::filesystem::path> rg_files_with_matches(const std::filesystem::path &repo_root,
                                                         const SearcherOptions &options,
                                                         const SearchRequest &request,
                                                         const QueryTerm &term);

}  // namespace codegrep::internal

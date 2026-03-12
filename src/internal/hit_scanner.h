#pragma once

#include "codegrep/result.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace codegrep::internal {

std::vector<std::string> read_lines(const std::filesystem::path &path);

std::optional<Hit> build_path_hit(const std::filesystem::path &repo_root,
                                  const std::filesystem::path &path,
                                  const QueryTerm &term,
                                  double score);

std::vector<Hit> scan_content_hits(const std::vector<std::string> &lines,
                                   const QueryTerm &term,
                                   double hit_score);

}  // namespace codegrep::internal

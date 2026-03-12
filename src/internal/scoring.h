#pragma once

#include "codegrep/result.h"

#include <filesystem>

namespace codegrep::internal {

bool default_case_sensitive(QueryKind kind);
ScoreComponentKind content_component_kind(QueryKind kind);

double path_hit_score(const std::filesystem::path& repo_root,
                      const std::filesystem::path& path,
                      const QueryTerm& term);

double content_hit_score(const QueryTerm& term);
double required_term_bonus(const QueryTerm& term);
double distinct_terms_bonus(std::size_t distinct_terms);
double multi_hit_bonus(std::size_t total_content_hits);

}  // namespace codegrep::internal

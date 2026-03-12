#pragma once

#include "codegrep/result.h"

#include <string>
#include <vector>

namespace codegrep::internal {

std::vector<Snippet> build_snippets(const std::vector<std::string>& lines,
                                    const std::vector<Hit>& hits,
                                    const SnippetOptions& options);

}  // namespace codegrep::internal

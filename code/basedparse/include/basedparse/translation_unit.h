#ifndef BASEDPARSE_TRANSLATION_UNIT_H
#define BASEDPARSE_TRANSLATION_UNIT_H

#include <memory>
#include <vector>

#include "statement.h"

namespace basedparse
{

  struct Translation_unit
  {
    std::vector<std::unique_ptr<Statement>> statements;
  };

} // namespace basedparse

#endif // BASEDPARSE_TRANSLATION_UNIT_H

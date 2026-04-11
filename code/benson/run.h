#ifndef BASED_RUN_H
#define BASED_RUN_H

#include <iosfwd>
#include <span>
#include <string_view>

namespace benson
{

  int run(
    std::istream &input,
    std::string_view function_name,
    std::span<std::string_view const> args,
    std::ostream &out,
    std::ostream &err
  );

} // namespace benson

#endif // BASED_RUN_H

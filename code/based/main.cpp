#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "run.h"

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    std::cerr << "usage: based <file> <function> [args...]\n";
    return 1;
  }
  auto const path = std::string{argv[1]};
  auto const function_name = std::string{argv[2]};
  auto file = std::ifstream{path};
  if (!file)
  {
    std::cerr << "error: could not open " << path << '\n';
    return 1;
  }
  auto args = std::vector<std::string_view>{};
  args.reserve(argc - 3);
  for (auto i = 3; i < argc; ++i)
  {
    args.push_back(argv[i]);
  }
  return based::run(file, function_name, args, std::cout, std::cerr);
}

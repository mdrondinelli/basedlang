#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include <basedhlir/compile.h>
#include <basedhlir/interpret.h>
#include <basedhlir/type.h>
#include <basedlex/istream_binary_stream.h>
#include <basedlex/lexeme_stream.h>
#include <basedlex/lexeme_stream_reader.h>
#include <basedlex/utf8_char_stream.h>
#include <basedparse/parser.h>

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
  auto bs = basedlex::Istream_binary_stream{&file};
  auto cs = basedlex::Utf8_char_stream{&bs};
  auto ls = basedlex::Lexeme_stream{&cs};
  auto lr = basedlex::Lexeme_stream_reader{&ls};
  auto parser = basedparse::Parser{&lr};
  auto const ast = parser.parse_translation_unit();
  auto types = basedhlir::Type_pool{};
  try
  {
    auto const tu = basedhlir::compile(*ast, &types);
    auto const it = tu.function_table.find(function_name);
    if (it == tu.function_table.end())
    {
      std::cerr << "error: no function named '" << function_name << "'\n";
      return 1;
    }
    auto const &func = *it->second;
    auto args = std::vector<basedhlir::Constant_value>{};
    for (auto i = 3; i < argc; ++i)
    {
      auto const arg = std::string{argv[i]};
      if (arg == "true")
      {
        args.push_back(true);
      }
      else if (arg == "false")
      {
        args.push_back(false);
      }
      else
      {
        args.push_back(static_cast<std::int32_t>(std::stoi(arg)));
      }
    }
    auto const result = basedhlir::interpret(func, args);
    std::visit(
      [](auto const &v)
      {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::int32_t>)
        {
          std::cout << v << '\n';
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
          std::cout << (v ? "true" : "false") << '\n';
        }
        else if constexpr (std::is_same_v<T, basedhlir::Void_value>)
        {
          std::cout << "void\n";
        }
        else if constexpr (std::is_same_v<T, basedhlir::Type_value>)
        {
          std::cout << "<type>\n";
        }
        else if constexpr (std::is_same_v<T, basedhlir::Function_value>)
        {
          std::cout << "<function>\n";
        }
      },
      result
    );
  }
  catch (basedhlir::Compilation_failure const &e)
  {
    std::cerr << e.what();
    return 1;
  }
  return 0;
}

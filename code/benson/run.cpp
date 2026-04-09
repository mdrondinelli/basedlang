#include "run.h"

#include <charconv>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <variant>
#include <vector>

#include <bensonir/compile.h>
#include <bensonir/interpret.h>
#include <bensonir/type.h>
#include <bensonlex/istream_binary_stream.h>
#include <bensonlex/lexeme_stream.h>
#include <bensonlex/lexeme_stream_reader.h>
#include <bensonlex/utf8_char_stream.h>
#include <bensonparse/parser.h>

namespace benson
{

  int run(
    std::istream &input,
    std::string_view function_name,
    std::span<std::string_view const> args,
    std::ostream &out,
    std::ostream &err
  )
  {
    auto bs = bensonlex::Istream_binary_stream{&input};
    auto cs = bensonlex::Utf8_char_stream{&bs};
    auto ls = bensonlex::Lexeme_stream{&cs};
    auto lr = bensonlex::Lexeme_stream_reader{&ls};
    auto parser = bensonparse::Parser{&lr};

    benson::ast::Translation_unit ast;
    try
    {
      ast = parser.parse_translation_unit();
    }
    catch (bensonlex::Lexeme_stream::Lex_error const &e)
    {
      err << "lex error at " << e.location.line << ":" << e.location.column
          << '\n';
      return 1;
    }
    catch (bensonlex::Utf8_char_stream::Decode_error const &e)
    {
      err << e.what() << '\n';
      return 1;
    }
    catch (std::runtime_error const &e)
    {
      err << e.what() << '\n';
      return 1;
    }

    auto types = benson::ir::Type_pool{};
    try
    {
      auto const tu = benson::ir::compile(ast, &types);
      auto const it = tu.function_table.find(std::string{function_name});
      if (it == tu.function_table.end())
      {
        err << "error: no function named '" << function_name << "'\n";
        return 1;
      }

      auto const &func = *it->second;
      auto const &param_types =
        std::get<benson::ir::Function_type>(func.type->data).parameter_types;
      auto constant_args = std::vector<benson::ir::Constant_value>{};
      constant_args.reserve(args.size());
      for (auto i = std::size_t{}; i < args.size(); ++i)
      {
        auto const arg = args[i];
        if (arg == "true")
        {
          constant_args.push_back(true);
          continue;
        }
        if (arg == "false")
        {
          constant_args.push_back(false);
          continue;
        }
        auto const *target_type =
          (i < param_types.size()) ? param_types[i] : nullptr;
        if (target_type &&
            std::holds_alternative<benson::ir::Float32_type>(target_type->data))
        {
          float val{};
          auto const [ptr, ec] =
            std::from_chars(arg.data(), arg.data() + arg.size(), val);
          if (ec != std::errc{} || ptr != arg.data() + arg.size())
          {
            err << "error: invalid Float32 argument: " << arg << '\n';
            return 1;
          }
          constant_args.push_back(val);
        }
        else if (target_type &&
                 std::holds_alternative<benson::ir::Float64_type>(
                   target_type->data
                 ))
        {
          double val{};
          auto const [ptr, ec] =
            std::from_chars(arg.data(), arg.data() + arg.size(), val);
          if (ec != std::errc{} || ptr != arg.data() + arg.size())
          {
            err << "error: invalid Float64 argument: " << arg << '\n';
            return 1;
          }
          constant_args.push_back(val);
        }
        else if (target_type &&
                 std::holds_alternative<benson::ir::Int8_type>(target_type->data))
        {
          std::int32_t val{};
          auto const [ptr, ec] =
            std::from_chars(arg.data(), arg.data() + arg.size(), val);
          if (ec != std::errc{} || ptr != arg.data() + arg.size() ||
              val < std::numeric_limits<std::int8_t>::min() ||
              val > std::numeric_limits<std::int8_t>::max())
          {
            err << "error: invalid Int8 argument: " << arg << '\n';
            return 1;
          }
          constant_args.push_back(static_cast<std::int8_t>(val));
        }
        else if (target_type &&
                 std::holds_alternative<benson::ir::Int16_type>(
                   target_type->data
                 ))
        {
          std::int32_t val{};
          auto const [ptr, ec] =
            std::from_chars(arg.data(), arg.data() + arg.size(), val);
          if (ec != std::errc{} || ptr != arg.data() + arg.size() ||
              val < std::numeric_limits<std::int16_t>::min() ||
              val > std::numeric_limits<std::int16_t>::max())
          {
            err << "error: invalid Int16 argument: " << arg << '\n';
            return 1;
          }
          constant_args.push_back(static_cast<std::int16_t>(val));
        }
        else if (target_type &&
                 std::holds_alternative<benson::ir::Int64_type>(
                   target_type->data
                 ))
        {
          std::int64_t val{};
          auto const [ptr, ec] =
            std::from_chars(arg.data(), arg.data() + arg.size(), val);
          if (ec != std::errc{} || ptr != arg.data() + arg.size())
          {
            err << "error: invalid Int64 argument: " << arg << '\n';
            return 1;
          }
          constant_args.push_back(val);
        }
        else
        {
          std::int32_t val{};
          auto const [ptr, ec] =
            std::from_chars(arg.data(), arg.data() + arg.size(), val);
          if (ec != std::errc{} || ptr != arg.data() + arg.size())
          {
            err << "error: invalid Int32 argument: " << arg << '\n';
            return 1;
          }
          constant_args.push_back(val);
        }
      }

      auto const result = benson::ir::interpret(func, constant_args);
      std::visit(
        [&](auto const &value)
        {
          using T = std::decay_t<decltype(value)>;
          if constexpr (std::is_same_v<T, std::int8_t>)
          {
            out << static_cast<std::int32_t>(value) << '\n';
          }
          else if constexpr (std::is_same_v<T, bool>)
          {
            out << (value ? "true" : "false") << '\n';
          }
          else if constexpr (std::is_same_v<T, benson::ir::Void_value>)
          {
            out << "void\n";
          }
          else if constexpr (std::is_same_v<T, benson::ir::Type_value>)
          {
            out << "<type>\n";
          }
          else if constexpr (std::is_same_v<T, benson::ir::Function_value>)
          {
            out << "<function>\n";
          }
          else
          {
            out << value << '\n';
          }
        },
        result
      );
    }
    catch (benson::ir::Compilation_failure const &e)
    {
      err << e.what();
      return 1;
    }

    return 0;
  }

} // namespace benson

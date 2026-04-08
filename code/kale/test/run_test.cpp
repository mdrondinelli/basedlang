#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "run.h"

TEST_CASE("kale::run executes a valid program")
{
  auto input = std::istringstream{
    "let main = fn(): Int32 => { return 42; };"
  };
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{};

  auto const exit_code = kale::run(input, "main", args, out, err);

  CHECK(exit_code == 0);
  CHECK(out.str() == "42\n");
  CHECK(err.str().empty());
}

TEST_CASE("kale::run reports lexing errors")
{
  auto input = std::istringstream{
    "let main = fn(): Int32 => { @ }"
  };
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{};

  auto const exit_code = kale::run(input, "main", args, out, err);

  CHECK(exit_code == 1);
  CHECK(out.str().empty());
  CHECK(err.str() == "lex error at 1:29\n");
}

TEST_CASE("kale::run reports parsing errors")
{
  auto input = std::istringstream{
    "let main = fn(: Int32 => { return 0; }"
  };
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{};

  auto const exit_code = kale::run(input, "main", args, out, err);

  CHECK(exit_code == 1);
  CHECK(out.str().empty());
  CHECK(err.str() == "unexpected token ':' at 1:15\n");
}

TEST_CASE("kale::run reports UTF-8 decoding errors")
{
  auto input = std::istringstream{std::string{"\x80", 1}};
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{};

  auto const exit_code = kale::run(input, "main", args, out, err);

  CHECK(exit_code == 1);
  CHECK(out.str().empty());
  CHECK(err.str() == "invalid UTF-8 byte sequence\n");
}

TEST_CASE("kale::run rejects non-numeric Int32 argument")
{
  auto input = std::istringstream{
    "let add = fn(a: Int32, b: Int32): Int32 => { return a + b; };"
  };
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{"abc", "1"};

  auto const exit_code = kale::run(input, "add", args, out, err);

  CHECK(exit_code == 1);
  CHECK(out.str().empty());
  CHECK(err.str() == "error: invalid Int32 argument: abc\n");
}

TEST_CASE("kale::run rejects out-of-range Int8 argument")
{
  auto input = std::istringstream{
    "let f = fn(a: Int8): Int8 => { return a; };"
  };
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{"200"};

  auto const exit_code = kale::run(input, "f", args, out, err);

  CHECK(exit_code == 1);
  CHECK(out.str().empty());
  CHECK(err.str() == "error: invalid Int8 argument: 200\n");
}

TEST_CASE("kale::run rejects out-of-range Int16 argument")
{
  auto input = std::istringstream{
    "let f = fn(a: Int16): Int16 => { return a; };"
  };
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{"40000"};

  auto const exit_code = kale::run(input, "f", args, out, err);

  CHECK(exit_code == 1);
  CHECK(out.str().empty());
  CHECK(err.str() == "error: invalid Int16 argument: 40000\n");
}

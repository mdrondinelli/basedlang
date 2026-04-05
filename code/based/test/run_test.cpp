#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "run.h"

TEST_CASE("based::run executes a valid program")
{
  auto input = std::istringstream{
    "let main = fn(): Int32 => { return 42; };"
  };
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{};

  auto const exit_code = based::run(input, "main", args, out, err);

  CHECK(exit_code == 0);
  CHECK(out.str() == "42\n");
  CHECK(err.str().empty());
}

TEST_CASE("based::run reports lexing errors")
{
  auto input = std::istringstream{
    "let main = fn(): Int32 => { @ }"
  };
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{};

  auto const exit_code = based::run(input, "main", args, out, err);

  CHECK(exit_code == 1);
  CHECK(out.str().empty());
  CHECK(err.str() == "lex error at 1:29\n");
}

TEST_CASE("based::run reports parsing errors")
{
  auto input = std::istringstream{
    "let main = fn(: Int32 => { return 0; }"
  };
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{};

  auto const exit_code = based::run(input, "main", args, out, err);

  CHECK(exit_code == 1);
  CHECK(out.str().empty());
  CHECK(err.str() == "unexpected token ':' at 1:15\n");
}

TEST_CASE("based::run reports UTF-8 decoding errors")
{
  auto input = std::istringstream{std::string{"\x80", 1}};
  auto out = std::ostringstream{};
  auto err = std::ostringstream{};
  auto const args = std::vector<std::string_view>{};

  auto const exit_code = based::run(input, "main", args, out, err);

  CHECK(exit_code == 1);
  CHECK(out.str().empty());
  CHECK(err.str() == "invalid UTF-8 byte sequence\n");
}

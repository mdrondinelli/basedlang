#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "streams/char_input_stream_reader.h"
#include "streams/istream_binary_input_stream.h"
#include "streams/utf8_char_input_stream.h"

TEST_CASE("Char_input_stream_reader - sequential buffered reads")
{
  auto ss = std::istringstream{std::string(1100, 'a') + std::string(1100, 'b')};
  auto binary = benson::Istream_binary_input_stream{&ss};
  auto chars = benson::Utf8_char_input_stream{&binary};
  auto reader = benson::Char_input_stream_reader{&chars};
  auto codepoints = std::vector<uint32_t>{};

  for (;;)
  {
    auto const c = reader.read();
    if (!c)
    {
      break;
    }
    codepoints.push_back(*c);
  }

  REQUIRE(codepoints.size() == 2200);
  REQUIRE(
    std::all_of(
      codepoints.begin(),
      codepoints.begin() + 1100,
      [](uint32_t c)
      {
        return c == 'a';
      }
    )
  );
  REQUIRE(
    std::all_of(
      codepoints.begin() + 1100,
      codepoints.end(),
      [](uint32_t c)
      {
        return c == 'b';
      }
    )
  );
  REQUIRE(!reader.read());
}

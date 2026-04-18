#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "streams/istream_binary_stream.h"
#include "streams/utf8_char_stream.h"

TEST_CASE("Utf8_char_stream - valid sequences")
{
  auto const with_stream = [](std::string const &bytes, auto &&fn)
  {
    auto ss = std::istringstream{bytes};
    auto binary = benson::Istream_binary_stream{&ss};
    auto chars = benson::Utf8_char_stream{&binary};
    fn(chars);
  };
  SECTION("empty stream returns nullopt")
  {
    with_stream(
      "",
      [](auto &chars)
      {
        REQUIRE(!chars.read_character());
      }
    );
  }
  SECTION("ASCII characters")
  {
    with_stream(
      "Hi!",
      [](auto &chars)
      {
        REQUIRE(chars.read_character() == 'H');
        REQUIRE(chars.read_character() == 'i');
        REQUIRE(chars.read_character() == '!');
        REQUIRE(!chars.read_character());
      }
    );
  }
  SECTION("2-byte sequence (U+00E9 \xC3\xA9)")
  {
    with_stream(
      "\xC3\xA9",
      [](auto &chars)
      {
        REQUIRE(chars.read_character() == 0x00E9u);
        REQUIRE(!chars.read_character());
      }
    );
  }
  SECTION("3-byte sequence (U+4E2D \xE4\xB8\xAD)")
  {
    with_stream(
      "\xE4\xB8\xAD",
      [](auto &chars)
      {
        REQUIRE(chars.read_character() == 0x4E2Du);
        REQUIRE(!chars.read_character());
      }
    );
  }
  SECTION("4-byte sequence (U+1F600 \xF0\x9F\x98\x80)")
  {
    with_stream(
      "\xF0\x9F\x98\x80",
      [](auto &chars)
      {
        REQUIRE(chars.read_character() == 0x1F600u);
        REQUIRE(!chars.read_character());
      }
    );
  }
  SECTION("mixed widths")
  {
    with_stream(
      "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80Z",
      [](auto &chars)
      {
        REQUIRE(chars.read_character() == 'A');
        REQUIRE(chars.read_character() == 0x00E9u);
        REQUIRE(chars.read_character() == 0x4E2Du);
        REQUIRE(chars.read_character() == 0x1F600u);
        REQUIRE(chars.read_character() == 'Z');
        REQUIRE(!chars.read_character());
      }
    );
  }
}

TEST_CASE("Utf8_char_stream - invalid sequences throw Decode_error")
{
  auto const throws = [](std::string const &bytes)
  {
    auto ss = std::istringstream{bytes};
    auto binary = benson::Istream_binary_stream{&ss};
    auto chars = benson::Utf8_char_stream{&binary};
    REQUIRE_THROWS_AS(
      chars.read_character(),
      benson::Utf8_char_stream::Decode_error
    );
  };
  SECTION("continuation byte at start")
  {
    throws("\x80");
  }
  SECTION("invalid start byte")
  {
    throws("\xFF");
  }
  SECTION("2-byte: non-continuation second byte")
  {
    throws("\xC3\x41");
  }
  SECTION("2-byte: truncated at EOF")
  {
    throws("\xC3");
  }
  SECTION("3-byte: non-continuation second byte")
  {
    throws("\xE4\x41");
  }
  SECTION("3-byte: non-continuation third byte")
  {
    throws("\xE4\xB8\x41");
  }
  SECTION("3-byte: truncated after first byte")
  {
    throws("\xE4");
  }
  SECTION("3-byte: truncated after second byte")
  {
    throws("\xE4\xB8");
  }
  SECTION("4-byte: non-continuation fourth byte")
  {
    throws("\xF0\x9F\x98\x41");
  }
  SECTION("4-byte: truncated after third byte")
  {
    throws("\xF0\x9F\x98");
  }
}

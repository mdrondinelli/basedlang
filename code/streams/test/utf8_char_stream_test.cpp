#include <array>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "streams/binary_stream.h"
#include "streams/istream_binary_stream.h"
#include "streams/utf8_char_stream.h"

TEST_CASE("Istream_binary_stream - read_bytes")
{
  SECTION("empty stream returns 0")
  {
    auto ss = std::istringstream{""};
    auto binary = benson::Istream_binary_stream{&ss};
    auto buffer = std::array<uint8_t, 4>{};
    REQUIRE(binary.read_bytes(buffer) == 0);
  }
  SECTION("fills full buffer when enough bytes exist")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_stream{&ss};
    auto buffer = std::array<uint8_t, 4>{};
    REQUIRE(binary.read_bytes(buffer) == 4);
    CHECK(buffer[0] == 'h');
    CHECK(buffer[1] == 'e');
    CHECK(buffer[2] == 'l');
    CHECK(buffer[3] == 'l');
  }
  SECTION("short final read returns remaining bytes")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_stream{&ss};
    auto buffer = std::array<uint8_t, 8>{};
    REQUIRE(binary.read_bytes(buffer) == 5);
    CHECK(buffer[0] == 'h');
    CHECK(buffer[1] == 'e');
    CHECK(buffer[2] == 'l');
    CHECK(buffer[3] == 'l');
    CHECK(buffer[4] == 'o');
    REQUIRE(binary.read_bytes(buffer) == 0);
  }
  SECTION("repeated reads reconstruct source")
  {
    auto ss = std::istringstream{"abcdefg"};
    auto binary = benson::Istream_binary_stream{&ss};
    auto buffer = std::array<uint8_t, 3>{};
    auto bytes = std::vector<uint8_t>{};
    for (;;)
    {
      auto const count = binary.read_bytes(buffer);
      if (count == 0)
      {
        break;
      }
      bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + count);
    }
    REQUIRE(bytes == std::vector<uint8_t>{'a', 'b', 'c', 'd', 'e', 'f', 'g'});
  }
}

TEST_CASE("Binary_stream - read_byte compatibility helper")
{
  class Chunked_binary_stream: public benson::Binary_stream
  {
  public:
    std::ptrdiff_t read_bytes(std::span<uint8_t> buffer) override
    {
      if (_offset == static_cast<std::ptrdiff_t>(_bytes.size()) ||
          buffer.empty())
      {
        return 0;
      }
      auto const count =
        std::min(static_cast<std::ptrdiff_t>(buffer.size()), std::ptrdiff_t{2});
      auto const available =
        static_cast<std::ptrdiff_t>(_bytes.size()) - _offset;
      auto const actual = std::min(count, available);
      for (auto i = std::ptrdiff_t{0}; i < actual; ++i)
      {
        buffer[i] = _bytes[_offset + i];
      }
      _offset += actual;
      return actual;
    }

  private:
    std::array<uint8_t, 4> _bytes{'x', 'y', 'z', '!'};
    std::ptrdiff_t _offset{};
  };

  auto binary = Chunked_binary_stream{};
  REQUIRE(binary.read_byte() == 'x');
  REQUIRE(binary.read_byte() == 'y');
  REQUIRE(binary.read_byte() == 'z');
  REQUIRE(binary.read_byte() == '!');
  REQUIRE(!binary.read_byte());
}

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
  SECTION("boundary scalar values")
  {
    with_stream(
      "\xC2\x80\xE0\xA0\x80\xF0\x90\x80\x80\xF4\x8F\xBF\xBF",
      [](auto &chars)
      {
        REQUIRE(chars.read_character() == 0x0080u);
        REQUIRE(chars.read_character() == 0x0800u);
        REQUIRE(chars.read_character() == 0x10000u);
        REQUIRE(chars.read_character() == 0x10FFFFu);
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
  SECTION("2-byte: overlong encoding (C0)")
  {
    throws("\xC0\x80");
  }
  SECTION("2-byte: overlong encoding (C1)")
  {
    throws("\xC1\x80");
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
  SECTION("3-byte: overlong encoding")
  {
    throws("\xE0\x80\x80");
  }
  SECTION("3-byte: surrogate codepoint")
  {
    throws("\xED\xA0\x80");
  }
  SECTION("4-byte: non-continuation fourth byte")
  {
    throws("\xF0\x9F\x98\x41");
  }
  SECTION("4-byte: truncated after third byte")
  {
    throws("\xF0\x9F\x98");
  }
  SECTION("4-byte: overlong encoding")
  {
    throws("\xF0\x80\x80\x80");
  }
  SECTION("4-byte: above Unicode range")
  {
    throws("\xF4\x90\x80\x80");
  }
}

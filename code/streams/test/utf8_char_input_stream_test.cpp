#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "streams/char_input_stream.h"
#include "streams/istream_input_stream.h"
#include "streams/utf8_char_input_stream.h"

TEST_CASE("Utf8_char_input_stream - read_characters")
{
  auto const with_stream = [](std::string const &bytes, auto &&fn)
  {
    auto ss = std::istringstream{bytes};
    auto binary = benson::Istream_input_stream{&ss};
    auto chars = benson::Utf8_char_input_stream{&binary};
    fn(chars);
  };
  SECTION("empty stream returns 0")
  {
    with_stream(
      "",
      [](auto &chars)
      {
        auto buffer = std::array<uint32_t, 4>{};
        REQUIRE(chars.read_characters(buffer) == 0);
      }
    );
  }
  SECTION("empty buffer is no-op")
  {
    with_stream(
      "A\xC3\xA9",
      [](auto &chars)
      {
        auto empty = std::span<uint32_t>{};
        auto buffer = std::array<uint32_t, 2>{};
        REQUIRE(chars.read_characters(empty) == 0);
        REQUIRE(chars.read_characters(buffer) == 2);
        CHECK(buffer[0] == 'A');
        CHECK(buffer[1] == 0x00E9u);
      }
    );
  }
  SECTION("fills full buffer when enough codepoints exist")
  {
    with_stream(
      "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80Z",
      [](auto &chars)
      {
        auto buffer = std::array<uint32_t, 5>{};
        REQUIRE(chars.read_characters(buffer) == 5);
        CHECK(buffer[0] == 'A');
        CHECK(buffer[1] == 0x00E9u);
        CHECK(buffer[2] == 0x4E2Du);
        CHECK(buffer[3] == 0x1F600u);
        CHECK(buffer[4] == 'Z');
      }
    );
  }
  SECTION("short final read returns remaining codepoints")
  {
    with_stream(
      "Hi!",
      [](auto &chars)
      {
        auto buffer = std::array<uint32_t, 8>{};
        REQUIRE(chars.read_characters(buffer) == 3);
        CHECK(buffer[0] == 'H');
        CHECK(buffer[1] == 'i');
        CHECK(buffer[2] == '!');
        REQUIRE(chars.read_characters(buffer) == 0);
      }
    );
  }
  SECTION("repeated reads reconstruct source")
  {
    with_stream(
      "A\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80Z",
      [](auto &chars)
      {
        auto buffer = std::array<uint32_t, 2>{};
        auto codepoints = std::vector<uint32_t>{};
        for (;;)
        {
          auto const count = chars.read_characters(buffer);
          if (count == 0)
          {
            break;
          }
          codepoints
            .insert(codepoints.end(), buffer.begin(), buffer.begin() + count);
        }
        REQUIRE(
          codepoints ==
          std::vector<uint32_t>{'A', 0x00E9u, 0x4E2Du, 0x1F600u, 'Z'}
        );
      }
    );
  }
  SECTION("invalid sequence mid-buffer throws Decode_error")
  {
    with_stream(
      "A\xFF",
      [](auto &chars)
      {
        auto buffer = std::array<uint32_t, 4>{};
        REQUIRE_THROWS_AS(
          chars.read_characters(buffer),
          benson::Utf8_char_input_stream::Decode_error
        );
      }
    );
  }
}

TEST_CASE("Char_input_stream - read_character compatibility helper")
{
  class Chunked_char_input_stream: public benson::Char_input_stream
  {
  public:
    std::ptrdiff_t read_characters(std::span<uint32_t> buffer) override
    {
      if (_offset == static_cast<std::ptrdiff_t>(_codepoints.size()) ||
          buffer.empty())
      {
        return 0;
      }
      auto const count =
        std::min(static_cast<std::ptrdiff_t>(buffer.size()), std::ptrdiff_t{2});
      auto const available =
        static_cast<std::ptrdiff_t>(_codepoints.size()) - _offset;
      auto const actual = std::min(count, available);
      for (auto i = std::ptrdiff_t{0}; i < actual; ++i)
      {
        buffer[i] = _codepoints[_offset + i];
      }
      _offset += actual;
      return actual;
    }

  private:
    std::array<uint32_t, 4> _codepoints{'w', 0x00E9u, 0x4E2Du, 0x1F600u};
    std::ptrdiff_t _offset{};
  };

  auto chars = Chunked_char_input_stream{};
  REQUIRE(chars.read_character() == 'w');
  REQUIRE(chars.read_character() == 0x00E9u);
  REQUIRE(chars.read_character() == 0x4E2Du);
  REQUIRE(chars.read_character() == 0x1F600u);
  REQUIRE(!chars.read_character());
}

TEST_CASE("Utf8_char_input_stream - valid sequences")
{
  auto const with_stream = [](std::string const &bytes, auto &&fn)
  {
    auto ss = std::istringstream{bytes};
    auto binary = benson::Istream_input_stream{&ss};
    auto chars = benson::Utf8_char_input_stream{&binary};
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

TEST_CASE("Utf8_char_input_stream - invalid sequences throw Decode_error")
{
  auto const throws = [](std::string const &bytes)
  {
    auto ss = std::istringstream{bytes};
    auto binary = benson::Istream_input_stream{&ss};
    auto chars = benson::Utf8_char_input_stream{&binary};
    REQUIRE_THROWS_AS(
      chars.read_character(),
      benson::Utf8_char_input_stream::Decode_error
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

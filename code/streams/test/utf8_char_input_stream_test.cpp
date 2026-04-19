#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

#include <catch2/catch_test_macros.hpp>

#include "streams/binary_input_stream.h"
#include "streams/char_input_stream.h"
#include "streams/char_input_stream_reader.h"
#include "streams/istream_binary_input_stream.h"
#include "streams/lookahead_char_input_stream_reader.h"
#include "streams/posix_binary_input_stream.h"
#include "streams/utf8_char_input_stream.h"

TEST_CASE("Istream_binary_input_stream - read_bytes")
{
  SECTION("empty stream returns 0")
  {
    auto ss = std::istringstream{""};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto buffer = std::array<uint8_t, 4>{};
    REQUIRE(binary.read_bytes(buffer) == 0);
  }
  SECTION("empty buffer is no-op")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto empty = std::span<uint8_t>{};
    auto buffer = std::array<uint8_t, 5>{};
    REQUIRE(binary.read_bytes(empty) == 0);
    REQUIRE(binary.read_bytes(buffer) == 5);
    CHECK(buffer[0] == 'h');
    CHECK(buffer[1] == 'e');
    CHECK(buffer[2] == 'l');
    CHECK(buffer[3] == 'l');
    CHECK(buffer[4] == 'o');
  }
  SECTION("fills full buffer when enough bytes exist")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_input_stream{&ss};
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
    auto binary = benson::Istream_binary_input_stream{&ss};
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
    auto binary = benson::Istream_binary_input_stream{&ss};
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

TEST_CASE("Binary_input_stream - read_byte compatibility helper")
{
  class Chunked_binary_input_stream: public benson::Binary_input_stream
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

  auto binary = Chunked_binary_input_stream{};
  REQUIRE(binary.read_byte() == 'x');
  REQUIRE(binary.read_byte() == 'y');
  REQUIRE(binary.read_byte() == 'z');
  REQUIRE(binary.read_byte() == '!');
  REQUIRE(!binary.read_byte());
}

TEST_CASE("Utf8_char_input_stream - read_characters")
{
  auto const with_stream = [](std::string const &bytes, auto &&fn)
  {
    auto ss = std::istringstream{bytes};
    auto binary = benson::Istream_binary_input_stream{&ss};
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

TEST_CASE("Lookahead_char_input_stream_reader - peek and read")
{
  auto ss = std::istringstream{"abcdef"};
  auto binary = benson::Istream_binary_input_stream{&ss};
  auto chars = benson::Utf8_char_input_stream{&binary};
  auto reader = benson::Lookahead_char_input_stream_reader{&chars, 3};

  REQUIRE(reader.peek(0) == 'a');
  REQUIRE(reader.peek(1) == 'b');
  REQUIRE(reader.peek(2) == 'c');
  REQUIRE(reader.peek(3) == 'd');
  REQUIRE(reader.peek(3) == 'd');
  REQUIRE(reader.read() == 'a');
  REQUIRE(reader.peek(0) == 'b');
  REQUIRE(reader.read() == 'b');
  REQUIRE(reader.read() == 'c');
  REQUIRE(reader.peek(2) == 'f');
  REQUIRE(reader.read() == 'd');
  REQUIRE(reader.read() == 'e');
  REQUIRE(reader.read() == 'f');
  REQUIRE(!reader.peek());
  REQUIRE(!reader.read());
}

TEST_CASE("Lookahead_char_input_stream_reader - wraparound")
{
  auto ss = std::istringstream{"abcdefghi"};
  auto binary = benson::Istream_binary_input_stream{&ss};
  auto chars = benson::Utf8_char_input_stream{&binary};
  auto reader = benson::Lookahead_char_input_stream_reader{&chars, 3};

  REQUIRE(reader.peek(3) == 'd');
  REQUIRE(reader.read() == 'a');
  REQUIRE(reader.read() == 'b');
  REQUIRE(reader.peek(3) == 'f');
  REQUIRE(reader.read() == 'c');
  REQUIRE(reader.peek(3) == 'g');
  REQUIRE(reader.read() == 'd');
  REQUIRE(reader.read() == 'e');
  REQUIRE(reader.peek(3) == 'i');
  REQUIRE(reader.read() == 'f');
  REQUIRE(reader.read() == 'g');
  REQUIRE(reader.peek(1) == 'i');
  REQUIRE(reader.read() == 'h');
  REQUIRE(reader.read() == 'i');
  REQUIRE(!reader.peek());
}

TEST_CASE("Lookahead_char_input_stream_reader - non-power-of-two lookahead")
{
  auto ss = std::istringstream{"abcdefg"};
  auto binary = benson::Istream_binary_input_stream{&ss};
  auto chars = benson::Utf8_char_input_stream{&binary};
  auto reader = benson::Lookahead_char_input_stream_reader{&chars, 2};

  REQUIRE(reader.peek(2) == 'c');
  REQUIRE(reader.read() == 'a');
  REQUIRE(reader.read() == 'b');
  REQUIRE(reader.peek(2) == 'e');
  REQUIRE(reader.read() == 'c');
  REQUIRE(reader.read() == 'd');
  REQUIRE(reader.peek(2) == 'g');
  REQUIRE(reader.read() == 'e');
  REQUIRE(reader.read() == 'f');
  REQUIRE(reader.read() == 'g');
  REQUIRE(!reader.peek());
  REQUIRE(!reader.read());
}

TEST_CASE("Utf8_char_input_stream - valid sequences")
{
  auto const with_stream = [](std::string const &bytes, auto &&fn)
  {
    auto ss = std::istringstream{bytes};
    auto binary = benson::Istream_binary_input_stream{&ss};
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
    auto binary = benson::Istream_binary_input_stream{&ss};
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

TEST_CASE("Posix_binary_input_stream - read_bytes")
{
  auto const make_pipe = []()
  {
    auto fds = std::array<int, 2>{};
    REQUIRE(::pipe(fds.data()) == 0);
    return fds;
  };
  SECTION("empty pipe returns 0")
  {
    auto const fds = make_pipe();
    ::close(fds[1]);
    auto posix = benson::Posix_binary_input_stream{fds[0]};
    auto buffer = std::array<uint8_t, 4>{};
    REQUIRE(posix.read_bytes(buffer) == 0);
    ::close(fds[0]);
  }
  SECTION("empty buffer is no-op")
  {
    auto const fds = make_pipe();
    auto const data = std::string{"hello"};
    ::write(fds[1], data.data(), data.size());
    ::close(fds[1]);
    auto posix = benson::Posix_binary_input_stream{fds[0]};
    auto empty = std::span<uint8_t>{};
    auto buffer = std::array<uint8_t, 5>{};
    REQUIRE(posix.read_bytes(empty) == 0);
    REQUIRE(posix.read_bytes(buffer) == 5);
    CHECK(buffer[0] == 'h');
    CHECK(buffer[1] == 'e');
    CHECK(buffer[2] == 'l');
    CHECK(buffer[3] == 'l');
    CHECK(buffer[4] == 'o');
    ::close(fds[0]);
  }
  SECTION("fills full buffer when enough bytes exist")
  {
    auto const fds = make_pipe();
    auto const data = std::string{"hello"};
    ::write(fds[1], data.data(), data.size());
    ::close(fds[1]);
    auto posix = benson::Posix_binary_input_stream{fds[0]};
    auto buffer = std::array<uint8_t, 4>{};
    REQUIRE(posix.read_bytes(buffer) == 4);
    CHECK(buffer[0] == 'h');
    CHECK(buffer[1] == 'e');
    CHECK(buffer[2] == 'l');
    CHECK(buffer[3] == 'l');
    ::close(fds[0]);
  }
  SECTION("short final read returns remaining bytes")
  {
    auto const fds = make_pipe();
    auto const data = std::string{"hello"};
    ::write(fds[1], data.data(), data.size());
    ::close(fds[1]);
    auto posix = benson::Posix_binary_input_stream{fds[0]};
    auto buffer = std::array<uint8_t, 8>{};
    REQUIRE(posix.read_bytes(buffer) == 5);
    CHECK(buffer[0] == 'h');
    CHECK(buffer[1] == 'e');
    CHECK(buffer[2] == 'l');
    CHECK(buffer[3] == 'l');
    CHECK(buffer[4] == 'o');
    ::close(fds[0]);
  }
  SECTION("repeated reads reconstruct source")
  {
    auto const fds = make_pipe();
    auto const data = std::string{"hello"};
    ::write(fds[1], data.data(), data.size());
    ::close(fds[1]);
    auto posix = benson::Posix_binary_input_stream{fds[0]};
    auto result = std::string{};
    auto buffer = std::array<uint8_t, 2>{};
    for (;;)
    {
      auto const n = posix.read_bytes(buffer);
      if (n == 0)
      {
        break;
      }
      result.append(
        reinterpret_cast<char const *>(buffer.data()),
        static_cast<std::size_t>(n)
      );
    }
    CHECK(result == "hello");
    ::close(fds[0]);
  }
}

TEST_CASE("Posix_binary_input_stream - read_byte")
{
  auto fds = std::array<int, 2>{};
  REQUIRE(::pipe(fds.data()) == 0);
  auto const data = std::string{"hello"};
  ::write(fds[1], data.data(), data.size());
  ::close(fds[1]);
  auto posix = benson::Posix_binary_input_stream{fds[0]};
  auto result = std::string{};
  for (;;)
  {
    auto const byte = posix.read_byte();
    if (!byte)
    {
      break;
    }
    result.push_back(static_cast<char>(*byte));
  }
  CHECK(result == "hello");
  ::close(fds[0]);
}

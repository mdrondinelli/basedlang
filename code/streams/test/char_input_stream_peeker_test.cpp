#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "streams/char_input_stream_peeker.h"
#include "streams/istream_input_stream.h"
#include "streams/utf8_char_input_stream.h"

TEST_CASE("Char_input_stream_peeker - peek and read")
{
  auto ss = std::istringstream{"abcdef"};
  auto binary = benson::Istream_input_stream{&ss};
  auto chars = benson::Utf8_char_input_stream{&binary};
  auto reader = benson::Char_input_stream_peeker{&chars, 3};

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

TEST_CASE("Char_input_stream_peeker - wraparound")
{
  auto ss = std::istringstream{"abcdefghi"};
  auto binary = benson::Istream_input_stream{&ss};
  auto chars = benson::Utf8_char_input_stream{&binary};
  auto reader = benson::Char_input_stream_peeker{&chars, 3};

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

TEST_CASE("Char_input_stream_peeker - non-power-of-two lookahead")
{
  auto ss = std::istringstream{"abcdefg"};
  auto binary = benson::Istream_input_stream{&ss};
  auto chars = benson::Utf8_char_input_stream{&binary};
  auto reader = benson::Char_input_stream_peeker{&chars, 2};

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

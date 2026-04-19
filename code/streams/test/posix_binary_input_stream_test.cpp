#include <array>
#include <string>

#if defined(__unix__) || defined(__APPLE__)
  #include <unistd.h>
#endif

#include <catch2/catch_test_macros.hpp>

#if defined(__unix__) || defined(__APPLE__)
  #include "streams/posix_binary_input_stream.h"
#endif

#if defined(__unix__) || defined(__APPLE__)
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
    REQUIRE(
      ::write(fds[1], data.data(), data.size()) ==
      static_cast<ssize_t>(data.size())
    );
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
    REQUIRE(
      ::write(fds[1], data.data(), data.size()) ==
      static_cast<ssize_t>(data.size())
    );
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
    REQUIRE(
      ::write(fds[1], data.data(), data.size()) ==
      static_cast<ssize_t>(data.size())
    );
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
    REQUIRE(
      ::write(fds[1], data.data(), data.size()) ==
      static_cast<ssize_t>(data.size())
    );
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
  REQUIRE(
    ::write(fds[1], data.data(), data.size()) ==
    static_cast<ssize_t>(data.size())
  );
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
#endif

#include <array>
#include <cstddef>
#include <string>
#include <system_error>

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
    auto buffer = std::array<std::byte, 4>{};
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
    auto empty = std::span<std::byte>{};
    auto buffer = std::array<std::byte, 5>{};
    REQUIRE(posix.read_bytes(empty) == 0);
    REQUIRE(posix.read_bytes(buffer) == 5);
    CHECK(buffer[0] == std::byte{'h'});
    CHECK(buffer[1] == std::byte{'e'});
    CHECK(buffer[2] == std::byte{'l'});
    CHECK(buffer[3] == std::byte{'l'});
    CHECK(buffer[4] == std::byte{'o'});
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
    auto buffer = std::array<std::byte, 4>{};
    REQUIRE(posix.read_bytes(buffer) == 4);
    CHECK(buffer[0] == std::byte{'h'});
    CHECK(buffer[1] == std::byte{'e'});
    CHECK(buffer[2] == std::byte{'l'});
    CHECK(buffer[3] == std::byte{'l'});
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
    auto buffer = std::array<std::byte, 8>{};
    REQUIRE(posix.read_bytes(buffer) == 5);
    CHECK(buffer[0] == std::byte{'h'});
    CHECK(buffer[1] == std::byte{'e'});
    CHECK(buffer[2] == std::byte{'l'});
    CHECK(buffer[3] == std::byte{'l'});
    CHECK(buffer[4] == std::byte{'o'});
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
    auto buffer = std::array<std::byte, 2>{};
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

TEST_CASE("Posix_binary_input_stream - read failure preserves errno")
{
  auto posix = benson::Posix_binary_input_stream{-1};
  auto buffer = std::array<std::byte, 1>{};

  try
  {
    static_cast<void>(posix.read_bytes(buffer));
    FAIL("expected std::system_error");
  }
  catch (std::system_error const &e)
  {
    CHECK(e.code().category() == std::system_category());
    CHECK(e.code().value() == EBADF);
  }
}
#endif

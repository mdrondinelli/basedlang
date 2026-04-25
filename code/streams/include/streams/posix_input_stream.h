#ifndef BASEDSTREAMS_POSIX_INPUT_STREAM_H
#define BASEDSTREAMS_POSIX_INPUT_STREAM_H

#include <cerrno>
#include <cstddef>
#include <span>
#include <system_error>

#if defined(__unix__) || defined(__APPLE__)
  #include <unistd.h>
#endif

#include "streams/input_stream.h"

namespace benson
{

#if defined(__unix__) || defined(__APPLE__)
  class Posix_input_stream: public Input_stream
  {
  public:
    explicit Posix_input_stream(int fd) noexcept
        : _fd{fd}
    {
    }

    std::ptrdiff_t read_bytes(std::span<std::byte> buffer) override
    {
      if (buffer.empty())
      {
        return 0;
      }
      for (;;)
      {
        auto const result = ::read(_fd, buffer.data(), buffer.size());
        if (result >= 0)
        {
          return static_cast<std::ptrdiff_t>(result);
        }
        if (errno != EINTR)
        {
          auto const error = errno;
          throw std::system_error{
            error,
            std::system_category(),
            "Posix_input_stream: read failed"
          };
        }
      }
    }

  private:
    int _fd;
  };
#endif

} // namespace benson

#endif // BASEDSTREAMS_POSIX_INPUT_STREAM_H

#ifndef BASEDLEX_TEST_ISTREAM_BINARY_STREAM_H
#define BASEDLEX_TEST_ISTREAM_BINARY_STREAM_H

#include <cstdint>
#include <istream>
#include <optional>

#include "basedlex/binary_stream.h"

namespace basedlex
{

  class Istream_binary_stream: public Binary_stream
  {
  public:
    explicit Istream_binary_stream(std::istream *stream) noexcept;

    std::optional<uint8_t> read_byte() override;

  private:
    std::istream *_stream;
  };

} // namespace basedlex

#endif // BASEDLEX_TEST_ISTREAM_BINARY_STREAM_H

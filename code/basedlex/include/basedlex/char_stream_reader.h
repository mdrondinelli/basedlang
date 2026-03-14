#ifndef BASEDLEX_CHAR_STREAM_READER_H
#define BASEDLEX_CHAR_STREAM_READER_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "char_stream.h"

namespace basedlex
{

  /// Wraps a Char_stream with arbitrary lookahead, exposing peek/read
  /// operations.
  class Char_stream_reader
  {
  public:
    explicit Char_stream_reader(Char_stream *stream) noexcept;

    std::optional<uint32_t> peek(std::ptrdiff_t offset = 0);

    std::optional<uint32_t> read();

  private:
    void buffer_to(std::size_t count);

    Char_stream *_stream;
    std::vector<uint32_t> _buffer;
  };

} // namespace basedlex

#endif // BASEDLEX_CHAR_STREAM_READER_H

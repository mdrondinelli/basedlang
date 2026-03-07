#ifndef BASEDLEX_CHAR_STREAM_READER_H
#define BASEDLEX_CHAR_STREAM_READER_H

#include <cstdint>
#include <optional>

#include "char_stream.h"

namespace basedlex
{

  class Char_stream_reader
  {
  public:
    explicit Char_stream_reader(Char_stream *stream) noexcept;

    std::optional<uint32_t> peek() const noexcept;
    std::optional<uint32_t> read();

  private:
    Char_stream *_stream;
    std::optional<uint32_t> _next;
  };

} // namespace basedlex

#endif // BASEDLEX_CHAR_STREAM_READER_H

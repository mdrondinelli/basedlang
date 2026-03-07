#include "basedlex/istream_binary_stream.h"

namespace basedlex
{

  Istream_binary_stream::Istream_binary_stream(std::istream *stream) noexcept
      : _stream{stream}
  {
  }

  std::optional<uint8_t> Istream_binary_stream::read_byte()
  {
    auto const c = _stream->get();
    if (c == std::istream::traits_type::eof())
    {
      return std::nullopt;
    }
    return static_cast<uint8_t>(c);
  }

} // namespace basedlex

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "basedinterp/interpreter.h"
#include "basedir/compiler.h"
#include "basedlex/istream_binary_stream.h"
#include "basedlex/lexeme_stream.h"
#include "basedlex/lexeme_stream_reader.h"
#include "basedlex/utf8_char_stream.h"
#include "basedparse/parser.h"

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "usage: basedi <file>\n";
    return 1;
  }
  auto file = std::ifstream{argv[1]};
  if (!file)
  {
    std::cerr << "error: cannot open " << argv[1] << "\n";
    return 1;
  }
  auto binary_stream = basedlex::Istream_binary_stream{&file};
  auto char_stream = basedlex::Utf8_char_stream{&binary_stream};
  auto lexeme_stream = basedlex::Lexeme_stream{&char_stream};
  auto reader = basedlex::Lexeme_stream_reader{&lexeme_stream};
  auto parser = basedparse::Parser{&reader};
  auto const unit = parser.parse_translation_unit();
  auto compiler = basedir::Compiler{};
  auto const program = compiler.compile(*unit);
  auto interpreter = basedinterp::Interpreter{&program};
  auto const result = interpreter.call("main", {});
  auto const exit_code = std::get_if<std::int32_t>(&result.data);
  if (!exit_code)
  {
    std::cerr << "error: main did not return an integer\n";
    return 1;
  }
  return static_cast<int>(*exit_code);
}

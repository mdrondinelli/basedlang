#ifndef BENSON_BYTECODE_BYTECODE_WRITER_H
#define BENSON_BYTECODE_BYTECODE_WRITER_H

#include "bytecode/opcode.h"
#include "bytecode/register.h"
#include "streams/binary_output_stream.h"
#include "streams/binary_output_stream_writer.h"

namespace benson::bytecode
{

  class Bytecode_writer
  {
  public:
    explicit Bytecode_writer(Binary_output_stream *stream);

    void emit_nop();
    void emit_exit();
    void emit_neg_i8(Register dst, Register src);
    void emit_neg_i16(Register dst, Register src);
    void emit_neg_i32(Register dst, Register src);
    void emit_neg_i64(Register dst, Register src);
    void emit_neg_f32(Register dst, Register src);
    void emit_neg_f64(Register dst, Register src);

    void flush();

  private:
    void emit_opcode(Opcode opcode);
    void emit_unary_register_instruction(
      Opcode opcode,
      Register dst,
      Register src
    );

    Binary_output_stream_writer _writer;
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_BYTECODE_WRITER_H

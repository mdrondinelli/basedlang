#include "bytecode/bytecode_writer.h"

namespace benson::bytecode
{

  Bytecode_writer::Bytecode_writer(Binary_output_stream *stream)
      : _writer{stream}
  {
  }

  void Bytecode_writer::emit_nop()
  {
    emit_opcode(Opcode::nop);
  }

  void Bytecode_writer::emit_exit()
  {
    emit_opcode(Opcode::exit);
  }

  void Bytecode_writer::emit_neg_i8(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::neg_i8, dst, src);
  }

  void Bytecode_writer::emit_neg_i16(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::neg_i16, dst, src);
  }

  void Bytecode_writer::emit_neg_i32(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::neg_i32, dst, src);
  }

  void Bytecode_writer::emit_neg_i64(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::neg_i64, dst, src);
  }

  void Bytecode_writer::emit_neg_f32(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::neg_f32, dst, src);
  }

  void Bytecode_writer::emit_neg_f64(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::neg_f64, dst, src);
  }

  void Bytecode_writer::flush()
  {
    _writer.flush();
  }

  void Bytecode_writer::emit_opcode(Opcode opcode)
  {
    _writer.write(static_cast<std::byte>(opcode));
  }

  void Bytecode_writer::emit_unary_register_instruction(
    Opcode opcode,
    Register dst,
    Register src
  )
  {
    emit_opcode(opcode);
    _writer.write(static_cast<std::byte>(dst));
    _writer.write(static_cast<std::byte>(src));
  }

} // namespace benson::bytecode

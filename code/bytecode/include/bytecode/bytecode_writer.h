#ifndef BENSON_BYTECODE_BYTECODE_WRITER_H
#define BENSON_BYTECODE_BYTECODE_WRITER_H

#include "bytecode/constant.h"
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

    void emit_exit();
    void emit_lookup_k(Register dst, Wide_constant k);
    void emit_neg_i8(Register dst, Register src);
    void emit_neg_i16(Register dst, Register src);
    void emit_neg_i32(Register dst, Register src);
    void emit_neg_i64(Register dst, Register src);
    void emit_neg_f32(Register dst, Register src);
    void emit_neg_f64(Register dst, Register src);
    void emit_add_i8(Register dst, Register lhs, Register rhs);
    void emit_add_i8_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_add_i16(Register dst, Register lhs, Register rhs);
    void emit_add_i16_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_add_i32(Register dst, Register lhs, Register rhs);
    void emit_add_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_add_i64(Register dst, Register lhs, Register rhs);
    void emit_add_i64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_add_f32(Register dst, Register lhs, Register rhs);
    void emit_add_f32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_add_f64(Register dst, Register lhs, Register rhs);
    void emit_add_f64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_i8(Register dst, Register lhs, Register rhs);
    void emit_sub_i8_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_i16(Register dst, Register lhs, Register rhs);
    void emit_sub_i16_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_i32(Register dst, Register lhs, Register rhs);
    void emit_sub_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_i64(Register dst, Register lhs, Register rhs);
    void emit_sub_i64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_f32(Register dst, Register lhs, Register rhs);
    void emit_sub_f32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_f64(Register dst, Register lhs, Register rhs);
    void emit_sub_f64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_i8(Register dst, Register lhs, Register rhs);
    void emit_mul_i8_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_i16(Register dst, Register lhs, Register rhs);
    void emit_mul_i16_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_i32(Register dst, Register lhs, Register rhs);
    void emit_mul_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_i64(Register dst, Register lhs, Register rhs);
    void emit_mul_i64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_f32(Register dst, Register lhs, Register rhs);
    void emit_mul_f32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_f64(Register dst, Register lhs, Register rhs);
    void emit_mul_f64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_i8(Register dst, Register lhs, Register rhs);
    void emit_div_i8_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_i16(Register dst, Register lhs, Register rhs);
    void emit_div_i16_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_i32(Register dst, Register lhs, Register rhs);
    void emit_div_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_i64(Register dst, Register lhs, Register rhs);
    void emit_div_i64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_f32(Register dst, Register lhs, Register rhs);
    void emit_div_f32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_f64(Register dst, Register lhs, Register rhs);
    void emit_div_f64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mod_i8(Register dst, Register lhs, Register rhs);
    void emit_mod_i8_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mod_i16(Register dst, Register lhs, Register rhs);
    void emit_mod_i16_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mod_i32(Register dst, Register lhs, Register rhs);
    void emit_mod_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mod_i64(Register dst, Register lhs, Register rhs);
    void emit_mod_i64_k(Register dst, Register lhs, Wide_constant rhs);

    void flush();

  private:
    void emit_opcode(Opcode opcode);

    void emit_unary_register_instruction(
      Opcode opcode,
      Register dst,
      Register src
    );

    void emit_binary_register_instruction(
      Opcode opcode,
      Register dst,
      Register lhs,
      Register rhs
    );

    void emit_binary_constant_instruction(
      Opcode opcode,
      Register dst,
      Register lhs,
      Wide_constant rhs
    );

    Binary_output_stream_writer _writer;
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_BYTECODE_WRITER_H

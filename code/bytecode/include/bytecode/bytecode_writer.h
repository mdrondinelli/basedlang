#ifndef BENSON_BYTECODE_BYTECODE_WRITER_H
#define BENSON_BYTECODE_BYTECODE_WRITER_H

#include "bytecode/constant.h"
#include "bytecode/immediate.h"
#include "bytecode/opcode.h"
#include "bytecode/register.h"
#include "streams/binary_output_stream.h"

namespace benson::bytecode
{

  class Bytecode_writer
  {
  public:
    explicit Bytecode_writer(Binary_output_stream *stream);

    void emit_exit();
    void emit_jmp_i(Wide_immediate offset);
    void emit_lookup_k(Register dst, Wide_constant k);
    void emit_load_8(Register dst, Register base, Wide_immediate offset);
    void emit_load_16(Register dst, Register base, Wide_immediate offset);
    void emit_load_32(Register dst, Register base, Wide_immediate offset);
    void emit_load_64(Register dst, Register base, Wide_immediate offset);
    void emit_store_8(Register src, Register base, Wide_immediate offset);
    void emit_store_16(Register src, Register base, Wide_immediate offset);
    void emit_store_32(Register src, Register base, Wide_immediate offset);
    void emit_store_64(Register src, Register base, Wide_immediate offset);
    void emit_neg_i32(Register dst, Register src);
    void emit_neg_i64(Register dst, Register src);
    void emit_neg_f32(Register dst, Register src);
    void emit_neg_f64(Register dst, Register src);
    void emit_add_i32(Register dst, Register lhs, Register rhs);
    void emit_add_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_add_i32_i(Register dst, Register lhs, Wide_immediate rhs);
    void emit_add_i64(Register dst, Register lhs, Register rhs);
    void emit_add_i64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_add_i64_i(Register dst, Register lhs, Wide_immediate rhs);
    void emit_add_f32(Register dst, Register lhs, Register rhs);
    void emit_add_f32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_add_f64(Register dst, Register lhs, Register rhs);
    void emit_add_f64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_i32(Register dst, Register lhs, Register rhs);
    void emit_sub_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_i32_i(Register dst, Register lhs, Wide_immediate rhs);
    void emit_sub_i64(Register dst, Register lhs, Register rhs);
    void emit_sub_i64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_i64_i(Register dst, Register lhs, Wide_immediate rhs);
    void emit_sub_f32(Register dst, Register lhs, Register rhs);
    void emit_sub_f32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_sub_f64(Register dst, Register lhs, Register rhs);
    void emit_sub_f64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_i32(Register dst, Register lhs, Register rhs);
    void emit_mul_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_i32_i(Register dst, Register lhs, Wide_immediate rhs);
    void emit_mul_i64(Register dst, Register lhs, Register rhs);
    void emit_mul_i64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_i64_i(Register dst, Register lhs, Wide_immediate rhs);
    void emit_mul_f32(Register dst, Register lhs, Register rhs);
    void emit_mul_f32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mul_f64(Register dst, Register lhs, Register rhs);
    void emit_mul_f64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_i32(Register dst, Register lhs, Register rhs);
    void emit_div_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_i32_i(Register dst, Register lhs, Wide_immediate rhs);
    void emit_div_i64(Register dst, Register lhs, Register rhs);
    void emit_div_i64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_i64_i(Register dst, Register lhs, Wide_immediate rhs);
    void emit_div_f32(Register dst, Register lhs, Register rhs);
    void emit_div_f32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_div_f64(Register dst, Register lhs, Register rhs);
    void emit_div_f64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mod_i32(Register dst, Register lhs, Register rhs);
    void emit_mod_i32_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mod_i32_i(Register dst, Register lhs, Wide_immediate rhs);
    void emit_mod_i64(Register dst, Register lhs, Register rhs);
    void emit_mod_i64_k(Register dst, Register lhs, Wide_constant rhs);
    void emit_mod_i64_i(Register dst, Register lhs, Wide_immediate rhs);

    void flush();

  private:
    void write_byte(std::byte byte);

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

    void emit_binary_immediate_instruction(
      Opcode opcode,
      Register dst,
      Register lhs,
      Wide_immediate rhs
    );

    void emit_immediate_instruction(Opcode opcode, Wide_immediate value);

    Binary_output_stream *_stream;
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_BYTECODE_WRITER_H

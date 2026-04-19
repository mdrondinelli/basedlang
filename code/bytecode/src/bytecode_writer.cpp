#include "bytecode/bytecode_writer.h"

namespace benson::bytecode
{

  Bytecode_writer::Bytecode_writer(Binary_output_stream *stream)
      : _writer{stream}
  {
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

  void Bytecode_writer::emit_add_i8(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_i8, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i8_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_add_i8_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::add_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i16(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_i16, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i16_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_add_i16_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::add_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_add_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::add_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_add_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::add_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_add_f32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::add_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_add_f64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::add_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i8(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_i8, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i8_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_sub_i8_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::sub_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i16(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_i16, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i16_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_sub_i16_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::sub_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_sub_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::sub_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_sub_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::sub_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_sub_f32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::sub_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_sub_f64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::sub_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i8(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_i8, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i8_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mul_i8_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mul_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i16(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_i16, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i16_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mul_i16_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mul_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mul_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mul_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mul_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mul_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mul_f32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mul_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mul_f64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mul_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i8(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_i8, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i8_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_div_i8_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::div_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i16(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_i16, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i16_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_div_i16_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::div_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_div_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::div_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_div_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::div_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_div_f32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::div_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_div_f64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::div_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i8(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mod_i8, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i8_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mod_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mod_i8_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mod_i8_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i16(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mod_i16, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i16_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mod_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mod_i16_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mod_i16_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mod_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mod_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mod_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mod_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mod_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mod_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_wide_mod_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_wide_binary_constant_instruction(Opcode::mod_i64_k, dst, lhs, rhs);
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

  void Bytecode_writer::emit_binary_register_instruction(
    Opcode opcode,
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_opcode(opcode);
    _writer.write(static_cast<std::byte>(dst));
    _writer.write(static_cast<std::byte>(lhs));
    _writer.write(static_cast<std::byte>(rhs));
  }

  void Bytecode_writer::emit_binary_constant_instruction(
    Opcode opcode,
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_opcode(opcode);
    _writer.write(static_cast<std::byte>(dst));
    _writer.write(static_cast<std::byte>(lhs));
    _writer.write(static_cast<std::byte>(rhs));
  }

  void Bytecode_writer::emit_wide_binary_constant_instruction(
    Opcode opcode,
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_opcode(opcode);
    _writer.write(static_cast<std::byte>(dst));
    _writer.write(static_cast<std::byte>(lhs));
    _writer.write(static_cast<std::byte>(rhs));
    _writer.write(static_cast<std::byte>(rhs >> 8));
  }

} // namespace benson::bytecode

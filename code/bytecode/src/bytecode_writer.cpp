#include "bytecode/bytecode_writer.h"

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace benson::bytecode
{

  namespace
  {

    bool is_wide(Register reg)
    {
      return reg.value > std::numeric_limits<std::uint8_t>::max();
    }

    bool is_wide(Constant k)
    {
      return k.value > std::numeric_limits<std::uint8_t>::max();
    }

    bool is_wide(Immediate i)
    {
      return i.value < std::numeric_limits<std::int8_t>::min() ||
             i.value > std::numeric_limits<std::int8_t>::max();
    }

  } // namespace

  Bytecode_writer::Bytecode_writer(Output_stream *stream)
      : _stream{stream}, _position{}
  {
  }

  void Bytecode_writer::emit_exit()
  {
    emit_opcode(Opcode::exit);
  }

  void Bytecode_writer::emit_jmp(std::ptrdiff_t target)
  {
    auto const narrow_offset = target - (_position + 2);
    if (std::in_range<std::int8_t>(narrow_offset))
    {
      emit_opcode(Opcode::jmp_i);
      write_byte(static_cast<std::byte>(narrow_offset));
      return;
    }
    auto const wide_offset = target - (_position + 4);
    if (std::in_range<Immediate::Underlying_type>(wide_offset))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(Opcode::jmp_i);
      write_byte(static_cast<std::byte>(wide_offset));
      write_byte(static_cast<std::byte>(wide_offset >> 8));
      return;
    }
    throw std::runtime_error{"jmp target out of range"};
  }

  void Bytecode_writer::emit_call(std::ptrdiff_t target)
  {
    auto const narrow_offset = target - (_position + 2);
    if (std::in_range<std::int8_t>(narrow_offset))
    {
      emit_opcode(Opcode::call_i);
      write_byte(static_cast<std::byte>(narrow_offset));
      return;
    }
    auto const wide_offset = target - (_position + 4);
    if (std::in_range<Immediate::Underlying_type>(wide_offset))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(Opcode::call_i);
      write_byte(static_cast<std::byte>(wide_offset));
      write_byte(static_cast<std::byte>(wide_offset >> 8));
      return;
    }
    throw std::runtime_error{"call target out of range"};
  }

  void Bytecode_writer::emit_ret()
  {
    emit_opcode(Opcode::ret);
  }

  void Bytecode_writer::emit_jnz(Register src, std::ptrdiff_t target)
  {
    auto const narrow_offset = target - (_position + 3);
    if (!is_wide(src) && std::in_range<std::int8_t>(narrow_offset))
    {
      emit_opcode(Opcode::jnz_i);
      write_narrow_register(src);
      write_byte(static_cast<std::byte>(narrow_offset));
      return;
    }
    auto const wide_offset = target - (_position + 6);
    if (std::in_range<Immediate::Underlying_type>(wide_offset))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(Opcode::jnz_i);
      write_wide_register(src);
      write_byte(static_cast<std::byte>(wide_offset));
      write_byte(static_cast<std::byte>(wide_offset >> 8));
      return;
    }
    throw std::runtime_error{"jmp target out of range"};
  }

  void Bytecode_writer::emit_lookup_k(Register dst, Constant k)
  {
    if (is_wide(dst) || is_wide(k))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(Opcode::lookup_k);
      write_wide_register(dst);
      write_byte(static_cast<std::byte>(k.value));
      write_byte(static_cast<std::byte>(k.value >> 8));
    }
    else
    {
      emit_opcode(Opcode::lookup_k);
      write_narrow_register(dst);
      write_byte(static_cast<std::byte>(k.value));
    }
  }

  void Bytecode_writer::emit_load_8(
    Register dst,
    Register base,
    Immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::load_8, dst, base, offset);
  }

  void Bytecode_writer::emit_load_16(
    Register dst,
    Register base,
    Immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::load_16, dst, base, offset);
  }

  void Bytecode_writer::emit_load_32(
    Register dst,
    Register base,
    Immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::load_32, dst, base, offset);
  }

  void Bytecode_writer::emit_load_64(
    Register dst,
    Register base,
    Immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::load_64, dst, base, offset);
  }

  void Bytecode_writer::emit_store_8(
    Register src,
    Register base,
    Immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::store_8, src, base, offset);
  }

  void Bytecode_writer::emit_store_16(
    Register src,
    Register base,
    Immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::store_16, src, base, offset);
  }

  void Bytecode_writer::emit_store_32(
    Register src,
    Register base,
    Immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::store_32, src, base, offset);
  }

  void Bytecode_writer::emit_store_64(
    Register src,
    Register base,
    Immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::store_64, src, base, offset);
  }

  void Bytecode_writer::emit_mov(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::mov, dst, src);
  }

  void Bytecode_writer::emit_mov_i(Register dst, Immediate src)
  {
    emit_unary_immediate_instruction(Opcode::mov_i, dst, src);
  }

  void Bytecode_writer::emit_sx_8(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::sx_8, dst, src);
  }

  void Bytecode_writer::emit_sx_16(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::sx_16, dst, src);
  }

  void Bytecode_writer::emit_sx_32(Register dst, Register src)
  {
    emit_unary_register_instruction(Opcode::sx_32, dst, src);
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

  void Bytecode_writer::emit_add_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::add_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::add_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::add_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::sub_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::sub_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::sub_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::mul_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::mul_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mul_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::div_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::div_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::div_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mod_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i32_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mod_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::mod_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mod_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i64_k(Register dst, Register lhs, Constant rhs)
  {
    emit_binary_constant_instruction(Opcode::mod_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::mod_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_i32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_eq_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_i32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_eq_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_eq_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_i64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_eq_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_i64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_eq_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_eq_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_f32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_eq_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_f32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_eq_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_f64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_eq_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_eq_f64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_eq_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_i32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_ne_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_i32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_ne_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_ne_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_i64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_ne_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_i64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_ne_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_ne_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_f32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_ne_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_f32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_ne_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_f64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_ne_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ne_f64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_ne_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_i32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_lt_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_i32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_lt_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_lt_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_i64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_lt_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_i64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_lt_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_lt_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_f32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_lt_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_f32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_lt_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_f64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_lt_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_lt_f64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_lt_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_i32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_le_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_i32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_le_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_le_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_i64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_le_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_i64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_le_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_le_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_f32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_le_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_f32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_le_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_f64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_le_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_le_f64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_le_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_i32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_gt_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_i32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_gt_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_gt_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_i64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_gt_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_i64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_gt_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_gt_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_f32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_gt_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_f32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_gt_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_f64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_gt_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_gt_f64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_gt_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_i32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_ge_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_i32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_ge_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_i32_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_ge_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_i64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_ge_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_i64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_ge_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_i64_i(
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::cmp_ge_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_f32(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_ge_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_f32_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_ge_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_f64(
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_binary_register_instruction(Opcode::cmp_ge_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_cmp_ge_f64_k(
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::cmp_ge_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::flush()
  {
    _stream->flush();
  }

  void Bytecode_writer::write_byte(std::byte byte)
  {
    _stream->write_bytes(std::span{&byte, std::size_t{1}});
    ++_position;
  }

  void Bytecode_writer::emit_opcode(Opcode opcode)
  {
    write_byte(static_cast<std::byte>(opcode));
  }

  void Bytecode_writer::write_narrow_register(Register reg)
  {
    assert(!is_wide(reg));
    write_byte(static_cast<std::byte>(reg.value));
  }

  void Bytecode_writer::write_wide_register(Register reg)
  {
    write_byte(static_cast<std::byte>(reg.value));
    write_byte(static_cast<std::byte>(reg.value >> 8));
  }

  void Bytecode_writer::emit_unary_register_instruction(
    Opcode opcode,
    Register dst,
    Register src
  )
  {
    auto const wide = is_wide(dst) || is_wide(src);
    if (wide)
    {
      emit_opcode(Opcode::wide);
      emit_opcode(opcode);
      write_wide_register(dst);
      write_wide_register(src);
    }
    else
    {
      emit_opcode(opcode);
      write_narrow_register(dst);
      write_narrow_register(src);
    }
  }

  void Bytecode_writer::emit_unary_immediate_instruction(
    Opcode opcode,
    Register dst,
    Immediate src
  )
  {
    if (is_wide(dst) || is_wide(src))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(opcode);
      write_wide_register(dst);
      write_byte(static_cast<std::byte>(src.value));
      write_byte(static_cast<std::byte>(src.value >> 8));
    }
    else
    {
      emit_opcode(opcode);
      write_narrow_register(dst);
      write_byte(static_cast<std::byte>(src.value));
    }
  }

  void Bytecode_writer::emit_binary_register_instruction(
    Opcode opcode,
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    auto const wide = is_wide(dst) || is_wide(lhs) || is_wide(rhs);
    if (wide)
    {
      emit_opcode(Opcode::wide);
      emit_opcode(opcode);
      write_wide_register(dst);
      write_wide_register(lhs);
      write_wide_register(rhs);
    }
    else
    {
      emit_opcode(opcode);
      write_narrow_register(dst);
      write_narrow_register(lhs);
      write_narrow_register(rhs);
    }
  }

  void Bytecode_writer::emit_binary_constant_instruction(
    Opcode opcode,
    Register dst,
    Register lhs,
    Constant rhs
  )
  {
    if (is_wide(dst) || is_wide(lhs) || is_wide(rhs))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(opcode);
      write_wide_register(dst);
      write_wide_register(lhs);
      write_byte(static_cast<std::byte>(rhs.value));
      write_byte(static_cast<std::byte>(rhs.value >> 8));
    }
    else
    {
      emit_opcode(opcode);
      write_narrow_register(dst);
      write_narrow_register(lhs);
      write_byte(static_cast<std::byte>(rhs.value));
    }
  }

  void Bytecode_writer::emit_binary_immediate_instruction(
    Opcode opcode,
    Register dst,
    Register lhs,
    Immediate rhs
  )
  {
    if (is_wide(dst) || is_wide(lhs) || is_wide(rhs))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(opcode);
      write_wide_register(dst);
      write_wide_register(lhs);
      write_byte(static_cast<std::byte>(rhs.value));
      write_byte(static_cast<std::byte>(rhs.value >> 8));
    }
    else
    {
      emit_opcode(opcode);
      write_narrow_register(dst);
      write_narrow_register(lhs);
      write_byte(static_cast<std::byte>(rhs.value));
    }
  }

} // namespace benson::bytecode

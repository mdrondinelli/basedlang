#include "bytecode/bytecode_writer.h"

#include <limits>

namespace benson::bytecode
{

  namespace
  {

    bool is_wide(Wide_constant k)
    {
      return k >
             static_cast<Wide_constant>(std::numeric_limits<Constant>::max());
    }

    bool is_wide(Wide_immediate i)
    {
      return i < static_cast<Wide_immediate>(
                   std::numeric_limits<Immediate>::min()
                 ) ||
             i > static_cast<Wide_immediate>(
                   std::numeric_limits<Immediate>::max()
                 );
    }

  } // namespace

  Bytecode_writer::Bytecode_writer(Binary_output_stream *stream)
      : _stream{stream}
  {
  }

  void Bytecode_writer::emit_exit()
  {
    emit_opcode(Opcode::exit);
  }

  void Bytecode_writer::emit_jmp_i(Wide_immediate offset)
  {
    emit_immediate_instruction(Opcode::jmp_i, offset);
  }

  void Bytecode_writer::emit_lookup_k(Register dst, Wide_constant k)
  {
    if (is_wide(k))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(Opcode::lookup_k);
      write_byte(static_cast<std::byte>(dst));
      write_byte(static_cast<std::byte>(k));
      write_byte(static_cast<std::byte>(k >> 8));
    }
    else
    {
      emit_opcode(Opcode::lookup_k);
      write_byte(static_cast<std::byte>(dst));
      write_byte(static_cast<std::byte>(k));
    }
  }

  void Bytecode_writer::emit_load_8(
    Register dst,
    Register base,
    Wide_immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::load_8, dst, base, offset);
  }

  void Bytecode_writer::emit_load_16(
    Register dst,
    Register base,
    Wide_immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::load_16, dst, base, offset);
  }

  void Bytecode_writer::emit_load_32(
    Register dst,
    Register base,
    Wide_immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::load_32, dst, base, offset);
  }

  void Bytecode_writer::emit_load_64(
    Register dst,
    Register base,
    Wide_immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::load_64, dst, base, offset);
  }

  void Bytecode_writer::emit_store_8(
    Register src,
    Register base,
    Wide_immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::store_8, src, base, offset);
  }

  void Bytecode_writer::emit_store_16(
    Register src,
    Register base,
    Wide_immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::store_16, src, base, offset);
  }

  void Bytecode_writer::emit_store_32(
    Register src,
    Register base,
    Wide_immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::store_32, src, base, offset);
  }

  void Bytecode_writer::emit_store_64(
    Register src,
    Register base,
    Wide_immediate offset
  )
  {
    emit_binary_immediate_instruction(Opcode::store_64, src, base, offset);
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

  void Bytecode_writer::emit_add_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::add_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i32_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::add_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::add_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_i64_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::add_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::add_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::add_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_add_f64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::add_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::sub_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i32_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::sub_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::sub_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_i64_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::sub_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::sub_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::sub_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_sub_f64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::sub_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::mul_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i32_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::mul_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::mul_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_i64_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::mul_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::mul_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mul_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mul_f64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::mul_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::div_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i32_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::div_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::div_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_i64_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::div_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_f32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::div_f32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::div_f64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_div_f64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::div_f64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i32(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mod_i32, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i32_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::mod_i32_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i32_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::mod_i32_i, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i64(Register dst, Register lhs, Register rhs)
  {
    emit_binary_register_instruction(Opcode::mod_i64, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i64_k(
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    emit_binary_constant_instruction(Opcode::mod_i64_k, dst, lhs, rhs);
  }

  void Bytecode_writer::emit_mod_i64_i(
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    emit_binary_immediate_instruction(Opcode::mod_i64_i, dst, lhs, rhs);
  }

  void Bytecode_writer::flush()
  {
    _stream->flush();
  }

  void Bytecode_writer::write_byte(std::byte byte)
  {
    _stream->write_bytes(std::span{&byte, std::size_t{1}});
  }

  void Bytecode_writer::emit_opcode(Opcode opcode)
  {
    write_byte(static_cast<std::byte>(opcode));
  }

  void Bytecode_writer::emit_unary_register_instruction(
    Opcode opcode,
    Register dst,
    Register src
  )
  {
    emit_opcode(opcode);
    write_byte(static_cast<std::byte>(dst));
    write_byte(static_cast<std::byte>(src));
  }

  void Bytecode_writer::emit_binary_register_instruction(
    Opcode opcode,
    Register dst,
    Register lhs,
    Register rhs
  )
  {
    emit_opcode(opcode);
    write_byte(static_cast<std::byte>(dst));
    write_byte(static_cast<std::byte>(lhs));
    write_byte(static_cast<std::byte>(rhs));
  }

  void Bytecode_writer::emit_binary_constant_instruction(
    Opcode opcode,
    Register dst,
    Register lhs,
    Wide_constant rhs
  )
  {
    if (is_wide(rhs))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(opcode);
      write_byte(static_cast<std::byte>(dst));
      write_byte(static_cast<std::byte>(lhs));
      write_byte(static_cast<std::byte>(rhs));
      write_byte(static_cast<std::byte>(rhs >> 8));
    }
    else
    {
      emit_opcode(opcode);
      write_byte(static_cast<std::byte>(dst));
      write_byte(static_cast<std::byte>(lhs));
      write_byte(static_cast<std::byte>(static_cast<Constant>(rhs)));
    }
  }

  void Bytecode_writer::emit_binary_immediate_instruction(
    Opcode opcode,
    Register dst,
    Register lhs,
    Wide_immediate rhs
  )
  {
    if (is_wide(rhs))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(opcode);
      write_byte(static_cast<std::byte>(dst));
      write_byte(static_cast<std::byte>(lhs));
      write_byte(static_cast<std::byte>(rhs));
      write_byte(static_cast<std::byte>(rhs >> 8));
    }
    else
    {
      emit_opcode(opcode);
      write_byte(static_cast<std::byte>(dst));
      write_byte(static_cast<std::byte>(lhs));
      write_byte(static_cast<std::byte>(static_cast<Immediate>(rhs)));
    }
  }

  void Bytecode_writer::emit_immediate_instruction(
    Opcode opcode,
    Wide_immediate value
  )
  {
    if (is_wide(value))
    {
      emit_opcode(Opcode::wide);
      emit_opcode(opcode);
      write_byte(static_cast<std::byte>(value));
      write_byte(static_cast<std::byte>(value >> 8));
    }
    else
    {
      emit_opcode(opcode);
      write_byte(static_cast<std::byte>(static_cast<Immediate>(value)));
    }
  }

} // namespace benson::bytecode

#ifndef BENSON_BYTECODE_BYTECODE_WRITER_H
#define BENSON_BYTECODE_BYTECODE_WRITER_H

#include <optional>

#include "bytecode/constant.h"
#include "bytecode/function.h"
#include "bytecode/immediate.h"
#include "bytecode/opcode.h"
#include "bytecode/register.h"
#include "streams/output_stream.h"

namespace benson::bytecode
{
  class Bytecode_writer
  {
  public:
    explicit Bytecode_writer(Output_stream *stream);

    void emit_exit();

    void emit_jmp(std::ptrdiff_t target);

    void emit_call_i(Function function, Register base, Register dst);

    void emit_call_void_i(Function function, Register base);

    void emit_ret(Register src);

    void emit_ret_void();

    template <typename JumpTargetProvider>
    void emit_jmp(JumpTargetProvider &&provider)
    {
      auto const patch_position = position() + 2;
      auto const target = provider.target(patch_position);
      static_assert(std::is_same_v<
                    std::remove_cv_t<decltype(target)>,
                    std::optional<std::ptrdiff_t>
      >);
      if (target)
      {
        emit_jmp(*target);
      }
      else
      {
        emit_opcode(Opcode::wide);
        emit_opcode(Opcode::jmp_i);
        write_byte({});
        write_byte({});
      }
    }

    void emit_jnz(Register src, std::ptrdiff_t target);

    template <typename JumpTargetProvider>
    void emit_jnz(Register src, JumpTargetProvider &&provider)
    {
      auto const patch_position = position() + 4;
      auto const target = provider.target(patch_position);
      static_assert(std::is_same_v<
                    std::remove_cv_t<decltype(target)>,
                    std::optional<std::ptrdiff_t>
      >);
      if (target)
      {
        emit_jnz(src, *target);
      }
      else
      {
        emit_opcode(Opcode::wide);
        emit_opcode(Opcode::jnz_i);
        write_wide_register(src);
        write_byte({});
        write_byte({});
      }
    }

    void emit_lookup_k(Constant k, Register dst);

    void emit_load_8(Register base, Immediate offset, Register dst);

    void emit_load_16(Register base, Immediate offset, Register dst);

    void emit_load_32(Register base, Immediate offset, Register dst);

    void emit_load_64(Register base, Immediate offset, Register dst);

    void emit_store_8(Register base, Immediate offset, Register src);

    void emit_store_16(Register base, Immediate offset, Register src);

    void emit_store_32(Register base, Immediate offset, Register src);

    void emit_store_64(Register base, Immediate offset, Register src);

    void emit_mov(Register src, Register dst);

    void emit_mov_i(Immediate src, Register dst);

    void emit_sx_8(Register src, Register dst);

    void emit_sx_16(Register src, Register dst);

    void emit_sx_32(Register src, Register dst);

    void emit_neg_i32(Register src, Register dst);

    void emit_neg_i64(Register src, Register dst);

    void emit_neg_f32(Register src, Register dst);

    void emit_neg_f64(Register src, Register dst);

    void emit_add_i32(Register lhs, Register rhs, Register dst);

    void emit_add_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_add_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_add_i64(Register lhs, Register rhs, Register dst);

    void emit_add_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_add_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_add_f32(Register lhs, Register rhs, Register dst);

    void emit_add_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_add_f64(Register lhs, Register rhs, Register dst);

    void emit_add_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_sub_i32(Register lhs, Register rhs, Register dst);

    void emit_sub_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_sub_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_sub_i64(Register lhs, Register rhs, Register dst);

    void emit_sub_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_sub_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_sub_f32(Register lhs, Register rhs, Register dst);

    void emit_sub_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_sub_f64(Register lhs, Register rhs, Register dst);

    void emit_sub_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_mul_i32(Register lhs, Register rhs, Register dst);

    void emit_mul_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_mul_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_mul_i64(Register lhs, Register rhs, Register dst);

    void emit_mul_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_mul_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_mul_f32(Register lhs, Register rhs, Register dst);

    void emit_mul_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_mul_f64(Register lhs, Register rhs, Register dst);

    void emit_mul_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_div_i32(Register lhs, Register rhs, Register dst);

    void emit_div_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_div_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_div_i64(Register lhs, Register rhs, Register dst);

    void emit_div_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_div_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_div_f32(Register lhs, Register rhs, Register dst);

    void emit_div_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_div_f64(Register lhs, Register rhs, Register dst);

    void emit_div_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_mod_i32(Register lhs, Register rhs, Register dst);

    void emit_mod_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_mod_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_mod_i64(Register lhs, Register rhs, Register dst);

    void emit_mod_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_mod_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_eq_i32(Register lhs, Register rhs, Register dst);

    void emit_cmp_eq_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_eq_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_eq_i64(Register lhs, Register rhs, Register dst);

    void emit_cmp_eq_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_eq_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_eq_f32(Register lhs, Register rhs, Register dst);

    void emit_cmp_eq_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_eq_f64(Register lhs, Register rhs, Register dst);

    void emit_cmp_eq_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_ne_i32(Register lhs, Register rhs, Register dst);

    void emit_cmp_ne_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_ne_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_ne_i64(Register lhs, Register rhs, Register dst);

    void emit_cmp_ne_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_ne_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_ne_f32(Register lhs, Register rhs, Register dst);

    void emit_cmp_ne_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_ne_f64(Register lhs, Register rhs, Register dst);

    void emit_cmp_ne_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_lt_i32(Register lhs, Register rhs, Register dst);

    void emit_cmp_lt_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_lt_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_lt_i64(Register lhs, Register rhs, Register dst);

    void emit_cmp_lt_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_lt_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_lt_f32(Register lhs, Register rhs, Register dst);

    void emit_cmp_lt_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_lt_f64(Register lhs, Register rhs, Register dst);

    void emit_cmp_lt_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_le_i32(Register lhs, Register rhs, Register dst);

    void emit_cmp_le_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_le_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_le_i64(Register lhs, Register rhs, Register dst);

    void emit_cmp_le_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_le_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_le_f32(Register lhs, Register rhs, Register dst);

    void emit_cmp_le_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_le_f64(Register lhs, Register rhs, Register dst);

    void emit_cmp_le_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_gt_i32(Register lhs, Register rhs, Register dst);

    void emit_cmp_gt_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_gt_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_gt_i64(Register lhs, Register rhs, Register dst);

    void emit_cmp_gt_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_gt_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_gt_f32(Register lhs, Register rhs, Register dst);

    void emit_cmp_gt_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_gt_f64(Register lhs, Register rhs, Register dst);

    void emit_cmp_gt_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_ge_i32(Register lhs, Register rhs, Register dst);

    void emit_cmp_ge_i32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_ge_i32_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_ge_i64(Register lhs, Register rhs, Register dst);

    void emit_cmp_ge_i64_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_ge_i64_i(Register lhs, Immediate rhs, Register dst);

    void emit_cmp_ge_f32(Register lhs, Register rhs, Register dst);

    void emit_cmp_ge_f32_k(Register lhs, Constant rhs, Register dst);

    void emit_cmp_ge_f64(Register lhs, Register rhs, Register dst);

    void emit_cmp_ge_f64_k(Register lhs, Constant rhs, Register dst);

    void emit_alloca_i(Immediate amount);

    void emit_alloca(Register amount);

    void emit_mov_sp_i(Immediate offset, Register dst);

    void emit_load_sp_8(Immediate offset, Register dst);

    void emit_load_sp_16(Immediate offset, Register dst);

    void emit_load_sp_32(Immediate offset, Register dst);

    void emit_load_sp_64(Immediate offset, Register dst);

    void emit_store_sp_8(Immediate offset, Register src);

    void emit_store_sp_16(Immediate offset, Register src);

    void emit_store_sp_32(Immediate offset, Register src);

    void emit_store_sp_64(Immediate offset, Register src);

    std::ptrdiff_t position() const noexcept
    {
      return _position;
    }

    void flush();

  private:
    void write_byte(std::byte byte);

    void emit_opcode(Opcode opcode);

    void write_narrow_register(Register reg);

    void write_wide_register(Register reg);

    void emit_unary_register_instruction(
      Opcode opcode,
      Register src,
      Register dst
    );

    void emit_unary_immediate_instruction(
      Opcode opcode,
      Immediate src,
      Register dst
    );

    void emit_immediate_register_instruction(
      Opcode opcode,
      Immediate offset,
      Register reg
    );

    void emit_binary_register_instruction(
      Opcode opcode,
      Register lhs,
      Register rhs,
      Register dst
    );

    void emit_binary_constant_instruction(
      Opcode opcode,
      Register lhs,
      Constant rhs,
      Register dst
    );

    void emit_binary_immediate_instruction(
      Opcode opcode,
      Register lhs,
      Immediate rhs,
      Register dst
    );

    Output_stream *_stream;
    std::ptrdiff_t _position;
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_BYTECODE_WRITER_H

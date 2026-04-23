#ifndef BENSON_BYTECODE_MODULE_BUILDER_H
#define BENSON_BYTECODE_MODULE_BUILDER_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <vector>

#include "bytecode/bytecode_writer.h"
#include "bytecode/constant.h"
#include "bytecode/module.h"

namespace benson
{

  class Binary_output_stream;

} // namespace benson

namespace benson::bytecode
{

  namespace detail
  {

    template <typename T>
    concept Supported_constant =
      std::same_as<T, std::int32_t> || std::same_as<T, std::int64_t> ||
      std::same_as<T, float> || std::same_as<T, double>;

  } // namespace detail

  class Module_builder_storage
  {
  protected:
    Module_builder_storage();

    struct Code_output_stream: Binary_output_stream
    {
      explicit Code_output_stream(std::vector<std::byte> *code);

      void write_bytes(std::span<std::byte const> buffer) override;

    private:
      std::vector<std::byte> *_code;
    };

    Module _module;
    Code_output_stream _output_stream;
  };

  class Module_builder: private Module_builder_storage, private Bytecode_writer
  {
  public:
    struct Label
    {
      std::size_t id;
    };

    Module_builder();

    using Bytecode_writer::emit_add_f32;
    using Bytecode_writer::emit_add_f32_k;

    void emit_add_f32_k(Register dst, Register lhs, float rhs)
    {
      Bytecode_writer::emit_add_f32_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_add_f64;
    using Bytecode_writer::emit_add_f64_k;

    void emit_add_f64_k(Register dst, Register lhs, double rhs)
    {
      Bytecode_writer::emit_add_f64_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_add_i32;
    using Bytecode_writer::emit_add_i32_i;
    using Bytecode_writer::emit_add_i32_k;

    void emit_add_i32_k(Register dst, Register lhs, std::int32_t rhs)
    {
      Bytecode_writer::emit_add_i32_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_add_i64;
    using Bytecode_writer::emit_add_i64_i;
    using Bytecode_writer::emit_add_i64_k;

    void emit_add_i64_k(Register dst, Register lhs, std::int64_t rhs)
    {
      Bytecode_writer::emit_add_i64_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_div_f32;
    using Bytecode_writer::emit_div_f32_k;

    void emit_div_f32_k(Register dst, Register lhs, float rhs)
    {
      Bytecode_writer::emit_div_f32_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_div_f64;
    using Bytecode_writer::emit_div_f64_k;

    void emit_div_f64_k(Register dst, Register lhs, double rhs)
    {
      Bytecode_writer::emit_div_f64_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_div_i32;
    using Bytecode_writer::emit_div_i32_i;
    using Bytecode_writer::emit_div_i32_k;

    void emit_div_i32_k(Register dst, Register lhs, std::int32_t rhs)
    {
      Bytecode_writer::emit_div_i32_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_div_i64;
    using Bytecode_writer::emit_div_i64_i;
    using Bytecode_writer::emit_div_i64_k;

    void emit_div_i64_k(Register dst, Register lhs, std::int64_t rhs)
    {
      Bytecode_writer::emit_div_i64_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_exit;
    using Bytecode_writer::emit_jmp_i;
    using Bytecode_writer::emit_load_16;
    using Bytecode_writer::emit_load_32;
    using Bytecode_writer::emit_load_64;
    using Bytecode_writer::emit_load_8;
    using Bytecode_writer::emit_lookup_k;

    template <detail::Supported_constant T>
    void emit_lookup_k(Register dst, T value)
    {
      Bytecode_writer::emit_lookup_k(dst, constant(value));
    }

    using Bytecode_writer::emit_mod_i32;
    using Bytecode_writer::emit_mod_i32_i;
    using Bytecode_writer::emit_mod_i32_k;

    void emit_mod_i32_k(Register dst, Register lhs, std::int32_t rhs)
    {
      Bytecode_writer::emit_mod_i32_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_mod_i64;
    using Bytecode_writer::emit_mod_i64_i;
    using Bytecode_writer::emit_mod_i64_k;

    void emit_mod_i64_k(Register dst, Register lhs, std::int64_t rhs)
    {
      Bytecode_writer::emit_mod_i64_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_mul_f32;
    using Bytecode_writer::emit_mul_f32_k;

    void emit_mul_f32_k(Register dst, Register lhs, float rhs)
    {
      Bytecode_writer::emit_mul_f32_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_mul_f64;
    using Bytecode_writer::emit_mul_f64_k;

    void emit_mul_f64_k(Register dst, Register lhs, double rhs)
    {
      Bytecode_writer::emit_mul_f64_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_mul_i32;
    using Bytecode_writer::emit_mul_i32_i;
    using Bytecode_writer::emit_mul_i32_k;

    void emit_mul_i32_k(Register dst, Register lhs, std::int32_t rhs)
    {
      Bytecode_writer::emit_mul_i32_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_mul_i64;
    using Bytecode_writer::emit_mul_i64_i;
    using Bytecode_writer::emit_mul_i64_k;

    void emit_mul_i64_k(Register dst, Register lhs, std::int64_t rhs)
    {
      Bytecode_writer::emit_mul_i64_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_neg_f32;
    using Bytecode_writer::emit_neg_f64;
    using Bytecode_writer::emit_neg_i32;
    using Bytecode_writer::emit_neg_i64;
    using Bytecode_writer::emit_store_16;
    using Bytecode_writer::emit_store_32;
    using Bytecode_writer::emit_store_64;
    using Bytecode_writer::emit_store_8;
    using Bytecode_writer::emit_sub_f32;
    using Bytecode_writer::emit_sub_f32_k;

    void emit_sub_f32_k(Register dst, Register lhs, float rhs)
    {
      Bytecode_writer::emit_sub_f32_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_sub_f64;
    using Bytecode_writer::emit_sub_f64_k;

    void emit_sub_f64_k(Register dst, Register lhs, double rhs)
    {
      Bytecode_writer::emit_sub_f64_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_sub_i32;
    using Bytecode_writer::emit_sub_i32_i;
    using Bytecode_writer::emit_sub_i32_k;

    void emit_sub_i32_k(Register dst, Register lhs, std::int32_t rhs)
    {
      Bytecode_writer::emit_sub_i32_k(dst, lhs, constant(rhs));
    }

    using Bytecode_writer::emit_sub_i64;
    using Bytecode_writer::emit_sub_i64_i;
    using Bytecode_writer::emit_sub_i64_k;

    void emit_sub_i64_k(Register dst, Register lhs, std::int64_t rhs)
    {
      Bytecode_writer::emit_sub_i64_k(dst, lhs, constant(rhs));
    }

    [[nodiscard]] auto make_label() -> Label;

    template <detail::Supported_constant T>
    [[nodiscard]] auto constant(T value) -> Wide_constant
    {
      return constant(std::as_bytes(std::span{&value, std::size_t{1}}));
    }

    void place_label(Label label);
    void emit_jmp_i(Label target);

    [[nodiscard]] auto build() -> Module;

  private:
    struct Pending_jump
    {
      Label target;
      std::ptrdiff_t immediate_offset;
      std::ptrdiff_t next_instruction_offset;
    };

    [[nodiscard]] auto flushed_code_size() -> std::ptrdiff_t;
    [[nodiscard]] auto constant(std::span<std::byte const> bytes)
      -> Wide_constant;

    std::vector<std::ptrdiff_t> _label_offsets{};
    std::vector<Pending_jump> _pending_jumps{};
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_MODULE_BUILDER_H

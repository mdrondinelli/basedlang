#ifndef BENSON_BYTECODE_MODULE_BUILDER_H
#define BENSON_BYTECODE_MODULE_BUILDER_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
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

  // This class contains references to its own members.
  class Module_builder
  {
  public:
    struct Label
    {
      std::size_t index;
    };

    class Label_jump_target_provider
    {
    public:
      [[nodiscard]] std::optional<std::ptrdiff_t>
      target(std::ptrdiff_t patch_position) const;

    private:
      friend class Module_builder;

      explicit Label_jump_target_provider(Module_builder *builder, Label label)
          : _builder{builder}, _label{label}
      {
      }

      Module_builder *_builder;
      Label _label;
    };

    Module_builder();

    Module_builder(Module_builder const &) = delete;

    Module_builder &operator=(Module_builder const &) = delete;

    [[nodiscard]] Bytecode_writer &writer() noexcept;

    [[nodiscard]] Label make_label();

    void place_label(Label label);

    Label_jump_target_provider label_target(Label label);

    template <detail::Supported_constant T>
    [[nodiscard]] Constant constant(T value)
    {
      return constant(std::as_bytes(std::span{&value, std::size_t{1}}));
    }

    [[nodiscard]] Module build();

  private:
    struct Vector_output_stream: Binary_output_stream
    {
      explicit Vector_output_stream(std::vector<std::byte> *code);

      void write_bytes(std::span<std::byte const> buffer) override;

    private:
      std::vector<std::byte> *_code;
    };

    struct Unresolved_jump_offset
    {
      std::ptrdiff_t position;
      Label label;
    };

    [[nodiscard]] auto constant(std::span<std::byte const> bytes) -> Constant;

    Module _module;
    Vector_output_stream _output_stream;
    Bytecode_writer _writer;
    std::vector<std::optional<std::ptrdiff_t>> _label_positions{};
    std::vector<Unresolved_jump_offset> _unresolved_jump_offsets{};
  };

} // namespace benson::bytecode

#endif // BENSON_BYTECODE_MODULE_BUILDER_H

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "bytecode/bytecode_writer.h"
#include "vm/vm.h"

namespace
{

  class Recording_binary_output_stream: public benson::Binary_output_stream
  {
  public:
    std::ptrdiff_t write_bytes(std::span<std::byte const> buffer) override
    {
      _bytes.insert(_bytes.end(), buffer.begin(), buffer.end());
      return static_cast<std::ptrdiff_t>(buffer.size());
    }

    [[nodiscard]] auto bytes() const -> std::vector<std::byte> const &
    {
      return _bytes;
    }

  private:
    std::vector<std::byte> _bytes{};
  };

} // namespace

TEMPLATE_TEST_CASE(
  "Virtual_machine typed register access round-trips values",
  "[vm]",
  std::int8_t,
  std::int16_t,
  std::int32_t,
  std::int64_t,
  float,
  double
)
{
  using benson::bytecode::Register;

  auto const value = [] -> TestType
  {
    if constexpr (std::same_as<TestType, std::int8_t>)
    {
      return std::int8_t{42};
    }
    else if constexpr (std::same_as<TestType, std::int16_t>)
    {
      return std::int16_t{1234};
    }
    else if constexpr (std::same_as<TestType, std::int32_t>)
    {
      return std::int32_t{12345678};
    }
    else if constexpr (std::same_as<TestType, std::int64_t>)
    {
      return std::int64_t{1234567890123456789LL};
    }
    else if constexpr (std::same_as<TestType, float>)
    {
      return 42.5F;
    }
    else
    {
      return 42.5;
    }
  }();

  auto vm = benson::Virtual_machine{};
  vm.set_register_value(Register::gpr_255, value);

  CHECK(vm.get_register_value<TestType>(Register::gpr_255) == value);
}

TEST_CASE("Virtual_machine keeps exact floating-point register bit pattern")
{
  auto vm = benson::Virtual_machine{};
  vm.set_register_value(benson::bytecode::Register::gpr_1, -0.0F);

  CHECK(
    std::bit_cast<std::uint32_t>(
      vm.get_register_value<float>(benson::bytecode::Register::gpr_1)
    ) == std::bit_cast<std::uint32_t>(-0.0F)
  );
}

TEST_CASE(
  "Virtual_machine runs nop and negation program emitted by Bytecode_writer"
)
{
  using benson::bytecode::Register;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_nop();
  writer.emit_neg_i32(Register::gpr_1, Register::gpr_2);
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(Register::gpr_2, 123);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == -123);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_2) == 123);
  CHECK(vm.instruction_pointer == stream.bytes().data() + 5);
}

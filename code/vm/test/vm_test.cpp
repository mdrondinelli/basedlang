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

TEST_CASE("Virtual_machine runs negation program emitted by Bytecode_writer")
{
  using benson::bytecode::Register;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_neg_i32(Register::gpr_1, Register::gpr_2);
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(Register::gpr_2, 123);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == -123);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_2) == 123);
  CHECK(vm.instruction_pointer == stream.bytes().data() + 4);
}

TEST_CASE(
  "Virtual_machine runs integer arithmetic program emitted by Bytecode_writer"
)
{
  using benson::bytecode::Register;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_add_i32(Register::gpr_1, Register::gpr_2, Register::gpr_3);
  writer.emit_sub_i32(Register::gpr_4, Register::gpr_1, Register::gpr_5);
  writer.emit_mul_i32(Register::gpr_6, Register::gpr_4, Register::gpr_7);
  writer.emit_div_i32(Register::gpr_8, Register::gpr_6, Register::gpr_9);
  writer.emit_mod_i32(Register::gpr_10, Register::gpr_8, Register::gpr_11);
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<std::int32_t>(Register::gpr_2, 10);
  vm.set_register_value<std::int32_t>(Register::gpr_3, 5);
  vm.set_register_value<std::int32_t>(Register::gpr_5, 3);
  vm.set_register_value<std::int32_t>(Register::gpr_7, 4);
  vm.set_register_value<std::int32_t>(Register::gpr_9, 6);
  vm.set_register_value<std::int32_t>(Register::gpr_11, 7);

  vm.run();

  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_1) == 15);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_4) == 12);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_6) == 48);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_8) == 8);
  CHECK(vm.get_register_value<std::int32_t>(Register::gpr_10) == 1);
  CHECK(vm.instruction_pointer == stream.bytes().data() + 21);
}

TEST_CASE(
  "Virtual_machine runs floating-point arithmetic program emitted by "
  "Bytecode_writer"
)
{
  using benson::bytecode::Register;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};
  writer.emit_add_f64(Register::gpr_1, Register::gpr_2, Register::gpr_3);
  writer.emit_sub_f64(Register::gpr_4, Register::gpr_1, Register::gpr_5);
  writer.emit_mul_f64(Register::gpr_6, Register::gpr_4, Register::gpr_7);
  writer.emit_div_f64(Register::gpr_8, Register::gpr_6, Register::gpr_9);
  writer.emit_exit();
  writer.flush();

  auto vm = benson::Virtual_machine{};
  vm.instruction_pointer = stream.bytes().data();
  vm.set_register_value<double>(Register::gpr_2, 10.0);
  vm.set_register_value<double>(Register::gpr_3, 2.5);
  vm.set_register_value<double>(Register::gpr_5, 1.5);
  vm.set_register_value<double>(Register::gpr_7, 4.0);
  vm.set_register_value<double>(Register::gpr_9, 2.0);

  vm.run();

  CHECK(vm.get_register_value<double>(Register::gpr_1) == 12.5);
  CHECK(vm.get_register_value<double>(Register::gpr_4) == 11.0);
  CHECK(vm.get_register_value<double>(Register::gpr_6) == 44.0);
  CHECK(vm.get_register_value<double>(Register::gpr_8) == 22.0);
  CHECK(vm.instruction_pointer == stream.bytes().data() + 17);
}

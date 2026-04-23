#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "bytecode/bytecode_writer.h"

namespace
{

  class Recording_binary_output_stream: public benson::Binary_output_stream
  {
  public:
    void write_bytes(std::span<std::byte const> buffer) override
    {
      _bytes.insert(_bytes.end(), buffer.begin(), buffer.end());
    }

    [[nodiscard]] auto bytes() const -> std::vector<std::byte> const &
    {
      return _bytes;
    }

  private:
    std::vector<std::byte> _bytes{};
  };

} // namespace

TEST_CASE("Bytecode_writer emits exit opcode")
{
  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_exit();
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(benson::bytecode::Opcode::exit),
                      }
  );
}

TEST_CASE("Bytecode_writer emits unary negate instruction operands")
{
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_neg_f64(Register::gpr_254, Register::gpr_255);
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::neg_f64),
                        static_cast<std::byte>(Register::gpr_254),
                        static_cast<std::byte>(Register::gpr_255),
                      }
  );
}

TEST_CASE("Bytecode_writer emits lookup constant instruction operands")
{
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_constant;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_lookup_k(Register::gpr_1, Wide_constant{3});
  writer.emit_lookup_k(Register::gpr_2, Wide_constant{0x0405});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::lookup_k),
                        static_cast<std::byte>(Register::gpr_1),
                        std::byte{0x03},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::lookup_k),
                        static_cast<std::byte>(Register::gpr_2),
                        std::byte{0x05},
                        std::byte{0x04},
                      }
  );
}

TEST_CASE("Bytecode_writer emits binary arithmetic instruction operands")
{
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_add_i32(Register::gpr_1, Register::gpr_2, Register::gpr_3);
  writer.emit_mod_i64(Register::gpr_4, Register::gpr_5, Register::gpr_6);
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::add_i32),
                        static_cast<std::byte>(Register::gpr_1),
                        static_cast<std::byte>(Register::gpr_2),
                        static_cast<std::byte>(Register::gpr_3),
                        static_cast<std::byte>(Opcode::mod_i64),
                        static_cast<std::byte>(Register::gpr_4),
                        static_cast<std::byte>(Register::gpr_5),
                        static_cast<std::byte>(Register::gpr_6),
                      }
  );
}

TEST_CASE(
  "Bytecode_writer emits binary arithmetic immediate instruction operands"
)
{
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_immediate;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_add_i32_i(Register::gpr_1, Register::gpr_2, Wide_immediate{-3});
  writer.emit_mul_i32_i(Register::gpr_4, Register::gpr_5, Wide_immediate{0x0102});
  writer.emit_mod_i64_i(Register::gpr_7, Register::gpr_8, Wide_immediate{-0x0203});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::add_i32_i),
                        static_cast<std::byte>(Register::gpr_1),
                        static_cast<std::byte>(Register::gpr_2),
                        std::byte{0xFD},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::mul_i32_i),
                        static_cast<std::byte>(Register::gpr_4),
                        static_cast<std::byte>(Register::gpr_5),
                        std::byte{0x02},
                        std::byte{0x01},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::mod_i64_i),
                        static_cast<std::byte>(Register::gpr_7),
                        static_cast<std::byte>(Register::gpr_8),
                        std::byte{0xFD},
                        std::byte{0xFD},
                      }
  );
}

TEST_CASE(
  "Bytecode_writer emits binary arithmetic constant instruction operands"
)
{
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_constant;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_sub_i32_k(Register::gpr_1, Register::gpr_2, Wide_constant{3});
  writer.emit_div_f64_k(Register::gpr_4, Register::gpr_5, Wide_constant{6});
  writer.emit_mod_i32_k(Register::gpr_7, Register::gpr_8, Wide_constant{9});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::sub_i32_k),
                        static_cast<std::byte>(Register::gpr_1),
                        static_cast<std::byte>(Register::gpr_2),
                        std::byte{3},
                        static_cast<std::byte>(Opcode::div_f64_k),
                        static_cast<std::byte>(Register::gpr_4),
                        static_cast<std::byte>(Register::gpr_5),
                        std::byte{6},
                        static_cast<std::byte>(Opcode::mod_i32_k),
                        static_cast<std::byte>(Register::gpr_7),
                        static_cast<std::byte>(Register::gpr_8),
                        std::byte{9},
                      }
  );
}

TEST_CASE(
  "Bytecode_writer emits wide binary arithmetic constant instruction operands"
)
{
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;
  using benson::bytecode::Wide_constant;

  auto stream = Recording_binary_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_sub_i32_k(Register::gpr_1, Register::gpr_2, Wide_constant{0x0304});
  writer.emit_div_f64_k(Register::gpr_4, Register::gpr_5, Wide_constant{0x0607});
  writer.emit_mod_i32_k(Register::gpr_7, Register::gpr_8, Wide_constant{0x090A});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::sub_i32_k),
                        static_cast<std::byte>(Register::gpr_1),
                        static_cast<std::byte>(Register::gpr_2),
                        std::byte{0x04},
                        std::byte{0x03},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::div_f64_k),
                        static_cast<std::byte>(Register::gpr_4),
                        static_cast<std::byte>(Register::gpr_5),
                        std::byte{0x07},
                        std::byte{0x06},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::mod_i32_k),
                        static_cast<std::byte>(Register::gpr_7),
                        static_cast<std::byte>(Register::gpr_8),
                        std::byte{0x0A},
                        std::byte{0x09},
                      }
  );
}

#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "bytecode/bytecode_writer.h"

namespace
{
  using benson::bytecode::gpr;
  using benson::bytecode::sp;

  auto reg_byte(benson::bytecode::Register reg) -> std::byte
  {
    return static_cast<std::byte>(reg.value);
  }

  class Recording_output_stream: public benson::Output_stream
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
  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_exit();
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(benson::bytecode::Opcode::exit),
                      }
  );
}

TEST_CASE("Bytecode_writer emits unary sign extend instruction operands")
{
  using enum benson::bytecode::Opcode;
  using benson::bytecode::gpr;
  using benson::bytecode::sp;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_sx_32(gpr(254), gpr(255));
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(sx_32),
                        reg_byte(gpr(254)),
                        reg_byte(gpr(255)),
                      }
  );
}

TEST_CASE("Bytecode_writer emits unary negate instruction operands")
{
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_neg_f64(gpr(254), gpr(255));
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::neg_f64),
                        reg_byte(gpr(254)),
                        reg_byte(gpr(255)),
                      }
  );
}

TEST_CASE("Bytecode_writer emits lookup constant instruction operands")
{
  using benson::bytecode::Constant;
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_lookup_k(gpr(1), Constant{3});
  writer.emit_lookup_k(gpr(2), Constant{0x0405});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::lookup_k),
                        reg_byte(gpr(1)),
                        std::byte{0x03},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::lookup_k),
                        reg_byte(gpr(2)),
                        std::byte{0x00},
                        std::byte{0x05},
                        std::byte{0x04},
                      }
  );
}

TEST_CASE("Bytecode_writer emits move instruction operands")
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_mov(gpr(1), gpr(2));
  writer.emit_mov_i(gpr(3), Immediate{-4});
  writer.emit_mov_i(gpr(4), Immediate{0x0102});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::mov),
                        reg_byte(gpr(1)),
                        reg_byte(gpr(2)),
                        static_cast<std::byte>(Opcode::mov_i),
                        reg_byte(gpr(3)),
                        std::byte{0xFC},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::mov_i),
                        reg_byte(gpr(4)),
                        std::byte{0x00},
                        std::byte{0x02},
                        std::byte{0x01},
                      }
  );
}

TEST_CASE("Bytecode_writer emits binary arithmetic instruction operands")
{
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_add_i32(gpr(1), gpr(2), gpr(3));
  writer.emit_mod_i64(gpr(4), gpr(5), gpr(6));
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::add_i32),
                        reg_byte(gpr(1)),
                        reg_byte(gpr(2)),
                        reg_byte(gpr(3)),
                        static_cast<std::byte>(Opcode::mod_i64),
                        reg_byte(gpr(4)),
                        reg_byte(gpr(5)),
                        reg_byte(gpr(6)),
                      }
  );
}

TEST_CASE("Bytecode_writer emits wide register instruction operands")
{
  using benson::bytecode::Constant;
  using benson::bytecode::Immediate;
  using enum benson::bytecode::Opcode;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_add_i32(gpr(256), gpr(257), gpr(258));
  writer.emit_mov_i(gpr(300), Immediate{3});
  writer.emit_lookup_k(gpr(301), Constant{4});
  writer.emit_load_32(gpr(302), sp, Immediate{0});
  writer.emit_store_32(gpr(302), sp, Immediate{0});
  writer.emit_jnz(gpr(300), std::ptrdiff_t{43});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(add_i32),
                        std::byte{0x00},
                        std::byte{0x01},
                        std::byte{0x01},
                        std::byte{0x01},
                        std::byte{0x02},
                        std::byte{0x01},
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(mov_i),
                        std::byte{0x2C},
                        std::byte{0x01},
                        std::byte{0x03},
                        std::byte{0x00},
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(lookup_k),
                        std::byte{0x2D},
                        std::byte{0x01},
                        std::byte{0x04},
                        std::byte{0x00},
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(load_32),
                        std::byte{0x2E},
                        std::byte{0x01},
                        std::byte{0x00},
                        std::byte{0x00},
                        std::byte{0x00},
                        std::byte{0x00},
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(store_32),
                        std::byte{0x2E},
                        std::byte{0x01},
                        std::byte{0x00},
                        std::byte{0x00},
                        std::byte{0x00},
                        std::byte{0x00},
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(jnz_i),
                        std::byte{0x2C},
                        std::byte{0x01},
                        std::byte{0x01},
                        std::byte{0x00},
                      }
  );
}

TEST_CASE("Bytecode_writer emits jnz_i instruction operands")
{
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_jnz(gpr(3), std::ptrdiff_t{5});
  writer.emit_jnz(gpr(4), std::ptrdiff_t{0x0108});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::jnz_i),
                        reg_byte(gpr(3)),
                        std::byte{0x02},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::jnz_i),
                        reg_byte(gpr(4)),
                        std::byte{0x00},
                        std::byte{0xFF},
                        std::byte{0x00},
                      }
  );
}

TEST_CASE("Bytecode_writer emits call_i and ret instruction operands")
{
  using benson::bytecode::Opcode;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_call(std::ptrdiff_t{5});
  writer.emit_call(std::ptrdiff_t{0x0108});
  writer.emit_ret();
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::call_i),
                        std::byte{0x03},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::call_i),
                        std::byte{0x02},
                        std::byte{0x01},
                        static_cast<std::byte>(Opcode::ret),
                      }
  );
}

TEST_CASE(
  "Bytecode_writer emits binary arithmetic immediate instruction operands"
)
{
  using benson::bytecode::Immediate;
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_add_i32_i(gpr(1), gpr(2), Immediate{-3});
  writer.emit_mul_i32_i(gpr(4), gpr(5), Immediate{0x0102});
  writer.emit_mod_i64_i(gpr(7), gpr(8), Immediate{-0x0203});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::add_i32_i),
                        reg_byte(gpr(1)),
                        reg_byte(gpr(2)),
                        std::byte{0xFD},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::mul_i32_i),
                        reg_byte(gpr(4)),
                        std::byte{0x00},
                        reg_byte(gpr(5)),
                        std::byte{0x00},
                        std::byte{0x02},
                        std::byte{0x01},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::mod_i64_i),
                        reg_byte(gpr(7)),
                        std::byte{0x00},
                        reg_byte(gpr(8)),
                        std::byte{0x00},
                        std::byte{0xFD},
                        std::byte{0xFD},
                      }
  );
}

TEST_CASE(
  "Bytecode_writer emits binary arithmetic constant instruction operands"
)
{
  using benson::bytecode::Constant;
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_sub_i32_k(gpr(1), gpr(2), Constant{3});
  writer.emit_div_f64_k(gpr(4), gpr(5), Constant{6});
  writer.emit_mod_i32_k(gpr(7), gpr(8), Constant{9});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::sub_i32_k),
                        reg_byte(gpr(1)),
                        reg_byte(gpr(2)),
                        std::byte{3},
                        static_cast<std::byte>(Opcode::div_f64_k),
                        reg_byte(gpr(4)),
                        reg_byte(gpr(5)),
                        std::byte{6},
                        static_cast<std::byte>(Opcode::mod_i32_k),
                        reg_byte(gpr(7)),
                        reg_byte(gpr(8)),
                        std::byte{9},
                      }
  );
}

TEST_CASE(
  "Bytecode_writer emits wide binary arithmetic constant instruction operands"
)
{
  using benson::bytecode::Constant;
  using benson::bytecode::Opcode;
  using benson::bytecode::Register;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_sub_i32_k(gpr(1), gpr(2), Constant{0x0304});
  writer.emit_div_f64_k(gpr(4), gpr(5), Constant{0x0607});
  writer.emit_mod_i32_k(gpr(7), gpr(8), Constant{0x090A});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::sub_i32_k),
                        reg_byte(gpr(1)),
                        std::byte{0x00},
                        reg_byte(gpr(2)),
                        std::byte{0x00},
                        std::byte{0x04},
                        std::byte{0x03},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::div_f64_k),
                        reg_byte(gpr(4)),
                        std::byte{0x00},
                        reg_byte(gpr(5)),
                        std::byte{0x00},
                        std::byte{0x07},
                        std::byte{0x06},
                        static_cast<std::byte>(Opcode::wide),
                        static_cast<std::byte>(Opcode::mod_i32_k),
                        reg_byte(gpr(7)),
                        std::byte{0x00},
                        reg_byte(gpr(8)),
                        std::byte{0x00},
                        std::byte{0x0A},
                        std::byte{0x09},
                      }
  );
}

TEST_CASE("Bytecode_writer emits binary comparison instruction operands")
{
  using enum benson::bytecode::Opcode;
  using benson::bytecode::gpr;
  using benson::bytecode::sp;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_cmp_eq_i32(gpr(1), gpr(2), gpr(3));
  writer.emit_cmp_ne_i64(gpr(4), gpr(5), gpr(6));
  writer.emit_cmp_lt_f32(gpr(7), gpr(8), gpr(9));
  writer.emit_cmp_ge_f64(gpr(10), gpr(11), gpr(12));
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(cmp_eq_i32),
                        reg_byte(gpr(1)),
                        reg_byte(gpr(2)),
                        reg_byte(gpr(3)),
                        static_cast<std::byte>(cmp_ne_i64),
                        reg_byte(gpr(4)),
                        reg_byte(gpr(5)),
                        reg_byte(gpr(6)),
                        static_cast<std::byte>(cmp_lt_f32),
                        reg_byte(gpr(7)),
                        reg_byte(gpr(8)),
                        reg_byte(gpr(9)),
                        static_cast<std::byte>(cmp_ge_f64),
                        reg_byte(gpr(10)),
                        reg_byte(gpr(11)),
                        reg_byte(gpr(12)),
                      }
  );
}

TEST_CASE(
  "Bytecode_writer emits binary comparison constant and immediate instruction "
  "operands"
)
{
  using benson::bytecode::Constant;
  using benson::bytecode::Immediate;
  using enum benson::bytecode::Opcode;
  using benson::bytecode::gpr;
  using benson::bytecode::sp;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_cmp_eq_i32_k(gpr(1), gpr(2), Constant{3});
  writer.emit_cmp_ne_i64_i(gpr(4), gpr(5), Immediate{-7});
  writer.emit_cmp_lt_f32_k(gpr(6), gpr(7), Constant{0x0102});
  writer.emit_cmp_ge_i32_i(gpr(8), gpr(9), Immediate{0x0304});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(cmp_eq_i32_k),
                        reg_byte(gpr(1)),
                        reg_byte(gpr(2)),
                        std::byte{0x03},
                        static_cast<std::byte>(cmp_ne_i64_i),
                        reg_byte(gpr(4)),
                        reg_byte(gpr(5)),
                        std::byte{0xF9},
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(cmp_lt_f32_k),
                        reg_byte(gpr(6)),
                        std::byte{0x00},
                        reg_byte(gpr(7)),
                        std::byte{0x00},
                        std::byte{0x02},
                        std::byte{0x01},
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(cmp_ge_i32_i),
                        reg_byte(gpr(8)),
                        std::byte{0x00},
                        reg_byte(gpr(9)),
                        std::byte{0x00},
                        std::byte{0x04},
                        std::byte{0x03},
                      }
  );
}

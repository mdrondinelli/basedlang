#include <cstddef>
#include <span>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "bytecode/bytecode_writer.h"

namespace
{
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

TEST_CASE("Bytecode_writer emits zero-based register operands")
{
  using benson::bytecode::gpr;
  using enum benson::bytecode::Opcode;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_mov(gpr(0), gpr(1));
  writer.emit_add_i32(gpr(2), gpr(0), gpr(1));
  writer.emit_exit();
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(mov),
                        reg_byte(gpr(0)),
                        reg_byte(gpr(1)),
                        static_cast<std::byte>(add_i32),
                        reg_byte(gpr(2)),
                        reg_byte(gpr(0)),
                        reg_byte(gpr(1)),
                        static_cast<std::byte>(exit),
                      }
  );
}

TEST_CASE("Bytecode_writer emits indexed call and return operands")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using enum benson::bytecode::Opcode;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_call_i(Immediate{3}, gpr(4), gpr(5));
  writer.emit_call_void_i(Immediate{6}, gpr(7));
  writer.emit_ret(gpr(8));
  writer.emit_ret_void();
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(call_i),
                        std::byte{3},
                        reg_byte(gpr(4)),
                        reg_byte(gpr(5)),
                        static_cast<std::byte>(call_void_i),
                        std::byte{6},
                        reg_byte(gpr(7)),
                        static_cast<std::byte>(ret),
                        reg_byte(gpr(8)),
                        static_cast<std::byte>(ret_void),
                      }
  );
}

TEST_CASE("Bytecode_writer emits wide indexed call and return operands")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using enum benson::bytecode::Opcode;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_call_i(Immediate{0x0102}, gpr(300), gpr(301));
  writer.emit_call_void_i(Immediate{0x0203}, gpr(302));
  writer.emit_ret(gpr(303));
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(call_i),
                        std::byte{0x02},
                        std::byte{0x01},
                        std::byte{0x2C},
                        std::byte{0x01},
                        std::byte{0x2D},
                        std::byte{0x01},
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(call_void_i),
                        std::byte{0x03},
                        std::byte{0x02},
                        std::byte{0x2E},
                        std::byte{0x01},
                        static_cast<std::byte>(wide),
                        static_cast<std::byte>(ret),
                        std::byte{0x2F},
                        std::byte{0x01},
                      }
  );
}

TEST_CASE("Bytecode_writer emits stack pointer operations")
{
  using benson::bytecode::gpr;
  using benson::bytecode::Immediate;
  using enum benson::bytecode::Opcode;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_push_sp_i(Immediate{24});
  writer.emit_push_sp(gpr(1));
  writer.emit_mov_sp_i(gpr(2), Immediate{8});
  writer.emit_store_sp_64(gpr(3), Immediate{0});
  writer.emit_load_sp_64(gpr(4), Immediate{0});
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(push_sp_i),
                        std::byte{24},
                        static_cast<std::byte>(push_sp),
                        reg_byte(gpr(1)),
                        static_cast<std::byte>(mov_sp_i),
                        reg_byte(gpr(2)),
                        std::byte{8},
                        static_cast<std::byte>(store_sp_64),
                        reg_byte(gpr(3)),
                        std::byte{0},
                        static_cast<std::byte>(load_sp_64),
                        reg_byte(gpr(4)),
                        std::byte{0},
                      }
  );
}

TEST_CASE("Bytecode_writer emits branch and constant instructions")
{
  using benson::bytecode::Constant;
  using benson::bytecode::gpr;
  using enum benson::bytecode::Opcode;

  auto stream = Recording_output_stream{};
  auto writer = benson::bytecode::Bytecode_writer{&stream};

  writer.emit_lookup_k(gpr(0), Constant{1});
  writer.emit_jnz(gpr(0), std::ptrdiff_t{8});
  writer.emit_jmp(std::ptrdiff_t{10});
  writer.emit_exit();
  writer.flush();

  CHECK(
    stream.bytes() == std::vector<std::byte>{
                        static_cast<std::byte>(lookup_k),
                        reg_byte(gpr(0)),
                        std::byte{1},
                        static_cast<std::byte>(jnz_i),
                        reg_byte(gpr(0)),
                        std::byte{2},
                        static_cast<std::byte>(jmp_i),
                        std::byte{2},
                        static_cast<std::byte>(exit),
                      }
  );
}

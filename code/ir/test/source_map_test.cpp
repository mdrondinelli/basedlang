#include <algorithm>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#include "ir/hlir.h"

namespace
{

  benson::Source_span span_at(std::int32_t line, std::int32_t column)
  {
    return benson::Source_span{
      .start = benson::Source_location{.line = line, .column = column},
      .end = benson::Source_location{.line = line, .column = column},
    };
  }

  void check_span(
    std::optional<benson::Source_span> const &span,
    std::int32_t line,
    std::int32_t column
  )
  {
    REQUIRE(span.has_value());
    CHECK(span->start.line == line);
    CHECK(span->start.column == column);
    CHECK(span->end.line == line);
    CHECK(span->end.column == column);
  }

} // namespace

TEST_CASE("Instruction - source span is stored on the instruction")
{
  auto const result = benson::ir::Register{0};
  auto instruction = benson::ir::Instruction{
    benson::ir::Constant_instruction<std::int32_t>{
      .result = result,
      .value = 1,
    },
    span_at(1, 2),
  };

  check_span(instruction.source_span, 1, 2);
}

TEST_CASE("Terminator - source span is stored on the terminator")
{
  auto terminator = benson::ir::Terminator{
    benson::ir::Return_terminator{
      .value = benson::ir::Constant_value{std::int32_t{1}},
    },
    span_at(3, 4),
  };

  check_span(terminator.source_span, 3, 4);
}

TEST_CASE("Instruction - source span follows instruction when reordered")
{
  auto const first_result = benson::ir::Register{0};
  auto const second_result = benson::ir::Register{1};
  auto block = benson::ir::Basic_block{};
  block.instructions.push_back(
    benson::ir::Instruction{
      benson::ir::Constant_instruction<std::int32_t>{
        .result = first_result,
        .value = 1,
      },
      span_at(1, 2),
    }
  );
  block.instructions.push_back(
    benson::ir::Instruction{
      benson::ir::Constant_instruction<std::int32_t>{
        .result = second_result,
        .value = 2,
      },
      span_at(3, 4),
    }
  );

  std::ranges::swap(block.instructions[0], block.instructions[1]);

  check_span(block.instructions[0].source_span, 3, 4);
  check_span(block.instructions[1].source_span, 1, 2);
}

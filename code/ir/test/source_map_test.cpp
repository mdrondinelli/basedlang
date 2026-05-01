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

} // namespace

TEST_CASE(
  "Source_map - instruction and terminator sites are tracked separately"
)
{
  auto source_map = benson::ir::Source_map{};
  auto function = benson::ir::Function{};
  auto block = benson::ir::Basic_block{};
  auto const instruction_site = benson::ir::Instruction_site{
    .function = &function,
    .block = &block,
    .instruction_index = 0,
  };
  auto const terminator_site = benson::ir::Terminator_site{
    .function = &function,
    .block = &block,
  };
  source_map.add(instruction_site, span_at(1, 2));
  source_map.add(terminator_site, span_at(3, 4));

  auto const instruction_span = source_map.lookup(instruction_site);
  REQUIRE(instruction_span.has_value());
  CHECK(instruction_span->start.line == 1);
  CHECK(instruction_span->start.column == 2);

  auto const terminator_span = source_map.lookup(terminator_site);
  REQUIRE(terminator_span.has_value());
  CHECK(terminator_span->start.line == 3);
  CHECK(terminator_span->start.column == 4);
}

TEST_CASE("Source_map - missing sites return nullopt")
{
  auto source_map = benson::ir::Source_map{};
  auto function = benson::ir::Function{};
  auto block = benson::ir::Basic_block{};
  auto other_block = benson::ir::Basic_block{};
  source_map.add(
    benson::ir::Instruction_site{
      .function = &function,
      .block = &block,
      .instruction_index = 0,
    },
    span_at(1, 1)
  );
  source_map.add(
    benson::ir::Terminator_site{
      .function = &function,
      .block = &block,
    },
    span_at(2, 1)
  );

  CHECK_FALSE(source_map
                .lookup(
                  benson::ir::Instruction_site{
                    .function = &function,
                    .block = &block,
                    .instruction_index = 1,
                  }
                )
                .has_value());
  CHECK_FALSE(source_map
                .lookup(
                  benson::ir::Terminator_site{
                    .function = &function,
                    .block = &other_block,
                  }
                )
                .has_value());
}

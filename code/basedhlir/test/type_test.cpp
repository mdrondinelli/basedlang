#include <catch2/catch_test_macros.hpp>

#include "basedhlir/type.h"

TEST_CASE("Type_pool - primitive types are lazily created")
{
  auto pool = basedhlir::Type_pool{};
  auto const i32 = pool.i32_type();
  auto const bool_ = pool.bool_type();
  auto const void_ = pool.void_type();
  CHECK(i32 != nullptr);
  CHECK(bool_ != nullptr);
  CHECK(void_ != nullptr);
  CHECK(i32 != bool_);
  CHECK(i32 != void_);
  CHECK(bool_ != void_);
}

TEST_CASE("Type_pool - same primitive type returns same pointer")
{
  auto pool = basedhlir::Type_pool{};
  CHECK(pool.i32_type() == pool.i32_type());
  CHECK(pool.bool_type() == pool.bool_type());
  CHECK(pool.void_type() == pool.void_type());
}

TEST_CASE("Type_pool - pointer type is interned via pointee")
{
  auto pool = basedhlir::Type_pool{};
  auto const i32 = pool.i32_type();
  auto const ptr = pool.pointer_type(i32);
  CHECK(ptr == pool.pointer_type(i32));
  CHECK(ptr != i32);
  auto const ptr_data = std::get_if<basedhlir::Pointer_type>(&ptr->data);
  REQUIRE(ptr_data != nullptr);
  CHECK(ptr_data->pointee == i32);
}

TEST_CASE("Type_pool - pointer to different types are distinct")
{
  auto pool = basedhlir::Type_pool{};
  auto const ptr_i32 = pool.pointer_type(pool.i32_type());
  auto const ptr_bool = pool.pointer_type(pool.bool_type());
  CHECK(ptr_i32 != ptr_bool);
}

TEST_CASE("Type_pool - pointer to pointer")
{
  auto pool = basedhlir::Type_pool{};
  auto const i32 = pool.i32_type();
  auto const ptr = pool.pointer_type(i32);
  auto const ptr_ptr = pool.pointer_type(ptr);
  CHECK(ptr_ptr != ptr);
  CHECK(ptr_ptr == pool.pointer_type(ptr));
}

TEST_CASE("Type_pool - sized array type is interned by element and size")
{
  auto pool = basedhlir::Type_pool{};
  auto const i32 = pool.i32_type();
  auto const arr = pool.sized_array_type(i32, 10);
  CHECK(arr == pool.sized_array_type(i32, 10));
  CHECK(arr != pool.sized_array_type(i32, 20));
  auto const arr_data = std::get_if<basedhlir::Sized_array_type>(&arr->data);
  REQUIRE(arr_data != nullptr);
  CHECK(arr_data->element == i32);
  CHECK(arr_data->size == 10);
}

TEST_CASE("Type_pool - unsized array type is interned by element")
{
  auto pool = basedhlir::Type_pool{};
  auto const i32 = pool.i32_type();
  auto const arr = pool.unsized_array_type(i32);
  CHECK(arr == pool.unsized_array_type(i32));
  CHECK(arr != pool.sized_array_type(i32, 10));
  auto const arr_data = std::get_if<basedhlir::Unsized_array_type>(&arr->data);
  REQUIRE(arr_data != nullptr);
  CHECK(arr_data->element == i32);
}

TEST_CASE("Type_pool - array of pointers")
{
  auto pool = basedhlir::Type_pool{};
  auto const ptr_i32 = pool.pointer_type(pool.i32_type());
  auto const arr = pool.sized_array_type(ptr_i32, 5);
  auto const arr_data = std::get_if<basedhlir::Sized_array_type>(&arr->data);
  REQUIRE(arr_data != nullptr);
  CHECK(arr_data->element == ptr_i32);
}

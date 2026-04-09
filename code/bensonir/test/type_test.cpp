#include <catch2/catch_test_macros.hpp>

#include "bensonir/type.h"

TEST_CASE("Type_pool - primitive types are lazily created")
{
  auto pool = benson::ir::Type_pool{};
  auto const int32 = pool.int32_type();
  auto const bool_ = pool.bool_type();
  auto const void_ = pool.void_type();
  CHECK(int32 != nullptr);
  CHECK(bool_ != nullptr);
  CHECK(void_ != nullptr);
  CHECK(int32 != bool_);
  CHECK(int32 != void_);
  CHECK(bool_ != void_);
}

TEST_CASE("Type_pool - same primitive type returns same pointer")
{
  auto pool = benson::ir::Type_pool{};
  CHECK(pool.int32_type() == pool.int32_type());
  CHECK(pool.bool_type() == pool.bool_type());
  CHECK(pool.void_type() == pool.void_type());
}

TEST_CASE("Type_pool - pointer type is interned via pointee")
{
  auto pool = benson::ir::Type_pool{};
  auto const int32 = pool.int32_type();
  auto const ptr = pool.pointer_type(int32, false);
  CHECK(ptr == pool.pointer_type(int32, false));
  CHECK(ptr != int32);
  auto const ptr_data = std::get_if<benson::ir::Pointer_type>(&ptr->data);
  REQUIRE(ptr_data != nullptr);
  CHECK(ptr_data->pointee == int32);
  CHECK(ptr_data->is_mutable == false);
}

TEST_CASE("Type_pool - mutable pointer type is interned via pointee")
{
  auto pool = benson::ir::Type_pool{};
  auto const int32 = pool.int32_type();
  auto const ptr = pool.pointer_type(int32, true);
  CHECK(ptr == pool.pointer_type(int32, true));
  CHECK(ptr != int32);
  auto const ptr_data = std::get_if<benson::ir::Pointer_type>(&ptr->data);
  REQUIRE(ptr_data != nullptr);
  CHECK(ptr_data->pointee == int32);
  CHECK(ptr_data->is_mutable == true);
}

TEST_CASE("Type_pool - mutable and non-mutable pointers are distinct")
{
  auto pool = benson::ir::Type_pool{};
  auto const int32 = pool.int32_type();
  CHECK(pool.pointer_type(int32, false) != pool.pointer_type(int32, true));
}

TEST_CASE("Type_pool - pointer to different types are distinct")
{
  auto pool = benson::ir::Type_pool{};
  auto const ptr_int32 = pool.pointer_type(pool.int32_type(), false);
  auto const ptr_bool = pool.pointer_type(pool.bool_type(), false);
  CHECK(ptr_int32 != ptr_bool);
}

TEST_CASE("Type_pool - pointer to pointer")
{
  auto pool = benson::ir::Type_pool{};
  auto const int32 = pool.int32_type();
  auto const ptr = pool.pointer_type(int32, false);
  auto const ptr_ptr = pool.pointer_type(ptr, false);
  CHECK(ptr_ptr != ptr);
  CHECK(ptr_ptr == pool.pointer_type(ptr, false));
}

TEST_CASE("Type_pool - sized array type is interned by element and size")
{
  auto pool = benson::ir::Type_pool{};
  auto const int32 = pool.int32_type();
  auto const arr = pool.sized_array_type(int32, 10);
  CHECK(arr == pool.sized_array_type(int32, 10));
  CHECK(arr != pool.sized_array_type(int32, 20));
  auto const arr_data = std::get_if<benson::ir::Sized_array_type>(&arr->data);
  REQUIRE(arr_data != nullptr);
  CHECK(arr_data->element == int32);
  CHECK(arr_data->size == 10);
}

TEST_CASE("Type_pool - unsized array type is interned by element")
{
  auto pool = benson::ir::Type_pool{};
  auto const int32 = pool.int32_type();
  auto const arr = pool.unsized_array_type(int32);
  CHECK(arr == pool.unsized_array_type(int32));
  CHECK(arr != pool.sized_array_type(int32, 10));
  auto const arr_data = std::get_if<benson::ir::Unsized_array_type>(&arr->data);
  REQUIRE(arr_data != nullptr);
  CHECK(arr_data->element == int32);
}

TEST_CASE("Type_pool - array of pointers")
{
  auto pool = benson::ir::Type_pool{};
  auto const ptr_int32 = pool.pointer_type(pool.int32_type(), false);
  auto const arr = pool.sized_array_type(ptr_int32, 5);
  auto const arr_data = std::get_if<benson::ir::Sized_array_type>(&arr->data);
  REQUIRE(arr_data != nullptr);
  CHECK(arr_data->element == ptr_int32);
}

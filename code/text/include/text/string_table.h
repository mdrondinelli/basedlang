#ifndef BASEDTEXT_STRING_TABLE_H
#define BASEDTEXT_STRING_TABLE_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace benson
{

  class String_table;

  // Interned_string is only valid while its originating String_table remains
  // alive.
  class Interned_string
  {
  public:
    Interned_string() = default;

    Interned_string(Interned_string const &) = delete;

    Interned_string &operator=(Interned_string const &) = delete;

    Interned_string(Interned_string &&other) noexcept
        : _table{std::exchange(other._table, nullptr)},
          _row{std::exchange(other._row, nullptr)}
    {
    }

    Interned_string &operator=(Interned_string &&other) noexcept
    {
      if (this == &other)
      {
        return *this;
      }
      release();
      _table = std::exchange(other._table, nullptr);
      _row = std::exchange(other._row, nullptr);
      return *this;
    }

    ~Interned_string()
    {
      release();
    }

    [[nodiscard]] auto empty() const noexcept -> bool
    {
      return _row == nullptr;
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
      return !empty();
    }

    [[nodiscard]] auto view() const noexcept -> std::u32string_view;

    [[nodiscard]] auto clone() -> Interned_string;

    friend auto
    operator==(Interned_string const &lhs, Interned_string const &rhs) noexcept
      -> bool
    {
      return lhs._table == rhs._table && lhs._row == rhs._row;
    }

  private:
    struct Row
    {
      std::u32string spelling;
      std::int32_t refcount{};
    };

    friend class String_table;
    friend struct Interned_string_hash;
    friend struct std::hash<Interned_string>;

    Interned_string(String_table *table, Row *row) noexcept
        : _table{table}, _row{row}
    {
    }

    void release() noexcept;

    String_table *_table{};
    Row *_row{};
  };

  struct Interned_string_hash
  {
    [[nodiscard]] auto operator()(Interned_string const &value) const noexcept
      -> std::size_t
    {
      auto const row_hash = std::hash<Interned_string::Row *>{}(value._row);
      auto const table_hash = std::hash<String_table *>{}(value._table);
      return row_hash ^
             (table_hash + 0x9e3779b9 + (row_hash << 6) + (row_hash >> 2));
    }
  };

  class String_table
  {
  public:
    String_table() = default;

    String_table(String_table const &) = delete;

    auto operator=(String_table const &) -> String_table & = delete;

    String_table(String_table &&) = delete;

    auto operator=(String_table &&) -> String_table & = delete;

    // The table must outlive every Interned_string it creates because handles
    // store raw pointers into table-owned rows.
    [[nodiscard]] auto intern(std::u32string_view spelling) -> Interned_string
    {
      if (auto const it = _rows.find(spelling); it != _rows.end())
      {
        ++it->second->refcount;
        return Interned_string{this, it->second.get()};
      }
      auto row = std::make_unique<Interned_string::Row>();
      row->spelling = std::u32string{spelling};
      row->refcount = 1;
      auto const key = std::u32string_view{row->spelling};
      auto *row_ptr = row.get();
      _rows.emplace(key, std::move(row));
      return Interned_string{this, row_ptr};
    }

    [[nodiscard]] auto row_count() const noexcept -> std::size_t
    {
      return _rows.size();
    }

  private:
    friend class Interned_string;

    void retain(Interned_string::Row *row) noexcept
    {
      assert(row != nullptr);
      ++row->refcount;
    }

    void release(Interned_string::Row *row) noexcept
    {
      assert(row != nullptr);
      assert(row->refcount > 0);
      --row->refcount;
      if (row->refcount == 0)
      {
        auto const it = _rows.find(std::u32string_view{row->spelling});
        assert(it != _rows.end());
        _rows.erase(it);
      }
    }

    std::unordered_map<
      std::u32string_view,
      std::unique_ptr<Interned_string::Row>,
      std::hash<std::u32string_view>
    >
      _rows;
  };

  inline auto Interned_string::view() const noexcept -> std::u32string_view
  {
    if (_row == nullptr)
    {
      return {};
    }
    return _row->spelling;
  }

  inline auto Interned_string::clone() -> Interned_string
  {
    if (_row == nullptr)
    {
      return {};
    }
    _table->retain(_row);
    return Interned_string{_table, _row};
  }

  inline void Interned_string::release() noexcept
  {
    if (_row == nullptr)
    {
      return;
    }
    _table->release(_row);
    _table = nullptr;
    _row = nullptr;
  }

} // namespace benson

namespace std
{
  template <>
  struct hash<benson::Interned_string>
  {
    [[nodiscard]] auto
    operator()(benson::Interned_string const &value) const noexcept
      -> std::size_t
    {
      return benson::Interned_string_hash{}(value);
    }
  };
} // namespace std

#endif

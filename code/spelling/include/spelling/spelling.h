#ifndef BASEDSPELLING_SPELLING_H
#define BASEDSPELLING_SPELLING_H

#include <bit>
#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace benson
{

  struct Spelling
  {
    std::uint32_t value{};

    [[nodiscard]] constexpr bool has_value() const noexcept
    {
      return value != 0;
    }

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
      return has_value();
    }

    friend auto operator==(Spelling, Spelling) -> bool = default;
  };

  class Spelling_table
  {
  public:
    class Builder
    {
    public:
      Builder(Builder const &) = delete;

      auto operator=(Builder const &) -> Builder & = delete;

      constexpr Builder(Builder &&other) noexcept
          : _table{std::exchange(other._table, nullptr)}
      {
      }

      constexpr auto operator=(Builder &&other) noexcept -> Builder &
      {
        auto temp{std::move(other)};
        swap(temp);
        return *this;
      }

      ~Builder()
      {
        if (_table != nullptr)
        {
          _table->cancel_build();
        }
      }

    private:
      constexpr void swap(Builder &other) noexcept
      {
        std::swap(_table, other._table);
      }

      void append(char c)
      {
        assert(_table != nullptr);
        _table->_bytes.push_back(c);
      }

      void append(std::string_view text)
      {
        assert(_table != nullptr);
        _table->_bytes.append(text);
      }

      [[nodiscard]] auto finalize() && -> Spelling
      {
        assert(_table != nullptr);
        auto const table = std::exchange(_table, nullptr);
        assert(table->_building);
        auto const text = table->build_view();
        auto const hash = hash_bytes(text);
        auto const mask = table->_buckets.size() - 1;
        auto index = hash & mask;
        for (;;)
        {
          auto const &bucket = table->_buckets[index];
          if (!bucket.has_value())
          {
            break;
          }
          auto const &entry = table->_entries[bucket.value - 1];
          if (table->entry_view(entry) == text)
          {
            table->cancel_build();
            return bucket;
          }
          index = (index + 1) & mask;
        }
        if ((table->_entries.size() + 1) * 3 > table->_buckets.size() * 2)
        {
          table->rehash(table->_buckets.size() * 2);
          auto const rehash_mask = table->_buckets.size() - 1;
          index = hash & rehash_mask;
          while (table->_buckets[index].has_value())
          {
            index = (index + 1) & rehash_mask;
          }
        }
        table->_entries.push_back(
          Entry{
            .offset = table->_building_start,
            .length = static_cast<std::uint32_t>(text.size()),
          }
        );
        auto const spelling = Spelling{
          static_cast<std::uint32_t>(table->_entries.size()),
        };
        table->_buckets[index] = spelling;
#ifndef NDEBUG
        table->_building = false;
#endif
        return spelling;
      }

    private:
      friend class Spelling_table;

      constexpr explicit Builder(Spelling_table *table) noexcept
          : _table{table}
      {
      }

      Spelling_table *_table{};
    };

    Spelling_table()
        : _buckets{}
    {
      _buckets.resize(32);
    }

    Spelling_table(Spelling_table const &) = delete;

    auto operator=(Spelling_table const &) -> Spelling_table & = delete;

    [[nodiscard]] auto builder() -> Builder
    {
      assert(!_building);
#ifndef NDEBUG
      _building = true;
#endif
      _building_start = static_cast<std::uint32_t>(_bytes.size());
      return Builder{this};
    }

    [[nodiscard]] auto lookup(Spelling spelling) const -> std::string_view
    {
      assert(spelling.has_value());
      assert(spelling.value <= _entries.size());
      auto const &entry = _entries[spelling.value - 1];
      return {
        _bytes.data() + entry.offset,
        entry.length,
      };
    }

    [[nodiscard]] auto size() const noexcept -> std::uint32_t
    {
      return static_cast<std::uint32_t>(_entries.size());
    }

    [[nodiscard]] auto bucket_count() const noexcept -> std::uint32_t
    {
      return static_cast<std::uint32_t>(_buckets.size());
    }

  private:
    struct Entry
    {
      std::uint32_t offset{};
      std::uint32_t length{};
    };

    [[nodiscard]] static auto hash_bytes(std::string_view text) noexcept
      -> std::size_t
    {
      auto hash = std::uint64_t{14695981039346656037ull};
      for (auto const c : text)
      {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ull;
      }
      return static_cast<std::size_t>(hash);
    }

    [[nodiscard]] static auto
    bucket_index(std::uint64_t hash, std::size_t bucket_count) noexcept
      -> std::size_t
    {
      assert(bucket_count > 0);
      assert(std::has_single_bit(bucket_count));
      return static_cast<std::size_t>(hash & (bucket_count - 1));
    }

    [[nodiscard]] auto entry_view(Entry const &entry) const noexcept
      -> std::string_view
    {
      return {
        _bytes.data() + entry.offset,
        entry.length,
      };
    }

    [[nodiscard]] auto build_view() const noexcept -> std::string_view
    {
      return entry_view(
        Entry{
          .offset = _building_start,
          .length = static_cast<std::uint32_t>(_bytes.size() - _building_start),
        }
      );
    }

    void cancel_build() noexcept
    {
      assert(_building);
      _bytes.resize(_building_start);
#ifndef NDEBUG
      _building = false;
#endif
    }

    void rehash(std::size_t bucket_count)
    {
      assert(bucket_count > 0);
      assert(std::has_single_bit(bucket_count));
      auto const mask = bucket_count - 1;
      auto buckets = std::vector<Spelling>{};
      buckets.resize(bucket_count);
      for (auto i = std::uint32_t{0}; i < _entries.size(); ++i)
      {
        auto const text = entry_view(_entries[i]);
        auto const hash = hash_bytes(text);
        auto index = bucket_index(hash, bucket_count);
        while (buckets[index].has_value())
        {
          index = (index + 1) & mask;
        }
        buckets[index].value = i + 1;
      }
      _buckets = std::move(buckets);
    }

    std::string _bytes;
    std::vector<Entry> _entries;
    std::vector<Spelling> _buckets;
    std::uint32_t _building_start{};
#ifndef NDEBUG
    bool _building{};
#endif
  };

} // namespace benson

#endif

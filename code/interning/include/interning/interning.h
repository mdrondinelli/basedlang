#ifndef BASEDTEXT_STRING_TABLE_H
#define BASEDTEXT_STRING_TABLE_H

#include <bit>
#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace benson
{

  class String_pool
  {
  public:
    String_pool()
        : String_pool{32}
    {
    }

    String_pool(String_pool const &) = delete;

    auto operator=(String_pool const &) -> String_pool & = delete;

    ~String_pool()
    {
      clear();
    }

    [[nodiscard]] auto size() const noexcept -> std::int64_t
    {
      return _size;
    }

    [[nodiscard]] auto bucket_count() const noexcept -> std::int64_t
    {
      return static_cast<std::int64_t>(_buckets.size());
    }

    auto rehash(std::int64_t bucket_count) -> void
    {
      assert(bucket_count > 0);
      assert(std::has_single_bit(static_cast<std::uint64_t>(bucket_count)));

      auto rebuilt = String_pool{bucket_count};

      auto node = _head;
      while (node != nullptr)
      {
        auto const next = node->next;
        node->prev = nullptr;
        node->next = nullptr;
        rebuilt.insert_node(node);
        node = next;
      }

      _head = nullptr;
      for (auto &bucket : _buckets)
      {
        bucket = nullptr;
      }
      _size = 0;

      swap(rebuilt);
    }

  private:
    struct Node
    {
      String_pool *owner{};
      Node *prev{};
      Node *next{};
      std::u32string intern;
      std::uint64_t hash{};
      std::int32_t refcount{};
    };

    friend class String_ref;

    [[nodiscard]] static std::int64_t
    index_for_hash(std::uint64_t hash, std::size_t bucket_count) noexcept
    {
      assert(bucket_count > 0);
      assert(std::has_single_bit(bucket_count));
      return static_cast<std::int64_t>(hash & (bucket_count - 1));
    }

    explicit String_pool(std::int64_t bucket_count)
        : _buckets(static_cast<std::size_t>(bucket_count), nullptr)
    {
      assert(bucket_count > 0);
      assert(std::has_single_bit(static_cast<std::uint64_t>(bucket_count)));
    }

    auto insert_node(Node *node) -> void
    {
      assert(node != nullptr);
      assert(node->prev == nullptr);
      assert(node->next == nullptr);
      auto const bucket_index = index_for_hash(node->hash, _buckets.size());
      auto const bucket_head = _buckets[bucket_index];
      if (bucket_head != nullptr)
      {
        node->prev = bucket_head->prev;
        node->next = bucket_head;
        bucket_head->prev = node;
        if (node->prev != nullptr)
        {
          node->prev->next = node;
        }
        else
        {
          _head = node;
        }
      }
      else
      {
        node->next = _head;
        if (_head != nullptr)
        {
          _head->prev = node;
        }
        _head = node;
      }
      _buckets[bucket_index] = node;
      ++_size;
    }


    void swap(String_pool &other) noexcept
    {
      std::swap(_buckets, other._buckets);
      std::swap(_head, other._head);
      std::swap(_size, other._size);
    }

    void destroy_node(Node *node) noexcept
    {
      assert(node != nullptr);
      auto const bucket_index = index_for_hash(node->hash, _buckets.size());
      if (_buckets[bucket_index] == node)
      {
        _buckets[bucket_index] = node->next =
          node->next != nullptr &&
              index_for_hash(node->next->hash, _buckets.size()) == bucket_index
            ? node->next
            : nullptr;
      }
      if (_head == node)
      {
        _head = node->next;
      }
      else
      {
        node->prev->next = node->next;
      }
      if (node->next != nullptr)
      {
        node->next->prev = node->prev;
      }
      delete node;
      --_size;
    }

    void clear() noexcept
    {
      auto node = _head;
      while (node != nullptr)
      {
        auto const next = node->next;
        delete node;
        node = next;
      }
      for (auto &bucket : _buckets)
      {
        bucket = nullptr;
      }
      _head = nullptr;
      _size = 0;
    }

    std::vector<Node *> _buckets;
    Node *_head{};
    std::int64_t _size{};
  };

  class String_ref
  {
  public:
    constexpr String_ref(String_ref &&other) noexcept
        : _node{std::exchange(other._node, nullptr)}
    {
    }

    constexpr String_ref &operator=(String_ref &&other) noexcept
    {
      auto temp{std::move(other)};
      swap(temp);
      return *this;
    }

    ~String_ref()
    {
      if (_node && --_node->refcount == 0)
      {
        _node->owner->destroy_node(_node);
      }
    }

    [[nodiscard]] operator bool() const noexcept
    {
      return _node != nullptr;
    }

    [[nodiscard]] bool has_value() const noexcept
    {
      return _node != nullptr;
    }

    [[nodiscard]] std::u32string_view view() const noexcept
    {
      assert(has_value());
      return _node->intern;
    }

    [[nodiscard]] String_ref clone() noexcept
    {
      return String_ref{_node};
    }

  private:
    constexpr explicit String_ref(String_pool::Node *node) noexcept
        : _node{node}
    {
      assert(node);
      ++_node->refcount;
    }

    constexpr void swap(String_ref &other) noexcept
    {
      std::swap(_node, other._node);
    }

    String_pool::Node *_node{};
  };

} // namespace benson

#endif

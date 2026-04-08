#ifndef BASEDLEX_RING_BUFFER_H
#define BASEDLEX_RING_BUFFER_H

#include <cstddef>
#include <new>
#include <utility>

namespace kalelex
{

  template <typename T>
  class Ring_buffer
  {
  public:
    Ring_buffer() noexcept
        : _buffer{nullptr}, _capacity{0}, _head{0}, _size{0}
    {
    }

    ~Ring_buffer()
    {
      for (auto i = std::size_t{0}; i < _size; ++i)
      {
        _buffer[(_head + i) & (_capacity - 1)].~T();
      }
      operator delete(_buffer);
    }

    Ring_buffer(Ring_buffer const &) = delete;
    Ring_buffer &operator=(Ring_buffer const &) = delete;

    void push_back(T value)
    {
      if (_size == _capacity)
      {
        auto const new_capacity =
          _capacity == 0 ? std::size_t{4} : _capacity * 2;
        auto * const new_buffer =
          static_cast<T *>(operator new(new_capacity * sizeof(T)));
        for (auto i = std::size_t{0}; i < _size; ++i)
        {
          new (new_buffer + i)
            T{std::move(_buffer[(_head + i) & (_capacity - 1)])};
          _buffer[(_head + i) & (_capacity - 1)].~T();
        }
        operator delete(_buffer);
        _buffer = new_buffer;
        _capacity = new_capacity;
        _head = 0;
      }
      new (_buffer + ((_head + _size) & (_capacity - 1))) T{std::move(value)};
      ++_size;
    }

    void pop_front()
    {
      _buffer[_head].~T();
      _head = (_head + 1) & (_capacity - 1);
      --_size;
    }

    T &operator[](std::size_t index)
    {
      return _buffer[(_head + index) & (_capacity - 1)];
    }

    T const &operator[](std::size_t index) const
    {
      return _buffer[(_head + index) & (_capacity - 1)];
    }

    std::size_t size() const noexcept
    {
      return _size;
    }

  private:
    T *_buffer;
    std::size_t _capacity;
    std::size_t _head;
    std::size_t _size;
  };

} // namespace kalelex

#endif // BASEDLEX_RING_BUFFER_H

#ifndef SHIFT_CORE_TEST_PROBE_HPP
#define SHIFT_CORE_TEST_PROBE_HPP

#include <cstdint>
#include <atomic>
#include <ostream>

template <typename T>
class probe
{
public:
  probe()
  {
    ++_counter;
  }

  probe(const probe& other) : _value(other._value)
  {
    ++_counter;
  }

  probe(probe&& other) noexcept
  {
    _value = other._value;
    other._value = 0;
    ++_counter;
  }

  probe(T value) : _value(value)
  {
    ++_counter;
  }

  ~probe()
  {
    --_counter;
  }

  probe& operator=(const probe& other)
  {
    _value = other._value;
    return *this;
  }

  probe& operator=(probe&& other) noexcept
  {
    _value = other._value;
    other._value = 0;
    return *this;
  }

  bool operator==(const probe& other) const
  {
    return _value == other._value;
  }

  bool operator!=(const probe& other) const
  {
    return !(*this == other);
  }

  T& value()
  {
    return _value;
  }

  const T& value() const
  {
    return _value;
  }

  static std::int32_t counter()
  {
    return _counter;
  }

private:
  T _value = T{};
  static std::atomic<std::int32_t> _counter;
};

template <typename T>
std::atomic<std::int32_t> probe<T>::_counter = ATOMIC_VAR_INIT(0);

namespace std
{
template <typename T>
ostream& operator<<(ostream& stream, const probe<T>& probe)
{
  stream << probe.value();
  return stream;
}
}

#endif

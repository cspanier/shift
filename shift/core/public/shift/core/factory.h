#ifndef SHIFT_CORE_FACTORY_H
#define SHIFT_CORE_FACTORY_H

#include <memory>

namespace shift::core
{
/// Factory policy class for abstract_factory and prototype_factory.
template <typename T, typename Base = T>
struct factory_using_raw_pointer
{
  using pointer_t = Base*;

  template <typename... Args>
  static pointer_t create_instance(Args&&... args)
  {
    return new T(std::forward<Args>(args)...);
  }
};

/// Factory policy class for abstract_factory and prototype_factory.
template <typename T, typename Base = T>
struct factory_using_unique_ptr
{
  using pointer_t = std::unique_ptr<Base>;

  template <typename... Args>
  static pointer_t create_instance(Args&&... args)
  {
    return std::unique_ptr<Base>(
      std::make_unique<T>(std::forward<Args>(args)...).release());
  }
};

/// Factory policy class for abstract_factory and prototype_factory.
template <typename T, typename Base = T>
struct factory_using_shared_ptr
{
  using pointer_t = std::shared_ptr<Base>;

  template <typename... Args>
  static pointer_t create_instance(Args&&... args)
  {
    return std::static_pointer_cast<Base>(
      std::make_shared<T>(std::forward<Args>(args)...));
  }
};
}

#endif

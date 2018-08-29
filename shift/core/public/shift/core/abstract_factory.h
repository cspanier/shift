#ifndef SHIFT_CORE_ABSTRACTFACTORY_H
#define SHIFT_CORE_ABSTRACTFACTORY_H

#include <memory>
#include <unordered_map>
#include <functional>
#include "shift/core/core.h"
#include "shift/core/mpl.h"
#include "shift/core/factory.h"

namespace shift::core
{
/// An abstract factory template class able to use any type of key and common
/// base class for the types offered to instantiate.
/// @tparam Key
///   The type of key which will be associated with each type.
/// @tparam Base
///   The base class for all types being registered with this factory.
/// @tparam Factory
///   A policy class of how to create and store instances.
/// ToDo: Use variadic templates to allow passing parameters to the type's
/// constructor.
template <typename Key, class Base, template <typename, typename> class Factory,
          typename... Args>
class abstract_factory
{
public:
  using key_t = Key;
  using pointer_t = typename Factory<Base, Base>::pointer_t;

  /// Registers a new type.
  /// @tparam T
  ///   The class to be registered, must be derived from Base.
  ///   T must implement a default constructor.
  /// @param key
  ///   A unique identifier which will be used to select this type later on.
  template <class T>
  void register_type(key_t key);

  /// Create a new instance associated with key.
  /// @param key
  ///   The unique identifier of the type to be constructed.
  /// @return
  ///   If key exists, a new instance of the assigned type is returned.
  ///   Otherwise null.
  pointer_t create_instance(key_t key, Args&&... args) const;

private:
  std::unordered_map<key_t, std::function<pointer_t(Args&&...)>> _factories;
};

template <typename Key, class Base, template <typename, typename> class Factory,
          typename... Args>
template <class T>
void abstract_factory<Key, Base, Factory, Args...>::register_type(
  typename abstract_factory<Key, Base, Factory, Args...>::key_t key)
{
  using namespace std::placeholders;
  BOOST_ASSERT(_factories.find(key) == _factories.end());
  _factories[key] = [](Args&&... args) {
    return Factory<T, Base>::create_instance(std::forward<Args>(args)...);
  };
}

template <typename Key, class Base, template <typename, typename> class Factory,
          typename... Args>
typename Factory<Base, Base>::pointer_t
abstract_factory<Key, Base, Factory, Args...>::create_instance(
  typename abstract_factory<Key, Base, Factory, Args...>::key_t key,
  Args&&... args) const
{
  auto factory_iterator = _factories.find(key);
  if (factory_iterator == _factories.end())
    return nullptr;
  return factory_iterator->second(std::forward<Args>(args)...);
}
}

#endif

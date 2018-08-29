#ifndef SHIFT_CORE_PROTOTYPEFACTORY_H
#define SHIFT_CORE_PROTOTYPEFACTORY_H

#include <memory>
#include <map>
#include "shift/core/core.h"

namespace shift::core
{
/// A generic prototype factory template.
/// @param Key
///   The type of key which will be associated with each prototype.
/// @param Base
///   The base class for all types being registered with this factory. It
///   needs to define the following method:
///   virtual Factory<Base>::pointer_t Base::clone().
/// @tparam Factory
///   A policy class of how to create and store instances.
template <typename Key, class Base, template <typename> class Factory>
class prototype_factory
{
public:
  using key_t = Key;

  /// Registers a new type.
  /// @param T
  ///   Must be a type derived from Base and overwriting the method
  ///   Base* Base::clone().
  /// @param key
  ///   A unique identifier which will be used to select this type later on.
  /// @param args
  ///   Any list of parameters the constructor expects.
  template <typename T, typename... Args>
  void register_type(key_t key, Args&&... args)
  {
    BOOST_ASSERT(_prototypes.find(key) == _prototypes.end());
    if (_prototypes.find(key) != _prototypes.end())
      return;
    _prototypes[key] = Factory<T>::create_instance(std::forward<Args>(args)...);
  }

  /// Registers a new custom built type.
  void register_type(key_t key, typename Factory<Base>::pointer_t prototype);

  /// Unregisters an existing type.
  void unregister(key_t key);

  /// Create a new instance associated with key.
  /// @param key
  ///    The unique identifier of the type to be constructed.
  /// @return
  ///    If key exists, a new instance of the assigned type is returned.
  ///    Otherwise nullptr.
  typename Factory<Base>::pointer_t create_instance(key_t key) const;

private:
  std::map<key_t, typename Factory<Base>::pointer_t> _prototypes;
};

template <typename Key, class Base>
void prototype_factory<Key, Base>::register_type(
  prototype_factory<Key, Base>::key_t key,
  typename Factory<Base>::pointer_t prototype)
{
  BOOST_ASSERT(_prototypes.find(key) == _prototypes.end());
  _prototypes[key] = std::move(prototype);
}

template <typename Key, class Base>
void prototype_factory<Key, Base>::unregister(
  prototype_factory<Key, Base>::key_t key)
{
  auto prototype = _prototypes.find(key);
  if (prototype != _prototypes.end())
    _prototypes.erase(prototype);
}

template <typename Key, class Base>
typename Factory<Base>::pointer_t prototype_factory<Key, Base>::create_instance(
  prototype_factory<Key, Base>::key_t key) const
{
  auto iterator = _prototypes.find(key);
  if (iterator != _prototypes.end())
    return iterator->second->clone();
  return nullptr;
}
}

#endif

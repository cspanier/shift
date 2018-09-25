#ifndef SHIFT_RENDER_VK_LAYER2_OBJECT_H
#define SHIFT_RENDER_VK_LAYER2_OBJECT_H

#include <unordered_set>

namespace shift::render::vk::layer2
{
///
enum class availability_state
{
  unloaded,
  loaded,
  updated
};

class parent_object
{
public:
  parent_object() noexcept = default;
  parent_object(const parent_object&) = delete;
  parent_object(parent_object&&) = delete;
  virtual ~parent_object() noexcept = 0;
  parent_object& operator=(const parent_object&) = delete;
  parent_object& operator=(parent_object&&) = delete;

  /// Signals state changes from dependent child objects.
  /// @remarks
  ///   Only the number of unavailable dependencies is tracked. You are not
  ///   supposed to call this manually.
  virtual void signal(availability_state state) = 0;

protected:
  std::size_t _unloaded_children = 0;
};

class child_object
{
public:
  /// Returns the object's availability state.
  availability_state availability() const noexcept;

  /// Updates the object's availability and signals registered parents
  /// accordingly.
  void availability(availability_state new_state);

  /// @pre
  ///   The renderer's _render_graph_mutex must be held by the current thread.
  /// @return
  ///   True, if this object is a new dependency to the passed parent and this
  ///   object is in unloaded state.
  bool add_parent(parent_object& parent);

  /// @pre
  ///   The renderer's _render_graph_mutex must be held by the current thread.
  /// @return
  ///   True, if this object was a dependency to the passed parent and this
  ///   object was in unloaded state.
  bool remove_parent(parent_object& parent);

protected:
  std::unordered_set<parent_object*> _parents;
  availability_state _availability = availability_state::unloaded;
};

class buffer;
class mesh;
class texture;
class sampler;
class material;
class model;
}

#endif

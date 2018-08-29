#ifndef SHIFT_PARSER_XML_SCHEME_H
#define SHIFT_PARSER_XML_SCHEME_H

#include <type_traits>
#include <memory>
#include <vector>
#include <unordered_map>
#include <shift/core/boost_disable_warnings.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/core.h>
#include <shift/core/types.h>
#include <shift/core/string_util.h>
#include <shift/log/log.h>
#include "shift/parser/xml/node.h"

namespace boost
{
template <>
inline bool lexical_cast<bool, std::string>(const std::string& string)
{
  std::istringstream ss(string);
  bool result;
  ss >> std::boolalpha >> result;
  return result;
}

template <>
inline std::string lexical_cast<std::string, bool>(const bool& value)
{
  std::ostringstream ss;
  ss << std::boolalpha << value;
  return ss.str();
}

template <typename T, ENABLE_IF(core::is_std_vector<T>::value)>
inline T lexical_cast(const std::string& string)
{
  T result;
  core::split(string, is_any_of(" \n\r"), [&](std::string token) {
    result.push_back(lexical_cast<T::value_type, std::string>(token));
  });
  return result;
}

template <typename T, ENABLE_IF(core::is_std_vector<T>::value)>
inline std::string lexical_cast(const T& values)
{
  std::ostringstream ss;
  bool first = true;
  for (const auto& value : values)
  {
    if (first)
      first = false;
    else
      ss << ' ';
    ss << lexical_cast<std::string>(value);
  }
  return ss.str();
}
}

namespace shift::parser::xml
{
template <typename Target, typename Source>
inline bool lexical_cast_wrapper(const Source& source, Target& target)
{
  return boost::conversion::try_lexical_convert(source, target);
}

/// An XML node scheme definition allowing automated transformation of XML
/// tree structures into custom types.
template <class T>
class Scheme
{
public:
  /// Registers a transformation of the current node's text data into the
  /// custom member type M.
  /// @param member
  ///   The member variable of type T to store the transformed value in.
  /// @param required
  ///   When set to false the value may be empty. Otherwise an exception of
  ///   type ParserException is being thrown if the text value is empty.
  /// @remarks
  ///   Only one text value transformation may be registered.
  template <typename M>
  Scheme& text(M T::*member, bool required)
  {
    _text = std::unique_ptr<BaseTransformation>(new Text<M>(member, required));
    return *this;
  }

  /// Registers a transformation of the named attribute value into the
  /// custom
  /// member type M.
  /// @param name
  ///   The name of the XML attribute.
  /// @param member
  ///   The member variable of type T to store the transformed value in.
  /// @param required
  ///   When set to false the attribute may be may be missing or empty.
  ///   Otherwise an exception of type ParserException is being thrown if
  ///   either of both cases appear.
  template <typename M, typename U>
  Scheme& attribute(const std::string& name, M U::*member, bool required)
  {
    _attributes.push_back(std::unique_ptr<BaseTransformation>(
      new Attribute<M>(name, static_cast<M T::*>(member), required)));
    return *this;
  }

  /// Registers a transformation of the named child node's text data into
  /// the
  /// custom member type M.
  /// @param name
  ///   The name of the XML node.
  /// @param member
  ///   The member variable of type T to store the transformed value in.
  /// @param required
  ///   When set to false the value may be empty and the named XML child
  ///   node
  ///   may be missing. Otherwise an exception of type ParserException is
  ///   being thrown if either of both cases appear.
  template <typename M>
  Scheme& childText(const std::string& name, M T::*member, bool required)
  {
    _attributes.push_back(std::unique_ptr<BaseTransformation>(
      new ChildText<M>(name, member, required)));
    return *this;
  }

  /// Registers a transformation of an XML node into the custom type C by
  /// storing it into a member variable of type M.
  /// @param name
  ///   The name of the XML node.
  /// @param member
  ///   The member variable of type T to store transformed children of type
  ///   C
  ///   in.
  /// @param scheme
  ///   The XML node's scheme definition.
  /// @param required
  ///   When set to false the node's arity is [0..1], otherwise exactly one
  ///   child node is required to be present. If the latter case fails an
  ///   exception of type ParserException is being thrown.
  template <typename M, class C>
  Scheme& child(const std::string& name, M T::*member, Scheme<C>& scheme,
                bool required)
  {
    _children.push_back(std::unique_ptr<BaseTransformation>(
      new Child<M, C>(name, member, scheme, required)));
    return *this;
  }

  /// Registers a transformation of XML nodes into the custom type C by
  /// storing these into a member variable of type std::vector<M>.
  /// @param name
  ///   The name of the XML node.
  /// @param member
  ///   The member vector variable of type T to store transformed children
  ///   of
  ///   type C in.
  /// @param scheme
  ///   The XML node's scheme definition.
  /// @param required
  ///   When set to false the node's arity is [0..*], otherwise at least one
  ///   child node is required to be present. If the latter case fails an
  ///   exception of type ParserException is being thrown.
  template <typename M, class C>
  Scheme& child(const std::string& name, std::vector<M> T::*member,
                Scheme<C>& scheme, bool required)
  {
    _children.push_back(std::unique_ptr<BaseTransformation>(
      new ChildVector<M, C>(name, member, scheme, required)));
    return *this;
  }

  /// Registers a transformation of XML nodes into the custom type C by
  /// storing these into a member variable of type std::unordered_map<K, M>
  /// while each element is identified by a key named keyName.
  /// @param name
  ///   The name of the XML node.
  /// @param member
  ///   The member vector variable of type T to store transformed children
  ///   of
  ///   type C in.
  /// @param scheme
  ///   The XML node's scheme definition.
  /// @param required
  ///   When set to false the node's arity is [0..*], otherwise at least one
  ///   child node is required to be present. If the latter case fails an
  ///   exception of type ParserException is being thrown.
  template <typename M, typename K, class C>
  Scheme& child(const std::string& name, std::unordered_map<K, M> T::*member,
                Scheme<C>& scheme, const std::string& keyName, bool required)
  {
    _children.push_back(std::unique_ptr<BaseTransformation>(
      new ChildMap<M, K, C>(name, member, scheme, keyName, required)));
    return *this;
  }

  /// Transforms the XML node into the user defined type T by enumerating
  /// all
  /// previously registered node attributes and children.
  bool transform(const node& source, T& instance)
  {
    if (_text)
    {
      if (!_text->transform(source, instance))
        return false;
    }
    for (const auto& attribute : _attributes)
    {
      if (!attribute->transform(source, instance))
        return false;
    }
    for (const auto& child : _children)
    {
      if (!child->transform(source, instance))
        return false;
    }
    return true;
  };

  /// Overload of Scheme::transform that dynamically creates an instance of
  /// type T and stors it in a std::unique_ptr.
  bool transform(const node& source, std::unique_ptr<T>& instance)
  {
    if (!instance)
      instance = std::make_unique<T>();
    return transform(source, *instance);
  };

  /// Overload of Scheme::transform that dynamically creates an instance of
  /// type T and stors it in a std::shared_ptr.
  bool transform(const node& source, std::shared_ptr<T>& instance)
  {
    if (!instance)
      instance = std::make_shared<T>();
    return transform(source, *instance);
  };

  /// Transforms the user defined type T into an XML node by enumerating all
  /// previously registered node attributes and children.
  bool transform(const T& instance, node& destination)
  {
    if (_text)
    {
      if (!_text->transform(instance, destination))
        return false;
    }
    for (auto& attribute : _attributes)
    {
      if (!attribute->transform(instance, destination))
        return false;
    }
    for (auto& child : _children)
    {
      if (!child->transform(instance, destination))
        return false;
    }
    return true;
  };

private:
  /// The base class for any transformations to perform on this XML node.
  class BaseTransformation
  {
  public:
    /// Performs the actual transformation.
    /// @param node
    ///   The XML node to perform the transformation on.
    /// @param instance
    ///   An instance of the custom structore type T that contains the
    ///   destination storage member variable.
    virtual bool transform(const node& source, T& instance) = 0;

    /// Performs the actual transformation.
    /// @param node
    ///   The XML node to perform the transformation on.
    /// @param instance
    ///   An instance of the custom structore type T that contains the
    ///   destination storage member variable.
    virtual bool transform(const T& instance, node& destination) = 0;
  };

  /// Transforms an XML node's text value into type M (e.g. <node>value
  /// </node>).
  template <class M>
  class Text : public BaseTransformation
  {
  public:
    /// Constructor.
    Text(M T::*member, bool required) : _member(member), _required(required)
    {
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const node& source, T& instance) override
    {
      if (!source.text.empty())
      {
        if (!lexical_cast_wrapper(source.text, instance.*_member))
        {
          core::warning() << "Failed interpreting text data.";
          return false;
        }
      }
      else if (_required)
      {
        core::warning() << "Missing required text data.";
        return false;
      }
      return true;
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const T& instance, node& destination) override
    {
      if (!(instance.*_member)->empty())
      {
        if (!lexical_cast_wrapper(instance.*_member, destination.text))
        {
          core::warning() << "Failed interpreting text data.";
          return false;
        }
      }
      else if (_required)
      {
        core::warning() << "Missing required text data.";
        return false;
      }
      return true;
    }

  private:
    M T::*_member;
    bool _required;
  };

  /// Reads a node's attribute (e.g. <node name="data" />).
  template <class M>
  class Attribute : public BaseTransformation
  {
  public:
    /// Constructor.
    Attribute(const std::string& name, M T::*member, bool required)
    : _name(name), _member(member), _required(required)
    {
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const node& source, T& instance) override
    {
      if (source.hasAttribute(_name))
      {
        if (!lexical_cast_wrapper(source.attribute(_name), instance.*_member))
        {
          core::warning() << "Failed interpreting value of attribute \""
                          << _name << "\".";
          return false;
        }
      }
      else if (_required)
      {
        core::warning() << "Missing required attribute \"" << _name << "\".";
        return false;
      }
      return true;
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const T& instance, node& destination) override
    {
      std::string value;
      if (!lexical_cast_wrapper(instance.*_member, value))
      {
        core::warning() << "Failed interpreting value of attribute \"" << _name
                        << "\".";
        return false;
      }
      destination.attributes[_name] = value;
      return true;
    }

  private:
    std::string _name;
    M T::*_member;
    bool _required;
  };

  /// Reads the text section of a child node (e.g. <node><child>data</child>
  /// </node>).
  template <class M>
  class ChildText : public BaseTransformation
  {
  public:
    /// Constructor.
    ChildText(const std::string& name, M T::*member, bool required)
    : _name(name), _member(member), _required(required)
    {
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const node& source, T& instance) override
    {
      auto* childNode = source.childByTagName(_name);
      if (childNode && !childNode->text().empty())
      {
        if (!lexical_cast_wrapper(childNode->text(), instance.*_member))
        {
          core::warning() << "Failed interpreting text data of child node "
                             "\""
                          << childNode->name() << "\".";
          return false;
        }
      }
      else if (_required)
      {
        core::warning() << "Missing required child node \"" << _name
                        << "\" or its text data.";
        return false;
      }
      return true;
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const T& instance, node& destination) override
    {
      std::string value;
      if (!lexical_cast_wrapper(instance.*_member, value))
      {
        core::warning() << "Failed interpreting text data of child node "
                           "\""
                        << _name << "\".";
        return false;
      }

      auto* childNode = destination.childByTagName(_name);
      if (!childNode && (_required || !value.empty()))
      {
        auto position =
          destination.children.insert(destination.children.end(), node(_name));
        childNode = boost::get<node>(&*position);
      }
      if (childNode)
        childNode->children.push_back(value);
      return true;
    }

  private:
    std::string _name;
    M T::*_member;
    bool _required;
  };

  /// Transforms a single child node.
  template <typename M, class C>
  class Child : public BaseTransformation
  {
  public:
    /// Constructor.
    Child(const std::string& name, M T::*member, Scheme<C>& scheme,
          bool required)
    : _name(name), _member(member), _scheme(scheme), _required(required)
    {
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const node& source, T& instance) override
    {
      auto* childNode = source.childByTagName(_name);
      if (childNode)
        return _scheme.transform(*childNode, instance.*_member);
      else if (_required)
      {
        core::warning() << "Missing required child node \"" << _name << "\".";
        return false;
      }
      return true;
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const T& instance, node& destination) override
    {
      auto* childNode = destination.childByTagName(_name);
      if (!childNode)
      {
        auto position =
          destination.children.insert(destination.children.end(), node(_name));
        childNode = boost::get<node>(&*position);
      }
      return _scheme.transform(instance.*_member, *childNode);
    }

  private:
    std::string _name;
    M T::*_member;
    Scheme<C>& _scheme;
    bool _required;
  };

  /// Transforms a list of child nodes all of the same type.
  template <typename M, class C>
  class ChildVector : public BaseTransformation
  {
  public:
    /// Constructor.
    ChildVector(const std::string& name, std::vector<M> T::*member,
                Scheme<C>& scheme, bool required)
    : _name(name), _member(member), _scheme(scheme), _required(required)
    {
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const node& source, T& instance) override
    {
      if (!source.findNodeByTagName(
            _name,
            [&](const node& childNode) {
              (instance.*_member).resize((instance.*_member).size() + 1);
              return _scheme.transform(childNode, (instance.*_member).back());
            },
            false))
      {
        return false;
      }

      if ((instance.*_member).empty() && _required)
      {
        core::warning() << "Missing required child node \"" << _name << "\".";
        return false;
      }
      return true;
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const T& instance, node& destination) override
    {
      if ((instance.*_member).empty() && _required)
      {
        core::warning() << "Missing required child node \"" << _name << "\".";
        return false;
      }
      for (const auto& value : instance.*_member)
      {
        auto position =
          destination.children.insert(destination.children.end(), node(_name));
        auto* childNode = boost::get<node>(&*position);
        BOOST_ASSERT(childNode);
        if (childNode)
        {
          if (!_scheme.transform(value, *childNode))
            return false;
        }
      }
      return true;
    }

  private:
    std::string _name;
    std::vector<M> T::*_member;
    Scheme<C>& _scheme;
    std::string _keyName;
    bool _required;
  };

  /// Transforms a map of child nodes all of the same type.
  template <typename M, typename K, class C>
  class ChildMap : public BaseTransformation
  {
  public:
    /// Constructor.
    ChildMap(const std::string& name, std::unordered_map<K, M> T::*map,
             Scheme<C>& scheme, const std::string& keyName, bool required)
    : _name(name),
      _map(map),
      _scheme(scheme),
      _keyName(keyName),
      _required(required)
    {
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const node& source, T& instance) override
    {
      if (!source.findNodeByTagName(
            _name,
            [&](const node& childNode) {
              // Skip children which are missing the required identification
              // key.
              if (!childNode.hasAttribute(_keyName))
              {
                core::warning()
                  << "Child nodes of type \"" << _name
                  << "\" require a key attribute named \"" << _keyName << "\".";
                return false;
              }
              K key;
              if (!lexical_cast_wrapper(childNode.attribute(_keyName), key))
              {
                core::warning() << "Failed interpreting value of attribute \""
                                << _keyName << "\".";
                return false;
              }
              auto existing = (instance.*_map).find(key);
              if (existing != (instance.*_map).end())
                return _scheme.transform(childNode, existing->second);
              else
              {
                (instance.*_map)[key] = M();
                return _scheme.transform(childNode, (instance.*_map)[key]);
              }
              return true;
            },
            false))
      {
        return false;
      }

      if ((instance.*_map).empty() && _required)
      {
        core::warning() << "Missing required child node \"" << _name << "\".";
        return false;
      }
      return true;
    }

    /// @see BaseTransformation::transform.
    virtual bool transform(const T& instance, node& destination) override
    {
      if ((instance.*_map).empty() && _required)
      {
        core::warning() << "Missing required child node \"" << _name << "\".";
        return false;
      }
      for (const auto& value : instance.*_map)
      {
        auto position =
          destination.children.insert(destination.children.end(), node(_name));
        auto* childNode = boost::get<node>(&*position);
        BOOST_ASSERT(childNode);
        if (childNode)
        {
          if (!_scheme.transform(value.second, *childNode))
            return false;
          childNode->attributes[_keyName] = value.first;
        }
      }
      return true;
    }

  private:
    std::string _name;
    std::unordered_map<K, M> T::*_map;
    Scheme<C>& _scheme;
    std::string _keyName;
    bool _required;
  };

private:
  std::unique_ptr<BaseTransformation> _text;
  std::vector<std::unique_ptr<BaseTransformation>> _attributes;
  std::vector<std::unique_ptr<BaseTransformation>> _children;
};
}

#endif

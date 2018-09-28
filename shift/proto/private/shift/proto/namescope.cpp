#include "shift/proto/namescope.h"
#include "shift/proto/hash.h"
#include <shift/proto/nodeunpacker.h>
#include <shift/proto/semanticanalyzer.h>
#include "shift/proto/grammar.h"
#include <shift/core/core.h>
#include <shift/platform/assert.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/crc.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <algorithm>
#include <cstdint>
#include <string>

namespace shift::proto
{
namescope::~namescope() = default;

type* namescope::find_type(const namescope_path& namescope_path,
                           std::size_t current_depth)
{
  if (namescope_path.size() <= current_depth)
    return nullptr;
  else if (namescope_path.size() == current_depth + 1)
  {
    for (auto& child : dynamic_children)
    {
      std::string type_name;
      if (const auto* alias = std::get_if<proto::alias>(child.get()))
        type_name = alias->name;
      else if (const auto* enumeration =
                 std::get_if<proto::enumeration>(child.get()))
      {
        type_name = enumeration->name;
      }
      else if (const auto* message = std::get_if<proto::message>(child.get()))
        type_name = message->name;
      else if (const auto* interface =
                 std::get_if<proto::interface>(child.get()))
      {
        type_name = interface->name;
      }
      else if (const auto* service = std::get_if<proto::service>(child.get()))
        type_name = service->name;
      else
        continue;

      if (namescope_path[current_depth] == type_name)
        return child.get();
    }
  }
  else
  {
    for (auto& child : dynamic_children)
    {
      if (auto* interface = std::get_if<proto::interface>(child.get()))
      {
        if (namescope_path[current_depth] == interface->name)
          return interface->find_type(namescope_path, current_depth + 1);
      }
      else if (auto* namescope = std::get_if<proto::namescope>(child.get()))
      {
        if (namescope_path[current_depth] == namescope->name)
          return namescope->find_type(namescope_path, current_depth + 1);
      }
    }
  }

  if (current_depth == 0 && (parent != nullptr))
    return parent->find_type(namescope_path);
  else
    return nullptr;
}

void namescope::parse(std::string_view content)
{
  auto begin = std::begin(content);
  auto end = std::end(content);

  skipper skipper;
  grammar grammar;
  qi::phrase_parse(begin, end, grammar, skipper, qi::skip_flag::postskip,
                   static_children);
}

void namescope::symbol_source(const std::string& source_path)
{
  for (auto& child : dynamic_children)
  {
    if (auto* alias = std::get_if<proto::alias>(child.get()))
    {
      alias->attribute("source_filename", source_path,
                       attribute_assignment::keep);
    }
    else if (auto* enumeration = std::get_if<proto::enumeration>(child.get()))
    {
      enumeration->attribute("source_filename", source_path,
                             attribute_assignment::keep);
    }
    else if (auto* message = std::get_if<proto::message>(child.get()))
    {
      message->attribute("source_filename", source_path,
                         attribute_assignment::keep);
    }
    else if (auto* interface = std::get_if<proto::interface>(child.get()))
    {
      interface->attribute("source_filename", source_path,
                           attribute_assignment::keep);
      for (auto& interface_message : interface->messages)
      {
        interface_message->attribute("source_filename", source_path,
                                     attribute_assignment::keep);
      }
    }
    else if (auto* service = std::get_if<proto::service>(child.get()))
    {
      service->attribute("source_filename", source_path,
                         attribute_assignment::keep);
    }
    else if (auto* namescope = std::get_if<proto::namescope>(child.get()))
      namescope->symbol_source(source_path);
    else
    {
      BOOST_ASSERT(false);
      return;
    }
  }
}

void namescope::merge()
{
  for (auto child_iter = static_children.begin();
       child_iter != static_children.end(); ++child_iter)
  {
    if (auto* namescope = std::get_if<proto::namescope>(&*child_iter))
    {
      // Merge name scopes with same name.
      auto next_child_iter = child_iter;
      ++next_child_iter;
      for (auto other_child_iter = next_child_iter;
           other_child_iter != static_children.end();)
      {
        auto* other_namescope =
          std::get_if<proto::namescope>(&*other_child_iter);
        if ((other_namescope != nullptr) &&
            namescope->name == other_namescope->name)
        {
          for (auto& other_attribute : other_namescope->public_attributes)
          {
            namescope->attribute(std::move(other_attribute),
                                 attribute_assignment::keep,
                                 attribute_visibility::internal);
          }

          for (auto& other_attribute : other_namescope->internal_attributes)
          {
            namescope->attribute(std::move(other_attribute),
                                 attribute_assignment::keep,
                                 attribute_visibility::published);
          }

          for (auto& other_sub_type : other_namescope->static_children)
            namescope->static_children.push_back(std::move(other_sub_type));

          other_child_iter = static_children.erase(other_child_iter);
        }
        else
          ++other_child_iter;
      }
      namescope->merge();
    }
  }
}

void namescope::convert_type_vectors()
{
  for (auto& child : static_children)
  {
    if (auto* namescope = std::get_if<proto::namescope>(&child))
      namescope->convert_type_vectors();
    else if (auto* interface = std::get_if<proto::interface>(&child))
      interface->convert_type_vectors();

    dynamic_children.emplace_back(
      std::make_shared<proto::type>(std::move(child)));
  }
  static_children.clear();
}

void namescope::unpack()
{
  for (auto& child : dynamic_children)
    std::visit(node_unpacker{*this}, *child);
}

void namescope::sort()
{
  std::sort(namescopes.begin(), namescopes.end(),
            [](const namescope* lhs, const namescope* rhs) {
              return lhs->name < rhs->name;
            });
  std::sort(
    aliases.begin(), aliases.end(),
    [](const alias* lhs, const alias* rhs) { return lhs->name < rhs->name; });
  std::sort(enumerations.begin(), enumerations.end(),
            [](const enumeration* lhs, const enumeration* rhs) {
              return lhs->name < rhs->name;
            });
  std::sort(messages.begin(), messages.end(),
            [](const message* lhs, const message* rhs) {
              return lhs->name < rhs->name;
            });
  std::sort(interfaces.begin(), interfaces.end(),
            [](const interface* lhs, const interface* rhs) {
              return lhs->name < rhs->name;
            });
  std::sort(services.begin(), services.end(),
            [](const service* lhs, const service* rhs) {
              return lhs->name < rhs->name;
            });

  for (auto* namescope : namescopes)
    namescope->sort();
}

bool namescope::analyze()
{
  semantic_analyzer semantic_analyzer;
  return semantic_analyzer.analyze_namescope(*this);
}

void namescope::generate_uids()
{
  if (!has_attribute("uid"))
  {
    crypto::sha256::digest_t hash{};
    crypto::sha256 context(hash);
    (context << *this).finalize();
    boost::crc_32_type crc;
    for (auto c : hash)
      crc(c);
    attribute("uid", crc(), attribute_assignment::keep);
  }

  for (auto* namescope : namescopes)
    namescope->generate_uids();
  for (auto* alias : aliases)
    alias->generate_uids();
  for (auto* enumeration : enumerations)
    enumeration->generate_uids();
  for (auto* message : messages)
    message->generate_uids();
  for (auto* interface : interfaces)
    interface->generate_uids();
  for (auto* service : services)
    service->generate_uids();
}
}

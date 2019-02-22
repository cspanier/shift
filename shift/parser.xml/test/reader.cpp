#include <shift/parser/xml/xml.hpp>
#include <shift/platform/environment.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>

static std::filesystem::path cwd =
  std::filesystem::path(shift::platform::environment::executable_path())
    .parent_path()
    .parent_path()
    .parent_path();

using namespace shift;
using namespace shift::parser::xml;

BOOST_AUTO_TEST_CASE(reader_empty_document)
{
  std::ifstream file(
    (cwd / "private/test/parser.xml/empty.xml").generic_string(),
    std::ios_base::in);
  BOOST_CHECK(file);
  node root;
  BOOST_CHECK_NO_THROW(file >> root);
}

BOOST_AUTO_TEST_CASE(reader_simple_document)
{
  std::ifstream file(
    (cwd / "private/test/parser.xml/simple.xml").generic_string(),
    std::ios_base::in);
  BOOST_CHECK(file);
  node root;
  BOOST_CHECK_NO_THROW(file >> root);

  BOOST_CHECK_EQUAL(root.children.size(), 1);
  auto* root_node = root.element_by_name("simple");
  BOOST_CHECK(root_node);
  if (root_node != nullptr)
  {
    BOOST_CHECK_EQUAL(root_node->name, "simple");

    auto* text1 = root_node->element_by_name("text1");
    BOOST_CHECK(text1);
    if (text1 != nullptr)
    {
      for (auto& child : text1->children)
      {
        BOOST_CHECK(child->type == node_type::text);
        BOOST_CHECK_EQUAL(child->text, "blah");
      }
    }

    auto* text2 = root_node->element_by_name("text2");
    BOOST_CHECK(text2);
    if (text2 != nullptr)
    {
      for (auto& child : text2->children)
      {
        BOOST_CHECK(child->type == node_type::text);
        BOOST_CHECK_EQUAL(child->text, "\n    blah with\n    line breaks\n  ");
      }
    }

    auto* entities = root_node->element_by_name("entities");
    BOOST_CHECK(entities);
    if (entities != nullptr)
    {
      for (auto& child : entities->children)
      {
        BOOST_CHECK(child->type == node_type::text);
        BOOST_CHECK_EQUAL(child->text, "xxx<xxx>xxx&xxx'xxx\"xxx");
      }
    }
  }
}

/// ToDo: Not supported, yet.
// BOOST_AUTO_TEST_CASE(reader_utf8_document)
//{
//  std::ifstream file((cwd /
//  "private/test/parser.xml/utf8.xml").generic_string(),
//  std::ios_base::in); BOOST_CHECK(file); node root; BOOST_CHECK_NO_THROW(file
//  >> root);
//}

BOOST_AUTO_TEST_CASE(reader_comments)
{
  std::ifstream file(
    (cwd / "private/test/parser.xml/comments.xml").generic_string(),
    std::ios_base::in);
  BOOST_CHECK(file);
  node root;
  BOOST_CHECK_NO_THROW(file >> root);

  auto* root_node = root.element_by_name("comments");
  BOOST_CHECK(root_node);
  if (root_node != nullptr)
  {
    auto* element1 = root_node->element_by_name("node1");
    BOOST_CHECK(element1);
    auto* element2 = root_node->element_by_name("node2");
    BOOST_CHECK(element2);
    auto* element3 = root_node->element_by_name("node3");
    BOOST_CHECK(element3);
    auto* element4 = root_node->element_by_name("node4");
    BOOST_CHECK(element4);
    if (element4 != nullptr)
    {
      auto* element5 = element4->element_by_name("node5");
      BOOST_CHECK(element5);
    }
  }
}

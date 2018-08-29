#include <shift/core/string_util.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.h>

using namespace shift::core;

BOOST_AUTO_TEST_CASE(stringutil_split)
{
  {
    const std::string s = "";
    auto r = split(s, [](char c) { return c == ';'; });
    BOOST_CHECK_EQUAL(r.size(), 0);
  }
  {
    const std::string s = "";
    auto r = split(s, [](char c) { return c == ';'; }, true);
    BOOST_CHECK_EQUAL(r.size(), 1);
    BOOST_CHECK_EQUAL(r.at(0), "");
  }
  {
    const std::string s = "a";
    auto r = split(s, [](char c) { return c == ';'; });
    BOOST_CHECK_EQUAL(r.size(), 1);
    BOOST_CHECK_EQUAL(r.at(0), "a");
  }
  {
    const std::string s = "a;b;c";
    auto r = split(s, [](char c) { return c == ';'; });
    BOOST_CHECK_EQUAL(r.size(), 3);
    BOOST_CHECK_EQUAL(r.at(0), "a");
    BOOST_CHECK_EQUAL(r.at(1), "b");
    BOOST_CHECK_EQUAL(r.at(2), "c");
  }
  {
    const std::string s = "aa;bb;cc";
    auto r = split(s, [](char c) { return c == ';'; });
    BOOST_CHECK_EQUAL(r.size(), 3);
    BOOST_CHECK_EQUAL(r.at(0), "aa");
    BOOST_CHECK_EQUAL(r.at(1), "bb");
    BOOST_CHECK_EQUAL(r.at(2), "cc");
  }
  {
    const std::string s = ";bb;cc";
    auto r = split(s, [](char c) { return c == ';'; });
    BOOST_CHECK_EQUAL(r.size(), 2);
    BOOST_CHECK_EQUAL(r.at(0), "bb");
    BOOST_CHECK_EQUAL(r.at(1), "cc");
  }
  {
    const std::string s = "aa;;cc";
    auto r = split(s, [](char c) { return c == ';'; });
    BOOST_CHECK_EQUAL(r.size(), 2);
    BOOST_CHECK_EQUAL(r.at(0), "aa");
    BOOST_CHECK_EQUAL(r.at(1), "cc");
  }
  {
    const std::string s = "aa;bb;";
    auto r = split(s, [](char c) { return c == ';'; });
    BOOST_CHECK_EQUAL(r.size(), 2);
    BOOST_CHECK_EQUAL(r.at(0), "aa");
    BOOST_CHECK_EQUAL(r.at(1), "bb");
  }
  {
    const std::string s = ";bb;cc";
    auto r = split(s, [](char c) { return c == ';'; }, true);
    BOOST_CHECK_EQUAL(r.size(), 3);
    BOOST_CHECK_EQUAL(r.at(0), "");
    BOOST_CHECK_EQUAL(r.at(1), "bb");
    BOOST_CHECK_EQUAL(r.at(2), "cc");
  }
  {
    const std::string s = "aa;;cc";
    auto r = split(s, [](char c) { return c == ';'; }, true);
    BOOST_CHECK_EQUAL(r.size(), 3);
    BOOST_CHECK_EQUAL(r.at(0), "aa");
    BOOST_CHECK_EQUAL(r.at(1), "");
    BOOST_CHECK_EQUAL(r.at(2), "cc");
  }
  {
    const std::string s = "aa;bb;";
    auto r = split(s, [](char c) { return c == ';'; }, true);
    BOOST_CHECK_EQUAL(r.size(), 3);
    BOOST_CHECK_EQUAL(r.at(0), "aa");
    BOOST_CHECK_EQUAL(r.at(1), "bb");
    BOOST_CHECK_EQUAL(r.at(2), "");
  }
}

BOOST_AUTO_TEST_CASE(stringutil_to_lower)
{
  using namespace std::string_literals;

  BOOST_CHECK_EQUAL(to_lower(""s), ""s);
  BOOST_CHECK_EQUAL(to_lower("A"s), "a"s);
  BOOST_CHECK_EQUAL(to_lower("AB"s), "ab"s);
  BOOST_CHECK_EQUAL(to_lower("ABC"s), "abc"s);

  // BOOST_CHECK_EQUAL can't print wstring out of the box.
  BOOST_CHECK(to_lower(L""s) == L""s);
  BOOST_CHECK(to_lower(L"A"s) == L"a"s);
  BOOST_CHECK(to_lower(L"AB"s) == L"ab"s);
  BOOST_CHECK(to_lower(L"ABC"s) == L"abc"s);

  /// ToDo: The following checks fail.
  //BOOST_CHECK_EQUAL(to_lower("ÄÖÜß"s), "äöüß"s);
  //BOOST_CHECK_EQUAL(to_lower(u8"ÄÖÜß"s), u8"äöüß"s);
  //BOOST_CHECK_EQUAL(to_lower(u"ÄÖÜß"s), u"äöüß"s);
  //BOOST_CHECK_EQUAL(to_lower(U"ÄÖÜß"s), U"äöüß"s);
}

BOOST_AUTO_TEST_CASE(stringutil_to_upper)
{
  using namespace std::string_literals;

  BOOST_CHECK_EQUAL(to_upper(""s), ""s);
  BOOST_CHECK_EQUAL(to_upper("a"s), "A"s);
  BOOST_CHECK_EQUAL(to_upper("ab"s), "AB"s);
  BOOST_CHECK_EQUAL(to_upper("abc"s), "ABC"s);

  // BOOST_CHECK_EQUAL can't print wstring out of the box.
  BOOST_CHECK(to_upper(L""s) == L""s);
  BOOST_CHECK(to_upper(L"a"s) == L"A"s);
  BOOST_CHECK(to_upper(L"ab"s) == L"AB"s);
  BOOST_CHECK(to_upper(L"abc"s) == L"ABC"s);

  /// ToDo: The following checks fail.
  //BOOST_CHECK_EQUAL(to_upper("äöüß"s), "ÄÖÜSS"s);
  //BOOST_CHECK_EQUAL(to_upper(u8"äöüß"s), u8"ÄÖÜSS"s);
  //BOOST_CHECK_EQUAL(to_upper(u"äöüß"s), u"ÄÖÜSS"s);
  //BOOST_CHECK_EQUAL(to_upper(U"äöüß"s), U"ÄÖÜSS"s);
}

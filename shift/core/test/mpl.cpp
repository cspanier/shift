#include <shift/core/mpl.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/test/unit_test.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <sstream>
#include <cstdint>
#include <iostream>

using namespace shift::core;

BOOST_AUTO_TEST_CASE(mpl_is_valid)
{
  BOOST_STATIC_ASSERT(is_valid<vector<>>::value);
  BOOST_STATIC_ASSERT(is_valid<vector<int>>::value);
  BOOST_STATIC_ASSERT(is_valid<vector<int, float>>::value);
  BOOST_STATIC_ASSERT(is_valid<vector<int, float, float>>::value);

  BOOST_STATIC_ASSERT(is_valid<set<>>::value);
  BOOST_STATIC_ASSERT(is_valid<set<int>>::value);
  BOOST_STATIC_ASSERT(is_valid<set<int, float>>::value);
  BOOST_STATIC_ASSERT(!is_valid<set<int, float, float>>::value);

  BOOST_STATIC_ASSERT(is_valid<std::tuple<>>::value);
  BOOST_STATIC_ASSERT(is_valid<std::tuple<int>>::value);
  BOOST_STATIC_ASSERT(is_valid<std::tuple<int, float>>::value);
  BOOST_STATIC_ASSERT(is_valid<std::tuple<int, float, float>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_in)
{
  BOOST_STATIC_ASSERT(!in<set<>, int>::value);
  BOOST_STATIC_ASSERT(in<set<int>, int>::value);
  BOOST_STATIC_ASSERT(in<set<char, int>, int>::value);
  BOOST_STATIC_ASSERT(in<set<int, char>, int>::value);
  BOOST_STATIC_ASSERT(!in<set<char>, int>::value);
  BOOST_STATIC_ASSERT(!in<set<char, float>, int>::value);
}

BOOST_AUTO_TEST_CASE(mpl_intersection)
{
  // No common element
  BOOST_STATIC_ASSERT(std::is_same<intersection_t<set<>, set<>>, set<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<intersection_t<set<int>, set<>>, set<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<intersection_t<set<>, set<int>>, set<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<intersection_t<set<float>, set<int>>, set<>>::value);

  // One common element
  BOOST_STATIC_ASSERT(
    std::is_same<intersection_t<set<int>, set<int>>, set<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<intersection_t<set<int, char>, set<int>>, set<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<intersection_t<set<int>, set<int, char>>, set<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<intersection_t<set<char, int>, set<int>>, set<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<intersection_t<set<int>, set<char, int>>, set<int>>::value);

  // Order resulting types by the first type list.
  BOOST_STATIC_ASSERT(
    std::is_same<intersection_t<set<char, int>, set<int, char>>,
                 set<char, int>>::value);
  BOOST_STATIC_ASSERT(
    !std::is_same<intersection_t<set<char, int>, set<int, char>>,
                  set<int, char>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_push_front)
{
  BOOST_STATIC_ASSERT(
    std::is_same<push_front_t<vector<>, int>, vector<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<push_front_t<vector<float>, int>, vector<int, float>>::value);
  BOOST_STATIC_ASSERT(std::is_same<push_front_t<vector<float, char>, int>,
                                   vector<int, float, char>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_push_back)
{
  BOOST_STATIC_ASSERT(
    std::is_same<push_back_t<vector<>, int>, vector<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<push_back_t<vector<float>, int>, vector<float, int>>::value);
  BOOST_STATIC_ASSERT(std::is_same<push_back_t<vector<float, char>, int>,
                                   vector<float, char, int>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_insert)
{
  BOOST_STATIC_ASSERT(
    std::is_same<insert_t<vector<>, char>, vector<char>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<insert_t<vector<int>, char>, vector<int, char>>::value);
  BOOST_STATIC_ASSERT(std::is_same<insert_t<vector<int, float>, char>,
                                   vector<int, float, char>>::value);
  BOOST_STATIC_ASSERT(std::is_same<insert_t<vector<int, float, char>, char>,
                                   vector<int, float, char, char>>::value);

  BOOST_STATIC_ASSERT(std::is_same<insert_t<set<>, char>, set<char>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<insert_t<set<int>, char>, set<int, char>>::value);
  BOOST_STATIC_ASSERT(std::is_same<insert_t<set<int, float>, char>,
                                   set<int, float, char>>::value);
  BOOST_STATIC_ASSERT(std::is_same<insert_t<set<int, float, char>, char>,
                                   set<int, float, char>>::value);

  BOOST_STATIC_ASSERT(std::is_same<insert_t<map<>, pair<int, bool>>,
                                   map<pair<int, bool>>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<insert_t<map<pair<int, bool>>, pair<float, bool>>,
                 map<pair<int, bool>, pair<float, bool>>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_sorted_insert)
{
  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<set<>, int, set<char, int, float>>,
                 set<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<set<char>, int, set<char, int, float>>,
                 set<char, int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<set<int>, int, set<char, int, float>>,
                 set<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<set<float>, int, set<char, int, float>>,
                 set<int, float>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<set<char, int>, int, set<char, int, float>>,
                 set<char, int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<set<int, float>, int, set<char, int, float>>,
                 set<int, float>>::value);

  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<vector<>, int, vector<char, int, float>>,
                 vector<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<vector<char>, int, vector<char, int, float>>,
                 vector<char, int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<vector<int>, int, vector<char, int, float>>,
                 vector<int, int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<sorted_insert_t<vector<float>, int, vector<char, int, float>>,
                 vector<int, float>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<
      sorted_insert_t<vector<char, int>, int, vector<char, int, float>>,
      vector<char, int, int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<
      sorted_insert_t<vector<int, float>, int, vector<char, int, float>>,
      vector<int, int, float>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_concat)
{
  BOOST_STATIC_ASSERT(
    std::is_same<concat_t<vector<>, vector<>>, vector<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<concat_t<vector<float>, vector<>>, vector<float>>::value);
  BOOST_STATIC_ASSERT(std::is_same<concat_t<vector<float, char>, vector<>>,
                                   vector<float, char>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<concat_t<vector<>, vector<int>>, vector<int>>::value);
  BOOST_STATIC_ASSERT(std::is_same<concat_t<vector<float>, vector<int>>,
                                   vector<float, int>>::value);
  BOOST_STATIC_ASSERT(std::is_same<concat_t<vector<float, char>, vector<int>>,
                                   vector<float, char, int>>::value);
  BOOST_STATIC_ASSERT(std::is_same<concat_t<vector<>, vector<char, int>>,
                                   vector<char, int>>::value);
  BOOST_STATIC_ASSERT(std::is_same<concat_t<vector<float>, vector<char, int>>,
                                   vector<float, char, int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<concat_t<vector<float, char>, vector<char, int>>,
                 vector<float, char, char, int>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_transform)
{
  BOOST_STATIC_ASSERT(
    std::is_same<transform_t<vector<>, vector<>, std::decay>, vector<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<transform_t<set<>, vector<>, std::decay>, vector<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<transform_t<vector<>, set<>, std::decay>, set<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<transform_t<set<>, set<>, std::decay>, set<>>::value);

  BOOST_STATIC_ASSERT(
    std::is_same<
      transform_t<set<const char, int&, float(double)>, set<>, std::decay>,
      set<char, int, float (*)(double)>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_fold)
{
  BOOST_STATIC_ASSERT(
    std::is_same<fold_t<vector<>, vector<int>, push_back>, vector<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<fold_t<vector<float, char>, vector<>, push_back>,
                 vector<float, char>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<fold_t<vector<float, char>, vector<int>, push_back>,
                 vector<int, float, char>>::value);

  BOOST_STATIC_ASSERT(std::is_same<fold_t<vector<>, vector<int>, push_front>,
                                   vector<int>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<fold_t<vector<float, char>, vector<>, push_front>,
                 vector<char, float>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<fold_t<vector<float, char>, vector<int>, push_front>,
                 vector<char, float, int>>::value);

  BOOST_STATIC_ASSERT(
    std::is_same<fold_t<vector<>, set<>, insert>, set<>>::value);
  BOOST_STATIC_ASSERT(std::is_same<fold_t<vector<float, char>, set<>, insert>,
                                   set<float, char>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<fold_t<vector<float, char>, set<int>, insert>,
                 set<int, float, char>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<fold_t<vector<float, char>, set<int, float>, insert>,
                 set<int, float, char>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_select)
{
  BOOST_STATIC_ASSERT(
    std::is_same<select_t<set<>, std::is_floating_point>, set<>>::value);
  BOOST_STATIC_ASSERT(std::is_same<select_t<set<float>, std::is_floating_point>,
                                   set<float>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<select_t<set<int>, std::is_floating_point>, set<>>::value);

  BOOST_STATIC_ASSERT(
    std::is_same<
      select_t<vector<float, int, double, char>, std::is_floating_point>,
      vector<float, double>>::value);
  BOOST_STATIC_ASSERT(
    !std::is_same<
      select_t<vector<float, int, double, char>, std::is_floating_point>,
      vector<double, float>>::value);

  BOOST_STATIC_ASSERT(
    std::is_same<
      select_t<set<float, int, double, char>, std::is_floating_point>,
      set<float, double>>::value);
  BOOST_STATIC_ASSERT(
    !std::is_same<
      select_t<set<float, int, double, char>, std::is_floating_point>,
      set<double, float>>::value);

  BOOST_STATIC_ASSERT(
    std::is_same<select_t<set<char, int, float>, std::is_same, int>,
                 set<int>>::value);
}

BOOST_AUTO_TEST_CASE(mpl_single_type)
{
  BOOST_STATIC_ASSERT(std::is_same<single_type_t<set<int>>, int>::value);
  BOOST_STATIC_ASSERT(std::is_same<single_type_t<vector<int>>, int>::value);
  BOOST_STATIC_ASSERT(std::is_same<single_type_t<std::tuple<int>>, int>::value);
}

BOOST_AUTO_TEST_CASE(mpl_index_of)
{
  BOOST_STATIC_ASSERT(index_of<vector<int, float, char>, int>::value == 0);
  BOOST_STATIC_ASSERT(index_of<vector<int, float, char>, float>::value == 1);
  BOOST_STATIC_ASSERT(index_of<vector<int, float, char>, char>::value == 2);

  BOOST_STATIC_ASSERT(
    index_of<map<pair<int, set<>>, pair<float, set<>>>, int>::value == 0);
  BOOST_STATIC_ASSERT(
    index_of<map<pair<int, set<>>, pair<float, set<>>>, float>::value == 1);
}

BOOST_AUTO_TEST_CASE(mpl_index_of_optional)
{
  BOOST_STATIC_ASSERT(
    index_of_optional<vector<int, float, char>, double>::value == -1);
  BOOST_STATIC_ASSERT(index_of_optional<vector<int, float, char>, int>::value ==
                      0);
  BOOST_STATIC_ASSERT(
    index_of_optional<vector<int, float, char>, float>::value == 1);
  BOOST_STATIC_ASSERT(
    index_of_optional<vector<int, float, char>, char>::value == 2);

  BOOST_STATIC_ASSERT(
    index_of_optional<map<pair<int, set<>>, pair<float, set<>>>, char>::value ==
    -1);
  BOOST_STATIC_ASSERT(
    index_of_optional<map<pair<int, set<>>, pair<float, set<>>>, int>::value ==
    0);
  BOOST_STATIC_ASSERT(
    index_of_optional<map<pair<int, set<>>, pair<float, set<>>>,
                      float>::value == 1);
}

BOOST_AUTO_TEST_CASE(mpl_conversion)
{
  BOOST_STATIC_ASSERT(
    std::is_same<to_std_tuple_t<vector<>>, std::tuple<>>::value);
  BOOST_STATIC_ASSERT(std::is_same<to_std_tuple_t<vector<int, float>>,
                                   std::tuple<int, float>>::value);
  BOOST_STATIC_ASSERT(std::is_same<to_std_tuple_t<vector<int, float, float>>,
                                   std::tuple<int, float, float>>::value);

  BOOST_STATIC_ASSERT(std::is_same<to_std_tuple_t<set<>>, std::tuple<>>::value);
  BOOST_STATIC_ASSERT(std::is_same<to_std_tuple_t<set<int, float>>,
                                   std::tuple<int, float>>::value);

  BOOST_STATIC_ASSERT(std::is_same<to_vector_t<std::tuple<>>, vector<>>::value);
  BOOST_STATIC_ASSERT(std::is_same<to_vector_t<std::tuple<int, float>>,
                                   vector<int, float>>::value);
  BOOST_STATIC_ASSERT(std::is_same<to_vector_t<std::tuple<int, float, float>>,
                                   vector<int, float, float>>::value);

  BOOST_STATIC_ASSERT(std::is_same<to_vector_t<set<>>, vector<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<to_vector_t<set<int, float>>, vector<int, float>>::value);

  BOOST_STATIC_ASSERT(std::is_same<to_set_t<std::tuple<>>, set<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<to_set_t<std::tuple<int, float>>, set<int, float>>::value);
  BOOST_STATIC_ASSERT(std::is_same<to_set_t<std::tuple<int, float, float>>,
                                   set<int, float>>::value);

  BOOST_STATIC_ASSERT(std::is_same<to_set_t<vector<>>, set<>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<to_set_t<vector<int, float>>, set<int, float>>::value);
  BOOST_STATIC_ASSERT(
    std::is_same<to_set_t<vector<int, float, float>>, set<int, float>>::value);
}

class for_each_visitor
{
public:
  for_each_visitor(int& counter) : _counter(counter)
  {
  }

  for_each_visitor(const for_each_visitor&) = delete;
  for_each_visitor(for_each_visitor&&) = default;
  for_each_visitor& operator=(const for_each_visitor&) = delete;
  for_each_visitor& operator=(for_each_visitor&&) = default;

  template <typename T>
  void operator()(const T* /*unused*/)
  {
    ++_counter;
  }

private:
  int& _counter;
};

BOOST_AUTO_TEST_CASE(mpl_for_each)
{
  int i = 0;
  for_each_visitor v{i};
  for_each<set<int, float>>{v};
  BOOST_CHECK_EQUAL(i, 2);
}

class recursive_visitor
{
public:
  using return_type = bool;

  recursive_visitor() = default;

  recursive_visitor(const recursive_visitor&) = delete;
  recursive_visitor(recursive_visitor&&) = default;
  recursive_visitor& operator=(const recursive_visitor&) = delete;
  recursive_visitor& operator=(recursive_visitor&&) = default;

  template <typename RecursionHelper>
  return_type operator()(RecursionHelper /*recursion_helper*/)
  {
    return true;
  }

  template <typename RecursionHelper, typename T>
  return_type operator()(RecursionHelper recursion_helper, const T* /*unused*/)
  {
    ++_counter;
    return recursion_helper(*this);
  }

  int counter() const
  {
    return _counter;
  }

private:
  int _counter = 0;
};

BOOST_AUTO_TEST_CASE(mpl_recursion_helper)
{
  recursive_visitor v;
  BOOST_CHECK((recursion_helper<set<int, float, double>>()(v)));
  BOOST_CHECK_EQUAL(v.counter(), 3);
}

struct cat
{
  void meow()
  {
  }
};

struct dog
{
  void woof()
  {
  }
};

using both_cat_and_dog = gen_linear_hierarchy<vector<cat, dog>>;

BOOST_AUTO_TEST_CASE(mpl_gen_linear_hierarchy)
{
  both_cat_and_dog magic_animal;
  magic_animal.meow();
  magic_animal.woof();
}

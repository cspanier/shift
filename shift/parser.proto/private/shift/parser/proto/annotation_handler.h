#ifndef SHIFT_PARSER_PROTO_ANNOTATIONHANDLER_H
#define SHIFT_PARSER_PROTO_ANNOTATIONHANDLER_H

#include <algorithm>
#include <string>
#include <map>
#include <shift/core/boost_disable_warnings.h>
#include <boost/spirit/include/qi.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/parser/proto/proto.h"

namespace shift::parser::proto
{
/// An qi::on_success handler to store exact location of a successfuly
/// parsed token.
template <typename Iterator>
struct annotation_handler
{
  using result_type = void;

  template <typename Value>
  result_type operator()(Value& value, Iterator token_begin, Iterator /*end*/,
                         Iterator token_end) const
  {
    using std::distance;

    Iterator begin = source->code.begin();

    auto& info = static_cast<annotation_info&>(value);
    info.source = source;
    info.token_begin = distance(begin, token_begin);
    info.token_end = distance(begin, token_end);
  }

  std::shared_ptr<source_module> source;
};
}

#endif

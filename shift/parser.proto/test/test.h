#ifndef SHIFT_PARSER_PROTO_TEST_H
#define SHIFT_PARSER_PROTO_TEST_H

#include <string>
#include <shift/parser/proto/proto.h>

///
bool test_parser_and_ast(std::string_view source_code,
                         std::string_view source_name,
                         std::string_view expected_error = "");

///
bool test_parser_and_ast(shift::parser::proto::document& document,
                         std::string_view source_code,
                         std::string_view source_name,
                         std::string_view expected_error = "");

#endif

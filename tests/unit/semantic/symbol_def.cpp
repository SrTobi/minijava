#include "semantic/symbol_def.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#define BOOST_TEST_MODULE  semantic_symbol_def
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "symbol/symbol_pool.hpp"

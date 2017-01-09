#include "opt/load_store.hpp"

using namespace minijava::opt;

bool load_store::handle(firm::ir_node* /*node*/) {
	return _changed;
}
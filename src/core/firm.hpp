#pragma once

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MINIJAVA_PARSED_BY_DOXYGEN

namespace firm
{

	extern "C"
	{

#include "libfirm/firm.h"

// The `get_nodes_block` function is very bad.  Don't use it.
#pragma GCC poison get_nodes_block

	}  // extern "C"

}  // namespace firm

#endif  // MINIJAVA_PARSED_BY_DOXYGEN

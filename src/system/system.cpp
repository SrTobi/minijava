#include "system/system.hpp"

#include "global.hpp"


#define MINIJAVA_INCLUDED_FROM_SYSTEM_SYSTEM_HPP
#  if MINIJAVA_HAVE_RLIMIT
#    include "system/rlimit_stack_posix.tpp"
#  else
#    include "system/rlimit_stack_generic.tpp"
#  endif
#undef MINIJAVA_INCLUDED_FROM_SYSTEM_SYSTEM_HPP

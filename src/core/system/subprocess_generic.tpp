#ifndef MINIJAVA_INCLUDED_FROM_SYSTEM_SUBPROCESS_CPP
#error "Never `#include` the source file `<system/subprocess_generic.tpp>`"
#endif


namespace /* anonymous */
{

	void do_run_subprocess(std::vector<char*> argv)
	{
		(void) &throw_subprocess_failed;
		throw_invoke_subprocess_failed(ENOSYS, argv.front());
	}

}

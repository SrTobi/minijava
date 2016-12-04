#ifndef MINIJAVA_INCLUDED_FROM_SYSTEM_SUBPROCESS_CPP
#error "Never `#include` the source file `<system/subprocess_windows.tpp>`"
#endif

#include <process.h>

namespace /* anonymous */
{

	void do_run_subprocess(std::vector<char*> argv)
	{
		auto ret = _spawnvp(_P_WAIT, argv[0], argv.data());
		if (ret == -1) {
			throw_invoke_subprocess_failed(errno, argv.front());
		} else if (ret) {
			throw_subprocess_failed(argv.front());
		}
	}

}  // namespace /* anonymous */

#include "system/host_cc.hpp"

#ifdef _WIN32
#   include <process.h>
#else
#   include <fcntl.h>
#   include <sys/wait.h>
#   include <unistd.h>
#endif

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <system_error>

#include <boost/filesystem.hpp>

#include "io/file_output.hpp"
#include "runtime/runtime.hpp"

namespace minijava
{

	namespace /* anonymous */
	{

#ifndef _WIN32
		auto create_pipe() {
			auto close_fds = [](int* fd) {
				if (fd[0] != -1) {
					close(fd[0]);
				}
				if (fd[1] != -1) {
					close(fd[1]);
				}
				delete[] fd;
			};
			std::unique_ptr<int[], decltype(close_fds)> fds{
					new int[2], close_fds
			};
			fds[0] = -1;
			fds[1] = -1;
			if (pipe(fds.get()) == -1) {
				const auto ec = std::error_code{errno, std::system_category()};
				throw std::system_error{ec, "Could not create pipe."};
			}
			if (fcntl(fds[1], F_SETFD, fcntl(fds[1], F_GETFD) | FD_CLOEXEC) == -1) {
				const auto ec = std::error_code{errno, std::system_category()};
				throw std::system_error{ec, "Could not create pipe."};
			}
			return fds;
		}
#endif

		[[noreturn]]
		void throw_invoke_compiler_failed(int error,
		                                  const std::string& compiler_command)
		{
			using namespace std::string_literals;
			const auto ec = std::error_code{error, std::system_category()};
			throw std::system_error{ec, "Could not start '"s + compiler_command + "'."};
		}

		[[noreturn]]
		void throw_compiler_failed(const std::string& compiler_command)
		{
			using namespace std::string_literals;
			throw std::runtime_error{
					"Compilation failed. '"s + compiler_command
					+ "' exited with a non-zero status."
			};
		}

	}  //  namespace /* anonymous */

	std::string get_default_c_compiler_command()
	{
		if (const char* compiler_binary = std::getenv("CC")) {
			return std::string(compiler_binary);
		}

#if __APPLE__
		return "clang";
#else
		return "gcc";
#endif
	}

	void link_runtime(const std::string& compiler_command,
	                  const std::string& output_file,
	                  const std::string& minijava_assembly)
	{
		namespace fs = boost::filesystem;
		auto runtime_path = fs::temp_directory_path() / fs::unique_path();
		auto minijava_runtime_path = runtime_path.string();
		auto runtime_file = file_output{minijava_runtime_path};
		runtime_file.write(runtime_source());
		runtime_file.close();
#ifdef _WIN32
		auto ret = _spawnlp(
				_P_WAIT,
				compiler_command.c_str(),
				"-o",
				output_file.c_str(),
				minijava_assembly.c_str(),
				minijava_runtime_path.c_str(),
				nullptr
		);
		if (ret == -1) {
			throw_invoke_compiler_failed(errno, compiler_command);
		} else if (ret) {
			throw_compiler_failed(compiler_command);
		}
#else
		// create pipe to communicate error code of execlp()
		auto pipe_fds = create_pipe();
		pid_t pid = fork();
		if (pid == -1) {
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec, "Could not fork compiler process."};
		} else if (pid == 0) {
			close(pipe_fds[0]);
			pipe_fds[0] = -1;
			execlp(
					compiler_command.c_str(),
					"-o",
					output_file.c_str(),
					minijava_assembly.c_str(),
					minijava_runtime_path.c_str(),
			        nullptr
			);
			write(pipe_fds[1], &errno, sizeof(errno));
			_exit(EXIT_SUCCESS);
		} else {
			close(pipe_fds[1]);
			pipe_fds[1] = -1;
			ssize_t bytes_read;
			int code;
			while ((bytes_read = read(pipe_fds[0], &code, sizeof(errno))) == -1) {
				if (errno != EAGAIN && errno != EINTR) {
					throw std::runtime_error{"Unable to read from pipe."};
				}
			}
			if (bytes_read) {
				throw_invoke_compiler_failed(code, compiler_command);
			}
			int status;
			while (waitpid(pid, &status, 0) == -1) {
				if (errno != EINTR) {
					const auto ec = std::error_code{
							errno, std::system_category()
					};
					throw std::system_error{
							ec, "Could not wait for compiler process."
					};
				}
			}
			if (status) {
				if (WIFEXITED(status)) {
					throw_compiler_failed(compiler_command);
				} else {
					throw std::runtime_error{
							"The compiler process terminated unexpectedly."
					};
				}
			}
		}
#endif
	}

}  // namespace minijava

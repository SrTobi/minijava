#include "system/subprocess.hpp"

#ifdef _WIN32
#   include <process.h>
#else
#   include <fcntl.h>
#   include <sys/wait.h>
#   include <unistd.h>
#endif

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <system_error>

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
		void throw_invoke_subprocess_failed(
				int error, const std::vector<std::string>& command)
		{
			using namespace std::string_literals;
			const auto ec = std::error_code{error, std::system_category()};
			throw std::system_error{
					ec, "Could not start '"s + command.at(0) + "'."
			};
		}

		[[noreturn]]
		void throw_subprocess_failed(const std::vector<std::string>& command)
		{
			using namespace std::string_literals;
			throw std::runtime_error{
					"Subprocess failed. '"s + command.at(0)
					+ "' exited with a non-zero status."
			};
		}

	}  //  namespace /* anonymous */

	void run_subprocess(const std::vector<std::string>& command)
	{
		if (command.size() < 1) {
			throw std::invalid_argument{"`command` is empty"};
		}
		auto argv = std::vector<const char*>{command.size() + 1};
		std::transform(
				command.begin(), command.end(), argv.begin(), [](auto&& s) {
					return s.c_str();
				}
		);
		argv[command.size()] = nullptr;
#ifdef _WIN32
		auto ret = _spawnvp(_P_WAIT, argv[0], argv.data());
		if (ret == -1) {
			throw_invoke_subprocess_failed(errno, command);
		} else if (ret) {
			throw_subprocess_failed(command);
		}
#else
		// create pipe to communicate error code of execlp()
		auto pipe_fds = create_pipe();
		pid_t pid = fork();
		if (pid == -1) {
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec, "Could not fork subprocess."};
		} else if (pid == 0) {
			close(pipe_fds[0]);
			pipe_fds[0] = -1;
			// here be nasal demons
			execvp(argv[0], const_cast<char**>(argv.data()));
			write(pipe_fds[1], &errno, sizeof(errno));
			_exit(EXIT_SUCCESS);
		} else {
			close(pipe_fds[1]);
			pipe_fds[1] = -1;
			ssize_t bytes_read;
			int code;
			while ((bytes_read = read(pipe_fds[0], &code, sizeof(errno))) == -1) {
				if (errno != EAGAIN && errno != EINTR) {
					const auto ec = std::error_code{
							errno, std::system_category()
					};
					throw std::system_error{ec, "Unable to read from pipe."};
				}
			}
			if (bytes_read) {
				throw_invoke_subprocess_failed(code, command);
			}
			int status;
			while (waitpid(pid, &status, 0) == -1) {
				if (errno != EINTR) {
					const auto ec = std::error_code{
							errno, std::system_category()
					};
					throw std::system_error{
							ec, "Could not wait for subprocess."
					};
				}
			}
			if (status) {
				if (WIFEXITED(status)) {
					throw_subprocess_failed(command);
				} else {
					throw std::runtime_error{
							"The subprocess terminated unexpectedly."
					};
				}
			}
		}
#endif
	}

}  // namespace minijava

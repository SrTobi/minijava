#ifndef MINIJAVA_INCLUDED_FROM_SYSTEM_SUBPROCESS_CPP
#error "Never `#include` the source file `<system/subprocess_posix.tpp>`"
#endif

#include <memory>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>


namespace /* anonymous */
{

	auto create_pipe()
	{
		auto close_fds = [](int* fd) {
			if (fd[0] != -1) {
				close(fd[0]);
			}
			if (fd[1] != -1) {
				close(fd[1]);
			}
			delete[] fd;
		};
		std::unique_ptr<int[], decltype(close_fds)> fds{new int[2], close_fds};
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

	void do_run_subprocess(std::vector<char*> argv)
	{
		// create pipe to communicate error code of execlp()
		auto pipe_fds = create_pipe();
		const auto pid = fork();
		if (pid == -1) {
			const auto ec = std::error_code{errno, std::system_category()};
			throw std::system_error{ec, "Cannot fork subprocess"};
		} else if (pid == 0) {
			close(pipe_fds[0]);
			pipe_fds[0] = -1;
			execvp(argv[0], argv.data());
			write(pipe_fds[1], &errno, sizeof(errno));
			_exit(EXIT_SUCCESS);
		} else {
			close(pipe_fds[1]);
			pipe_fds[1] = -1;
			ssize_t bytes_read;
			int code;
			while ((bytes_read = read(pipe_fds[0], &code, sizeof(errno))) == -1) {
				if (errno != EAGAIN && errno != EINTR) {
					const auto ec = std::error_code{errno, std::system_category()};
					throw std::system_error{ec, "Cannot communicate with subprocess"};
				}
			}
			if (bytes_read) {
				throw_invoke_subprocess_failed(code, argv.front());
			}
			int status;
			while (waitpid(pid, &status, 0) == -1) {
				if (errno != EINTR) {
					const auto ec = std::error_code{errno, std::system_category()};
					throw std::system_error{ec, "Cannot not wait for subprocess"};
				}
			}
			if (status) {
				if (WIFEXITED(status)) {
					throw_subprocess_failed(argv.front());
				} else {
					throw std::runtime_error{"Subprocess terminated abnormally"};
				}
			}
		}
	}

}  //  namespace /* anonymous */

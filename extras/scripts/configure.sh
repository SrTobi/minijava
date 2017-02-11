#! /bin/bash -eu

shopt -s nullglob

default_prefix="${HOME}/var"

helptext="usage: configure [--prefix=DIR] [CONFIG...]

Configures the project in all selected configurations.  The build-directory for
configuration '\${cfg}' will be 'stage-\${cfg}'.  If custom run-time libraries
are required for a configuration, a symbolic link 'dynlib' to a directory that
should be added to 'LD_LIBRARY_PATH' will be created in the staging directory.

Options:

  --prefix=DIR    find custom-built libraries under DIR (default: '${default_prefix}')
  --help          show help text and exit
  --version       show version text and exit

Available Configurations:

  gcc-release     GNU compiler with libstdc++ in release mode
  gcc-debug       GNU compiler with libstdc++ in debug mode
  clang           Clang compiler with libc++ standard library
  asan            AddressSanitizer with default compiler in default mode
  ubsan           UndefinedBehaviorSanitizer with default compiler in default mode
  tsan            ThreadSanitizer with default compiler in default mode
  msan            MemorySanitizer with default compiler in default mode

ThreadSanitizer is pointless for a single-threaded program and MemorySanitizer
is currently not functional.  The default selection (if no configurations are
specified) therefore excludes these two.
"

function cmake_gcc_release {
	ln -s "${prefix}/gcc-release/lib/" dynlib
	cmake -DCMAKE_BUILD_TYPE=Release                                          \
	      -DMAINTAINER_MODE=ON                                                \
	      -DCMAKE_C_COMPILER="gcc"                                            \
	      -DCMAKE_CXX_COMPILER="g++"                                          \
	      -DCMAKE_INCLUDE_PATH="${prefix}/gcc-release/include/"               \
	      -DCMAKE_LIBRARY_PATH="${prefix}/gcc-release/lib/"                   \
	       "$@"
}

function cmake_gcc_debug {
	ln -s "${prefix}/gcc-debug/lib/" dynlib
	cmake -DCMAKE_BUILD_TYPE=Debug                                            \
	      -DMAINTAINER_MODE=ON                                                \
	      -DCMAKE_C_COMPILER="gcc"                                            \
	      -DCMAKE_CXX_COMPILER="g++"                                          \
	      -DCMAKE_CXX_FLAGS="-D_GLIBCXX_DEBUG"                                \
	      -DCMAKE_INCLUDE_PATH="${prefix}/gcc-debug/include/"                 \
	      -DCMAKE_LIBRARY_PATH="${prefix}/gcc-debug/lib/"                     \
	       "$@"
}

function cmake_clang {
	ln -s "${prefix}/clang/lib/" dynlib
	cmake -DCMAKE_BUILD_TYPE=None                                             \
	      -DMAINTAINER_MODE=ON                                                \
	      -DCMAKE_C_COMPILER="clang"                                          \
	      -DCMAKE_CXX_COMPILER="clang++"                                      \
	      -DCMAKE_INCLUDE_PATH="${prefix}/clang/include/"                     \
	      -DCMAKE_LIBRARY_PATH="${prefix}/clang/lib/"                         \
	      -DCMAKE_CXX_FLAGS="-stdlib=libc++"                                  \
	      -DCMAKE_EXE_LINKER_FLAGS="-lc++ -lc++abi"                           \
	       "$@"
}

function gen_cmake_sanitize {
	local tool; tool="$1"; shift
	cmake -DCMAKE_BUILD_TYPE=Debug                                            \
	      -DMAINTAINER_MODE=ON                                                \
	      -DCMAKE_C_FLAGS="-fsanitize=${tool} -fno-sanitize-recover=all"      \
	      -DCMAKE_CXX_FLAGS="-fsanitize=${tool} -fno-sanitize-recover=all"    \
	       "$@"
}

function cmake_asan { gen_cmake_sanitize address "$@"; }
function cmake_ubsan { gen_cmake_sanitize undefined "$@"; }
function cmake_tsan { gen_cmake_sanitize thread "$@"; }
function cmake_msan { gen_cmake_sanitize memory "$@"; }

declare -A configurations

configurations[gcc-release]+=cmake_gcc_release
configurations[gcc-debug]+=cmake_gcc_debug
configurations[clang]+=cmake_clang
configurations[asan]+=cmake_asan
configurations[ubsan]+=cmake_ubsan
configurations[tsan]+=cmake_tsan  # Pointless for single-threaded program
configurations[msan]+=cmake_msan  # Not supported by GCC and broken in Clang

default_selection=(gcc-release gcc-debug clang asan ubsan)

prefix="${default_prefix}"

declare -a selection

function die {
	echo "configure: error: $@" >&2
	exit 1
}

for arg in "$@"
do
	case "${arg}"
	in
		--help)
			echo "${helptext}"
			exit
			;;
		--version)
			echo "configure 1.0"
			echo "Configure the project in a variety of ways."
			exit
			;;
		--prefix=*)
			prefix="${arg#--prefix=}"
			[ -d "${prefix}" ] || die "Not a directory: ${prefix}"
			;;
		-*)
			die "Unknown option: ${arg}"
			;;
		*)
			[ "x${configurations[${arg}]+set}" = xset ]                       \
				|| die "No configuration for: ${arg}"
			selection+=("${arg}")
			;;
	esac
done
unset arg

[ "x${selection+set}" = xset ] || selection=("${default_selection[@]}")

[ -e .b6353339-fae0-4c5f-83bf-66463a3bafc6.tag ]                              \
	|| die "You have to run this script from the top-level directory"

for cfg in "${selection[@]}"
do
	rm -rf "stage-${cfg}/"                                                    \
		&& mkdir "stage-${cfg}/"                                              \
		&& ( cd "stage-${cfg}/" && ${configurations[$cfg]} .. )               \
			|| die "Configuration failed: ${cfg}"
done
unset cfg

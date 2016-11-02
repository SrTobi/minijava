#! /bin/bash -eu

shopt -s nullglob

default_prefix="${HOME}/var"
default_tempdir="."

helptext="usage: get-boost [--tempdir=DIR] [--prefix=DIR] [--read-only] [CONFIG...]

Downloads and builds the Boost libraries currently used by the project in some
non-standard configurations that are needed for testing and installs them in a
local directory.

Options:

  --prefix=DIR    installation prefix (default: '${default_prefix}')
  --tempdir=DIR   use DIR as the working directory (default: '${default_tempdir}')
  --read-only     make installed files read-only
  --help          show help text and exit
  --version       show version text and exit

Available Configurations:

  gcc-release     GNU compiler with libstdc++ in release mode
  gcc-debug       GNU compiler with libstdc++ in debug mode
  clang           Clang compiler with libc++ standard library

When run with no arguments, builds all configurations.  Otherwise, only those
specified.
"

boost_dir="boost_1_62_0"
boost_tar="${boost_dir}.tar.bz2"
boost_url="https://sourceforge.net/projects/boost/files/boost/1.62.0/${boost_tar}"
boost_sha256="36c96b0f6155c98404091d8ceb48319a28279ca0333fba1ad8611eb90afb2ca0"

b2_common_flags=(
	--build-dir='build-temp'
	--with-program_options
	--with-filesystem
	--with-system
	--with-test
)

function b2_gcc_release {
	b2 "${b2_common_flags[@]}"                                                \
	   toolset='gcc'                                                          \
	   "$@"
}

function b2_gcc_debug {
	b2 "${b2_common_flags[@]}"                                                \
	   toolset='gcc'                                                          \
	   cxxflags='-D_GLIBCXX_DEBUG'                                            \
	   "$@"
}

function b2_clang {
	b2 "${b2_common_flags[@]}"                                                \
	   toolset='clang'                                                        \
	   cxxflags='-stdlib=libc++'                                              \
	   linkflags='-lc++ -lc++abi'                                             \
	   "$@"
}

declare -A configurations

configurations[gcc-release]+=b2_gcc_release
configurations[gcc-debug]+=b2_gcc_debug
configurations[clang]+=b2_clang

prefix="${default_prefix}"
tempdir="${default_tempdir}"
rdonly=0

declare -a selection

function die {
	echo "get-boost: error: $@" >&2
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
			echo "get-boost 1.0"
			echo "Download and build Boost libraries and install them locally."
			exit
			;;
		--prefix=*)
			prefix="${arg#--prefix=}"
			[ -d "${prefix}" ] || die "Not a directory: ${prefix}"
			;;
		--tempdir=*)
			tempdir="${arg#--tempdir=}"
			[ -d "${tempdir}" ] || die "Not a directory: ${tempdir}"
			;;
		--read-only)
			rdonly=1
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

[ "x${selection+set}" = xset ] || selection=("${!configurations[@]}")

cd "${tempdir}" || die "Cannot enter working-directory: ${tempdir}"

[ -r "${boost_tar}" ] || wget "${boost_url}" || die "Cannot download Boost"
echo "${boost_sha256}  ${boost_tar}" | sha256sum --check || die "Download corrupted"

for cfg in "${selection[@]}"
do
	targetprefix="${prefix}/${cfg}"
	if [ -d "${targetprefix}/" ]
	then
		chmod -R +w "${targetprefix}/" 2>/dev/null || :
		rm -rf "${targetprefix}/" || die "Cannot remove existing installation: ${targetprefix}/"
	fi
	rm -rf "${boost_dir}" && tar -xf "${boost_tar}" || die "Cannot extract Boost"
	( cd "${boost_dir}/" && ${configurations[$cfg]} --prefix="${targetprefix}" stage )
	( cd "${boost_dir}/" && ${configurations[$cfg]} --prefix="${targetprefix}" install )
	if [ ${rdonly} -gt 0 ]; then chmod -R -w "${targetprefix}/"; fi
	rm -rf "${boost_dir}/"
done

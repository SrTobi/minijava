#! /bin/bash -eu

shopt -s nullglob

helptext="usage: build [--keep-going] [--test[=HOW]] [--run] [--doxygen] [CONFIG...]

Builds and tests selected configurations of the project.  The configurations
have to be created via the 'configure' script beforehand.

Options:

  --test[=HOW]    run unit tests via test-runner HOW after successful build
  --run           try executing the built compiler
  --doxygen       create Doxygen documentation
  --keep-going    continue building other configurations after the first failure
  --help          show help text and exit
  --version       show version text and exit

The following test-runners are available:

  None            do nothing
  CMake           run 'cmake --build . --target test'
  CTest           run 'ctest -T test --output-on-failure'

If no configurations are selected, this script will build whatever
configuration directories it finds.  It is assumed that configuration '\${cfg}'
is found in directory 'stage-\${cfg}'.  If your directory contains directories
with names that match 'stage-*' but were not created by the 'configure' script,
you might well be alerted by now.
"

howtotest=None
dorun=0
dodocs=0
keepon=0

declare -a selection

function die {
	echo "build: error: $@" >&2
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
			echo "build 1.0"
			echo "Build and test pre-configured version of the project."
			exit
			;;
		--test)
			howtotest=CMake
			;;
		--test=*)
			arg="${arg#--test=}"
			case "${arg,,}"
			in
				none)
					howtotest=None
					;;
				cmake)
					howtotest=CMake
					;;
				ctest)
					howtotest=CTest
					;;
				*)
					die "Unknown test-runner: ${arg}"
					;;
			esac
			;;
		--run)
			dorun=1
			;;
		--doxygen)
			dodocs=1
			;;
		--no-doxygen)
			dodocs=0
			;;
		--keep-going)
			keepon=1
			;;
		-*)
			die "Unknown option: ${arg}"
			;;
		*)
			[ -d "./stage-${arg}/" ] || die "No configuration for '${arg}'"
			selection+=("${arg}")
			;;
	esac
done
unset arg

[ -e .b6353339-fae0-4c5f-83bf-66463a3bafc6.tag ]                              \
	|| die "You have to run this script from the top-level directory"

if [ "x${selection+set}" != xset ]
then
	for dir in ./stage-*/
	do
		dir="${dir#./stage-}"
		dir="${dir%/}"
		selection+=("${dir}")
	done
	unset dir
fi

[ "x${selection+set}" = xset ] || die "There is nothing to build"

declare -A stati

for cfg in "${selection[@]}"
do
	echo "Building configuration '${cfg}' ..."
	builddir="stage-${cfg}"
	status=
	if (
		cd "${builddir}/"
		if [ -d dynlib ]
		then
			if [ "x${LD_LIBRARY_PATH+set}" = xset ]
			then
				export LD_LIBRARY_PATH="${PWD}/dynlib/:${LD_LIBRARY_PATH}"
			else
				export LD_LIBRARY_PATH="${PWD}/dynlib/"
			fi
		fi
		export LSAN_OPTIONS=suppressions="${PWD}/../extras/asan/suppressions.txt"
		cmake --build . || exit
		[ ${dodocs} -eq 0 ] || cmake --build . --target docs 2>doxygen.err || exit
		if [ -s doxygen.err ]
		then
			cat doxygen.err >&2
			die "Doxygen warnings treated as errors"
		fi
		case ${howtotest}
		in
			CMake)
				cmake --build . --target test || exit
				;;
			CTest)
				ctest -T test --output-on-failure || exit
				;;
			None)
				;;
			*)
				die "This is a bug in the build script (howtotest='${howtotest}')"
				;;
		esac
		[ ${dorun} -eq 0 ] || ./src/minijava --version || exit
	); then status=success; else status=failure; fi
	[ ${status} = success ] || [ ${keepon} -gt 0 ] || die "Build failed: ${cfg}"
	stati[${cfg}]+=${status}
done
unset cfg builddir status

failures=0

echo ""
echo "=================================================="
echo ""
echo "  Build Summary:"
echo ""
echo "--------------------------------------------------"
echo ""
for cfg in "${selection[@]}"
do
	if [ "${stati[${cfg}]}" != success ]
	then
		failures=$((${failures} + 1))
	fi
	printf '  %-18s %s\n' "${cfg}" "${stati[${cfg}]}"
done
echo ""
echo "=================================================="
echo ""
unset cfg

exit ${failures}

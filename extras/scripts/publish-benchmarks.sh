#! /bin/bash -eu

shopt -s nullglob

default_dest='.'
default_python='python3'

helptext="usage: publish-benchmarks --micro=FILE --macro=FILE [--dest=DIR] [--out-tol=X] [--python=CMD]

Plots some selected benchmark results.

Options:

  --micro=FILE    use FILE as history database for micro-benchmarks
  --macro=FILE    use FILE as history database for macro-benchmarks
  --dest=DIR      publish results in directory DIR (default: '${default_dest}')
  --out-tol=X     eliminate outliers with relative stdev of more than X times the median
  --python=CMD    command to run a Pytohn 3 interpreter (default: '${default_python}')
  --help          show help text and exit
  --version       show version text and exit

This script must be run from the project's top-level directory.
"

unset micro
unset macro
outtol=inf
dest="${default_dest}"
python="${default_python}"

function die {
	echo "publish-benchmarks: error: $@" >&2
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
			echo "publish-benchmarks 1.0"
			echo "Plots some selected benchmark results"
			exit
			;;
		--micro=*)
			micro="${arg#--micro=}"
			[ -n "${micro}" ] || die "Empty string is not a valid file-name"
			;;
		--macro=*)
			macro="${arg#--macro=}"
			[ -n "${macro}" ] || die "Empty string is not a valid file-name"
			;;
		--dest=*)
			dest="${arg#--dest=}"
			[ -n "${dest}" ] || die "Empty string is not a valid directory"
			;;
		--out-tol=*)
			outtol="${arg#--out-tol=}"
			printf '%f' "${outtol}" 1>/dev/null 2>/dev/null                   \
				|| die "Outlier tolerance must be a positive real number"
			;;
		--python=*)
			python="${arg#--python=}"
			[ -n "${python}" ] || die "Empty string is not a valid command"
			;;
		*)
			die "Unknown option or argument: ${arg}"
			;;
	esac
done
unset arg

[ "x${micro+set}" = xset ] || die "The --micro option is required"
[ "x${macro+set}" = xset ] || die "The --macro option is required"

[ -e .b6353339-fae0-4c5f-83bf-66463a3bafc6.tag ]                              \
	|| die "You have to run this script from the top-level directory"

[ -f "${micro}" ] || die "Database file does not exist: ${micro}"
[ -f "${macro}" ] || die "Database file does not exist: ${macro}"
[ -d "${dest}"  ] || die "Directory does not exist: ${dest}"

"${python}" --version 2>&1 | grep -q 'Python\s\+3[.][0-9.]*'                  \
	|| die "Does not seem to run a Python 3 interpreter: ${python}"

"${python}" -c 'print("hello", "world", sep="+")' 1>/dev/null 2>/dev/null     \
	|| die "Does not seem to run a Python 3 interpreter: ${python}"

function cleanup {
	if [ "x${tempdir+set}" = xset ]; then rm -rf "${tempdir}"; fi
}

trap cleanup EXIT

tempdir="$(mktemp -d)" || die "Cannot create temporary directory"

mkdir -p "${tempdir}/micro/"
mkdir -p "${tempdir}/macro/"


function export-micro {
	local name
	for name in "$@"
	do
		"${python}" "extras/benchmarks/history.py"                            \
			-H "${micro}"                                                     \
			'export' "${name}" "${outtol}" > "${tempdir}/micro/${name}.dat"
	done
}


function export-macro {
	local name
	for name in "$@"
	do
		"${python}" "extras/benchmarks/history.py"                            \
			-H "${macro}"                                                     \
			'export' "${name}" "${outtol}" > "${tempdir}/macro/${name}.dat"
	done
}


# -------------------------------------- #
#   Character Classification Functions   #
# -------------------------------------- #

export-micro character-space character-digit character-head character-tail

(cd "${tempdir}/micro/" && gnuplot) <<'EOF'
set terminal svg noenhanced size 800,600
set output 'character.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "classification time per character / ns"
set xrange [* : *]
set yrange [0 : *]
set format y '%.1f'
set title "Character Classification"
plot                                                                          \
	'character-space.dat'                                                     \
		using 1:(1e9 * $2):(1e9 * $3) with yerror                             \
		title "is_space",                                                     \
	'character-digit.dat'                                                     \
		using 1:(1e9 * $2):(1e9 * $3) with yerror                             \
		title "is_digit",                                                     \
	'character-head.dat'                                                      \
		using 1:(1e9 * $2):(1e9 * $3) with yerror                             \
		title "is_word_head",                                                 \
	'character-tail.dat'                                                      \
		using 1:(1e9 * $2):(1e9 * $3) with yerror                             \
		title "is_word_tail"
EOF


# -------------------------- #
#   Keyword Classification   #
# -------------------------- #

export-micro keyword

(cd "${tempdir}/micro/" && gnuplot) <<'EOF'
set terminal svg size 800,600
set output 'keyword.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "classification time per word / ns"
set xrange [* : *]
set yrange [0 : *]
set format y '%.0f'
set nokey
set title "Keyword Classification"
plot                                                                          \
	'keyword.dat'                                                             \
		using 1:(1e9 * $2):(1e9 * $3) with yerror                             \
		notitle
EOF


# ---------------------------- #
#   Toke-Type-Set Operations   #
# ---------------------------- #

export-micro tts-modify tts-lookup tts-combine

(cd "${tempdir}/micro/" && gnuplot) <<'EOF'
set terminal svg size 800,600
set output 'token-type-set.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "time per operation / ns"
set xrange [* : *]
set yrange [0 : *]
set format y '%.1f'
set title "Token-Type-Set Operations"
plot                                                                          \
	'tts-modify.dat'                                                          \
		using 1:(1e9 * $2):(1e9 * $3) with yerror                             \
		title "modify",                                                       \
	'tts-lookup.dat'                                                          \
		using 1:(1e9 * $2):(1e9 * $3) with yerror                             \
		title "lookup",                                                       \
	'tts-combine.dat'                                                         \
		using 1:(1e9 * $2):(1e9 * $3) with yerror                             \
		title "combine"
EOF


# --------------------------- #
#   Lexer (Micro-Benchmark)   #
# --------------------------- #

export-micro lexer

(cd "${tempdir}/micro/" && gnuplot) <<'EOF'
set terminal svg size 800,600
set output 'lexer.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "average time per token / us"
set xrange [* : *]
set yrange [0 : *]
set format y '%.2f'
set nokey
set title "Lexer (Micro-Benchmark)"
plot                                                                          \
	'lexer.dat'                                                               \
		using 1:(1e6 * $2):(1e6 * $3) with yerror                             \
		notitle
EOF


# ---------------------------- #
#   Parser (Micro-Benchmark)   #
# ---------------------------- #

export-micro parser

(cd "${tempdir}/micro/" && gnuplot) <<'EOF'
set terminal svg enhanced size 800,600
set output 'parser.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "average time per token / µs"
set xrange [* : *]
set yrange [0 : *]
set format y '%.2f'
set nokey
set title "Parser (Micro-Benchmark)"
plot                                                                          \
	'parser.dat'                                                              \
		using 1:(1e6 * $2):(1e6 * $3) with yerror                             \
		notitle
EOF


# -------------------------- #
#   Echo (Macro-Benchmark)   #
# -------------------------- #

export-macro echo-002-a echo-002-b echo-002-c echo-002-d echo-002-e echo-002-f

(cd "${tempdir}/macro/" && gnuplot) <<'EOF'
set terminal svg size 800,600
set output 'echo.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "execution time / s"
set xrange [* : *]
set yrange [0 : *]
set format y '%.2f'
set title "Copying 100 MiB Input to Output (--echo)"
plot                                                                          \
	'echo-002-a.dat'                                                          \
		using 1:2:3 with yerror                                               \
		title "file --> file",                                                \
	'echo-002-b.dat'                                                          \
		using 1:2:3 with yerror                                               \
		title "file --> stdout",                                              \
	'echo-002-c.dat'                                                          \
		using 1:2:3 with yerror                                               \
		title "stdin --> file",                                               \
	'echo-002-d.dat'                                                          \
		using 1:2:3 with yerror                                               \
		title "stdin --> stdout",                                             \
	'echo-002-e.dat'                                                          \
		using 1:2:3 with yerror                                               \
		title "file --> /dev/null",                                           \
	'echo-002-f.dat'                                                          \
		using 1:2:3 with yerror                                               \
		title "stdin --> /dev/null"
EOF


# --------------------------- #
#   Lexer (Macro-Benchmark)   #
# --------------------------- #

export-macro lexer-002 lexer-003 lexer-004-a lexer-004-b lexer-005

(cd "${tempdir}/macro/" && gnuplot) <<'EOF'
set terminal svg enhanced size 800,600
set output 'lexer.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "execution time / s"
set xrange [* : *]
set yrange [0 : *]
set format y '%.2f'
set title "Running the Lexer and Printing Scanned Tokens (--lextest)"
plot                                                                          \
	'lexer-002.dat'                                                           \
		using 1:2:3 with yerror                                               \
		title "empty file",                                                   \
	'lexer-003.dat'                                                           \
		using 1:2:3 with yerror                                               \
		title "single 100 MiB identifier",                                    \
	'lexer-004-a.dat'                                                         \
		using 1:2:3 with yerror                                               \
		title "1M different identifiers",                                     \
	'lexer-004-b.dat'                                                         \
		using 1:2:3 with yerror                                               \
		title "same identifier repeated 1M times",                            \
	'lexer-005.dat'                                                           \
		using 1:2:3 with yerror                                               \
		title "excessive repetition of block comments"
EOF


# -------------------------------- #
#   Lexer Fuzz (Macro-Benchmark)   #
# -------------------------------- #

export-macro lexer-fuzz-191

(cd "${tempdir}/macro/" && gnuplot) <<'EOF'
set terminal svg enhanced size 800,600
set output 'lexer-fuzz.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "execution time / ms MiB^{-1}"
set xrange [* : *]
set yrange [0 : *]
set format y '%.0f'
set title "Lexical Analysis and Printing Tokens (--lextest)"
plot                                                                          \
	'lexer-fuzz-191.dat'                                                      \
		using 1:($2 / 190.9e-3):($3 / 190.9e-3) with yerror                   \
		title "191 MiB"
EOF


# --------------------------------- #
#   Parser Fuzz (Macro-Benchmark)   #
# --------------------------------- #

export-macro parser-fuzz-1 parser-fuzz-5 parser-fuzz-20 parser-fuzz-33

(cd "${tempdir}/macro/" && gnuplot) <<'EOF'
set terminal svg enhanced size 800,600
set output 'parser-fuzz.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "execution time / ms MiB^{-1}"
set xrange [* : *]
set yrange [0 : *]
set format y '%.0f'
set title "Syntactic Analysys (--parsetest)"
plot                                                                          \
	'parser-fuzz-1.dat'                                                       \
		using 1:($2 / 1.059e-3):($3 / 1.059e-3) with yerror                   \
		title "1 MiB",                                                        \
	'parser-fuzz-5.dat'                                                       \
		using 1:($2 / 5.173e-3):($3 / 5.173e-3) with yerror                   \
		title "5 MiB",                                                        \
	'parser-fuzz-20.dat'                                                      \
		using 1:($2 / 19.31e-3):($3 / 19.31e-3) with yerror                   \
		title "20 MiB",                                                       \
	'parser-fuzz-33.dat'                                                      \
		using 1:($2 / 32.34e-3):($3 / 32.34e-3) with yerror                   \
		title "33 MiB"
EOF


# ----------------------------------------- #
#   Pretty-Printer Fuzz (Macro-Benchmark)   #
# ----------------------------------------- #

export-macro pretty-fuzz-20

(cd "${tempdir}/macro/" && gnuplot) <<'EOF'
set terminal svg enhanced size 800,600
set output 'pretty-fuzz.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "execution time / ms MiB^{-1}"
set xrange [* : *]
set yrange [0 : *]
set format y '%.0f'
set title "Syntactic Analysis and Printing of AST (--lextest)"
plot                                                                          \
	'pretty-fuzz-20.dat'                                                      \
		using 1:($2 / 19.31e-3):($3 / 19.31e-3) with yerror                   \
		title "20 MiB"
EOF


# -------------------------------- #
#   Check Fuzz (Macro-Benchmark)   #
# -------------------------------- #

export-macro check-fuzz-12 check-fuzz-48 check-fuzz-90 check-fuzz-119

(cd "${tempdir}/macro/" && gnuplot) <<'EOF'
set terminal svg enhanced size 800,600
set output 'parser-fuzz.svg'
set xdata time
set timefmt '%s'
set xlabel "point in history"
set ylabel "execution time / ms MiB^{-1}"
set xrange [* : *]
set yrange [0 : *]
set format y '%.0f'
set title "Semantic Analysys (--check)"
plot                                                                          \
	'check-fuzz-12.dat'                                                       \
		using 1:($2 / 11.46e-3):($3 / 11.46e-3) with yerror                   \
		title "12 MiB",                                                       \
	'check-fuzz-48.dat'                                                       \
		using 1:($2 / 47.36e-3):($3 / 47.36e-3) with yerror                   \
		title "48 MiB",                                                       \
	'check-fuzz-90.dat'                                                       \
		using 1:($2 / 89.10e-3):($3 / 89.10e-3) with yerror                   \
		title "90 MiB",                                                       \
	'check-fuzz-119.dat'                                                      \
		using 1:($2 / 118.5e-3):($3 / 118.5e-3) with yerror                   \
		title "119 MiB"
EOF


# ----------------------- #
#   Publish the Results   #
# ----------------------- #

install -m 0755 -d "${dest}/micro/"
install -m 0755 -d "${dest}/macro/"

install -m 0644 -t "${dest}/micro/" "${tempdir}/micro"/*.svg
install -m 0644 -t "${dest}/macro/" "${tempdir}/macro"/*.svg

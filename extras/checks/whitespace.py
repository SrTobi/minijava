#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-


import argparse
import itertools as it
import os
import shutil
import subprocess
import sys


# Number of spaces a tab is expanded to.
tab_width = 4

# Maximum allowed number of consecutive blank lines.
max_vskip = 2


class StyleError(Exception):

	"""
	Exception used to signal style guide violations.

	"""

	pass


def main(args):
	"""
	Runs the program.

	@param args : [str]
		command-line arguments (without program name)

	@returns : int
		exit status

	"""
	ap = argparse.ArgumentParser(
		prog='whitespace',
		usage="%(prog)s [--directory=DIR] [--summary | --quiet] [--fix] [--] [FILE...]",
		description=(
			"Checks white-space in files for conformance with the style"
			+ " guide.  If no files are specified, all regular files reported"
			+ " by 'git ls-files' are checked.  The exit status is the number"
			+ " of non-conforming files or 1 in case of an error.  All"
			+ " arguments after the first '--' will be interpreted as file"
			+ " names and never as options.  '-' is not a valid argument."
		),
		epilog=(
			"All checks are conservative and aim for zero false positives at"
			+ " the cost of potentially many false negatives.  Therefore, if"
			+ " an issue is reported, it is real and needs fixing.  The"
			+ " automatic repair (via the '--fix' option), on the other hand,"
			+ " is not perfect and cannot settle all issues.  It also might"
			+ " apply fixes you don't like so you should carefully check your"
			+ " files afterwards.  If you run it over file '{}', the original"
			+ " file will be backed up as '{}' in case you want to restore it"
			+ " or generate a diff."
		).format('foo.txt', make_backup_file_name('foo.txt'))
	)
	ap.add_argument(
		'filenames', metavar='FILE', nargs='*',
		type=reasonable_file_name, help="regular file to check"
	)
	ap.add_argument(
		'-s', '--summary', action='store_true',
		help="print only one line per file"
	)
	ap.add_argument(
		'-q', '--quiet', action='store_true',
		help="don't produce any output"
	)
	ap.add_argument(
		'-f', '--fix', action='store_true',
		help="automatically fix some issues"
	)
	ap.add_argument(
		'-C', '--directory', metavar='DIR',
		help="enter directory DIR before doing anything"
	)
	ns = ap.parse_args(args)
	if ns.directory is not None:
		os.chdir(ns.directory)
	for fn in it.filterfalse(is_regular_file, ns.filenames):
		raise ValueError("Not a regular file: {:s}".format(fn))
	with open(os.devnull, 'a') as devnull:
		return check_files(
			ns.filenames if ns.filenames else git_ls_files(),
			ns.fix,
			sys.stderr if not ns.quiet and not ns.summary else devnull,
			sys.stderr if not ns.quiet and     ns.summary else devnull,
		)


def check_files(filenames, fix, verboseout, summaryout):
	"""
	Checks zero or more files for style guide violations and optionally fixes
	them in-place.

	@param filenames : [str]
		file names of the files to check

	@param fix : bool
		whether to fix violations inline

	@param verboseout : file-like
		terminal to print per-line violations to

	@param summaryout : file-like
		terminal to print per-file violations to

	@returns : int
		number of non-conforming files

	"""
	okmsg = "OK" if not fix else "fixed"
	badmsg = "non-conforming"
	bad_files = 0
	for fn in filenames:
		lines = read_file_and_maybe_fix_it(fn, fix)
		if check_content(fn, lines, verboseout):
			print("{:s}: {}".format(fn, okmsg), file=summaryout)
		else:
			bad_files += 1
			msg = "{:s}: {}".format(fn, badmsg)
			print(msg, file=summaryout)
	return bad_files


def check_content(filename, lines, verboseout):
	"""
	Checks the entire content of a single file for style guide violations.

	@param filename : str
		name of the file (only used for diagnostic messages)

	@param lines : [str]
		contents (lines) of the file

	@param fix : bool
		whether to fix violations inline

	@returns : bool
		whether the file was conforming

	"""
	ok = True
	try:
		check_vertical(lines)
	except StyleError as e:
		ok = False
		msg = "{:s}: {:s}".format(filename, str(e))
		print(msg, file=verboseout)
	for (i, line) in enumerate(lines):
		try:
			check_horizontal(line)
		except StyleError as e:
			ok = False
			msg = "{:s}:{:d}: {:s}".format(filename, i + 1, str(e))
			print(msg, file=verboseout)
	return ok


def check_vertical(lines):
	"""
	Checks the lines of a file for vertical white-space issues.

	@param lines : [str]
		lines of the file

	@raises StyleError
		if vertical white-space issues are found

	"""
	blank_eh = list(map(is_blank, lines))
	filtergroup = lambda pred, seq : filter(pred, it.groupby(seq))
	for blank in it.takewhile(identity, blank_eh):
		raise StyleError("Blank lines at beginning of file")
	for blank in it.takewhile(identity, reversed(blank_eh)):
		raise StyleError("Blank lines at end of file")
	if max(map(count, map(second, filtergroup(first, blank_eh))), default=0) > max_vskip:
		raise StyleError("File contains excessive vertical white-space")
	if lines and not lines[-1].endswith('\n'):
		raise StyleError("No new-line character at end of last line")


def check_horizontal(line):
	"""
	Checks a single line of a file for horizontal white-space issues.

	@param line : str
		line to check (including trailing newline character)

	@raises StyleError
		if horizontal white-space issues are found

	"""
	line = line.rstrip('\n')
	if line.rstrip() != line:
		raise StyleError("Line has trailing white-space")
	if '\t' in line.lstrip('\t'):
		raise StyleError("Tabs may only be used for indentation")


def fix_vertical(lines):
	"""
	Fixes some issues with vertical white-space in a line.

	@param lines : [str]
		lines of the file to fix

	@returns [str]
		fixed lines

	"""
	tag = lambda pred, seq : zip(map(pred, seq), seq)
	flatten = it.chain.from_iterable
	flatmap = lambda func, seq : flatten(map(func, seq))
	listmap = lambda func, seq : list(map(func, seq))
	revlist = lambda seq : reversed(list(seq))
	dwf = lambda seq : it.dropwhile(first, seq)
	return listmap(
		# Strip the enumerator and predicate off again.
		lambda x : second(second(x)),
		filter(
			# Pick all lines from non-blank groups and the first max_vskip
			# lines from blank groups.
			lambda x : not first(second(x)) or first(x) < max_vskip,
			flatmap(
				second,
				map(
					lambda pair : (pair[0], enumerate(pair[1])),
					it.groupby(
						# Trim leading and trailing blank lines.
						dwf(revlist(dwf(revlist(tag(is_blank, lines))))),
						first)))))


def fix_horizontal(line):
	"""
	Fixes some issues with horizontal white-space in a line.

	@param line : str
		line to fix

	@returns str
		fixed line with trailing new-line character

	"""
	line = line.rstrip()
	line = untabify(line, tab_width)
	return line + '\n'


def is_blank(line):
	"""
	Tests whether a line contains only white-space.

	@param line : str
		line to test

	@returns bool
		whether the line consists entirely of white-space

	"""
	return not bool(line.strip())


def untabify(line, tabwidth):
	"""
	Expands all but leading tabs to spaces.

	@param line : str
		line to untabify

	@param tabwidth : int
		characters per tab

	@returns str
		untabified line

	"""
	assert tabwidth > 0
	body = list()
	i = 0
	while i < len(line) and line[i] == '\t':
		i += 1
	indent = '\t' * i
	while i < len(line):
		if line[i] == '\t':
			skip = tabwidth - (len(body) % tabwidth)
			body.extend(' ' for i in range(skip))
		else:
			body.append(line[i])
		i += 1
	return indent + ''.join(body)


def read_file_and_maybe_fix_it(filename, fix):
	"""
	`return`s the lines of a file as a `list` and optionally fixes it in-place.

	If `fix` is `False`, the function will merely read the file.  If `fix` is
	`True`, it will read the file, fix the contents, rename the file to a
	backup file, write back the fixed contents to the original file and finally
	`return` the fixed contents.

	@param filename : str
		name of the file

	@param fix : bool
		whether to fix violations inline

	@returns : [str]
		optionally fixed lines of the file

	"""
	with open(filename, 'r') as istr:
		lines = istr.readlines()
	if fix:
		lines = list(map(fix_horizontal, fix_vertical(lines)))
		backupname = make_backup_file_name(filename)
		shutil.copy(filename, backupname)
		with open(filename, 'w') as ostr:
			ostr.writelines(lines)
	return lines


def make_backup_file_name(filename):
	"""
	Given a file name, `return`s the file name for the corresponding backup
	file.

	@param filename : str
		file name for the original file

	@returns str
		file name for the backup file

	@raises ValueError
		if the file name already looks like a backup file name

	"""
	backup_suffix = '~' if os.name == 'posix' else '.backup'
	if filename.endswith(backup_suffix):
		raise ValueError("Looks like a backup file: " + filename)
	return filename + backup_suffix


def git_ls_files():
	"""
	`return`s a list of of version-controlled files as reported by
	`git ls-files` filtered to contain only regular files.

	@returns [str]
		list of file names

	@raises OSError
		if running Git fails

	"""
	proc = subprocess.Popen(
		['git', 'ls-files'],
		stdin=subprocess.DEVNULL,
		stdout=subprocess.PIPE,
		stderr=None
	)
	(stdout, stderr) = proc.communicate()
	if proc.returncode != 0:
		raise OSError("Cannot list version-controlled files")
	filenames = stdout.decode().split()
	return list(filter(is_regular_file, filenames))


def is_regular_file(filename):
	"""
	Checks whether `filename` corresponds to a regular file and is not a
	symbolic link.

	@param filename : str
		file name to check

	@returns bool
		whether `filename` is a regular file

	"""
	return os.path.isfile(filename) and not os.path.islink(filename)


def reasonable_file_name(filename):
	"""
	`return`s its argument unchanged except when it looks like a reasonable
	file name or otherwise `raise` a `ValueError`.

	@param filename : str
		file name to check

	@returns str
		filename

	@raises ValueError
		if `filename` does not look like a file name

	"""
	if filename == '-':
		raise ValueError("Not a valid file name: " + filename)
	return filename


def identity(x):
	"""
	`return`s its argument.
	"""
	return x


def first(pair):
	"""
	Obtains the first element in a pair.

	"""
	return pair[0]


def second(pair):
	"""
	Obtains the second element in a pair.

	"""
	return pair[1]


def count(seq):
	"""
	Counts the elements in a sequence.

	"""
	return sum(1 for x in seq)


if __name__ == '__main__':
	try:
		sys.exit(main(sys.argv[1:]))
	except Exception as e:
		print(e, file=sys.stderr)
		sys.exit(1)

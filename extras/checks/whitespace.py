#! /usr/bin/python3
#! -*- coding:utf-8; mode:python; -*-


import argparse
import os
import shutil
import subprocess
import sys


default_tab_width = 4


class StyleError(Exception):

	"""
	Exception used to signal style guide violations.

	"""

	pass


class NullContext(object):

	"""
	A context manager that manages nothing.

	"""

	def __enter__(self):
		return self

	def __exit__(self, exec_type, exec_val, exec_tb):
		pass


def maybe_open(filename, mode='r'):
	"""
	Like the builtin `open` function except that the `filename` may be `None`
	in which case a `NullContext` is `returned.

	@param filename : str | NoneType
		name of the file to open or `None`

	@param mode : str
		file-open mode (forwarded to `open`)

	@returns file-like | NullContext
		open file or `NullContext`

	@raises OSError
		if `raise`d by `open`

	"""
	if filename is None:
		return NullContext()
	else:
		return open(filename, mode)


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


def check_line(line):
	"""
	Checks a single line for conformance with the style guide.

	@param line : str
		line to check

	@raises StyleError
		if the line is non-conforming

	"""
	if not line.endswith('\n'):
		raise StyleError("No new-line character at end of line")
	line = line[:-1]
	if line.rstrip() != line:
		raise StyleError("Line has trailing white-space")
	if '\t' in line.lstrip('\t'):
		raise StyleError("Tabs may only be used for indentation")


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


def fix_line(line):
	"""
	Fixes some style guide violations in a line.

	For pretty obvious reasons, this function cannot fix all violations and has
	to apply conservative heuristics for the ones it can fix.  In particular,
	`check_line(fix_line(text) + '\n')` totally can fail.

	@param line : str
		line to fix

	@returns str
		fixed line without trailing new-line character

	"""
	line = line.rstrip()
	line = untabify(line, default_tab_width)
	return line


def check_file(filename, fix, verboseout):
	"""
	Checks a single file for style guide violations and optionally fixes (some
	of) them in-place.

	@param filename : str
		name of the file

	@param fix : bool
		whether to fix violations inline

	@param verboseout : file-like
		terminal to print violations to

	@returns : int
		number of violations found

	"""
	violations = 0
	infilename = filename
	if fix:
		infilename = make_backup_file_name(filename)
		shutil.copy(filename, infilename, follow_symlinks=False)
	with maybe_open(filename if fix else None, 'w') as ostr:
		with open(infilename, 'r') as istr:
			for (i, line) in enumerate(istr):
				try:
					check_line(line)
				except StyleError as e:
					violations += 1
					msg = "{:s}:{:d}: {:s}".format(filename, i + 1, str(e))
					print(msg, file=verboseout)
				if fix:
					print(fix_line(line), file=ostr)
	return violations


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
	for fn in filenames:
		if not is_regular_file(fn):
			raise ValueError("Not a regular file: " + fn)
	bad_files = 0
	for fn in filenames:
		violations = check_file(fn, fix, verboseout)
		if violations > 0:
			bad_files += 1
			msg = "{:s}: {:d} violations found".format(fn, violations)
			print(msg, file=summaryout)
		else:
			print("{:s}: ok".format(fn), file=summaryout)
	return bad_files


def real_file_name(filename):
	"""
	`return`s its argument unchanged except when it is `-` in which case a
	`ValueError` is `raise`d.

	@param filename : str
		file name to check

	@returns str
		filename

	@raises ValueError
		if `filename` is `-`

	"""
	if filename == '-':
		raise ValueError("Not a valid file name: " + filename)
	return filename


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
		type=real_file_name, help="regular file to check"
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
		help="Automatically fix some issues"
	)
	ap.add_argument(
		'-C', '--directory', metavar='DIR',
		help="enter directory DIR before doing anything"
	)
	ns = ap.parse_args(args)
	if ns.directory is not None:
		os.chdir(ns.directory)
	with open(os.devnull, 'a') as devnull:
		return check_files(
			ns.filenames if ns.filenames else git_ls_files(),
			ns.fix,
			sys.stderr if not ns.quiet and not ns.summary else devnull,
			sys.stderr if not ns.quiet and     ns.summary else devnull,
		)


if __name__ == '__main__':
	try:
		sys.exit(main(sys.argv[1:]))
	except Exception as e:
		print(e, file=sys.stderr)
		sys.exit(1)

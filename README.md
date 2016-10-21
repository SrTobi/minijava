# Compiler-Engineering WS&nbsp;2016/17 &ndash; Team&nbsp;2


## Requirements

In order to build the compiler, you will need the following.

 - CMake&nbsp;3.0 or later and a working native build system it can use as
   backend.

 - A C++ compiler that supports all of C++14 (with `-std=c++14` or `-std=c++1y`
   flags).

 - The following Boost libraries which must be installed in a way such that
   CMake can find them.

    - [*Filesystem*](http://www.boost.org/doc/libs/1_62_0/libs/filesystem/doc/index.htm)
    - [*Program Options*](http://www.boost.org/doc/libs/1_62_0/doc/html/program_options.html)
    - [*System*](http://www.boost.org/doc/libs/1_62_0/libs/system/doc/index.html)
    - [*Test*](http://www.boost.org/doc/libs/1_62_0/libs/test/doc/html/index.html)

   We recommend that you use the latest and greatest Boost version 1.62 as of
   October 2016.  Somewhat older versions might work, too &ndash; or they might
   not.

 - In order to check out the sources, you'll also need Git.

We have tested the setup on POSIX and Windows with MinGW or Cygwin and wish you
Good Luck if you're trying anything else.


## Building for Humans

    $ git clone https://git@slixe.de:7999/comprak/compiler.git
    $ cd compiler/
    $ mkdir ${builddir}/
    $ cd ${builddir}/
    $ cmake ..
    $ make
    $ make test

The `make test` is optional and will run all unit tests.

The compiler executable can be found in `${builddir}/src/minijava`.  Running it
with the `--help` option will give you some information how to use it.


## Building for Robots

    $ git clone https://git@slixe.de:7999/comprak/compiler.git

The top-level directory contains the shell scripts `build` and `run`.  If
executed from the top-level directory, `build` (with no arguments) configures
and builds the project and `run` invokes the compiler, forwarding it any and
all arguments that were passed to it.

This will only work on POSIX systems.  Since the scripts don't check who
invokes them, they actually work for humans, too.


## Contributing

Please develop your feature on a separate branch named
`${yourname}/${yourfeature}`.  You can push to that branch whenever you like.
Please keep commits clean.  A single commit should contain a single logical
change.  Also be sure to use descriptive commit messages with a short sentence
that summarizes your change as the first line followed by a blank line and
possibly more text after that.

> *Moritz Klammler:* I prefer to write commit messages in `ChangeLog` format.

Once your feature is ready (which could be as soon as after the first commit),
create a pull request to merge your branch with `master`.  Normally, you can
merge if you got a single positive review but if your changes are likely to
break things for other people, better give it more time.

If your change is obvious and non-breaking (fixing typos, comments, formatting,
&hellip;) you can also push it directly to `master`.  If you do so, be careful
not to cause more trouble than necessary for those who have branched off
`master` and eventually want to merge back.

Make sure that `git status` shows no false positives.  If necessary, update
`.gitignore` accordingly.  Never check binary files, large PDFs, images or
other crap into the repository.


## Coding Standards

Use standards-compliant modern idiomatic C++14.  Never invoke undefined
behavior and don't rely on non-standard features unless we have agreed upon
that they can be used.  All code must compile on GCC without warnings at the
`-Wall`, `-Wextra` and `-pedantic` level.

Good resources for modern coding standards can be found here.

 - [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)
 - [&ldquo;C++ Coding Standards&rdquo; by Herb Sutter and Andrei Alexandrescu](http://www.gotw.ca/publications/c++cs.htm)
 - [Scott Meyers &ldquo;Effective C++&rdquo; book series](http://www.aristeia.com/books.html)

Ignore the [Google C++ Style
Guide](https://google.github.io/styleguide/cppguide.html).  While some parts of
it are indisputable, it also contains a lot of poor advice that might work for
Google but is undesirable in general.  Other highly respected guidelines like
the [MISRA](http://www.programmingresearch.com/coding-standards/misra/) or
[JSF](http://www.stroustrup.com/JSF-AV-rules.pdf) standards are full of good
tips that anybody can profit from but too specific for their very special
domains to be applicable in general.

We have agreed to use the following non-standard extensions to C++.

 - Put `#pragma once` as the first line of code in a header file instead of
   traditional `#include` guards.


## Style Guide

We use the naming convention from the C++ standard library.  (All C++
identifiers are in lower-case and all macros are in upper-case.  Words are
separated by underscores.)

Except for `main`, all C++ symbols live in the `minijava` name-space and
pre-processor macros are prefixed with `MINIJAVA_`.  The unit-test support
library uses the `testaux` name-space for C++ and prefixes pre-processor macros
with `TESTAUX_`.

`private` members have a single `_` prefixed to their name.  (Never use two
consecutive underscores or a leading underscore followed by an upper-case
character because these identifiers are reserved and using them invokes
undefined behavior.)

We use the same style as the
[Firm](http://pp.ipd.kit.edu/firm/Coding_Conventions) project as far as it is
applicable to C++ and unless a deviation is mentioned below.  In a nutshell,
this is &ldquo;Stroustrup&rdquo; style with tabs for indentation.

> *Moritz Klammler:* I hate their convention to put the `*` of a pointer
> declaration next to the identifier.  This is in disagreement to what
> Stroustrup recommends.  It also doesn't work if you have `const`-qualifiers
> on the pointer.  Personally, I prefer putting a space on either side of the
> `*` (which is also not what Stroustrup recommends).

Don't assume that a tab always has the same width it has in your
editor.  (Therefore, only use tabs for indentation and spaces for
justification.)  Try to keep lines below 80 characters where each tab counts as
four characters for this purpose.
Meticulously trim trailing white-space and end each file with a new-line
character.  (Or teach your editor to do these things automatically for you.)

Document all public parts of components with Doxygen DocStrings that
clearly state the semantics and the contract of your functions and other
stuff.  Use JavaDoc style with Markdown formatting that looks like this.

    /**
     * @brief
     *         Computes the longest-common sub-string of two character strings.
     *
     * If both arguments are different from zero and either of them is not a
     * NUL-terminated character string, the behavior is undefined.
     *
     * @param s1
     *         first string
     *
     * @param s2
     *         second string
     *
     * @returns
     *         longest common sub-string of `s1` and `s2`
     *
     * @throws std::invalid_argument
     *         if either `s1` or `s2` are `nullptr`s
     *
     */
     std::string lcs(const char * s1, const char * s2);

If you have too much time at your disposal, consider watching John Lakos' great
talk &ldquo;Defensive Programming Done Right&rdquo;
([1<sup>st</sup>&nbsp;part](https://youtu.be/1QhtXRMp3Hg) and
[2<sup>nd</sup>&nbsp;part](https://youtu.be/tz2khnjnUx8)).  Use `assert` to
check for contract violations when feasible.  Do not use exceptions for this.

Use English for code and comments and prefer the American spelling when in
doubt.

Apply the same rules to CMake files and other source code as far as they are
applicable.

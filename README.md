# Compiler Engineering Winter Term &nbsp;2016/17 &ndash; Team&nbsp;2

This is the repository for the MiniJava compiler developed by team 2.


## Compliance with the Specification

This compiler aims to fully adhere to the [MiniJava specification](https://pp.info.uni-karlsruhe.de/lehre/WS201617/compprakt/intern/sprachbericht.pdf),
which is not completely unambiguous and therefore leaves room for
implementation-defined behavior.  Our interpretation of the specification
differs slightly from the official interpretation.  We believe that these
deviations are both within the boundaries set by the specification and
consistent as a whole.

This section describes the differences between our compiler and the behavior
expected by the official test suite.


### Treatment of `main`

Since static fields and methods do not exist in MiniJava, `main` is not treated
like a static method in our compiler.  Instead, it is merely regarded as a
syntactic construct for the program's main entry point.

As a result, our compiler will accept every program that is unquestionably valid
MiniJava according to the specification.  Furthermore, it gives
implementation-defined meaning to a few programs that other compliant MiniJava
compilers might reject.

In particular, the following two programs are regarded as valid:

```java
class Main {
    public int System;
    public static void main(String[] args) {
        System.out.println(0);
    }
}
```

Within `main`, all fields inside the enclosing class are completely ignored
during name lookup.

```java
class String {}
class Main {
    public void main(String[] args) {}
    public static void main(String[] args) {}
}
```

Since it is impossible to call `main` and since the actual type of the only
argument of `main` cannot be used or expressed in MiniJava, `main` will never
conflict with an instance method inside the same class, even if the argument
lists look the same.


## Developer Documentation

### Requirements

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
   
 - On Windows, you'll need to compile and install an
   [additional regex library](https://sourceforge.net/projects/mingw/files/Other/UserContributed/regex/mingw-regex-2.5.1/).
   Both the headers and the library itself must be installed into a folder where
   your compiler will find them, for example the MinGW `include` and `bin`
   directories.

 - In order to check out the sources, you'll also need Git.

 - If you want to build the HTML documentation (which is purely optional), you
   also need Doxygen.

We have tested the setup on POSIX and Windows with MinGW or Cygwin and wish you
Good Luck if you're trying anything else.


#### Firm

The project also depends on the [Firm](http://pp.ipd.kit.edu/firm/) library.
Unlike the other dependencies, it will be automatically downloaded and built
locally by the build system.  This will make a HTTP connection to a Git server
at the KIT.

If you don't want to connect to the KIT server, you can provide an alternative
URL to clone the repository from by setting the environment variable
`LIBFIRM_GIT_URL` prior to running CMake.

For example, the following commands will create a local mirror of the Firm
repository and then instruct CMake to clone from it.  Of course, now it is your
responsibility to keep the local mirror up to date.

    $ git clone --mirror http://pp.ipd.kit.edu/git/libfirm.git
    $ export LIBFIRM_GIT_URL="${PWD}/libfirm.git"


### Building for Humans

    $ git clone https://git@slixe.de:7999/comprak/compiler.git
    $ cd compiler/
    $ mkdir ${builddir}/
    $ cd ${builddir}/
    $ cmake ..
    $ cmake --build .
    $ cmake --build . --target test
    $ cmake --build . --target docs

The last two steps are optional.  They will run all tests and build the HTML
documentation respectively.  The latter will also show you undocumented
entities and you should fix those.

The compiler executable can be found in `${builddir}/src/minijava`.  Running it
with the `--help` option will give you some information how to use it.

If you choose a name for `${builddir}` that matches `stage`, `stage-*` or
`bld`, it will be `.gitignore`d.


### Building for Robots

    $ git clone https://git@slixe.de:7999/comprak/compiler.git

The top-level directory contains the shell scripts `build` and `run`.  If
executed from the top-level directory, `build` (with no arguments) configures
and builds the project and `run` invokes the compiler, forwarding it any and
all arguments that were passed to it.  (The script `exec()`s the compiler with
all standard file descriptors left at their defaults so there is no difference
compared to running the compiler directly.)

This will only work on POSIX systems.  Since the scripts don't check who
invokes them, they actually work for humans, too.


### Physical Project Layout

This section gives a quick overview how the files are organized in the source
tree and how the compiler is built from them.

Here is the directory hierarchy at a glance:

    /                     project root
    /src/                 C++ source code
    /src/core/            library providing compiler functionality
    /src/support/         library providing development support
    /src/minijava.cpp     contains 'main' for the compiler
    /tests/               tests
    /tests/unit/          unit tests
    /tests/fuzz/          fuzz tests
    /tests/comp/          compiler tests
    /benchmarks/          benchmarks
    /benchmarks/micro/    micro-benchmarks
    /benchmarks/macro/    macro-benchmarks
    extras/               auxiliary files for the build system

The compiler executable is built from the single source file
`/src/minijava.cpp` which is statically linked against a convenience library
`libcore` that provides the actual features.  The convenience library is made
up of *components*.  A component is a triple of a header file, source file and
unit test.  This terminology is borrowed from John Lakos.  (You can watch a
talk of his here: [1<sup>st</sup>&nbsp;part](https://youtu.be/QjFpKJ8Xx78),
[2<sup>nd</sup>&nbsp;part](https://youtu.be/fzFOLsFASjU) and
[3<sup>rd</sup>&nbsp;part](https://youtu.be/NrARQ7rHV-c).  The first part is
probably sufficient for this matter.)

Each component has a name.  Whenever you add a new component, say, `feature`,
exercise the following steps.

 1. Add `feature` to the definition of the variable `COMPONENTS` in
    `/CMakeLists.txt`.  Please put one name per line and keep the list sorted.

    We maintain this list manually instead of scanning the file-system because
    this allows CMake to determine correctly when the generated build-tools are
    outdated.  See the discussion about `GLOB`
    [here](https://cmake.org/cmake/help/v3.0/command/file.html) for some
    arguments.  In addition, it helps ensuring that each component has indeed
    the three required files.

    You may also place your feature into a sub-directory `${dir}/`; put
    `${dir}/` before the components name in the `COMPONENTS` variable in this
    case.

 2. Create the file `/src/core/${dir}/feature.hpp` with the interface of your
    component.  You don't have to put everything into a single header file as
    long as `/src/core/${dir}/feature.hpp` `#include`s all other files.

    If you are writing `template`-heavy code, it is recommended that you only
    put the public declarations into the `*.hpp` file and the implementation
    into `*.tpp` files.  This will help keeping the header clean so users can
    quickly understand it (and more easily resist the temptation to depend on
    implementation details) and also keeps the implementation stuff out of the
    reach of the Doxygen tool (which only processes `*.h` and `.hpp` files).

 3. Create the file `/src/core/${dir}/feature.cpp` with the implementation of
    your component.  It should `#include "feature.hpp"` as its first dependency
    to make sure it compiles stand-alone.  Do this even if the source file is
    otherwise empty.

 4. Create the file `/tests/unit/${dir}/feature.cpp` with the unit-tests for
    your component.  It will be linked against `libcore`.  In this file, you
    should also `#include "${dir}/feature.hpp"`.

A grab bag of utility features that were considered useful for writing unit
tests can be found in `/src/suport/testaux/`.  If you add new compiled source
files to it, don't forget to mention them in `/src/support/CMakeLists.txt`.


### Contributing

Please develop your feature on a separate branch named
`${yourname}/${yourfeature}`.  You can push to that branch whenever you like
and even apply `--force` if need be.  Please keep commits clean.  A single
commit should contain a single logical change.  Also be sure to use descriptive
commit messages with a short sentence that summarizes your change as the first
line followed by a blank line and possibly more text after that.

> *Moritz Klammler:* I prefer to write commit messages in `ChangeLog` format.

Once your feature is ready (which could be as soon as after the first commit),
create a pull request to merge your branch with `master`.  Normally, you can
merge if you got a single positive review but if your changes are likely to
break things for other people, better give it more time.  If your feature
branch is no longer needed, please delete it.

If your change is obvious and non-breaking (fixing typos, comments, formatting,
&hellip;) you can also push it directly to `master`.  If you do so, be careful
not to cause more trouble than necessary for those who have branched off
`master` and eventually want to merge back.

Make sure that `git status` shows no false positives.  If necessary, update
`.gitignore` accordingly.  Never check binary files, large PDFs, images or
other crap into the repository.


### Coding Standards

Use standards-compliant modern idiomatic C++14.  Never invoke undefined
behavior and don't rely on non-standard features unless we have agreed upon
that they can be used.  All code must compile on GCC without warnings at the
`-Wall`, `-Wextra` and `-pedantic` level.

Good resources for modern coding standards can be found here.

 - [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)
 - [&ldquo;C++ Coding Standards&rdquo; by Herb Sutter and Andrei Alexandrescu](http://www.gotw.ca/publications/c++cs.htm)
 - [Scott Meyers' &ldquo;Effective C++&rdquo; book series](http://www.aristeia.com/books.html)

Ignore the [Google C++ Style
Guide](https://google.github.io/styleguide/cppguide.html).  While some parts of
it are indisputable, it also contains a lot of poor advice that might work for
Google but is undesirable in general.  Other highly respected guidelines like
the [MISRA](http://www.programmingresearch.com/coding-standards/misra/) or
[JSF](http://www.stroustrup.com/JSF-AV-rules.pdf) standards provide many good
tips that anybody can profit from but are too specific for their very special
domains to be applicable in general.

We have agreed to use the following non-standard extensions to C++.

 - Put `#pragma once` as the first line of code in a header file instead of
   traditional `#include` guards.

The following third-party dependencies may be used.

 - Official [Boost](http://www.boost.org/doc/libs/) libraries.  (But if the
   same feature is also available from the C++14 standard library, use that
   instead.)

Do not depend on platform-specific features such as `<unistd.h>` or
`<windows.h>`.  Look if there is a cross-platform abstraction in Boost,
instead.


### Style Guide

We use the naming convention from the C++ standard library.  (All C++
identifiers are in lower-case and all macros are in upper-case.  Words are
separated by underscores.)

Except for `main`, all C++ symbols live in the `minijava` name-space and
pre-processor macros are prefixed with `MINIJAVA_`.  The unit-test support
library uses the `testaux` name-space for C++ and prefixes pre-processor macros
with `TESTAUX_`.

`private` members have a single &ldquo;`_`&rdquo; prefixed to their name.
(Never use two consecutive underscores or a leading underscore followed by an
upper-case character because these identifiers are reserved and using them
invokes undefined behavior.)

We use the same style as the
[Firm](http://pp.ipd.kit.edu/firm/Coding_Conventions) project as far as it is
applicable to C++ and unless a deviation is mentioned below.  In a nutshell,
this is &ldquo;Stroustrup&rdquo; style with tabs for indentation.

> *Moritz Klammler:* I hate their convention to put the `*` of a pointer
> declaration next to the identifier.  This is in disagreement to what
> Stroustrup recommends.  It also doesn't work if you have `const`-qualifiers
> on the pointer.  Personally, I prefer putting a space on either side of the
> `*` (which is also not what Stroustrup recommends).

Don't assume that a tab always has the same width it has in your editor.
Therefore, only use tabs for indentation and spaces for justification.  Try to
keep lines below 80 characters where each tab counts as four characters for
this purpose.  Meticulously trim trailing white-space and end each file with a
new-line character.  You can use the script `/extras/checks/whitespace.py` to
check and eventually fix some white-space issues before you commit.  Run it
with the `--help` option to see how it is used.

Document all public parts of components with Doxygen DocStrings that clearly
state the semantics and the contract of your functions and other stuff.  Only
`*.h` and `*.hpp` files are scanned by Doxygen so put the documentation there.

Use JavaDoc style with Markdown formatting that looks like this.

    /**
     * @brief
     *     Computes the longest-common sub-string of two character strings.
     *
     * If both arguments are different from zero and either of them is not a
     * NUL-terminated character string, the behavior is undefined.
     *
     * @param s1
     *     first string
     *
     * @param s2
     *     second string
     *
     * @returns
     *     longest common sub-string of `s1` and `s2`
     *
     * @throws std::invalid_argument
     *     if either `s1` or `s2` are `nullptr`s
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

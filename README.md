# Compiler-Engineering WS 2016/17 -- Team 2

## Build Instructions

You need a modern C++ compiler (supporting C++14) and have to install
the Boost C++ libraries and headers.  CMake is also needed.

    $ git clone https://git@slixe.de:7999/comprak/compiler.git
    $ cd compiler/
    $ mkdir build/
    $ cd build/
    $ cmake ..
    $ make

In order to run the unit tests, execute the following commends after
the above.

    $ ./bin/test-minijava

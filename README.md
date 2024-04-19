# Readme

A quick readme to get started with compiling and running the exam source.

#  Compiling

To compile the program run the `Makefile` using `make`.
This should generate three targets target called `index`, `test_index` and `bench_index`. The `index` executable needs a single argument: a directory containing text files to index.

The only library required for compilation is `ncurses`.

For Linux (and `wsl`) you may need to update the package index first. Curses can then be installed with:
```
sudo apt update
sudo apt install libncurses-dev
```

On MacOS it is available through homebrew:
```
brew install ncurses
```

Cygwin should come preinstalled with ncurses, if not, use the cygwin installer.

### Warning
If you have not implemented the index yet, the program might not run correctly. Don't panic, the UI should work once you've implemented the index.

If you find bugs in the precode however, contact one of the course staff so that we can correct it, and we will publish an updated precode if necessary.


## C Standard
The precode uses the `gnu17` C dialect.

# Running the program

The program can be run in two versions:

-  `./index data/` is without multi-word search.
- `./indexVersion2 data/` is with multi-word search.

The program can be run using `./index data/` or `./indexVersion2 data/` to use the text files contained in the data folder. Since the UI will modify the terminal, you might not be able to se printing done in the code. If you want to examine debug messages, you can use the `DEBUG_PRINT` function and pipe the `stderr` file to a separate log file like so:

```
./index data/ 2> log1.txt
```
```
./indexindexVersion2 data/ 2> log2.txt
```

This will put all your debug prints into `log.txt` or `log2.txt`. Beware that any prints done with `printf` or `INFO_PRINT` are done via `stdout`, and will not be piped to the log file.

# Generating the precode documentation

The documentation for the precode follows the doxygen JAVADOC format. Which means you can generate an `html` version of the documentation using `Doxygen`.

For Windows it is easiest to install `Doxywizard` and open the included `Doxyfile`. (https://www.doxygen.nl/download.html#srcbin)
On Mac it is available through homebrew as `doxygen`.
```shell
brew install doxygen
```

On Linux it is available through universe as `doxygen`
```
sudo apt install doxygen
```

Using the command
```
doxygen
```
In the same directory as the `Doxyfile` will generate a new directory called `doc/`. This directory contains an `html/` and a `latex/`. The `html/` folder contains the file `index.html` which will open an interactive documentation in a browser.

# Running unit tests and benchmarks

There are 3 make targets that help running unit tests and benchmarks for your code:

- `make test` builds the unit test program `test_index` and `test_index2`
- `make bench` builds the benchmark program `bench_index` and `bench_index2`
- `make run` builds and runs the unit tests and benchmark programs in to version (outputting benchmarks of the version 1 is without Hash Map to `benchVersion1.txt`, and version 2 with the Hash Map to `benchVersion2.txt`)

Both programs can be built and run separately. The benchmark program accepts two command line arguments:

```
./bench_index <n words> <n runs>
```
```
./bench_index2 <n words> <n runs>
```

Which will tell the program how many words to insert into the index, and how many times the benchmark should run (doubling `nwords` for each run).

It is recommeded to redirect `stdout` to a file, to get the benchmark data:


```
./bench_index 100000 10 1> benchmarkV1.txt
```
```
./bench_index2 100000 10 1> benchmarkV2.txt
```

The "unit" test program will try to call the `trie` and `index` functions and test rudimentry functionality. It will report any SEGFAULTS that it encounters as well. Beware, continuing to run a program after a segfault may result in undefined behaviour.

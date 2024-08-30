This folder contains lint scripts intended for use with the cppcheck CLI program.

Lint script naming convention should be in the form of `lint-<cppcheckrule>.sh`

lint-noexplicitconstructor.sh
======

Checks for 1-argument (non-converting) class/struct constructors that are not
marked as `explicit`. Allowing implicit conversion can lead to difficult to
track down bugs and unintended behavior.

run-cppcheck.sh
======

This script is responsible for building/rebuilding the cppcheck cache that
all other lint scripts use. cppcheck is run in "project" mode, which creates
a build cache to speed up subsequent runs. The result is output to a named
`cppcheck.txt` file, used by other lint scripts.

lint-all.sh
======

This is the base script used to initialize or run any lint script contained
in this directory. It can take an optional argument of `--setup` to
build/rebuild a cppcheck cache then exit, or be run with no arguments
(provided a cppcache exists) to then run any lint script in this directory.

Standard usage is to first run `./test/lint/cppcheck/lint-all.sh --setup` to
create or rebuild the cppcheck cache, then run `./test/llint/cppcheck/lint-all.sh`
with no arguments to perform any actual lint tests.
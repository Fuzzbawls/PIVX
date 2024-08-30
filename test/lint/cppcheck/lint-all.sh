#!/usr/bin/env bash
#
# Copyright (c) 2019 The Bitcoin Core developers
# Copyright (c) 2024 The PIVX Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#
# This script runs all test/lint/extended-lint-*.sh files, and fails if
# any exit with a non-zero status code.

# This script is intentionally locale dependent by not setting "export LC_ALL=C"
# in order to allow for the executed lint scripts to opt in or opt out of locale
# dependence themselves.

set -u

print_usage() {
    echo "Usage: $0 --setup to create/refresh cppcheck's cache and exit. No argument to run lint tests."
}

SCRIPTDIR=$(dirname "${BASH_SOURCE[0]}")
LINTALL=$(basename "${BASH_SOURCE[0]}")

if [[ $# != 0 ]]; then
    if [[ $1 == "--help" ]]; then
        print_usage
        exit 0
    fi
    if [[ $1 == "--setup" ]]; then
        if ! "${SCRIPTDIR}"/run-cppcheck.sh; then
          echo "cppcheck cache creation failed"
          exit 1
        fi
        exit 0
    fi
fi

for f in "${SCRIPTDIR}"/lint-*.sh; do
  if [ "$(basename "$f")" != "$LINTALL" ]; then
    if ! "$f"; then
      echo "^---- failure generated from $f"
      exit 1
    fi
  fi
done

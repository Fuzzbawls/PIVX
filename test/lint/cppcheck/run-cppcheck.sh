#!/usr/bin/env bash
#
# Copyright (c) 2024 The PIVX Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#

export LC_ALL=C

CPPCHECK_BUILD_DIR=".ci-cppcheck"

if ! command -v cppcheck > /dev/null; then
    echo "Skipping cppcheck linting since cppcheck is not installed. Install by running \"apt install cppcheck\""
    exit 1
fi

echo "Building cppcheck cache..."
if [ ! -d "${CPPCHECK_BUILD_DIR}" ]; then
  mkdir -p "${CPPCHECK_BUILD_DIR}"
fi

cppcheck --cppcheck-build-dir="${CPPCHECK_BUILD_DIR}" --enable=all -j "$(getconf _NPROCESSORS_ONLN)" \
--language=c++ --std=c++14 --template=gcc --check-level=exhaustive --file-filter=*.cpp --file-filter=*.h -isrc/chiabls \
-isrc/crc32c -isrc/immer -isrc/leveldb -isrc/secp256k1 -isrc/univalue -D__cplusplus -DCLIENT_VERSION_BUILD \
-DCLIENT_VERSION_IS_RELEASE -DCLIENT_VERSION_MAJOR -DCLIENT_VERSION_MINOR -DCLIENT_VERSION_REVISION -DCOPYRIGHT_YEAR \
-DDEBUG -DWITH_LOCK -DPACKAGE_NAME -DENABLE_MINING_RPC --library=boost --library=qt -I src/ -I src/qt/ src/ 2> >(sort -u > cppcheck.txt)

echo "cppcheck cache built."

exit 0

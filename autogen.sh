#!/bin/sh
# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

export LC_ALL=C
set -e

echo -e "/* src/src/chiabls/contrib/relic/include/relic_conf.h.in  Generated from configure.ac by autoheader.  */\n$(cat src/chiabls/contrib/relic/include/relic_conf.h.in)" > src/chiabls/contrib/relic/include/relic_conf.h.in

srcdir="$(dirname $0)"
cd "$srcdir"
if [ -z ${LIBTOOLIZE} ] && GLIBTOOLIZE="$(command -v glibtoolize)"; then
  LIBTOOLIZE="${GLIBTOOLIZE}"
  export LIBTOOLIZE
fi
command -v autoreconf >/dev/null || \
  (echo "configuration failed, please install autoconf first" && exit 1)
autoreconf --verbose --install --force --warnings=all

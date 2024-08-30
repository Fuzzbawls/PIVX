#!/usr/bin/env bash
#
# Copyright (c) 2019 The Bitcoin Core developers
# Copyright (c) 2024 The PIVX Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
#

export LC_ALL=C

ENABLED_CHECKS=(
     "\[noExplicitConstructor\]"
)

IGNORED_WARNINGS=(
    "src/immer/.*"
    "src/chiabls/.*"
    "src/arith_uint256.h:.* Class 'arith_uint160' has a constructor with 1 argument that is not explicit."
    "src/arith_uint256.h:.* Class 'arith_uint256' has a constructor with 1 argument that is not explicit."
    "src/arith_uint256.h:.* Class 'arith_uint512' has a constructor with 1 argument that is not explicit."
    "src/arith_uint256.h:.* Class 'base_uint < 160 >' has a constructor with 1 argument that is not explicit."
    "src/arith_uint256.h:.* Class 'base_uint < 256 >' has a constructor with 1 argument that is not explicit."
    "src/arith_uint256.h:.* Class 'base_uint < 512 >' has a constructor with 1 argument that is not explicit."
    "src/arith_uint256.h:.* Class 'base_uint' has a constructor with 1 argument that is not explicit."
    "src/coins.h:.* Class 'CCoinsViewBacked' has a constructor with 1 argument that is not explicit."
    "src/coins.h:.* Class 'CCoinsViewCache' has a constructor with 1 argument that is not explicit."
    "src/bls/bls_wrapper.h:.* Struct 'CBLSIdImplicit' has a constructor with 1 argument that is not explicit."
    "src/bls/bls_wrapper.h:.* Class 'CBLSId' has a constructor with 1 argument that is not explicit."
    "src/bls/bls_wrapper.h:.* Class 'CBLSWrapper < CBLSIdImplicit , 32 , CBLSId >' has a constructor with 1 argument that is not explicit."
    "src/bls/bls_wrapper.h:.* Class 'CBLSWrapper < bls :: G1Element , 48 , CBLSPublicKey >' has a constructor with 1 argument that is not explicit."
    "src/bls/bls_wrapper.h:.* Class 'CBLSWrapper < bls :: G2Element , 96 , CBLSSignature >' has a constructor with 1 argument that is not explicit."
    "src/bls/bls_wrapper.h:.* Class 'CBLSWrapper < bls :: PrivateKey , 32 , CBLSSecretKey >' has a constructor with 1 argument that is not explicit."
    "src/operationresult.h:.* Class 'OperationResult' has a constructor with 1 argument that is not explicit."
    "src/prevector.h:.* Class 'const_iterator' has a constructor with 1 argument that is not explicit."
    "src/prevector.h:.* Class 'const_reverse_iterator' has a constructor with 1 argument that is not explicit."
    "src/prevector.h:.* Class 'iterator' has a constructor with 1 argument that is not explicit."
    "src/prevector.h:.* Class 'reverse_iterator' has a constructor with 1 argument that is not explicit."
    "src/primitives/block.h:.* Class 'CBlock' has a constructor with 1 argument that is not explicit."
    "src/primitives/transaction.h:.* Class 'CTransaction' has a constructor with 1 argument that is not explicit."
    "src/primitives/transaction.h:.* Struct 'CMutableTransaction' has a constructor with 1 argument that is not explicit."
    "src/libzerocoin/bignum.h:.* Class 'CBigNum' has a constructor with 1 argument that is not explicit."
    "src/qt/walletmodel.h:.* Struct 'SendCoinsReturn' has a constructor with 1 argument that is not explicit."
    "src/rpc/server.h:.* Struct 'UniValueType' has a constructor with 1 argument that is not explicit."
    "src/sapling/incrementalmerkletree.h:.* Class 'PedersenHash' has a constructor with 1 argument that is not explicit."
    "src/sapling/incrementalmerkletree.h:.* Class 'SHA256Compress' has a constructor with 1 argument that is not explicit."
    "src/script/standard.h:.* Class 'CScriptID' has a constructor with 1 argument that is not explicit."
    "src/span.h:.* Class 'Span' has a constructor with 1 argument that is not explicit."
    "src/span.h:.* Class 'Span < const char >' has a constructor with 1 argument that is not explicit."
    "src/span.h:.* Class 'Span < const uint8_t >' has a constructor with 1 argument that is not explicit."
    "src/span.h:.* Class 'Span < const unsigned char >' has a constructor with 1 argument that is not explicit."
    "src/span.h:.* Class 'Span < uint8_t >' has a constructor with 1 argument that is not explicit."
    "src/span.h:.* Class 'Span < unsigned char >' has a constructor with 1 argument that is not explicit."
    "src/support/allocators/secure.h:.* Struct 'secure_allocator < char >' has a constructor with 1 argument that is not explicit."
    "src/support/allocators/secure.h:.* Struct 'secure_allocator < RNGState >' has a constructor with 1 argument that is not explicit."
    "src/support/allocators/secure.h:.* Struct 'secure_allocator < unsigned char >' has a constructor with 1 argument that is not explicit."
    "src/support/allocators/zeroafterfree.h:.* Struct 'zero_after_free_allocator < char >' has a constructor with 1 argument that is not explicit."
    "src/wallet/wallet.h:.* Struct 'Confirmation' has a constructor with 1 argument that is not explicit."
)

if [ ! -f cppcheck.txt ]; then
    echo "cppcheck.txt cache not found, skipping linting."
    exit 1
fi

function join_array {
    local IFS="$1"
    shift
    echo "$*"
}

ENABLED_CHECKS_REGEXP=$(join_array "|" "${ENABLED_CHECKS[@]}")
IGNORED_WARNINGS_REGEXP=$(join_array "|" "${IGNORED_WARNINGS[@]}")

WARNINGS=$(< cppcheck.txt grep -E "${ENABLED_CHECKS_REGEXP}" | grep -vE "${IGNORED_WARNINGS_REGEXP}")

if [[ ${WARNINGS} != "" ]]; then
    echo "${WARNINGS}"
    echo
    echo "Advice not applicable in this specific case? Add an exception by updating"
    echo "IGNORED_WARNINGS in $0"
    # Uncomment to enforce the developer note policy "By default, declare single-argument constructors `explicit`"
    # exit 1
fi
exit 0
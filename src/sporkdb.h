// Copyright (c) 2017-2020 The PIVX Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PIVX_SPORKDB_H
#define PIVX_SPORKDB_H

#include "fs.h"
#include "dbwrapper.h"
#include "spork.h"

class CSporkDB : public CDBWrapper
{
public:
    explicit CSporkDB(size_t nCacheSize, bool fMemory = false, bool fWipe = false);

private:
    CSporkDB(const CSporkDB&);
    void operator=(const CSporkDB&);

public:
    bool WriteSpork(const SporkId nSporkId, const CSporkMessage& spork);
    bool ReadSpork(const SporkId nSporkId, CSporkMessage& spork);
    bool SporkExists(const SporkId nSporkId);
};


#endif // PIVX_SPORKDB_H

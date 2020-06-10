// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2017-2019 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "keystore.h"

#include "crypter.h"
#include "key.h"
#include "script/script.h"
#include "script/standard.h"
#include "util.h"


bool CKeyStore::GetPubKey(const CKeyID& address, CPubKey& vchPubKeyOut) const
{
    CKey key;
    if (!GetKey(address, key))
        return false;
    vchPubKeyOut = key.GetPubKey();
    return true;
}

bool CKeyStore::AddKey(const CKey& key)
{
    return AddKeyPubKey(key, key.GetPubKey());
}

bool CBasicKeyStore::AddKeyPubKey(const CKey& key, const CPubKey& pubkey)
{
    LOCK(cs_KeyStore);
    mapKeys[pubkey.GetID()] = key;
    return true;
}

bool CBasicKeyStore::AddCScript(const CScript& redeemScript)
{
    if (redeemScript.size() > MAX_SCRIPT_ELEMENT_SIZE)
        return error("CBasicKeyStore::AddCScript() : redeemScripts > %i bytes are invalid", MAX_SCRIPT_ELEMENT_SIZE);

    LOCK(cs_KeyStore);
    mapScripts[CScriptID(redeemScript)] = redeemScript;
    return true;
}

bool CBasicKeyStore::HaveCScript(const CScriptID& hash) const
{
    LOCK(cs_KeyStore);
    return mapScripts.count(hash) > 0;
}

bool CBasicKeyStore::GetCScript(const CScriptID& hash, CScript& redeemScriptOut) const
{
    LOCK(cs_KeyStore);
    ScriptMap::const_iterator mi = mapScripts.find(hash);
    if (mi != mapScripts.end()) {
        redeemScriptOut = (*mi).second;
        return true;
    }
    return false;
}

bool CBasicKeyStore::AddWatchOnly(const CScript& dest)
{
    LOCK(cs_KeyStore);
    setWatchOnly.insert(dest);
    return true;
}

bool CBasicKeyStore::RemoveWatchOnly(const CScript& dest)
{
    LOCK(cs_KeyStore);
    setWatchOnly.erase(dest);
    return true;
}

bool CBasicKeyStore::HaveWatchOnly(const CScript& dest) const
{
    LOCK(cs_KeyStore);
    return setWatchOnly.count(dest) > 0;
}

bool CBasicKeyStore::HaveWatchOnly() const
{
    LOCK(cs_KeyStore);
    return (!setWatchOnly.empty());
}

bool CBasicKeyStore::HaveKey(const CKeyID& address) const
{
    bool result;
    {
        LOCK(cs_KeyStore);
        result = (mapKeys.count(address) > 0);
    }
    return result;
}

void CBasicKeyStore::GetKeys(std::set<CKeyID>& setAddress) const
{
    setAddress.clear();
    {
        LOCK(cs_KeyStore);
        KeyMap::const_iterator mi = mapKeys.begin();
        while (mi != mapKeys.end()) {
            setAddress.insert((*mi).first);
            mi++;
        }
    }
}

bool CBasicKeyStore::GetKey(const CKeyID& address, CKey& keyOut) const
{
    {
        LOCK(cs_KeyStore);
        KeyMap::const_iterator mi = mapKeys.find(address);
        if (mi != mapKeys.end()) {
            keyOut = mi->second;
            return true;
        }
    }
    return false;
}

//! Sapling
bool CBasicKeyStore::AddSaplingSpendingKey(
        const libzcash::SaplingExtendedSpendingKey &sk,
        const libzcash::SaplingPaymentAddress &defaultAddr)
{
    LOCK(cs_SpendingKeyStore);
    auto fvk = sk.expsk.full_viewing_key();

    // if SaplingFullViewingKey is not in SaplingFullViewingKeyMap, add it
    if (!AddSaplingFullViewingKey(fvk, defaultAddr)){
        return error("%s: adding new sapling fvk", __func__);
    }

    mapSaplingSpendingKeys[fvk] = sk;
    return true;
}

bool CBasicKeyStore::AddSaplingFullViewingKey(
        const libzcash::SaplingFullViewingKey &fvk,
        const libzcash::SaplingPaymentAddress &defaultAddr)
{
    LOCK(cs_SpendingKeyStore);
    auto ivk = fvk.in_viewing_key();
    mapSaplingFullViewingKeys[ivk] = fvk;

    return AddSaplingIncomingViewingKey(ivk, defaultAddr);
}

// This function updates the wallet's internal address->ivk map.
// If we add an address that is already in the map, the map will
// remain unchanged as each address only has one ivk.
bool CBasicKeyStore::AddSaplingIncomingViewingKey(
        const libzcash::SaplingIncomingViewingKey &ivk,
        const libzcash::SaplingPaymentAddress &addr)
{
    LOCK(cs_SpendingKeyStore);

    // Add addr -> SaplingIncomingViewing to SaplingIncomingViewingKeyMap
    mapSaplingIncomingViewingKeys[addr] = ivk;

    return true;
}

bool CBasicKeyStore::HaveSaplingSpendingKey(const libzcash::SaplingFullViewingKey &fvk) const
{
    return WITH_LOCK(cs_SpendingKeyStore, return mapSaplingSpendingKeys.count(fvk) > 0);
}

bool CBasicKeyStore::HaveSaplingFullViewingKey(const libzcash::SaplingIncomingViewingKey &ivk) const
{
    return WITH_LOCK(cs_SpendingKeyStore, return mapSaplingFullViewingKeys.count(ivk) > 0);
}

bool CBasicKeyStore::HaveSaplingIncomingViewingKey(const libzcash::SaplingPaymentAddress &addr) const
{
    return WITH_LOCK(cs_SpendingKeyStore, return mapSaplingIncomingViewingKeys.count(addr) > 0);
}

bool CBasicKeyStore::GetSaplingSpendingKey(const libzcash::SaplingFullViewingKey &fvk, libzcash::SaplingExtendedSpendingKey &skOut) const
{
    LOCK(cs_SpendingKeyStore);
    SaplingSpendingKeyMap::const_iterator mi = mapSaplingSpendingKeys.find(fvk);
    if (mi != mapSaplingSpendingKeys.end()) {
        skOut = mi->second;
        return true;
    }
    return false;
}

bool CBasicKeyStore::GetSaplingFullViewingKey(const libzcash::SaplingIncomingViewingKey &ivk,
                                              libzcash::SaplingFullViewingKey &fvkOut) const
{
    LOCK(cs_SpendingKeyStore);
    SaplingFullViewingKeyMap::const_iterator mi = mapSaplingFullViewingKeys.find(ivk);
    if (mi != mapSaplingFullViewingKeys.end()) {
        fvkOut = mi->second;
        return true;
    }
    return false;
}

bool CBasicKeyStore::GetSaplingIncomingViewingKey(const libzcash::SaplingPaymentAddress &addr,
                                                  libzcash::SaplingIncomingViewingKey &ivkOut) const
{
    LOCK(cs_SpendingKeyStore);
    SaplingIncomingViewingKeyMap::const_iterator mi = mapSaplingIncomingViewingKeys.find(addr);
    if (mi != mapSaplingIncomingViewingKeys.end()) {
        ivkOut = mi->second;
        return true;
    }
    return false;
}

bool CBasicKeyStore::GetSaplingExtendedSpendingKey(const libzcash::SaplingPaymentAddress &addr,
                                                   libzcash::SaplingExtendedSpendingKey &extskOut) const {
    libzcash::SaplingIncomingViewingKey ivk;
    libzcash::SaplingFullViewingKey fvk;

    return GetSaplingIncomingViewingKey(addr, ivk) &&
           GetSaplingFullViewingKey(ivk, fvk) &&
           GetSaplingSpendingKey(fvk, extskOut);
}

void CBasicKeyStore::GetSaplingPaymentAddresses(std::set<libzcash::SaplingPaymentAddress> &setAddress) const
{
    setAddress.clear();
    {
        LOCK(cs_SpendingKeyStore);
        auto mi = mapSaplingIncomingViewingKeys.begin();
        while (mi != mapSaplingIncomingViewingKeys.end())
        {
            setAddress.insert((*mi).first);
            mi++;
        }
    }
}
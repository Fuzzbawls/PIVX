// Copyright (c) 2018-2022 The Dash Core developers
// Copyright (c) 2023 The PIVX Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PIVX_LLMQ_QUORUMS_SIGNING_SHARES_H
#define PIVX_LLMQ_QUORUMS_SIGNING_SHARES_H

#include "chainparams.h"
#include "consensus/params.h"
#include "net.h"
#include "random.h"
#include "saltedhasher.h"
#include "serialize.h"
#include "sync.h"
#include "tinyformat.h"
#include "uint256.h"

#include "llmq/quorums.h"

#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

class CEvoDB;
class CScheduler;

namespace llmq
{
// <signHash, quorumMember>
typedef std::pair<uint256, uint16_t> SigShareKey;

class CSigShare
{
public:
    uint8_t llmqType;
    uint256 quorumHash;
    uint16_t quorumMember;
    uint256 id;
    uint256 msgHash;
    CBLSLazySignature sigShare;

    SigShareKey key;

public:
    void UpdateKey();
    const SigShareKey& GetKey() const
    {
        return key;
    }
    const uint256& GetSignHash() const
    {
        assert(!key.first.IsNull());
        return key.first;
    }

    SERIALIZE_METHODS(CSigShare, obj)
    {
        READWRITE(obj.llmqType);
        READWRITE(obj.quorumHash);
        READWRITE(obj.quorumMember);
        READWRITE(obj.id);
        READWRITE(obj.msgHash);
        READWRITE(obj.sigShare);
        SER_READ(obj, obj.UpdateKey());
    }
};

// Nodes will first announce a signing session with a sessionId to be used in all future P2P messages related to that
// session. We locally keep track of the mapping for each node. We also assign new sessionIds for outgoing sessions
// and send QSIGSESANN messages appropriately. All values except the max value for uint32_t are valid as sessionId
class CSigSesAnn
{
public:
    uint32_t sessionId{(uint32_t)-1};
    uint8_t llmqType;
    uint256 quorumHash;
    uint256 id;
    uint256 msgHash;

    SERIALIZE_METHODS(CSigSesAnn, obj)
    {
        READWRITE(VARINT(obj.sessionId));
        READWRITE(obj.llmqType);
        READWRITE(obj.quorumHash);
        READWRITE(obj.id);
        READWRITE(obj.msgHash);
    }

    std::string ToString() const;
};

class CSigSharesInv
{
public:
    uint32_t sessionId{(uint32_t)-1};
    std::vector<bool> inv;

public:
    SERIALIZE_METHODS(CSigSharesInv, obj)
    {
        uint64_t invSize = obj.inv.size();

        READWRITE(VARINT(obj.sessionId));
        READWRITE(COMPACTSIZE(invSize));
        READWRITE(AUTOBITSET(obj.inv, (size_t)invSize));
    }

    void Init(size_t size);
    bool IsSet(uint16_t quorumMember) const;
    void Set(uint16_t quorumMember, bool v);
    void SetAll(bool v);
    void Merge(const CSigSharesInv& inv2);

    size_t CountSet() const;
    std::string ToString() const;
};

// sent through the message QBSIGSHARES as a vector of multiple batches
class CBatchedSigShares
{
public:
    uint32_t sessionId{(uint32_t)-1};
    std::vector<std::pair<uint16_t, CBLSLazySignature>> sigShares;

public:
    SERIALIZE_METHODS(CBatchedSigShares, obj)
    {
        READWRITE(VARINT(obj.sessionId));
        READWRITE(obj.sigShares);
    }

    std::string ToInvString() const;
};

template <typename T>
class SigShareMap
{
private:
    std::unordered_map<uint256, std::unordered_map<uint16_t, T>, StaticSaltedHasher> internalMap;

public:
    bool Add(const SigShareKey& k, const T& v)
    {
        auto& m = internalMap[k.first];
        return m.emplace(k.second, v).second;
    }

    void Erase(const SigShareKey& k)
    {
        auto it = internalMap.find(k.first);
        if (it == internalMap.end()) {
            return;
        }
        it->second.erase(k.second);
        if (it->second.empty()) {
            internalMap.erase(it);
        }
    }

    void Clear()
    {
        internalMap.clear();
    }

    bool Has(const SigShareKey& k) const
    {
        auto it = internalMap.find(k.first);
        if (it == internalMap.end()) {
            return false;
        }
        return it->second.count(k.second) != 0;
    }

    T* Get(const SigShareKey& k)
    {
        auto it = internalMap.find(k.first);
        if (it == internalMap.end()) {
            return nullptr;
        }

        auto jt = it->second.find(k.second);
        if (jt == it->second.end()) {
            return nullptr;
        }

        return &jt->second;
    }

    T& GetOrAdd(const SigShareKey& k)
    {
        T* v = Get(k);
        if (!v) {
            Add(k, T());
            v = Get(k);
        }
        return *v;
    }

    const T* GetFirst() const
    {
        if (internalMap.empty()) {
            return nullptr;
        }
        return &internalMap.begin()->second.begin()->second;
    }

    size_t Size() const
    {
        size_t s = 0;
        for (auto& p : internalMap) {
            s += p.second.size();
        }
        return s;
    }

    size_t CountForSignHash(const uint256& signHash) const
    {
        auto it = internalMap.find(signHash);
        if (it == internalMap.end()) {
            return 0;
        }
        return it->second.size();
    }

    bool Empty() const
    {
        return internalMap.empty();
    }

    const std::unordered_map<uint16_t, T>* GetAllForSignHash(const uint256& signHash)
    {
        auto it = internalMap.find(signHash);
        if (it == internalMap.end()) {
            return nullptr;
        }
        return &it->second;
    }

    void EraseAllForSignHash(const uint256& signHash)
    {
        internalMap.erase(signHash);
    }

    template <typename F>
    void EraseIf(F&& f)
    {
        for (auto it = internalMap.begin(); it != internalMap.end();) {
            SigShareKey k;
            k.first = it->first;
            for (auto jt = it->second.begin(); jt != it->second.end();) {
                k.second = jt->first;
                if (f(k, jt->second)) {
                    jt = it->second.erase(jt);
                } else {
                    ++jt;
                }
            }
            if (it->second.empty()) {
                it = internalMap.erase(it);
            } else {
                ++it;
            }
        }
    }

    template <typename F>
    void ForEach(F&& f)
    {
        for (auto& p : internalMap) {
            SigShareKey k;
            k.first = p.first;
            for (auto& p2 : p.second) {
                k.second = p2.first;
                f(k, p2.second);
            }
        }
    }
};

class CSigSharesNodeState
{
public:
    // Used to avoid holding locks too long
    struct SessionInfo {
        Consensus::LLMQType llmqType;
        uint256 quorumHash;
        uint256 id;
        uint256 msgHash;
        uint256 signHash;

        CQuorumCPtr quorum;
    };

    struct Session {
        uint32_t recvSessionId{(uint32_t)-1};
        uint32_t sendSessionId{(uint32_t)-1};

        Consensus::LLMQType llmqType;
        uint256 quorumHash;
        uint256 id;
        uint256 msgHash;
        uint256 signHash;

        CQuorumCPtr quorum;

        CSigSharesInv announced;
        CSigSharesInv requested;
        CSigSharesInv knows;
    };
    // TODO limit number of sessions per node
    std::unordered_map<uint256, Session, StaticSaltedHasher> sessions;

    std::unordered_map<uint32_t, Session*> sessionByRecvId;
    uint32_t nextSendSessionId{1};

    SigShareMap<CSigShare> pendingIncomingSigShares;
    SigShareMap<int64_t> requestedSigShares;

    bool banned{false};

    Session& GetOrCreateSessionFromShare(const CSigShare& sigShare);
    Session& GetOrCreateSessionFromAnn(const CSigSesAnn& ann);
    Session* GetSessionBySignHash(const uint256& signHash);
    Session* GetSessionByRecvId(uint32_t sessionId);
    bool GetSessionInfoByRecvId(uint32_t sessionId, SessionInfo& retInfo);

    void RemoveSession(const uint256& signHash);
};

class CSignedSession
{
public:
    CSigShare sigShare;
    CQuorumCPtr quorum;

    int64_t nextAttemptTime{0};
    int attempt{0};
};

class CSigSharesManager : public CRecoveredSigsListener
{
    static const int64_t SESSION_NEW_SHARES_TIMEOUT = 60;
    static const int64_t SIG_SHARE_REQUEST_TIMEOUT = 5;

    // we try to keep total message size below 10k
    const size_t MAX_MSGS_CNT_QSIGSESANN = 100;
    const size_t MAX_MSGS_CNT_QGETSIGSHARES = 200;
    const size_t MAX_MSGS_CNT_QSIGSHARESINV = 200;
    // 400 is the maximum quorum size, so this is also the maximum number of sigs we need to support
    const size_t MAX_MSGS_TOTAL_BATCHED_SIGS = 400;

    const int64_t EXP_SEND_FOR_RECOVERY_TIMEOUT = 2000;
    const int64_t MAX_SEND_FOR_RECOVERY_TIMEOUT = 10000;
    const size_t MAX_MSGS_SIG_SHARES = 32;

private:
    RecursiveMutex cs;

    std::thread workThread;
    CThreadInterrupt interruptSigningShare;

    SigShareMap<CSigShare> sigShares;
    std::unordered_map<uint256, CSignedSession, StaticSaltedHasher> signedSessions;

    // stores time of last receivedSigShare. Used to detect timeouts
    std::unordered_map<uint256, int64_t, StaticSaltedHasher> timeSeenForSessions;

    std::unordered_map<NodeId, CSigSharesNodeState> nodeStates;
    SigShareMap<std::pair<NodeId, int64_t>> sigSharesRequested;
    SigShareMap<bool> sigSharesToAnnounce;

    std::vector<std::tuple<const CQuorumCPtr, uint256, uint256>> pendingSigns;

    // must be protected by cs
    FastRandomContext rnd;

    int64_t lastCleanupTime{0};
    std::atomic<uint32_t> recoveredSigsCounter{0};

public:
    CSigSharesManager();
    ~CSigSharesManager();

    void StartWorkerThread();
    void StopWorkerThread();
    void Interrupt();
    void RegisterAsRecoveredSigsListener();
    void UnregisterAsRecoveredSigsListener();

public:
    void ProcessMessage(CNode* pnode, const std::string& strCommand, CDataStream& vRecv, CConnman& connman);

    void AsyncSign(const CQuorumCPtr& quorum, const uint256& id, const uint256& msgHash);
    void Sign(const CQuorumCPtr& quorum, const uint256& id, const uint256& msgHash);
    void ForceReAnnouncement(const CQuorumCPtr& quorum, Consensus::LLMQType llmqType, const uint256& id, const uint256& msgHash);

    void HandleNewRecoveredSig(const CRecoveredSig& recoveredSig);

    static CDeterministicMNCPtr SelectMemberForRecovery(const CQuorumCPtr& quorum, const uint256& id, int attempt);

private:
    // all of these return false when the currently processed message should be aborted (as each message actually contains multiple messages)
    bool ProcessMessageSigSesAnn(CNode* pfrom, const CSigSesAnn& ann, CConnman& connman);
    bool ProcessMessageSigSharesInv(CNode* pfrom, const CSigSharesInv& inv, CConnman& connman);
    bool ProcessMessageGetSigShares(CNode* pfrom, const CSigSharesInv& inv, CConnman& connman);
    bool ProcessMessageBatchedSigShares(CNode* pfrom, const CBatchedSigShares& batchedSigShares, CConnman& connman);
    void ProcessMessageSigShare(NodeId fromId, const CSigShare& sigShare, CConnman& connman);

    bool VerifySigSharesInv(NodeId from, Consensus::LLMQType llmqType, const CSigSharesInv& inv);
    bool PreVerifyBatchedSigShares(NodeId nodeId, const CSigSharesNodeState::SessionInfo& session, const CBatchedSigShares& batchedSigShares, bool& retBan);

    void CollectPendingSigSharesToVerify(size_t maxUniqueSessions,
        std::unordered_map<NodeId, std::vector<CSigShare>>& retSigShares,
        std::unordered_map<std::pair<Consensus::LLMQType, uint256>, CQuorumCPtr, StaticSaltedHasher>& retQuorums);
    bool ProcessPendingSigShares(CConnman& connman);

    void ProcessPendingSigSharesFromNode(NodeId nodeId,
        const std::vector<CSigShare>& sigShares,
        const std::unordered_map<std::pair<Consensus::LLMQType, uint256>, CQuorumCPtr, StaticSaltedHasher>& quorums,
        CConnman& connman);

    void ProcessSigShare(NodeId nodeId, const CSigShare& sigShare, CConnman& connman, const CQuorumCPtr& quorum);
    void TryRecoverSig(const CQuorumCPtr& quorum, const uint256& id, const uint256& msgHash, CConnman& connman);

private:
    bool GetSessionInfoByRecvId(NodeId nodeId, uint32_t sessionId, CSigSharesNodeState::SessionInfo& retInfo);
    CSigShare RebuildSigShare(const CSigSharesNodeState::SessionInfo& session, const CBatchedSigShares& batchedSigShares, size_t idx);

    void Cleanup();
    void RemoveSigSharesForSession(const uint256& signHash);
    void RemoveBannedNodeStates();

    void BanNode(NodeId nodeId);

    bool SendMessages();
    void CollectSigSharesToRequest(std::unordered_map<NodeId, std::unordered_map<uint256, CSigSharesInv, StaticSaltedHasher>>& sigSharesToRequest);
    void CollectSigSharesToSend(std::unordered_map<NodeId, std::unordered_map<uint256, CBatchedSigShares, StaticSaltedHasher>>& sigSharesToSend);
    void CollectSigSharesToSend(std::unordered_map<NodeId, std::vector<CSigShare>>& sigSharesToSend, const std::vector<CNode*>& vNodes);
    void CollectSigSharesToAnnounce(std::unordered_map<NodeId, std::unordered_map<uint256, CSigSharesInv, StaticSaltedHasher>>& sigSharesToAnnounce);
    bool SignPendingSigShares();
    void WorkThreadMain();
};

extern std::unique_ptr<CSigSharesManager> quorumSigSharesManager;

} // namespace llmq

#endif // PIVX_LLMQ_QUORUMS_SIGNING_SHARES_H

// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2017-2021 The PIVX Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PIVX_VALIDATIONINTERFACE_H
#define PIVX_VALIDATIONINTERFACE_H

#include "optional.h"
#include "sapling/incrementalmerkletree.h"
#include "primitives/transaction.h"

#include <functional>
#include <memory>

class CBlock;
struct CBlockLocator;
class CBlockIndex;
class CConnman;
class CDeterministicMNList;
class CDeterministicMNListDiff;
class CValidationInterface;
class CValidationState;
class uint256;
class CScheduler;
enum class MemPoolRemovalReason;

// These functions dispatch to one or all registered wallets

/** Register a wallet to receive updates from core */
void RegisterValidationInterface(CValidationInterface* pwalletIn);
/** Unregister a wallet from core */
void UnregisterValidationInterface(CValidationInterface* pwalletIn);
/** Unregister all wallets from core */
void UnregisterAllValidationInterfaces();

// Alternate registration functions that release a shared_ptr after the last
// notification is sent. These are useful for race-free cleanup, since
// unregistration is nonblocking and can return before the last notification is
// processed.
void RegisterSharedValidationInterface(std::shared_ptr<CValidationInterface> pwalletIn);
void UnregisterSharedValidationInterface(std::shared_ptr<CValidationInterface> pwalletIn);

/**
 * Pushes a function to callback onto the notification queue, guaranteeing any
 * callbacks generated prior to now are finished when the function is called.
 *
 * Be very careful blocking on func to be called if any locks are held -
 * validation interface clients may not be able to make progress as they often
 * wait for things like cs_main, so blocking until func is called with cs_main
 * will result in a deadlock (that DEBUG_LOCKORDER will miss).
 */
void CallFunctionInValidationInterfaceQueue(std::function<void ()> func);
/**
 * This is a synonym for the following, which asserts certain locks are not
 * held:
 *     std::promise<void> promise;
 *     CallFunctionInValidationInterfaceQueue([&promise] {
 *         promise.set_value();
 *     });
 *     promise.get_future().wait();
 */
void SyncWithValidationInterfaceQueue();

/**
 * Implement this to subscribe to events generated in validation
 *
 * Each CValidationInterface() subscriber will receive event callbacks
 * in the order in which the events were generated by validation.
 * Furthermore, each ValidationInterface() subscriber may assume that
 * callbacks effectively run in a single thread with single-threaded
 * memory consistency. That is, for a given ValidationInterface()
 * instantiation, each callback will complete before the next one is
 * invoked. This means, for example when a block is connected that the
 * UpdatedBlockTip() callback may depend on an operation performed in
 * the BlockConnected() callback without worrying about explicit
 * synchronization. No ordering should be assumed across
 * ValidationInterface() subscribers.
 */
class CValidationInterface {
public:
    virtual ~CValidationInterface() = default;
    virtual void AcceptedBlockHeader(const CBlockIndex* pindexNew) {}

protected:
    /**
     * Notifies listeners when the block chain tip advances.
     *
     * When multiple blocks are connected at once, UpdatedBlockTip will be called on the final tip
     * but may not be called on every intermediate tip. If the latter behavior is desired,
     * subscribe to BlockConnected() instead.
     *
     * Called on a background thread.
     */
    virtual void UpdatedBlockTip(const CBlockIndex* pindexNew, const CBlockIndex* pindexFork, bool fInitialDownload) {}
    /**
     * Notifies listeners of a transaction having been added to mempool.
     *
     * Called on a background thread.
     */
    virtual void TransactionAddedToMempool(const CTransactionRef &ptxn) {}
    /**
     * Notifies listeners of a transaction leaving mempool.
     *
     * This notification fires for transactions that are removed from the
     * mempool for the following reasons:
     *
     * - EXPIRY (expired from mempool after -mempoolexpiry hours)
     * - SIZELIMIT (removed in size limiting if the mempool exceeds -maxmempool megabytes)
     * - REORG (removed during a reorg)
     * - CONFLICT (removed because it conflicts with in-block transaction)
     * - REPLACED (removed due to RBF replacement) -- not supported yet --
     *
     * This does not fire for transactions that are removed from the mempool
     * because they have been included in a block. Any client that is interested
     * in transactions removed from the mempool for inclusion in a block can learn
     * about those transactions from the BlockConnected notification.
     *
     * Transactions that are removed from the mempool because they conflict
     * with a transaction in the new block will have
     * TransactionRemovedFromMempool events fired *before* the BlockConnected
     * event is fired. If multiple blocks are connected in one step, then the
     * ordering could be:
     *
     * - TransactionRemovedFromMempool(tx1 from block A)
     * - TransactionRemovedFromMempool(tx2 from block A)
     * - TransactionRemovedFromMempool(tx1 from block B)
     * - TransactionRemovedFromMempool(tx2 from block B)
     * - BlockConnected(A)
     * - BlockConnected(B)
     *
     * Called on a background thread.
     */
    virtual void TransactionRemovedFromMempool(const CTransactionRef &ptx, MemPoolRemovalReason reason) {}
    /**
     * Notifies listeners of a block being connected.
     * Provides a vector of transactions evicted from the mempool as a result.
     *
     * Called on a background thread.
     */
    virtual void BlockConnected(const std::shared_ptr<const CBlock> &block, const CBlockIndex *pindex) {}
    /**
     * Notifies listeners of a block being disconnected
     *
     * Called on a background thread.
     */
    virtual void BlockDisconnected(const std::shared_ptr<const CBlock> &block, const uint256& blockHash, int nBlockHeight, int64_t blockTime) {}
    /**
     * Notifies listeners of the new active block chain on-disk.
     *
     * Called on a background thread.
     */
    virtual void SetBestChain(const CBlockLocator &locator) {}
    /** Tells listeners to broadcast their data. */
    virtual void ResendWalletTransactions(CConnman* connman) {}
    virtual void BlockChecked(const CBlock&, const CValidationState&) {}
    friend void ::RegisterSharedValidationInterface(std::shared_ptr<CValidationInterface>);
    friend void ::UnregisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();
    /** Notifies listeners of updated deterministic masternode list */
    virtual void NotifyMasternodeListChanged(bool undo, const CDeterministicMNList& oldMNList, const CDeterministicMNListDiff& diff) {}
};

struct MainSignalsInstance;
class CMainSignals {
private:
    std::unique_ptr<MainSignalsInstance> m_internals;

    friend void ::RegisterSharedValidationInterface(std::shared_ptr<CValidationInterface>);
    friend void ::UnregisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();
    friend void ::CallFunctionInValidationInterfaceQueue(std::function<void ()> func);

public:
    /** Register a CScheduler to give callbacks which should run in the background (may only be called once) */
    void RegisterBackgroundSignalScheduler(CScheduler& scheduler);
    /** Unregister a CScheduler to give callbacks which should run in the background - these callbacks will now be dropped! */
    void UnregisterBackgroundSignalScheduler();
    /** Call any remaining callbacks on the calling thread */
    void FlushBackgroundCallbacks();

    size_t CallbacksPending();

    void AcceptedBlockHeader(const CBlockIndex* pindexNew);
    void UpdatedBlockTip(const CBlockIndex *, const CBlockIndex *, bool fInitialDownload);
    void TransactionAddedToMempool(const CTransactionRef &ptxn);
    void TransactionRemovedFromMempool(const CTransactionRef&, MemPoolRemovalReason);
    void BlockConnected(const std::shared_ptr<const CBlock> &block, const CBlockIndex *pindex);
    void BlockDisconnected(const std::shared_ptr<const CBlock> &block, const uint256& blockHash, int nBlockHeight, int64_t blockTime);
    void SetBestChain(const CBlockLocator &);
    void Broadcast(CConnman* connman);
    void BlockChecked(const CBlock&, const CValidationState&);
    void NotifyMasternodeListChanged(bool undo, const CDeterministicMNList& oldMNList, const CDeterministicMNListDiff& diff);
};

CMainSignals& GetMainSignals();

#endif // PIVX_VALIDATIONINTERFACE_H

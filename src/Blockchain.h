#pragma once

#include <cstdint>
#include <vector>
#include <queue>

#include "Transactions.h"
#include "Settings.h"
#include "Block.h"
#include "AshLogger.h"

namespace ash
{

#ifdef _RELEASE
constexpr auto TARGET_TIMESPAN = 10u * 60u; // in seconds
constexpr auto BLOCK_INTERVAL = 25u; // in blocks
#else
constexpr auto TARGET_TIMESPAN  = 60u; // in seconds
constexpr auto BLOCK_INTERVAL   = 10u; // in blocks
#endif

class Blockchain;
using BlockChainPtr = std::unique_ptr<Blockchain>;

struct LedgerInfo;
using AddressLedger = std::vector<LedgerInfo>;

void to_json(nl::json& j, const Blockchain& b);
void from_json(const nl::json& j, Blockchain& b);

void to_json(nl::json& j, const LedgerInfo& li);
void from_json(const nl::json& j, LedgerInfo& li);
void to_json(nl::json& j, const AddressLedger& ledger);
void from_json(const nl::json& j, AddressLedger& ledger);

UnspentTxOuts GetUnspentTxOuts(const Blockchain& chain, const std::string& address = {});

AddressLedger GetAddressLedger(const Blockchain& chain, const std::string& address);

// TODO: should this return an optional?
double GetAddressBalance(const Blockchain& chain, const std::string& address);

std::tuple<TxResult, ash::Transaction> CreateTransaction(Blockchain& chain, std::string_view senderPK, std::string_view receiver, double amount);

// 0 - block index, 1 - tx index
using TxPoint = std::tuple<std::uint64_t, std::uint64_t>;
std::optional<TxPoint> FindTransaction(const Blockchain& chain, std::string_view txid);

// TODO: should this return an optional?
// fills in the TxIn TxPoint info for all the Transactions in the Block
Block GetBlockDetails(const Blockchain& chain, std::size_t index);

struct LedgerInfo
{
    std::uint64_t   blockIdx;
    std::string     txid;
    BlockTime       time;
    double          amount;

    bool operator==(const LedgerInfo& rhs) const
    {
        return blockIdx == rhs.blockIdx
            && txid == rhs.txid;
    }

    bool operator!=(const LedgerInfo& rhs) const
    {
        return !operator==(rhs);
    }
};

//! This class is not thread safe and assumes that the
//  client handles synchronization
class Blockchain final
{
    std::vector<Block>          _blocks;
    UnspentTxOuts               _unspentTxOuts;
    std::queue<Transaction>     _txQueue; // transactions waiting to be mined by this miner
    SpdLogPtr                   _logger;

    friend class ChainDatabase;
    friend void to_json(nl::json& j, const Blockchain& b);
    friend void from_json(const nl::json& j, Blockchain& b);

public:
    using iterator = std::vector<Block>::iterator;

    Blockchain();

    Blockchain(Blockchain&&) = default;
    Blockchain& operator=(const Blockchain&) = default;
    
    auto begin() const -> decltype(_blocks.begin())
    {
        return _blocks.begin();
    }

    auto end() const -> decltype(_blocks.end())
    {
        return _blocks.end();
    }

    auto rbegin() const -> decltype(_blocks.rbegin())
    {
        return _blocks.rbegin();
    }

    auto rend() const -> decltype(_blocks.rend())
    {
        return _blocks.rend();
    }

    auto front() const -> decltype(_blocks.front())
    {
        return _blocks.front();
    }

    auto back() const -> decltype(_blocks.back())
    {
        return _blocks.back();
    }

    std::size_t size() const 
    { 
        return _blocks.size(); 
    }

    void clear()
    {
        _blocks.clear();
    }

    void resize(std::size_t size)
    {
        _blocks.resize(size);
    }

    auto at(std::size_t index) const -> decltype(_blocks.at(index))
    {
        return _blocks.at(index);
    }

    const auto& txAt(std::size_t blockIndex, std::size_t txIndex) const
    {
        assert(blockIndex < size());
        assert(txIndex < at(blockIndex).transactions().size());
        return at(blockIndex).transactions().at(txIndex);
    }

    auto& txAt(std::size_t blockIndex, std::size_t txIndex)
    {
        return const_cast<Transaction&>(
                static_cast<const Blockchain&>(*this).txAt(blockIndex,txIndex));
    }

    bool addNewBlock(const Block& block);
    bool addNewBlock(const Block& block, bool checkPreviousBlock);
    BlockUniquePtr createUnminedBlock(const std::string& coinbasewallet);

    bool isValidBlockPair(std::size_t idx) const;
    bool isValidChain() const;

    std::uint64_t cumDifficulty() const;
    std::uint64_t cumDifficulty(std::size_t idx) const;
    std::uint64_t getAdjustedDifficulty();

    void queueTransaction(Transaction&& tx);
    std::size_t transactionQueueSize() const noexcept;
    std::size_t reQueueTransactions(Block& block);
};

}

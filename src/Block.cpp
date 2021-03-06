#include <sstream>
#include <ostream>
#include <ctime>

#include "CryptoUtils.h"
#include "Block.h"

namespace nl = nlohmann;

namespace ash
{

void to_json(nl::json& j, const Block& b)
{
    j["index"] = b.index();
    j["nonce"] = b.nonce();
    j["difficulty"] = b.difficulty();
    j["hash"] = b.hash();
    j["prev"] = b.previousHash();
    j["miner"] = b.miner();
    j["data"] = b.data();
    j["transactions"] = b.transactions();

    j["time"] = 
        static_cast<std::uint64_t>(b.time().time_since_epoch().count());
}

void from_json(const nl::json& j, Block& b)
{
    j["index"].get_to(b._hashed._index);
    j["nonce"].get_to(b._hashed._nonce);
    j["difficulty"].get_to(b._hashed._difficulty);
    j["data"].get_to(b._hashed._data);
    j["prev"].get_to(b._hashed._prev);
    j["hash"].get_to(b._hash);
    j["miner"].get_to(b._miner);
    j["transactions"].get_to(b._hashed._txs);

    b._hashed._time = 
        BlockTime{std::chrono::milliseconds{j["time"].get<std::uint64_t>()}};
}

bool ValidHash(const Block& block)
{
    const auto computedHash = CalculateBlockHash(block);
    if (computedHash != block.hash())
    {
        return false;
    }

    std::string zeros;
    zeros.assign(block.difficulty(), '0');
    if (block.hash().compare(0, block.difficulty(), zeros) != 0)
    {
        return false;
    }

    return true;
}

bool ValidNewBlock(const Block& block, const Block& prevblock)
{
    if (!ValidHash(block))
    {
        return false;
    }

    return block.index() == prevblock.index() + 1
        && block.previousHash() == prevblock.hash();
}

std::string CalculateBlockHash(
    std::uint64_t index, 
    std::uint64_t nonce, 
    std::uint64_t difficulty,
    BlockTime time,
    const std::string& data, 
    const std::string& previous,
    const std::string& extraText)
{
    std::stringstream ss;
    ss << index
        << nonce
        << difficulty
        << data
        << time.time_since_epoch().count()
        << previous
        << extraText;

    return ash::crypto::SHA256(ss.str());
}

std::string CalculateBlockHash(const Block& block)
{
    const auto extra = 
        ash::crypto::SHA256(nl::json(block.transactions()).dump());

    return CalculateBlockHash(
        block.index(),
        block.nonce(),
        block.difficulty(),
        block.time(),
        block.data(),
        block.previousHash(),
        extra );
}

Block::Block(std::uint64_t index, std::string_view prevHash, Transactions&& txs)
    : _logger(ash::initializeLogger("Block"))
{
    _hashed._index = index;
    _hashed._nonce = 0;
    _hashed._difficulty = 1;
    _hashed._time = 
        std::chrono::time_point_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now());
    _hashed._prev = prevHash;
    _hashed._txs = std::move(txs);

    _hash = CalculateBlockHash(*this);
}

bool Block::operator==(const Block & other) const
{
    return _hashed._index == other._hashed._index
        && _hashed._nonce == other._hashed._nonce
        && _hashed._data == other._hashed._data
        && _hashed._time == other._hashed._time;
}

} // namespace

namespace std
{

std::ostream& operator<<(std::ostream & os, const ash::Block & block)
{
    os << "block { index: " << block.index() << " hash: " << block.hash() << " data: " << block.data() << " }";
    return os;
}

} // namespace std
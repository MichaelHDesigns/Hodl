// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef HODLCASH_PRIMITIVES_BLOCK_H
#define HODLCASH_PRIMITIVES_BLOCK_H

#include "primitives/transaction.h"
#include "serialize.h"
#include "uint256.h"

#include "consensus/consensus.h" 

static const unsigned int FORK1HEIGHT = 25000;
static const unsigned int FORK2HEIGHT = 31000;
static const unsigned int FORK3HEIGHT = 75000;
static const unsigned int FORK4HEIGHT = 75100;

const uint32_t BIP_009_MASK = 0x20000000;
const uint32_t BASE_VERSION = 0x20000000;  // Will accept 2MB blocks
const uint32_t FORK_BIT_2MB = 0x10000000;  // Vote for 2MB fork
// HFP0 removed Classic DEFAULT_2MB_VOTE

// HFP0 FRK begin
const uint32_t FULL_FORK_VERSION_MIN = FORK4HEIGHT; // HFP0 starting version
const uint32_t FULL_FORK_VERSION_MAX = FULL_FORK_VERSION_MIN + 15; // HFP0 end of version range, so that others can fork safely
const uint32_t FULL_FORK_VERSION_CUR = FULL_FORK_VERSION_MIN;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    static const int32_t CURRENT_VERSION=4;
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
    uint32_t nStartLocation;
    uint32_t nFinalCalculation;


    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
        READWRITE(nStartLocation);
        READWRITE(nFinalCalculation);
    }

    void SetNull()
    {
        nVersion = CBlockHeader::CURRENT_VERSION;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        nStartLocation = 0;
        nFinalCalculation = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;
    uint256 GetHashNoCache() const;
    uint256 GetMidHash() const;
    uint256 FindBestPatternHash(int& collisions,char *scratchpad,int nThreads,int* minerStopFlag);

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransaction> vtx;

    // memory only
    mutable std::vector<uint256> vMerkleTree;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    static bool VersionKnown(int32_t nVersion, int32_t voteBits)
    {
        if (nVersion >= 1 && nVersion <= 4)
            return true;
        // BIP009 / versionbits:
        if (nVersion & BIP_009_MASK)
        {
            uint32_t v = nVersion & ~BIP_009_MASK;
            if ((v & ~voteBits) == 0)
                return true;
        }
        return false;
    }


    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        vMerkleTree.clear();
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion          = nVersion;
        block.hashPrevBlock     = hashPrevBlock;
        block.hashMerkleRoot    = hashMerkleRoot;
        block.nTime             = nTime;
        block.nBits             = nBits;
        block.nNonce            = nNonce;
        block.nStartLocation    = nStartLocation;
        block.nFinalCalculation = nFinalCalculation;
        return block;
    }

    // Build the in-memory merkle tree for this block and return the merkle root.
    // If non-NULL, *mutated is set to whether mutation was detected in the merkle
    // tree (a duplication of transactions in the block leading to an identical
    // merkle root).
    uint256 BuildMerkleTree(bool* mutated = NULL) const;

    std::vector<uint256> GetMerkleBranch(int nIndex) const;
    static uint256 CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex);
    std::string ToString() const;
};


/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

#endif // HODLCASH_PRIMITIVES_BLOCK_H

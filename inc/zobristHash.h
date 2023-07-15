#pragma once
#include <stdint.h>
#include "boardUtil.h"

namespace zobristHash
{

class ZobristHash
{
public:
  ZobristHash()
  {
    initHashtable();
  };
  ~ZobristHash() = default;
  uint64_t zobristHash(const piece::BoardState& pos);
  void initHashtable(int seed = 0);

  uint64_t pieceHashTable[12][64];
  uint64_t epHash[8];
  uint64_t castleHash[0b1111];
  uint64_t whiteTurn;
  uint64_t moveHash;
};

};  // namespace zobristHash
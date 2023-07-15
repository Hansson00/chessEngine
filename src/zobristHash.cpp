#include "../inc/zobristHash.h"
#include <random>
#include <intrin.h>
#include <stdint.h>
#include "../inc/magicalBitboards.h"
namespace zobristHash
{

void ZobristHash::initHashtable(int seed)
{
  std::random_device osSeed;
  std::mt19937 generator(0);
  std::uniform_int_distribution<uint64_t> distribute(0, LLONG_MAX);

  for (int i = 0; i < 12; i++)
  {
    for (int j = 0; j < 64; j++)
    {
      pieceHashTable[i][j] = distribute(generator);
    }
  }
  for (int i = 0; i < 8; i++)
  {
    epHash[i] = distribute(generator);
  }
  for (int i = 0; i < 0b1111; i++)
  {
    castleHash[i] = distribute(generator);
  }
  whiteTurn = distribute(generator);
  moveHash = distribute(generator);
}
uint64_t ZobristHash::zobristHash(const piece::BoardState& bs)
{
  uint64_t hash = bs.turns * moveHash;
  if (bs.whiteTurn)
    hash ^= whiteTurn;
  unsigned long position;

  for (int i = 0; i < 10; i++)
  {
    uint64_t currentBoard = bs.pieceBoards[i];
    while (currentBoard)
    {
      _BitScanForward64(&position, currentBoard);
      hash ^= pieceHashTable[i][position];
      currentBoard &= currentBoard - 1;
    }
  }
  hash ^= pieceHashTable[10][bs.kings[0]];
  hash ^= pieceHashTable[11][bs.kings[1]];
  hash ^= castleHash[(bs.castlingRights & 0b1111)];

  if (bs.enPassant)
  {
    hash ^= epHash[(bs.enPassant & 7)];  // % 8
  }
  return hash;
}

}  // namespace zobristHash
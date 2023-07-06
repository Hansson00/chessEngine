#include "../inc/boardUtil.h"

// TODO FIX THESE!?! BAD FUNCTIONS USE BISHOP AND ROOK MASKS INSTED!
uint64_t piece::pinnedStraight(uint8_t king, uint8_t piece)
{
  // On the same rank
  if ((king >> 3) == (piece >> 3))
    return ranks[king >> 3];

  // On the same file

  king &= 7;
  if (king == (piece & 7))
    return files[king];
  return 0;
}

uint64_t piece::pinnedDiagonal(uint8_t king, uint8_t piece)
{
  int diagonal = (king >> 3) + (king & 7);
  if (diagonal == ((piece >> 3) + (piece & 7)))
    return anti_diagonals[diagonal];

  diagonal = 7 - (king >> 3) + (king & 7);
  if (diagonal == (7 - (piece >> 3) + (piece & 7)))
    return main_diagonals[(7 - (king >> 3) + (king & 7))];

  return 0;
}

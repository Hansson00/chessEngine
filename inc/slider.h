#pragma once

#include <intrin.h>
#include <cinttypes>
#include "boardUtil.h"

namespace piece
{
namespace helper
{

class Slider
{
protected:
  /**
   * @Brief Adds moves to a move list by utilizing a given move mask
   *
   * @Param [in] board, current board mask
   * @Param [in] moves, available move mask
   * @Param [in] position, position of the current piece
   * @Param [in] type, which kind of piece
   * @Param [out] list which to add moves to
   */
  static void generateSlidingMoves(uint64_t board, uint64_t moves, uint32_t position, uint8_t type, boardUtil::MoveList& ml)
  {
    unsigned long dest;
    while (moves)
    {
      _BitScanForward64(&dest, moves);
      ml.add(position | (dest << 6) | type | ISCAPTURE(board, dest));
      moves &= moves - 1;
    }
  }
};

}  // namespace helper
}  // namespace piece
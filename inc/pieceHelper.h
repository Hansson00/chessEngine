#pragma once
#include "boardUtil.h"
#include <cinttypes>

namespace piece
{
namespace helper
{

class PieceHelper
{
public:
  PieceHelper()
  {
  }

  ~PieceHelper() = default;

  template<bool whiteTurn, bool pinnedSquares>
  constexpr void generateMoves(const boardUtil::BoardState& bs, boardUtil::MoveList&);

  template<bool whiteTurn, bool pinnedSquares>
  constexpr void generateAttacks(const boardUtil::BoardState& bs, boardUtil::MoveList&);

private:
};
}  // namespace helper
}  // namespace piece
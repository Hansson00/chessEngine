#pragma once
#include "boardUtil.h"
#include <cinttypes>

namespace piece {
namespace helper {

class PieceHelper
{
public:
  PieceHelper()
  {
  }

  ~PieceHelper() = default;

  virtual void generateMoves(const boardUtil::BoardState& bs, boardUtil::MoveList&);

private:
};
}  // namespace helper
}  // namespace piece
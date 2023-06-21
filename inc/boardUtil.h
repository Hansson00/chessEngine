
#pragma once
#include <cinttypes>

namespace boardUtil {

struct BoardState
{
  // 0-4 white Queen, white Rook, white Bishop, white Knight, white Pawn
  // 5-9 black Queen, black Rook, black Bishop, black Knight, black Pawn
  uint64_t pieceBoards[10] = {};
  uint64_t teamBoards[3] = {};

  // Mask for pieces that are pinned to the king
  uint64_t pinnedSquares = 0;

  // Mask for squares that has to be blocked
  uint64_t blockMask = ~0ULL;

  uint64_t attacks = 0;
  uint8_t kings[2] = {};
  uint8_t numCheckers = 0;
  uint8_t castlingRights = 0;
  uint8_t enPassant = 0;

  bool whiteTurn = 1;
};

struct MoveList
{
  uint32_t move[100];
  uint8_t end = 0;
  void inline add(uint32_t newMove)
  {
    move[end++] = newMove;
  }
  void inline reset()
  {
    end = 0;
    move[0] = 0;
  }
};

enum class pieceType
{
  Queen,
  Rook,
  Bishop,
  Knight,
  Pawn,
  King
};

}  // namespace boardUtil
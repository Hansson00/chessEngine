
#pragma once
#include <cinttypes>

namespace piece {
namespace boardUtil {

#define ISCAPTURE(board, destination) (static_cast<uint32_t>(boardUtil::MoveListType::CAPTURE) * ((board & (1ULL << destination)) != 0))

enum class MoveListType
{
  CAPTURE = 4194304
};

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

constexpr uint64_t files[8] = {0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL,
                               0x0808080808080808ULL, 0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL,
                               0x8080808080808080ULL};
constexpr uint64_t ranks[8] = {0xFFLL, 0xFF00LL, 0xFF0000LL, 0xFF000000LL, 0xFF00000000LL, 0xFF0000000000LL,
                               0xFF000000000000LL, static_cast<uint64_t>(0xFF00000000000000LL)};
constexpr uint64_t main_diagonals[15] = {0x0100000000000000ULL, 0x0201000000000000ULL, 0x0402010000000000ULL, 0x0804020100000000ULL,
                                         0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL,
                                         0x80402010080402ULL, 0x804020100804ULL, 0x8040201008ULL, 0x80402010ULL, 0x804020ULL, 0x8040ULL, 0x80ULL};
constexpr uint64_t anti_diagonals[15] = {0x1ULL, 0x0102ULL, 0x010204ULL, 0x01020408ULL, 0x0102040810ULL, 0x010204081020ULL, 0x01020408102040ULL,
                                         0x0102040810204080ULL, 0x0204081020408000ULL, 0x0408102040800000ULL, 0x0810204080000000ULL, 0x1020408000000000ULL,
                                         0x2040800000000000ULL, 0x4080000000000000ULL, 0x8000000000000000ULL};

}  // namespace boardUtil
}  // namespace piece

#pragma once
#include <cinttypes>

namespace piece
{

constexpr auto PIECE_OFFSET = 5;
#define GETPIECES(piece) bs.pieceBoards[piece + PIECE_OFFSET * whiteTurn]
#define GETENEMYPIECES(piece) bs.pieceBoards[piece + PIECE_OFFSET * whiteTurn]
#define CASTLING_RIGHTS (0b1100 - 0b1001 * whiteTurn)

constexpr uint64_t CASTELING_K_CHECK = 1 << 4;
constexpr uint64_t CASTELING_Q_CHECK = 1 << 5;

const static char piece[12] = {'Q', 'R', 'B', 'N', 'P', 'q', 'r', 'b', 'n', 'p'};

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

enum moveModifiers
{
  FROM = 63,
  TO = 4032,
  FROM_TO = 4095,
  /* Attack piece board */
  ATTACKERS = 258048,
  king = 4096,
  queen = 8192,
  rook = 16384,
  bishop = 32768,
  knight = 65536,
  pawn = 131072,

  WHITE_TO_MOVE = 262144,
  EN_PESSANT_CAP = 524288,
  CASTLE_KING = 1048576,
  CASTLE_QUEEN = 2097152,
  CAPTURE = 4194304,
  DPUSH = 8388608,
  PROMO_QUEEN = 16777216,
  PROMO_ROOK = 33554432,
  PROMO_BISHOP = 67108864,
  PROMO_KNIGHT = 134217728,
  PROMO = 251658240
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

}  // namespace piece
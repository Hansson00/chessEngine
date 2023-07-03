#pragma once

#include <cinttypes>
#include <stdint.h>

#include "boardUtil.h"
#include "magicalBitboards.h"

namespace piece
{

struct PawnMoves
{
  const char back;
  const char backx2;
  const char backLeft;
  const char backRight;
};

class MoveGeneration : private magicalBits::MagicalBitboards
{
public:
  MoveGeneration();
  ~MoveGeneration() = default;

  // Generation
  void startUp(BoardState& bs, MoveList& ml);
  void generatePossibleMoves(const BoardState& bs, MoveList& ml);
  void makeMove(BoardState& bs, const uint32_t move);

  template<bool whiteTurn, bool pins>
  const void generatePawnMoves(const BoardState& bs, MoveList& ml);

  template<bool whiteTurn>
  const void generatePawnAttacks(const BoardState& bs, uint64_t& attacks);

  const uint64_t singlePawnAttack(uint8_t king, bool white);

private:
  // init functions
  void initKingAttacks();
  void initKnightAttacks();
  void initPawnAttacks();

  // Attack functions
  template<piece::pieceType P>
  const inline uint64_t generatePieceAttacks(uint64_t board, uint32_t piece);

  const inline uint64_t getBishopAttacks(uint8_t square, uint64_t occupancy);
  const inline uint64_t getRookAttacks(uint8_t square, uint64_t occupancy);

  // Move generation
  template<bool whiteTurn>
  const void generateKingMoves(const BoardState& bs, MoveList& ml);

  template<bool whiteTurn, piece::pieceType p>
  const void generatePieceMoves(const piece::BoardState& bs,
                                piece::MoveList& ml);

  // Rule functions
  template<bool whiteTurn, bool castlingAllowed, bool enemyCastlingAllowed>
  const void movePiece(BoardState& bs, uint32_t move);

  template<bool whiteTurn>
  const void generateAttacks(BoardState& bs);

  template<bool whiteTurn>
  const void generatePinsBlocks(BoardState& bs);

  template<bool whiteTurn>
  const void generateCastlingOptions(BoardState& bs);

  template<bool whiteTurn>
  const void enPassantHelper(const BoardState& bs, MoveList& ml, uint64_t& epPosBitboard);

  template<bool whiteTurn>
  const void promotionHelper(const BoardState& bs, MoveList& ml, uint64_t& promoting);

  template<bool whiteTurn>
  void pinnedPawnsHelper(const BoardState& bs, MoveList& ml, uint64_t& pinnedPawns);

  // Helper functions

  template<piece::pieceType p>
  static void generateMultipleMoves(uint64_t board, uint64_t moves, uint32_t position, MoveList& ml);

  static inline uint32_t isCapture(uint64_t board, unsigned long dest);

  template<bool white, bool king>
  static constexpr uint64_t castlingRookStartPos();

  template<bool white, bool king>
  static constexpr uint64_t castlingRookEndPos();

  // PAWN stuff

  template<bool whiteTurn>
  static inline uint64_t shiftUp(uint64_t pawns);

  template<bool whiteTurn>
  static inline void shiftSide(uint64_t& right, uint64_t& left);

  template<bool pins>
  static void pawnBitScan(const BoardState& bs, MoveList& ml, uint64_t& pawns, const char dir, const uint32_t& move_type);

  template<bool whiteTurn>
  const bool checkPinEP(const BoardState& bs, uint32_t start_pos);

  std::function<uint64_t(uint8_t, uint64_t)> m_rowAttacks_f;

  static constexpr uint8_t whitePawns = 4;
  static constexpr uint8_t blackPawns = 9;

  // Test if these are correct
  static constexpr uint64_t rank7 = 0b11111111ULL << (8 * 6);
  static constexpr uint64_t rank6 = 0b11111111ULL << (8 * 5);
  static constexpr uint64_t rank3 = 0b11111111ULL << (8 * 2);
  static constexpr uint64_t rank2 = 0b11111111ULL << (8);

  static constexpr PawnMoves whiteMoves = {-8, -16, -7, -9};
  static constexpr PawnMoves blackMoves = {8, 16, 7, 9};
  // Memeber variables
  uint64_t m_kingAttacks[64];
  uint64_t m_knightAttacks[64];
  uint64_t m_pawnAttacks[2][56];
};

}  // namespace piece
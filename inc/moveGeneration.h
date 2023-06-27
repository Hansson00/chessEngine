#pragma once
#include <stdint.h>
#include "../inc/boardUtil.h"
#include <cinttypes>

namespace piece
{

class MoveGeneration
{
public:
  MoveGeneration();
  ~MoveGeneration() = default;

  // Generation
  void startUp(BoardState& bs, MoveList& ml);
  void generatePossibleMoves(const BoardState& bs, MoveList& ml);
  void makeMove(BoardState& bs, const uint32_t move);

private:
  // init functions
  void initKingAttacks();
  void initKnightAttacks();
  void initRowAttacks();
  void initMaindiagonalAttacks();
  void initAntidiagonalAttacks();

  // init helpers
  static uint64_t calcRowAttack(uint64_t rowOccupancy, int position);
  static uint64_t setMaindiagonalOccupancy(uint64_t diagonal, uint32_t occupancy);
  static uint64_t calcMaindiagonalAttack(uint64_t board_occupancy, int pos);
  static uint64_t setAntidiagonalOccupancy(uint64_t diagonal, uint32_t occupancy);
  static uint64_t calcAntidiagonalAttack(uint64_t board_occupancy, int position);
  static uint32_t longHighBitScan(uint64_t i);
  static uint32_t highBitScan(int32_t i);

  // Attack functions
  template<piece::pieceType P>
  uint64_t generatePieceAttacks(uint64_t board, uint32_t piece);

  // Helper Attacks
  const uint64_t fileAttacks(uint64_t board, uint32_t position);
  const uint64_t rankAttacks(uint64_t board, uint32_t position);

  const uint64_t mainDiagonalAttacks(uint64_t board, int position);
  const uint64_t antiDiagonalAttacks(uint64_t board, int position);

  const inline void fileToRank(uint64_t& bit_board, uint32_t file);
  const inline void rankToFile(uint64_t& bit_board, uint32_t rank);

  // Move generation
  template<bool whiteTurn>
  const void generateKingMoves(const BoardState& bs, MoveList& ml);

  template<bool whiteTurn, piece::pieceType p>
  const void generatePieceMoves(const piece::BoardState& bs, piece::MoveList& ml);

  template<bool whiteTurn>
  const void generatePawnMoves(const BoardState& bs, MoveList& ml);

  // Rule functions
  template<bool whiteTurn, bool castlingAllowed, bool enemyCastlingAllowed>
  const void movePiece(BoardState& bs, uint32_t move);

  template<bool whiteTurn>
  const void generateAttacks(BoardState& bs);

  template<bool whiteTurn>
  const void generatePinsBlocks(BoardState& bs);

  template<bool whiteTurn>
  const void generateCastlingOptions(BoardState& bs);

  // Helper functions
  template<bool whiteTurn>
  const void promotionHelper(const BoardState& bs, MoveList& ml, uint32_t promoting);

  static uint64_t pinnedRow(uint8_t king, uint8_t piece);
  static uint64_t pinnedDiagonal(uint8_t king, uint8_t piece);

  template<bool whiteTurn>
  static uint64_t pawnAttacks(uint64_t pawns);

  template<piece::pieceType p>
  static void generateMultipleMoves(uint64_t board, uint64_t moves, uint32_t position, MoveList& ml);

  static inline uint32_t isCapture(uint64_t board, unsigned long dest);

  template<bool whiteTurn>
  static inline uint64_t shiftUp(uint64_t pawns);

  template<bool whiteTurn>
  static inline void shiftSide(uint64_t& right, uint64_t& left);

  static void pawnBitScan(const BoardState& bs, MoveList& ml, uint64_t& pawns, const int& dir, const uint32_t& move_type);

  template<bool whiteTurn>
  const bool checkPinEP(const BoardState& bs, uint32_t start_pos);

  template<bool white, bool king>
  static constexpr uint64_t castlingRookStartPos();

  template<bool white, bool king>
  static constexpr uint64_t castlingRookEndPos();

private:
  // Memeber variables
  uint64_t m_kingAttacks[64];
  uint64_t m_knightAttacks[64];
  uint64_t** m_bishopMain[15];
  uint64_t** m_bishopAnti[15];
  uint64_t m_rookAttackTable[8][64];
  uint64_t m_shiftedAntiDiagonal = (1ULL) | (1ULL << 7) | (1ULL << 14) | (1ULL << 21) | (1ULL << 28) | (1ULL << 35) | (1ULL << 42) | (1ULL << 49);
};

const uint32_t mask_bits[15] = {0, 0, 1, 3, 7, 15, 31, 63, 31, 15, 7, 3, 1, 0, 0};

}  // namespace piece
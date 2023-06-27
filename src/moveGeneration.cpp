#include "../inc/moveGeneration.h"
#include <intrin.h>
#include <stdint.h>

namespace piece
{

MoveGeneration::MoveGeneration()
{
  initKingAttacks();
  initKnightAttacks();
  initRowAttacks();
  initMaindiagonalAttacks();
  initAntidiagonalAttacks();
}

////////////////////////////////////////////////////////////////
// Generation
////////////////////////////////////////////////////////////////

void MoveGeneration::generatePossibleMoves(const BoardState& bs, MoveList& ml)
{
  if (bs.whiteTurn)
  {
    // White moves
    generateKingMoves<true>(bs, ml);
    if (bs.numCheckers > 1)  // Only king moves are legal
      return;
    generatePawnMoves<true>(bs, ml);
    generatePieceMoves<true, piece::pieceType::Queen>(bs, ml);
    generatePieceMoves<true, piece::pieceType::Rook>(bs, ml);
    generatePieceMoves<true, piece::pieceType::Bishop>(bs, ml);
    generatePieceMoves<true, piece::pieceType::Knight>(bs, ml);
  }
  else
  {
    // Black moves
    generateKingMoves<false>(bs, ml);
    if (bs.numCheckers > 1)  // Only king moves are legal
      return;
    generatePawnMoves<false>(bs, ml);
    generatePieceMoves<false, piece::pieceType::Queen>(bs, ml);
    generatePieceMoves<false, piece::pieceType::Rook>(bs, ml);
    generatePieceMoves<false, piece::pieceType::Bishop>(bs, ml);
    generatePieceMoves<false, piece::pieceType::Knight>(bs, ml);
  }
}

void MoveGeneration::makeMove(BoardState& bs, const uint32_t move)
{
  if (bs.whiteTurn)
  {
    switch (bs.castlingRights)
    {
      case 0:
        movePiece<true, false, false>(bs, move);
        break;
      case 0b1:
      case 0b10:
        movePiece<true, true, false>(bs, move);
        break;
      case 0b100:
      case 0b1000:
        movePiece<true, false, true>(bs, move);
        break;
      default:
        movePiece<true, true, true>(bs, move);
        break;
    }
    bs.whiteTurn ^= 1;
    generateAttacks<true>(bs);
    generatePinsBlocks<true>(bs);

    bs.castlingRights &= 0b1111;
    if (!bs.numCheckers && bs.castlingRights & 0b11)
      generateCastlingOptions<true>(bs);
  }
  else
  {
    switch (bs.castlingRights)
    {
      case 0:
        movePiece<false, false, false>(bs, move);
        break;
      case 0b1:
      case 0b10:
        movePiece<false, false, true>(bs, move);
        break;
      case 0b100:
      case 0b1000:
        movePiece<false, true, false>(bs, move);
        break;
      default:
        movePiece<false, true, true>(bs, move);
        break;
    }

    bs.whiteTurn ^= 1;
    generateAttacks<false>(bs);
    generatePinsBlocks<false>(bs);

    bs.castlingRights &= 0b1111;
    if (!bs.numCheckers && bs.castlingRights & 0b1100)
      generateCastlingOptions<false>(bs);
  }
}

////////////////////////////////////////////////////////////////
// Init functions
////////////////////////////////////////////////////////////////

// Done
void MoveGeneration::initKingAttacks()
{
  for (int i = 0; i < 64; i++)
  {
    uint64_t king = 1ULL << i;
    uint64_t attack = (king >> 9) & ~(files[7] | ranks[7]);  // Diagonal left up
    attack |= (king >> 1) & ~(files[7]);                     // One step left
    attack |= (king << 7) & ~(files[7] | ranks[0]);          // Diagonal left down
    attack |= (king >> 8) & ~(ranks[7]);                     // One step up
    attack |= (king << 8) & ~(ranks[0]);                     // One step down
    attack |= (king << 1) & ~(files[0]);                     // One step right
    attack |= (king >> 7) & ~(files[0] | ranks[7]);          // Diagonal right up
    attack |= (king << 9) & ~(files[0] | ranks[0]);          // Diagonal right down
    m_kingAttacks[i] = attack;
  }
}
// Done
void MoveGeneration::initKnightAttacks()
{
  for (int i = 0; i < 64; i++)
  {
    uint64_t knight = 1LL << i;
    uint64_t attack = (knight >> 10) & ~(files[6] | files[7] | ranks[7]);  // Two files to left and up
    attack |= (knight << 6) & ~(files[6] | files[7] | ranks[0]);           // Two files to left and down
    attack |= (knight >> 17) & ~(files[7] | ranks[6] | ranks[7]);          // One file to left and up
    attack |= (knight << 15) & ~(files[7] | ranks[0] | ranks[1]);          // One file to left and down
    attack |= (knight >> 15) & ~(files[0] | ranks[6] | ranks[7]);          // One file to right and up
    attack |= (knight << 17) & ~(files[0] | ranks[0] | ranks[1]);          // One file to right and down
    attack |= (knight >> 6) & ~(files[0] | files[1] | ranks[7]);           // Two files to right and up
    attack |= (knight << 10) & ~(files[0] | files[1] | ranks[0]);          // Two files to right and down
    m_knightAttacks[i] = attack;
  }
}
// Done
void MoveGeneration::initRowAttacks()
{
  for (uint32_t i = 0; i < 8; i++)
  {
    for (uint64_t j = 0; j < 64; j++)
    {
      // Outer bits don't matter, therefore boardmask i.e occupancy only need 2^6 bits
      m_rookAttackTable[i][j] = calcRowAttack(j << 1, i);
    }
  }
}
// Done
void MoveGeneration::initMaindiagonalAttacks()
{
  m_bishopMain[0] = new uint64_t* [1] { new uint64_t[1]{ 0ULL } };
  m_bishopMain[1] = new uint64_t* [2] { new uint64_t[1]{1ULL << 57}, new uint64_t[1]{1ULL << 48} };
  for (int d = 2; d < 13; d++)
  {
    uint64_t diagonal = main_diagonals[d];
    uint64_t tempDiagonal = diagonal;
    int numOccupancies = d < 8 ? (1 << (d - 1)) : 1 << (13 - d);
    int numPos = d < 8 ? d + 1 : 15 - d;
    // Initialize a dynamic matrix
    uint64_t** diagonal_attacks = new uint64_t*[numPos];
    for (int i = 0; i < numPos; i++)
      diagonal_attacks[i] = new uint64_t[numOccupancies];

    for (int occupancy = 0; occupancy < numOccupancies; occupancy++)
    {
      uint64_t board = setMaindiagonalOccupancy(diagonal, occupancy);
      int array_pos = 0;
      while (tempDiagonal != 0)
      {
        unsigned long pos;
        _BitScanForward64(&pos, tempDiagonal);
        // Store attack for current bishop position with given occupancy
        diagonal_attacks[array_pos++][occupancy] = calcMaindiagonalAttack(board, pos);
        // Clear bit to get next bit in next iteration
        tempDiagonal &= tempDiagonal - 1;
      }
      tempDiagonal = diagonal;
    }
    m_bishopMain[d] = diagonal_attacks;
  }
  m_bishopMain[13] = new uint64_t* [2] { new uint64_t[1]{ 1ULL << 15 }, new uint64_t[1]{ 1ULL << 6 } };
  m_bishopMain[14] = new uint64_t* [1] { new uint64_t[1]{ 0ULL } };
}
// Done
void MoveGeneration::initAntidiagonalAttacks()
{
  m_bishopAnti[0] = new uint64_t* [1] { new uint64_t[1]{ 0ULL } };
  m_bishopAnti[1] = new uint64_t* [2] { new uint64_t[1]{ 1ULL << 1 }, new uint64_t[1]{ 1ULL << 8 } };
  for (int d = 2; d < 13; d++)
  {
    uint64_t diagonal = anti_diagonals[d];
    uint64_t tempDiagonal = diagonal;
    int numOccupancies = d < 8 ? (1 << (d - 1)) : 1 << (13 - d);
    int numPos = d < 8 ? d + 1 : 15 - d;
    // Initialize a dynamic matrix
    uint64_t** diagonal_attacks = new uint64_t*[numPos];
    for (int i = 0; i < numPos; i++)
      diagonal_attacks[i] = new uint64_t[numOccupancies];

    for (int occupancy = 0; occupancy < numOccupancies; occupancy++)
    {
      uint64_t board = setAntidiagonalOccupancy(diagonal, occupancy);
      int array_pos = 0;
      while (tempDiagonal != 0)
      {
        // First bit on diagonal
        int pos = 63 - longHighBitScan(tempDiagonal);
        // Store attack for current bishop position with given occupancy
        diagonal_attacks[array_pos++][occupancy] = calcAntidiagonalAttack(board, pos);
        // Clear bit to get next bit in next iteration
        tempDiagonal &= ~(1ULL << pos);
      }
      tempDiagonal = diagonal;
    }
    m_bishopAnti[d] = diagonal_attacks;
  }
  m_bishopAnti[13] = new uint64_t* [2] { new uint64_t[1]{ 1ULL << 55 }, new uint64_t[1]{ 1ULL << 62} };
  m_bishopAnti[14] = new uint64_t* [1] { new uint64_t[1]{ 0ULL } };
}

////////////////////////////////////////////////////////////////
// Init helper functions
////////////////////////////////////////////////////////////////
// Done
uint64_t MoveGeneration::calcRowAttack(uint64_t rowOccupancy, int position)
{
  uint64_t attack = 0ULL;
  uint64_t sq;
  int iter = position - 1;
  while (iter >= 0)
  {
    sq = 1ULL << iter;
    attack |= sq;
    if ((sq & rowOccupancy) == sq)
      break;
    iter--;
  }
  iter = position + 1;
  while (iter <= 7)
  {
    sq = 1ULL << iter;
    attack |= sq;
    if ((sq & rowOccupancy) == sq)
      break;
    iter++;
  }
  return attack;
}
// Done
uint64_t MoveGeneration::setMaindiagonalOccupancy(uint64_t diagonal, uint32_t occupancy)
{
  uint32_t* bits = new uint32_t[8];
  uint64_t res = 0L;
  int i;
  for (i = 0; diagonal != 0; i++)
  {
    unsigned long bPos;
    _BitScanForward64(&bPos, diagonal);

    bits[i] = bPos;
    diagonal &= diagonal - 1;  // Reset rightmost set bit
  }
  for (int j = 1; j < i - 1; j++)
  {
    if ((occupancy & 1) == 1)
      res |= 1ULL << bits[j];
    occupancy >>= 1;
  }
  return res;
}
// Done
uint64_t MoveGeneration::calcMaindiagonalAttack(uint64_t boardOccupancy, int position)
{
  uint64_t attack = 0ULL;
  uint64_t sq;
  int prev = position % 8;
  int temp_pos = position + 9;
  int next = temp_pos % 8;
  while (abs(next - prev) < 2 && (temp_pos & ~63) == 0)
  {
    sq = 1ULL << temp_pos;
    attack |= sq;
    if ((sq & boardOccupancy) != 0)
      break;
    temp_pos += 9;
    prev = next;
    next = temp_pos % 8;
  }
  temp_pos = position - 9;
  prev = position % 8;
  next = temp_pos % 8;
  while (abs(next - prev) < 2 && (temp_pos & ~63) == 0)
  {
    sq = 1ULL << temp_pos;
    attack |= sq;
    if ((sq & boardOccupancy) != 0)
      break;
    temp_pos -= 9;
    prev = next;
    next = temp_pos % 8;
  }
  return attack;
}
// Done
uint64_t MoveGeneration::setAntidiagonalOccupancy(uint64_t diagonal, uint32_t occupancy)
{
  uint32_t* bits = new uint32_t[8];
  uint64_t res = 0L;
  int i;
  for (i = 0; diagonal != 0; i++)
  {
    int bPos = 63 - longHighBitScan(diagonal);

    bits[i] = bPos;
    diagonal &= ~(1ULL << bPos);  // Reset rightmost set bit
  }
  for (int j = 1; j < i - 1; j++)
  {
    if ((occupancy & 1) == 1)
      res |= 1ULL << bits[j];
    occupancy >>= 1;
  }
  return res;
}
// Done
uint64_t MoveGeneration::calcAntidiagonalAttack(uint64_t boardOccupancy, int position)
{
  uint64_t attack = 0ULL;
  uint64_t sq;
  int prev = position % 8;
  int temp_pos = position + 7;
  int next = temp_pos % 8;
  while (abs(next - prev) < 2 && (temp_pos & ~63) == 0)
  {
    sq = 1ULL << temp_pos;
    attack |= sq;
    if ((sq & boardOccupancy) != 0)
      break;
    temp_pos += 7;
    prev = next;
    next = temp_pos % 8;
  }
  temp_pos = position - 7;
  prev = position % 8;
  next = temp_pos % 8;
  while (abs(next - prev) < 2 && (temp_pos & ~63) == 0)
  {
    sq = 1ULL << temp_pos;
    attack |= sq;
    if ((sq & boardOccupancy) != 0)
      break;
    temp_pos -= 7;
    prev = next;
    next = temp_pos % 8;
  }
  return attack;
}
// Done, however, check if needed
uint32_t MoveGeneration::longHighBitScan(uint64_t i)
{
  int32_t x = (int32_t)(i >> 32);
  return x == 0 ? 32 + highBitScan((int32_t)i)
                : highBitScan(x);
}
// Done, however, check if needed
uint32_t MoveGeneration::highBitScan(int32_t i)
{
  if (i <= 0)
    return i == 0 ? 32 : 0;
  int32_t n = 31;
  if (i >= 1 << 16)
  {
    n -= 16;
    i >>= 16;
  }
  if (i >= 1 << 8)
  {
    n -= 8;
    i >>= 8;
  }
  if (i >= 1 << 4)
  {
    n -= 4;
    i >>= 4;
  }
  if (i >= 1 << 2)
  {
    n -= 2;
    i >>= 2;
  }
  return n - (i >> 1);
}

////////////////////////////////////////////////////////////////
// Attacks
////////////////////////////////////////////////////////////////

template<piece::pieceType p>
inline uint64_t MoveGeneration::generatePieceAttacks(uint64_t board, uint32_t position)
{
  if constexpr (p == piece::pieceType::Queen)
  {
    return mainDiagonalAttacks(board, position) | antiDiagonalAttacks(board, position) | fileAttacks(board, position) | rankAttacks(board, position);
  }
  if constexpr (p == piece::pieceType::Rook)
  {
    return fileAttacks(board, position) | rankAttacks(board, position);
  }
  if constexpr (p == piece::pieceType::Bishop)
  {
    return mainDiagonalAttacks(board, position) | antiDiagonalAttacks(board, position);
  }
  if constexpr (p == piece::pieceType::Knight)
  {
    return m_knightAttacks[position];
  }
}

////////////////////////////////////////////////////////////////
// Helper attacks
////////////////////////////////////////////////////////////////

// TODO: Fix bad operators
const uint64_t MoveGeneration::rankAttacks(uint64_t board, uint32_t rook_pos)
{
  int column = rook_pos % 8;
  int shift = (rook_pos / 8) * 8;
  uint64_t occupancy = (board >> (shift + 1)) & 0x3F;
  return m_rookAttackTable[column][occupancy] << shift;
}

// TODO: Fix bad operators
const uint64_t MoveGeneration::fileAttacks(uint64_t board, uint32_t rook_pos)
{
  int file = rook_pos % 8;
  fileToRank(board, file);

  int occupancy = (int)((board >> 1) & 0b111111);
  uint64_t fileAttack = m_rookAttackTable[rook_pos / 8][occupancy];
  rankToFile(fileAttack, 0);
  return fileAttack >> (7 - file);
}

const uint64_t MoveGeneration::mainDiagonalAttacks(uint64_t board, int position)
{
  int offset = (position & 7) - (position >> 3);
  int index = 7 + offset;
  uint64_t diagonal = main_diagonals[index] & board;
  // Mask relevant bits

  const int shift = 2 + (index - 7) * (index > 7);

  // Shift down to get occupancy bits as LSBs
  int occupancy = (int)((diagonal * files[1]) >> (56 + shift));
  // Mask edge bits for corresponding diagonal
  occupancy &= mask_bits[index];

  // Fetch attack based on diagonal, position (column) and occupancy
  return m_bishopMain[index][(position & 7) - (offset) * (offset > 0)][occupancy];
}

// TODO: Fix bad operators
const uint64_t MoveGeneration::antiDiagonalAttacks(uint64_t board, int position)
{
  /*
  int index = (bishop_pos >> 3) + (bishop_pos & 7);
  //Occupied bits of relevant diagonal
  uint64_t diagonal = anti_diagonals[index] & board;

  int shift = 1 + (index > 7) * (index - 7);

  //Rotate diagonal to rank
  int occupancy = (int)((diagonal * files[0]) >> (56 + shift));
  //Get inner bits
  occupancy &= mask_bits[index];
  //Attack based on which diagonal, which position on the diagonal, and occupancy of the diagonal

  return bishop_anti[index][(bishop_pos & 7) - (shift + 1) * !(index < 8)][occupancy];*/

  // On a given anti diagonal, sum of column and row is equal for all squares => can be indexed by the sum
  int index = position / 8 + position % 8;
  // Occupied bits of relevant diagonal
  uint64_t diagonal = anti_diagonals[index] & board;
  // To get occupancy bits we will need to know how much to shift
  int shift = index < 8 ? 1 : index - 6;
  // Rotate diagonal to rank
  int occupancy = (int)((diagonal * files[0]) >> (56 + shift));
  // Get inner bits
  occupancy = occupancy & mask_bits[index];
  // Attack based on which diagonal, which position on the diagonal, and occupancy of the diagonal
  return m_bishopAnti[index][(index < 8 ? position % 8 : position % 8 - shift + 1)][occupancy];
}

const inline void MoveGeneration::fileToRank(uint64_t& bit_board, uint32_t file)
{
  bit_board = (bit_board << (7 - file)) & files[7];
  bit_board = bit_board * m_shiftedAntiDiagonal;
  bit_board >>= 56;
}

const inline void MoveGeneration::rankToFile(uint64_t& bit_board, uint32_t rank)
{
  bit_board = (bit_board >> rank * 8) & ranks[0];
  bit_board = (((bit_board * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101010101ULL) >> 56;
  bit_board *= main_diagonals[7];
  bit_board &= files[7];
}

////////////////////////////////////////////////////////////////
// Move generation
////////////////////////////////////////////////////////////////

template<bool whiteTurn>
const void MoveGeneration::generateKingMoves(const BoardState& bs, MoveList& ml)
{
  // both = 0, white = 1, black = 2
  const uint64_t board = bs.teamBoards[0];
  const uint64_t team = bs.teamBoards[2 - whiteTurn];

  const uint64_t not_attacked = ~bs.attacks;
  const uint8_t k_pos = bs.kings[!whiteTurn];

  if (bs.castlingRights & CASTELING_K_CHECK)
  {
    constexpr uint8_t castleMoveK = static_cast<uint8_t>((62 - 56 * whiteTurn) << 6);

    ml.add(k_pos | castleMoveK | moveModifiers::king | moveModifiers::CASTLE_KING);
  }
  if (bs.castlingRights & CASTELING_Q_CHECK)
  {
    constexpr uint8_t castleMoveQ = static_cast<uint8_t>((58 - 56 * whiteTurn) << 6);
    ml.add(k_pos | castleMoveQ | moveModifiers::king | moveModifiers::CASTLE_QUEEN);
  }

  // Regular moves
  uint64_t moves = m_kingAttacks[k_pos] & not_attacked & ~team;
  unsigned long dest;
  while (moves)
  {
    _BitScanForward64(&dest, moves);
    ml.add(k_pos | dest << 6 | moveModifiers::king | isCapture(board, dest));
    moves &= moves - 1;
  }
}

template<bool whiteTurn, piece::pieceType p>
const void MoveGeneration::generatePieceMoves(const piece::BoardState& bs, piece::MoveList& ml)
{
  // both = 0, white = 1, black = 2
  const uint64_t board = bs.teamBoards[0];
  const uint64_t team = bs.teamBoards[2 - whiteTurn];
  const uint64_t nonTeam = ~team;

  // 5 = pieces offset
  constexpr uint8_t pieceIndex = static_cast<uint8_t>(p) + whiteTurn * 5;
  uint64_t movingPieces = bs.pieceBoards[pieceIndex];
  uint64_t moves = 0;
  unsigned long piece;

  while (movingPieces)
  {
    _BitScanForward64(&piece, movingPieces);

    if (uint64_t pinnedPiece = 1ULL << piece & bs.pinnedSquares)
    {
      if constexpr (p == piece::pieceType::Queen)
      {
        moves = (bs.pinnedSquares ^ pinnedPiece) & pinnedRow(bs.kings[!whiteTurn], piece);
        moves |= (bs.pinnedSquares ^ pinnedPiece) & pinnedDiagonal(bs.kings[!whiteTurn], piece);
      }
      if constexpr (p == piece::pieceType::Rook)
      {
        moves = (bs.pinnedSquares ^ pinnedPiece) & pinnedRow(bs.kings[!whiteTurn], piece);
      }
      if constexpr (p == piece::pieceType::Bishop)
      {
        moves = (bs.pinnedSquares ^ pinnedPiece) & pinnedDiagonal(bs.kings[!whiteTurn], piece);
      }
    }
    else
    {
      moves = generatePieceAttacks<p>(board, piece) & nonTeam;
    }
    moves &= bs.blockMask;
    generateMultipleMoves<p>(board, moves, piece, ml);
    movingPieces &= movingPieces - 1;
  }
}
template<bool whiteTurn>
const void MoveGeneration::generatePawnMoves(const BoardState& bs, MoveList& ml)
{
  const uint64_t pawns = bs.pieceBoards[9 - whiteTurn * 5];
  uint64_t promoting = pawns & ranks[1 + whiteTurn * 5];
  uint64_t pinnedPawns = pawns & bs.pinnedSquares;

  // both = 0, white = 1, black = 2
  const uint64_t board = bs.teamBoards[0];
  const uint64_t team = bs.teamBoards[2 - whiteTurn];
  const uint64_t enemy = bs.teamBoards[1 + whiteTurn];
  const uint64_t nonOccupied = ~board;

  // set till att pinnedPawns inte är cooked
  const uint64_t nonPromoting = pawns & ~promoting & ~pinnedPawns;

  const uint64_t startPawns = ranks[5 - 3 * whiteTurn];
  const uint64_t block = bs.blockMask;

  constexpr uint32_t backLeft = 7 - 14 * whiteTurn;
  constexpr uint32_t backRight = 9 - 18 * whiteTurn;

  // Template
  uint64_t push = shiftUp<whiteTurn>(nonPromoting) & nonOccupied;
  uint64_t double_push = shiftUp<whiteTurn>(push & startPawns) & nonOccupied & block;
  push &= block;

  constexpr int back = 8 - whiteTurn * 16;
  unsigned long dest;

  // PUSH
  pawnBitScan(bs, ml, push, back, 0);

  // DOUBLE PUSH
  pawnBitScan(bs, ml, double_push, back * 2, moveModifiers::DPUSH);

  uint64_t pawn_left = nonPromoting & ~(0x8080808080808080ULL - 0x7F7F7F7F7F7F7F7FULL * whiteTurn);
  uint64_t pawn_right = nonPromoting & ~(0x0101010101010101ULL + 0x7F7F7F7F7F7F7F7FULL * whiteTurn);

  // TODO: template
  shiftSide<whiteTurn>(pawn_right, pawn_left);

  pawn_left &= enemy & block;
  pawn_right &= enemy & block;

  // PAWN LEFT
  pawnBitScan(bs, ml, pawn_left, backLeft, moveModifiers::CAPTURE);
  pawnBitScan(bs, ml, pawn_right, backRight, moveModifiers::CAPTURE);

  if (bs.enPassant)
  {
    uint64_t epPos = (1ULL << bs.enPassant);
    epPos = pawnAttacks<!whiteTurn>(epPos);
    epPos &= pawns & block;

    while (epPos)
    {
      _BitScanForward64(&dest, epPos);
      if (!checkPinEP<whiteTurn>(bs, dest))
      {
        ml.add(dest | ((uint16_t)bs.enPassant) << 6 | moveModifiers::pawn | moveModifiers::EN_PESSANT_CAP);
      }
      epPos &= epPos - 1;
    }
  }

  if (promoting)
    promotionHelper<whiteTurn>(bs, ml, promoting);

  if (pinnedPawns)
  {
    uint64_t push = shiftUp<whiteTurn>(pinnedPawns) & nonOccupied & block & bs.pinnedSquares;
    uint64_t double_push = shiftUp<whiteTurn>(push & startPawns) & nonOccupied & block & bs.pinnedSquares;

    uint64_t pawn_left = pinnedPawns & ~files[0];
    uint64_t pawn_right = pinnedPawns & ~files[7];

    // TODO: Template
    shiftSide<whiteTurn>(pawn_right, pawn_left);

    pawn_left &= enemy & block & bs.pinnedSquares;
    pawn_right &= enemy & block & bs.pinnedSquares;

    while (push)
    {
      _BitScanForward64(&dest, push);
      if (((1ULL << (dest + back)) & bs.pinnedSquares && (((1ULL << dest) & bs.pinnedSquares))) || !((1ULL << (dest + back)) & bs.pinnedSquares))
      {
        ml.add((dest + back) | dest << 6 | moveModifiers::pawn);
      }

      push &= push - 1;
    }

    while (double_push)
    {
      _BitScanForward64(&dest, double_push);

      if (((1ULL << (dest + back + back)) & bs.pinnedSquares && (((1ULL << dest) & bs.pinnedSquares))) || !((1ULL << (dest + back + back)) & bs.pinnedSquares))
      {
        ml.add((dest + back + back) | dest << 6 | moveModifiers::pawn | moveModifiers::DPUSH);
      }
      double_push &= double_push - 1;
    }

    while (pawn_right)
    {
      _BitScanForward64(&dest, pawn_right);
      if (((1ULL << (dest + backRight)) & bs.pinnedSquares && (((1ULL << dest) & bs.pinnedSquares))) || !((1ULL << (dest + backRight)) & bs.pinnedSquares))
      {
        ml.add((dest + backRight) | dest << 6 | moveModifiers::pawn | moveModifiers::CAPTURE);
      }
      pawn_right &= pawn_right - 1;
    }

    while (pawn_left)
    {
      _BitScanForward64(&dest, pawn_left);
      if (((1ULL << (dest + backLeft)) & bs.pinnedSquares && (((1ULL << dest) & bs.pinnedSquares))) || !((1ULL << (dest + backLeft)) & bs.pinnedSquares))
      {
        ml.add((dest + backLeft) | dest << 6 | moveModifiers::pawn | moveModifiers::CAPTURE);
      }
      pawn_left &= pawn_left - 1;
    }

    _BitScanForward64(&dest, pinnedPawns);
    pinnedPawns &= pinnedPawns - 1;
  }
}

////////////////////////////////////////////////////////////////
// Rule functions
////////////////////////////////////////////////////////////////

template<bool whiteTurn, bool castlingAllowed, bool enemyCastlingAllowed>
const void MoveGeneration::movePiece(BoardState& bs, uint32_t move)
{
  const uint32_t placePiece = move & moveModifiers::ATTACKERS;

  unsigned long attackerSquare;
  _BitScanForward64(&attackerSquare, placePiece);

  const uint8_t startSquare = move & moveModifiers::FROM;
  const uint64_t startBitboard = 1ULL << startSquare;

  const uint8_t endSquare = (move & moveModifiers::TO) >> 6;
  const uint64_t endBitboard = 1ULL << endSquare;

  // Update bitboards
  if constexpr (whiteTurn)
  {
    bs.teamBoards[1] ^= startBitboard | endBitboard;
  }
  else
  {
    bs.teamBoards[2] ^= startBitboard | endBitboard;
  }

  bs.enPassant = ((endSquare + 8) - (16 * bs.whiteTurn)) * ((move & moveModifiers::DPUSH) != 0);

  // if it's the king
  if (attackerSquare == 12)
  {
    if constexpr (whiteTurn)
    {
      bs.kings[0] = endSquare;
      if constexpr (castlingAllowed)
      {
        // Castling
        if (move & moveModifiers::CASTLE_KING)
        {
          const uint64_t castle = castlingRookStartPos<whiteTurn, true>() | castlingRookEndPos<whiteTurn, true>();
          bs.pieceBoards[1] ^= castle;
          bs.teamBoards[1] ^= castle;
        }
        else if (move & moveModifiers::CASTLE_QUEEN)
        {
          const uint64_t castle = castlingRookStartPos<whiteTurn, false>() | castlingRookEndPos<whiteTurn, false>();
          bs.pieceBoards[1] ^= castle;
          bs.teamBoards[1] ^= castle;
        }
        bs.castlingRights &= ~0b0011;
      }
    }
    else
    {
      bs.kings[1] = endSquare;
      if constexpr (castlingAllowed)
      {
        // Castling
        if (move & moveModifiers::CASTLE_KING)
        {
          const uint64_t castle = castlingRookStartPos<whiteTurn, true>() | castlingRookEndPos<whiteTurn, true>();
          bs.pieceBoards[6] ^= castle;
          bs.teamBoards[2] ^= castle;
        }
        else if (move & moveModifiers::CASTLE_QUEEN)
        {
          const uint64_t castle = castlingRookStartPos<whiteTurn, false>() | castlingRookEndPos<whiteTurn, false>();
          bs.pieceBoards[6] ^= castle;
          bs.teamBoards[2] ^= castle;
        }
        bs.castlingRights &= ~0b1100;
      }
    }
  }
  else
  {
    if constexpr (whiteTurn)
    {
      bs.pieceBoards[attackerSquare - 18] ^= startBitboard | endBitboard;
    }
    else
    {
      bs.pieceBoards[attackerSquare - 13] ^= startBitboard | endBitboard;
    }

    if (move & moveModifiers::PROMO)
    {
      // Undoing the assumed new position
      if constexpr (whiteTurn)
      {
        bs.pieceBoards[attackerSquare - 18] ^= endBitboard;
        unsigned long promo;
        _BitScanForward64(&promo, move & moveModifiers::PROMO);
        bs.pieceBoards[promo - 29] ^= endBitboard;
      }
      else
      {
        bs.pieceBoards[attackerSquare - 13] ^= endBitboard;
        unsigned long promo;
        _BitScanForward64(&promo, move & moveModifiers::PROMO);
        bs.pieceBoards[promo - 24] ^= endBitboard;
      }
    }

    if (move & moveModifiers::EN_PESSANT_CAP)
    {
      if constexpr (whiteTurn)
      {
        const uint64_t epCapBoard = endBitboard >> 8;
        bs.pieceBoards[4] ^= epCapBoard;
        bs.pieceBoards[2] ^= epCapBoard;
      }
      else
      {
        const uint64_t epCapBoard = endBitboard << 8;
        bs.pieceBoards[9] ^= epCapBoard;
        bs.pieceBoards[1] ^= epCapBoard;
      }
    }
    if constexpr (castlingAllowed)
    {
      if (move & moveModifiers::rook)
      {
        if constexpr (whiteTurn)
        {
          if (1ULL & startBitboard)
            bs.castlingRights &= ~0b0010;
          if ((1ULL << 7) & startBitboard)
            bs.castlingRights &= ~0b0001;
        }
        else
        {
          if ((1ULL << 56) & startBitboard)
            bs.castlingRights &= ~0b1000;
          if ((1ULL << 63) & startBitboard)
            bs.castlingRights &= ~0b0100;
        }
      }
    }
  }

  if (move & moveModifiers::CAPTURE)
  {
    // Find which board was attacked
    if constexpr (whiteTurn)
    {
      for (int i = 5; i > 10; i++)
      {
        if (endBitboard & bs.pieceBoards[i])
        {
          bs.pieceBoards[i] ^= endBitboard;
          bs.pieceBoards[2] ^= endBitboard;

          // TODO: if white can castle (will be removed with template)
          if constexpr (enemyCastlingAllowed)
          {
            if (endBitboard & castlingRookStartPos<!whiteTurn, false>())
            {
              // Queen castle
              bs.castlingRights &= ~0b1000;
            }
            else if (endBitboard & castlingRookStartPos<!whiteTurn, true>())
            {
              // King castle
              bs.castlingRights &= ~0b0100;
            }
          }
          break;
        }
      }
    }
    else
    {
      for (int i = 0; i > 5; i++)
      {
        if (endBitboard & bs.pieceBoards[i])
        {
          bs.pieceBoards[i] ^= endBitboard;
          bs.pieceBoards[1] ^= endBitboard;
          // TODO: if white can castle (will be removed with template)
          if constexpr (enemyCastlingAllowed)
          {
            if (endBitboard & castlingRookStartPos<whiteTurn, false>())
            {
              // Queen castle
              bs.castlingRights &= ~0b0010;
            }
            else if (endBitboard & castlingRookStartPos<whiteTurn, true>())
            {
              // King castle
              bs.castlingRights &= ~0b0001;
            }
          }
          break;
        }
      }
    }
  }

  // Update Board
  bs.teamBoards[0] = bs.teamBoards[1] + bs.teamBoards[2];
}

template<bool whiteTurn>
const void MoveGeneration::generateAttacks(BoardState& bs)
{
  uint64_t board = 1ULL << bs.kings[!whiteTurn] ^ bs.teamBoards[0];
  uint64_t attacks = 0;

  unsigned long attacker;

  uint64_t attack_piece = bs.pieceBoards[0 + PIECE_OFFSET * whiteTurn];
  while (attack_piece)
  {
    _BitScanForward64(&attacker, attack_piece);
    attacks |= generatePieceAttacks<pieceType::Queen>(board, attacker);

    attack_piece &= attack_piece - 1;
  }
  //
  // ROOKS
  //
  attack_piece = bs.pieceBoards[1 + PIECE_OFFSET * whiteTurn];
  while (attack_piece)
  {
    _BitScanForward64(&attacker, attack_piece);
    attacks |= generatePieceAttacks<pieceType::Rook>(board, attacker);
    attack_piece &= attack_piece - 1;
  }
  //
  // BISHOPS
  //
  attack_piece = bs.pieceBoards[2 + PIECE_OFFSET * whiteTurn];
  while (attack_piece)
  {
    _BitScanForward64(&attacker, attack_piece);
    attacks |= generatePieceAttacks<pieceType::Bishop>(board, attacker);
    attack_piece &= attack_piece - 1;
  }

  //
  // KNIGHTS
  //
  attack_piece = bs.pieceBoards[3 + PIECE_OFFSET * whiteTurn];
  while (attack_piece)
  {
    _BitScanForward64(&attacker, attack_piece);
    attacks |= generatePieceAttacks<pieceType::Knight>(board, attacker);
    attack_piece &= attack_piece - 1;
  }

  //
  // PAWNS
  //
  attack_piece = bs.pieceBoards[4 + PIECE_OFFSET * whiteTurn];
  // Don't know why this is the other way around?!
  uint64_t pawn_left = attack_piece & ~(0x8080808080808080ULL - 0x7F7F7F7F7F7F7F7FULL * whiteTurn);
  uint64_t pawn_right = attack_piece & ~(0x0101010101010101ULL + 0x7F7F7F7F7F7F7F7FULL * whiteTurn);

  shiftSide<!whiteTurn>(pawn_left, pawn_right);

  attacks |= pawn_left;
  attacks |= pawn_right;

  //
  // KING
  //
  attacks |= m_kingAttacks[bs.kings[whiteTurn]];

  // Add to attacks
  bs.attacks = attacks;
}

template<bool whiteTurn>
const void MoveGeneration::generatePinsBlocks(BoardState& bs)
{
  const uint8_t king = bs.kings[!whiteTurn];
  const uint64_t king_bitboard = (1ULL << king);

  uint8_t checks = 0;
  uint64_t block_mask = 0;
  uint64_t pin_mask = 0;

  const uint64_t x_ray_diagonal = main_diagonals[(7 - (king >> 3) + (king & 7))] | anti_diagonals[(king >> 3) + (king & 7)];
  const uint64_t x_ray_straigt = ranks[king >> 3] | files[king & 7];

  uint64_t diagonal_threats = (bs.pieceBoards[0 + PIECE_OFFSET * whiteTurn] | bs.pieceBoards[2 + PIECE_OFFSET * whiteTurn]) & x_ray_diagonal;
  uint64_t straight_threats = (bs.pieceBoards[0 + PIECE_OFFSET * whiteTurn] | bs.pieceBoards[1 + PIECE_OFFSET * whiteTurn]) & x_ray_straigt;

  const uint64_t pawns = pawnAttacks<whiteTurn>(king_bitboard) & GETENEMYPIECES(4);
  const uint64_t knights = m_kingAttacks[king] & GETENEMYPIECES(3);

  if (pawns)
  {
    checks++;
    block_mask |= pawns;
  }
  if (knights)
  {
    checks++;
    block_mask |= knights;
  }
  unsigned long threat;

  // TODO: Create a function!
  while (diagonal_threats)
  {
    _BitScanForward64(&threat, diagonal_threats);
    uint64_t threat_bitboard = 1ULL << threat;
    int way;

    if (king < threat)
    {
      way = ((king & 7) < (threat & 7)) * 2 + 7;
    }
    else
    {
      way = ((king & 7) > (threat & 7)) * (-2) - 7;
    }
    uint64_t temp_pin_mask = 0;
    uint8_t pieces = 0;
    for (int i = 1; i < 7; i++)
    {
      uint64_t pos = 1ULL << (king + way * i);
      temp_pin_mask |= pos;

      if (pos & threat_bitboard)
      {
        if (!pieces)
        {
          checks++;
          block_mask |= temp_pin_mask;
          break;
        }
        else if (pieces == 1)
        {
          pin_mask |= temp_pin_mask;
          break;
        }
      }
      else if (pos & bs.teamBoards[2 - whiteTurn])
      {
        pieces++;
      }
      else if (pos & bs.teamBoards[1 + whiteTurn])
      {
        break;
      }
    }
    diagonal_threats &= diagonal_threats - 1;
  }

  while (straight_threats)
  {
    _BitScanForward64(&threat, straight_threats);
    uint64_t threat_bitboard = 1ULL << threat;
    int way;

    if (king < threat)
    {
      way = ((king & 7) == (threat & 7)) * 7 + 1;
    }
    else
    {
      way = ((king & 7) == (threat & 7)) * (-7) - 1;
    }
    uint64_t temp_pin_mask = 0;
    uint8_t pieces = 0;
    for (int i = 1; i < 7; i++)
    {
      uint64_t pos = 1ULL << (king + way * i);
      temp_pin_mask |= pos;

      if (pos & threat_bitboard)
      {
        if (!pieces)
        {
          checks++;
          block_mask |= temp_pin_mask;
          break;
        }
        else if (pieces == 1)
        {
          pin_mask |= temp_pin_mask;
        }
      }
      else if (pos & bs.teamBoards[2 - whiteTurn])
      {
        pieces++;
      }
      else if (pos & bs.teamBoards[1 + whiteTurn])
      {
        break;
      }
    }
    straight_threats &= straight_threats - 1;
  }

  bs.pinnedSquares = pin_mask;

  if (block_mask)
  {
    bs.blockMask = block_mask;
  }
  else
  {
    bs.blockMask = ~0;
  }
  bs.numCheckers = checks;
}

template<bool whiteTurn>
const void MoveGeneration::generateCastlingOptions(BoardState& bs)
{
  const uint64_t board = bs.teamBoards[0];
  constexpr uint64_t castleKingAttacks = whiteTurn ? 0b1100000ULL : (0b1100000ULL << 56);
  constexpr uint64_t castleQueenAttacks = whiteTurn ? 0b1100ULL : (0b1100ULL << 56);
  constexpr uint64_t castleQueenMoveSquares = whiteTurn ? 0b1110ULL : (0b1110ULL << 56);

  uint8_t castling = bs.castlingRights & (0b1100 - 0b1001 * whiteTurn);
  if ((castling & 0b0101) && ((castleKingAttacks & (board | bs.attacks)) == 0))
  {
    bs.castlingRights |= CASTELING_K_CHECK;
  }
  if ((castling & 0b1010) && (((castleQueenAttacks & bs.attacks) == 0) && ((castleQueenMoveSquares & board) == 0)))
  {
    bs.castlingRights |= CASTELING_Q_CHECK;
  }
}
////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////

template<bool whiteTurn>
const void MoveGeneration::promotionHelper(const BoardState& bs, MoveList& ml, uint32_t promoting)
{
  unsigned long dest;
  while (promoting)
  {
    _BitScanForward64(&dest, promoting);

    const uint64_t startBitboard = (1ULL << dest);

    uint16_t promoPush = dest + (16 * whiteTurn - 8);
    const uint64_t promoPushBitboard = (1ULL << promoPush);
    promoPush <<= 6;

    uint64_t promo_attacks = pawnAttacks<whiteTurn>(startBitboard) & bs.teamBoards[1 + whiteTurn] & bs.blockMask;

    if (promoPushBitboard & ~bs.teamBoards[0] & bs.blockMask)
    {
      if ((startBitboard & bs.pinnedSquares) && (promoPushBitboard & bs.pinnedSquares) || !(startBitboard & bs.pinnedSquares))
      {
        const uint32_t combo = dest | promoPush | moveModifiers::pawn;
        ml.add(combo | moveModifiers::PROMO_QUEEN);
        ml.add(combo | moveModifiers::PROMO_ROOK);
        ml.add(combo | moveModifiers::PROMO_BISHOP);
        ml.add(combo | moveModifiers::PROMO_KNIGHT);
      }
    }

    unsigned long attack_square;
    while (promo_attacks)
    {
      _BitScanForward64(&attack_square, promo_attacks);
      uint64_t attack_square_bit_board = 1ULL << attack_square;

      attack_square <<= 6;

      if ((startBitboard & bs.pinnedSquares) && (attack_square_bit_board & bs.pinnedSquares) || !(startBitboard & bs.pinnedSquares))
      {
        const uint32_t combo = dest | attack_square | moveModifiers::pawn | moveModifiers::CAPTURE;
        ml.add(combo | moveModifiers::PROMO_QUEEN);
        ml.add(combo | moveModifiers::PROMO_ROOK);
        ml.add(combo | moveModifiers::PROMO_BISHOP);
        ml.add(combo | moveModifiers::PROMO_KNIGHT);
      }
      promo_attacks &= promo_attacks - 1;
    }
    promoting &= promoting - 1;
  }
}

uint64_t MoveGeneration::pinnedRow(uint8_t king, uint8_t piece)
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

uint64_t MoveGeneration::pinnedDiagonal(uint8_t king, uint8_t piece)
{
  int diagonal = (king >> 3) + (king & 7);
  if (diagonal == ((piece >> 3) + (piece & 7)))
    return anti_diagonals[diagonal];

  diagonal = 7 - (king >> 3) + (king & 7);
  if (diagonal == (7 - (piece >> 3) + (piece & 7)))
    return main_diagonals[(7 - (king >> 3) + (king & 7))];

  return 0;
}

template<bool whiteTurn>
uint64_t MoveGeneration::pawnAttacks(uint64_t pawns)
{
  if constexpr (whiteTurn)
  {
    return ((pawns & ~files[7]) << 9) | ((pawns & ~files[0]) << 7);
  }
  else
  {
    return ((pawns & ~files[0]) >> 9) | ((pawns & ~files[7]) >> 7);
  }
}

template<piece::pieceType p>
void MoveGeneration::generateMultipleMoves(uint64_t board, uint64_t moves, uint32_t position, MoveList& ml)
{
  uint32_t type;
  if constexpr (p == piece::pieceType::Queen)
  {
    type = static_cast<uint32_t>(moveModifiers::queen);
  }
  if constexpr (p == piece::pieceType::Rook)
  {
    type = static_cast<uint32_t>(moveModifiers::rook);
  }
  if constexpr (p == piece::pieceType::Bishop)
  {
    type = static_cast<uint32_t>(moveModifiers::bishop);
  }
  if constexpr (p == piece::pieceType::Knight)
  {
    type = static_cast<uint32_t>(moveModifiers::knight);
  }

  unsigned long dest;
  while (moves)
  {
    _BitScanForward64(&dest, moves);
    ml.add(position | (dest << 6) | type | isCapture(board, dest));
    moves &= moves - 1;
  }
}

inline uint32_t MoveGeneration::isCapture(uint64_t board, unsigned long dest)
{
  return (piece::moveModifiers::CAPTURE * ((board & (1ULL << dest)) != 0));
}

template<bool whiteTurn>
inline uint64_t MoveGeneration::shiftUp(uint64_t pawns)
{
  if constexpr (whiteTurn)
  {
    return pawns << 8;
  }
  else
  {
    return pawns >> 8;
  }
}

template<bool whiteTurn>
inline void MoveGeneration::shiftSide(uint64_t& right, uint64_t& left)
{
  if constexpr (whiteTurn)
  {
    right = (right << 9);
    left = (left << 7);
  }
  else
  {
    right = (right >> 9);
    left = (left >> 7);
  }
}

void MoveGeneration::pawnBitScan(const BoardState& bs, MoveList& ml, uint64_t& pawns, const int& dir, const uint32_t& move_type)
{
  unsigned long dest;
  while (pawns)
  {
    _BitScanForward64(&dest, pawns);
    ml.add((dest + dir) | dest << 6 | moveModifiers::pawn | move_type);
    pawns &= pawns - 1;
  }
}

template<bool whiteTurn>
const bool MoveGeneration::checkPinEP(const BoardState& bs, uint32_t start_pos)
{
  uint8_t king = bs.kings[!whiteTurn];
  uint64_t rank = ranks[king >> 3];
  uint64_t startPosBB = (1ULL << start_pos);

  const uint8_t offset = whiteTurn * PIECE_OFFSET;

  if (startPosBB & rank)
  {
    // Queens and Rooks
    uint64_t sliders = (bs.pieceBoards[offset] | bs.pieceBoards[1 + offset]) & rank;

    if (sliders)
    {
      const int back = 8 - whiteTurn * 16;
      uint64_t board = bs.teamBoards[0] & ~startPosBB & ~(1ULL << (bs.enPassant + back));
      uint64_t attacks = 0;

      unsigned long slidePos;
      while (sliders)
      {
        _BitScanForward64(&slidePos, sliders);

        attacks |= rankAttacks(board, slidePos);
        sliders &= sliders - 1;
      }
      return attacks & (1ULL << king);
    }
    return 0;
  }
  return 0;
}

template<bool white, bool king>
constexpr uint64_t MoveGeneration::castlingRookStartPos()
{
  if constexpr (white)
  {
    return king ? 0b10000000ULL : 0b1ULL;
  }
  else
  {
    return king ? 0b10000000ULL << 56 : 1ULL << 56;
  }
}
template<bool white, bool king>
constexpr uint64_t MoveGeneration::castlingRookEndPos()
{
  if constexpr (white)
  {
    return king ? 0b100000ULL : 0b1000ULL;
  }
  else
  {
    return king ? 0b100000ULL << 56 : 0b1000ULL << 56;
  }
}

}  // namespace piece
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

// Done
template<piece::pieceType p>
inline uint64_t MoveGeneration::generatePieceAttacks(uint64_t board, uint32_t position)
{
  if constexpr (p == piece::pieceType::Queen)
  {
    return maindiagonalAttacks(board, position) | antidiagonalAttacks(board, position) | fileAttacks(board, position) | rankAttacks(board, position);
  }
  if constexpr (p == piece::pieceType::Rook)
  {
    return fileAttacks(board, position) | rankAttacks(board, position);
  }
  if constexpr (p == piece::pieceType::Bishop)
  {
    return maindiagonalAttacks(board, position) | antidiagonalAttacks(board, position);
  }
  if constexpr (p == piece::pieceType::Knight)
  {
    return m_knightAttacks[position];
  }
}

////////////////////////////////////////////////////////////////
// Moves
////////////////////////////////////////////////////////////////

template<bool whiteTurn>
void MoveGeneration::generateKingMoves(const BoardState& bs, MoveList& ml)
{
  // both = 0, white = 1, black = 2
  const uint64_t board = bs.teamBoards[0];
  const uint64_t team = bs.teamBoards[2 - whiteTurn];

  const uint64_t not_attacked = ~bs.attacks;
  const uint8_t k_pos = bs.kings[!whiteTurn];

  if (bs.castlingRights & CASTELING_K_CHECK)
  {
    constexpr uint8_t castleMoveK((62 - 56 * whiteTurn) << 6);

    ml.add(k_pos | castleMoveK | moveModifiers::king | moveModifiers::CASTLE_KING);
  }
  if (bs.castlingRights & CASTELING_Q_CHECK)
  {
    constexpr uint8_t castleMoveQ((58 - 56 * whiteTurn) << 6);
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
void MoveGeneration::generatePieceMoves(const piece::BoardState& bs, piece::MoveList& ml)
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
void MoveGeneration::generatePawnMoves(const BoardState& bs, MoveList& ml)
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
  uint64_t push = shift_up<whiteTurn>(nonPromoting, whiteTurn) & nonOccupied;
  uint64_t double_push = shift_up<whiteTurn>(push & startPawns, whiteTurn) & nonOccupied & block;
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
  shift_side(pawn_right, pawn_left, whiteTurn);

  pawn_left &= enemy & block;
  pawn_right &= enemy & block;

  // PAWN LEFT
  pawnBitScan(bs, ml, pawn_left, backLeft, moveModifiers::CAPTURE);
  pawnBitScan(bs, ml, pawn_right, backRight, moveModifiers::CAPTURE);

  if (bs.enPassant)
  {
    uint64_t epPos = (1ULL << bs.enPassant);
    epPos = pawn_attacks(epPos, !whiteTurn);
    epPos &= pawns & block;

    while (epPos)
    {
      _BitScanForward64(&dest, epPos);
      if (!check_pin_EP(bs, dest))
      {
        ml.add(dest | ((uint16_t)bs.enPassant) << 6 | moveModifiers::pawn | moveModifiers::EN_PESSANT_CAP);
      }
      epPos &= epPos - 1;
    }
  }

  while (promoting)
  {
    _BitScanForward64(&dest, promoting);
    promoting &= promoting - 1;

    const uint64_t start_bitboard = (1ULL << dest);

    uint16_t promo_push = dest + (16 * whiteTurn - 8);
    const uint64_t promo_push_bitboard = (1ULL << promo_push);
    promo_push <<= 6;

    // TODO: template
    uint64_t promo_attacks = pawn_attacks(start_bitboard, whiteTurn) & enemy & block;

    // Lägg till alla sorters promotion
    if (promo_push_bitboard & nonOccupied & block)
    {
      if ((start_bitboard & bs.pinnedSquares) && (promo_push_bitboard & bs.pinnedSquares) || !(start_bitboard & bs.pinnedSquares))
      {
        ml.add(dest | promo_push | moveModifiers::pawn | moveModifiers::PROMO_QUEEN);
        ml.add(dest | promo_push | moveModifiers::pawn | moveModifiers::PROMO_ROOK);
        ml.add(dest | promo_push | moveModifiers::pawn | moveModifiers::PROMO_BISHOP);
        ml.add(dest | promo_push | moveModifiers::pawn | moveModifiers::PROMO_KNIGHT);
      }
    }

    unsigned long attack_square;
    while (promo_attacks)
    {
      _BitScanForward64(&attack_square, promo_attacks);
      uint64_t attack_square_bit_board = 1ULL << attack_square;

      attack_square <<= 6;

      if ((start_bitboard & bs.pinnedSquares) && (attack_square_bit_board & bs.pinnedSquares) || !(start_bitboard & bs.pinnedSquares))
      {
        ml.add(dest | attack_square | moveModifiers::pawn | moveModifiers::PROMO_QUEEN | moveModifiers::CAPTURE);
        ml.add(dest | attack_square | moveModifiers::pawn | moveModifiers::PROMO_ROOK | moveModifiers::CAPTURE);
        ml.add(dest | attack_square | moveModifiers::pawn | moveModifiers::PROMO_BISHOP | moveModifiers::CAPTURE);
        ml.add(dest | attack_square | moveModifiers::pawn | moveModifiers::PROMO_KNIGHT | moveModifiers::CAPTURE);
      }
      promo_attacks &= promo_attacks - 1;
    }
  }

  if (pinnedPawns)
  {
    uint64_t push = shift_up<whiteTurn>(pinnedPawns) & nonOccupied & block & bs.pinnedSquares;
    uint64_t double_push = shift_up<whiteTurn>(push & startPawns) & nonOccupied & block & bs.pinnedSquares;

    uint64_t pawn_left = pinnedPawns & ~files[0];
    uint64_t pawn_right = pinnedPawns & ~files[7];

    // TODO: Template
    shift_side(pawn_right, pawn_left, whiteTurn);

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
// Helper functions
////////////////////////////////////////////////////////////////

uint64_t MoveGeneration::pinnedRow(uint8_t king, uint8_t piece)
{
  // On the same rank
  if ((king >> 3) == (piece >> 3))
    return ranks[king >> 3];

  // On the same file
  if ((king & 7) == (piece & 7))
    return files[king & 7];
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

uint64_t MoveGeneration::maindiagonalAttacks(uint64_t board, int position)
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

uint64_t MoveGeneration::antidiagonalAttacks(uint64_t board, int position)
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
inline uint64_t MoveGeneration::shift_up(uint64_t pawns)
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

}  // namespace piece
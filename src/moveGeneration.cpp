
#include "../inc/moveGeneration.h"
#include "../inc/gui.h"
#include <intrin.h>

namespace piece
{

MoveGeneration::MoveGeneration()
{
  initKingAttacks();
  initKnightAttacks();
  initPawnAttacks();
}

////////////////////////////////////////////////////////////////
// Generation
////////////////////////////////////////////////////////////////

void MoveGeneration::startUp(BoardState& bs, MoveList& ml)
{
  if (bs.whiteTurn)
  {
    generatePinsBlocks<true>(bs);
    generateAttacks<true>(bs);
  }
  else
  {
    generatePinsBlocks<false>(bs);
    generateAttacks<false>(bs);
  }

  if (!bs.numCheckers)
  {
    if (bs.whiteTurn)
      generateCastlingOptions<true>(bs);
    else
      generateCastlingOptions<false>(bs);
  }
  generatePossibleMoves(bs, ml);
}

void MoveGeneration::generatePossibleMoves(const BoardState& bs, MoveList& ml)
{
  if (bs.whiteTurn)
  {
    // White moves
    generateKingMoves<true>(bs, ml);
    if (bs.numCheckers > 1)  // Only king moves are legal
      return;

    generatePieceMoves<true, piece::pieceType::Queen>(bs, ml);
    generatePieceMoves<true, piece::pieceType::Rook>(bs, ml);
    generatePieceMoves<true, piece::pieceType::Bishop>(bs, ml);
    generatePieceMoves<true, piece::pieceType::Knight>(bs, ml);
    if (bs.pieceBoards[4] & bs.pinnedSquares)
    {
      generatePawnMoves<true, true>(bs, ml);
    }
    else
    {
      generatePawnMoves<true, false>(bs, ml);
    }
  }
  else
  {
    // Black moves
    generateKingMoves<false>(bs, ml);
    if (bs.numCheckers > 1)  // Only king moves are legal
      return;
    generatePieceMoves<false, piece::pieceType::Queen>(bs, ml);
    generatePieceMoves<false, piece::pieceType::Rook>(bs, ml);
    generatePieceMoves<false, piece::pieceType::Bishop>(bs, ml);
    generatePieceMoves<false, piece::pieceType::Knight>(bs, ml);
    if (bs.pieceBoards[9] & bs.pinnedSquares)
    {
      generatePawnMoves<false, true>(bs, ml);
    }
    else
    {
      generatePawnMoves<false, false>(bs, ml);
    }
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
      case 0b11:
        movePiece<true, true, false>(bs, move);
        break;
      case 0b100:
      case 0b1000:
      case 0b1100:
        movePiece<true, false, true>(bs, move);
        break;
      default:
        movePiece<true, true, true>(bs, move);
        break;
    }
    bs.whiteTurn ^= 1;
    generateAttacks<false>(bs);
    generatePinsBlocks<false>(bs);

    bs.castlingRights &= 0b1111;
    if (!bs.numCheckers && bs.castlingRights & 0b1100)
      generateCastlingOptions<false>(bs);
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
    generateAttacks<true>(bs);
    generatePinsBlocks<true>(bs);

    bs.castlingRights &= 0b1111;

    if (!bs.numCheckers && bs.castlingRights & 0b11)
      generateCastlingOptions<true>(bs);
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
    uint64_t attack = (knight >> 10) & ~(files[6] | files[7] |
                                         ranks[7]);  // Two files to left and up
    attack |= (knight << 6) &
              ~(files[6] | files[7] | ranks[0]);  // Two files to left and down
    attack |= (knight >> 17) &
              ~(files[7] | ranks[6] | ranks[7]);  // One file to left and up
    attack |= (knight << 15) &
              ~(files[7] | ranks[0] | ranks[1]);  // One file to left and down
    attack |= (knight >> 15) &
              ~(files[0] | ranks[6] | ranks[7]);  // One file to right and up
    attack |= (knight << 17) &
              ~(files[0] | ranks[0] | ranks[1]);  // One file to right and down
    attack |= (knight >> 6) &
              ~(files[0] | files[1] | ranks[7]);  // Two files to right and up
    attack |= (knight << 10) &
              ~(files[0] | files[1] | ranks[0]);  // Two files to right and down
    m_knightAttacks[i] = attack;
  }
}

void MoveGeneration::initPawnAttacks()
{
  constexpr uint64_t file1 = ~0x0101010101010101ULL;
  constexpr uint64_t file8 = ~0x8080808080808080ULL;

  for (int i = 0; i < 64; i++)
  {
    uint64_t pawn = 1ULL << (i);
    uint64_t attack = 0;
    // Right
    attack |= (pawn & file8) << 9;
    // Left
    attack |= (pawn & file1) << 7;
    m_pawnAttacks[0][i] = attack;
    attack = 0;
    // Right
    attack |= (pawn & file1) >> 9;
    // Left
    attack |= (pawn & file8) >> 7;
    m_pawnAttacks[1][i] = attack;
  }
}

////////////////////////////////////////////////////////////////
// Attacks
////////////////////////////////////////////////////////////////

template<piece::pieceType p>
const inline uint64_t MoveGeneration::generatePieceAttacks(uint64_t board, uint32_t position)
{
  if constexpr (p == piece::pieceType::Queen)
  {
    return getBishopAttacks(position, board) | getRookAttacks(position, board);
  }
  if constexpr (p == piece::pieceType::Rook)
  {
    return getRookAttacks(position, board);
  }
  if constexpr (p == piece::pieceType::Bishop)
  {
    return getBishopAttacks(position, board);
  }
  if constexpr (p == piece::pieceType::Knight)
  {
    return m_knightAttacks[position];
  }
}

const inline uint64_t MoveGeneration::getBishopAttacks(uint8_t square, uint64_t occupancy)
{
  occupancy &= m_bishopMasks[square];
  occupancy *= m_bishopMagicBitboard[square];
  occupancy >>= (64 - m_occupacyCountBishop[square]);
  return m_bishopAttacks[square][occupancy];
}

const inline uint64_t MoveGeneration::getRookAttacks(uint8_t square, uint64_t occupancy)
{
  occupancy &= m_rookMasks[square];
  occupancy *= m_rookMagicBitboard[square];
  occupancy >>= (64 - m_occupacyCountRook[square]);
  return m_rookAttacks[square][occupancy];
}

const uint64_t inline MoveGeneration::singlePawnAttack(uint8_t position, bool white)
{
  return m_pawnAttacks[white][position];
}

template<bool whiteTurn>
const void MoveGeneration::generatePawnAttacks(const BoardState& bs, uint64_t& attacks)
{
  const uint64_t attack_piece = bs.pieceBoards[4 + PIECE_OFFSET * whiteTurn];

  uint64_t pawn_left = attack_piece & ~(0x8080808080808080ULL - 0x7F7F7F7F7F7F7F7FULL * whiteTurn);
  uint64_t pawn_right = attack_piece & ~(0x0101010101010101ULL + 0x7F7F7F7F7F7F7F7FULL * whiteTurn);

  shiftSide<!whiteTurn>(pawn_left, pawn_right);

  attacks |= pawn_left | pawn_right;
}

////////////////////////////////////////////////////////////////
// Move generation
////////////////////////////////////////////////////////////////

template<bool whiteTurn>
const void MoveGeneration::generateKingMoves(const BoardState& bs,
                                             MoveList& ml)
{
  // both = 0, white = 1, black = 2
  const uint64_t board = bs.teamBoards[0];
  const uint64_t team = bs.teamBoards[2 - whiteTurn];

  const uint64_t not_attacked = ~bs.attacks;
  const uint8_t k_pos = bs.kings[!whiteTurn];

  if (bs.castlingRights & CASTELING_K_CHECK)
  {
    constexpr uint16_t castleMoveK =
        static_cast<uint16_t>((62 - 56 * whiteTurn) << 6);
    ml.add(k_pos | castleMoveK | moveModifiers::king |
           moveModifiers::CASTLE_KING);
  }
  if (bs.castlingRights & CASTELING_Q_CHECK)
  {
    constexpr uint16_t castleMoveQ =
        static_cast<uint16_t>((58 - 56 * whiteTurn) << 6);
    ml.add(k_pos | castleMoveQ | moveModifiers::king |
           moveModifiers::CASTLE_QUEEN);
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
const void MoveGeneration::generatePieceMoves(const piece::BoardState& bs,
                                              piece::MoveList& ml)
{
  // both = 0, white = 1, black = 2
  const uint64_t board = bs.teamBoards[0];
  const uint64_t team = bs.teamBoards[2 - whiteTurn];
  const uint64_t nonTeam = ~team;

  // 5 = pieces offset
  constexpr uint8_t pieceIndex = static_cast<uint8_t>(p) + !whiteTurn * 5;
  uint64_t movingPieces = bs.pieceBoards[pieceIndex];

  unsigned long piece;

  while (movingPieces)
  {
    uint64_t moves = 0;
    _BitScanForward64(&piece, movingPieces);
    uint64_t pinnedPiece = (1ULL << piece);
    if (pinnedPiece & bs.pinnedSquares)
    {
      if constexpr (p == piece::pieceType::Queen)
      {
        moves = (bs.pinnedSquares ^ pinnedPiece) & pinnedStraight(bs.kings[!whiteTurn], piece);
        moves |= (bs.pinnedSquares ^ pinnedPiece) & pinnedDiagonal(bs.kings[!whiteTurn], piece);
      }
      if constexpr (p == piece::pieceType::Rook)
      {
        moves = (bs.pinnedSquares ^ pinnedPiece) & pinnedStraight(bs.kings[!whiteTurn], piece);
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

template<bool whiteTurn, bool pins>
const void MoveGeneration::generatePawnMoves(const BoardState& bs, MoveList& ml)
{
  constexpr PawnMoves pm = whiteTurn ? whiteMoves : blackMoves;
  const uint64_t pawns = whiteTurn ? bs.pieceBoards[whitePawns] : bs.pieceBoards[blackPawns];

  const uint64_t board = bs.teamBoards[0];
  const uint64_t enemy = whiteTurn ? bs.teamBoards[2] : bs.teamBoards[1];

  const uint64_t nonOccupied = ~board;
  uint64_t promoting = pawns & (whiteTurn ? rank7 : rank2);
  constexpr uint64_t startPawns = whiteTurn ? rank3 : rank6;

  const uint64_t nonPromoting = pawns & ~promoting;
  const uint64_t block = bs.blockMask;

  uint64_t push = shiftUp<whiteTurn>(nonPromoting) & nonOccupied;
  uint64_t double_push = shiftUp<whiteTurn>(push & startPawns) & nonOccupied & block;
  push &= block;

  // PUSH
  pawnBitScan<pins>(bs, ml, push, pm.back, 0);

  // DOUBLE PUSH
  pawnBitScan<pins>(bs, ml, double_push, pm.backx2, moveModifiers::DPUSH);

  uint64_t pawnLeft = nonPromoting & ~(0x8080808080808080ULL - 0x7F7F7F7F7F7F7F7FULL * whiteTurn);
  uint64_t pawnRight = nonPromoting & ~(0x0101010101010101ULL + 0x7F7F7F7F7F7F7F7FULL * whiteTurn);

  // Shifting attacks
  shiftSide<whiteTurn>(pawnRight, pawnLeft);

  // CONFUSING NAMES?!
  //  Making sure the pawn is capturing and/or blocking when attacking
  pawnLeft &= enemy & block;
  pawnRight &= enemy & block;

  // PAWN RIGHT
  pawnBitScan<pins>(bs, ml, pawnLeft, pm.backLeft, moveModifiers::CAPTURE);
  // PAWN LEFT
  pawnBitScan<pins>(bs, ml, pawnRight, pm.backRight, moveModifiers::CAPTURE);

  if (bs.enPassant)
  {
    if (uint64_t epPosBitboard = (1ULL << bs.enPassant) & (whiteTurn ? (block << 8) : (block >> 8)))
    {
      enPassantHelper<whiteTurn>(bs, ml, epPosBitboard);
    }
  }

  if (promoting)
  {
    promotionHelper<whiteTurn>(bs, ml, promoting);
  }
}

////////////////////////////////////////////////////////////////
// Rule functions
////////////////////////////////////////////////////////////////

template<bool whiteTurn, bool castlingAllowed, bool enemyCastlingAllowed>
const void MoveGeneration::movePiece(BoardState& bs, uint32_t move)
{
  const uint32_t placePiece = move & moveModifiers::ATTACKERS;

  // shit code here ->
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

  bs.enPassant = ((endSquare + 8) - (16 * bs.whiteTurn)) *
                 ((move & moveModifiers::DPUSH) != 0);

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
          const uint64_t castle = castlingRookStartPos<whiteTurn, true>() |
                                  castlingRookEndPos<whiteTurn, true>();
          bs.pieceBoards[1] ^= castle;
          bs.teamBoards[1] ^= castle;
        }
        else if (move & moveModifiers::CASTLE_QUEEN)
        {
          const uint64_t castle = castlingRookStartPos<whiteTurn, false>() |
                                  castlingRookEndPos<whiteTurn, false>();
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
          const uint64_t castle = castlingRookStartPos<whiteTurn, true>() |
                                  castlingRookEndPos<whiteTurn, true>();
          bs.pieceBoards[6] ^= castle;
          bs.teamBoards[2] ^= castle;
        }
        else if (move & moveModifiers::CASTLE_QUEEN)
        {
          const uint64_t castle = castlingRookStartPos<whiteTurn, false>() |
                                  castlingRookEndPos<whiteTurn, false>();
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
      bs.pieceBoards[attackerSquare - 13] ^= startBitboard | endBitboard;
    }
    else
    {
      bs.pieceBoards[attackerSquare - 8] ^= startBitboard | endBitboard;
    }

    if (move & moveModifiers::PROMO)
    {
      // Undoing the assumed new position
      if constexpr (whiteTurn)
      {
        bs.pieceBoards[attackerSquare - 13] ^= endBitboard;
        unsigned long promo;
        _BitScanForward64(&promo, move & moveModifiers::PROMO);
        bs.pieceBoards[promo - 24] ^= endBitboard;
      }
      else
      {
        bs.pieceBoards[attackerSquare - 8] ^= endBitboard;
        unsigned long promo;
        _BitScanForward64(&promo, move & moveModifiers::PROMO);
        bs.pieceBoards[promo - 19] ^= endBitboard;
      }
    }

    if (move & moveModifiers::EN_PESSANT_CAP)
    {
      if constexpr (whiteTurn)
      {
        const uint64_t epCapBoard = endBitboard >> 8;
        bs.pieceBoards[9] ^= epCapBoard;
        bs.teamBoards[2] ^= epCapBoard;
      }
      else
      {
        const uint64_t epCapBoard = endBitboard << 8;
        bs.pieceBoards[4] ^= epCapBoard;
        bs.teamBoards[1] ^= epCapBoard;
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
      for (int i = 5; i < 10; i++)
      {
        if (endBitboard & bs.pieceBoards[i])
        {
          bs.pieceBoards[i] ^= endBitboard;
          bs.teamBoards[2] ^= endBitboard;

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
      for (int i = 0; i < 5; i++)
      {
        if (endBitboard & bs.pieceBoards[i])
        {
          bs.pieceBoards[i] ^= endBitboard;
          bs.teamBoards[1] ^= endBitboard;

          if constexpr (enemyCastlingAllowed)
          {
            if (endBitboard & castlingRookStartPos<!whiteTurn, false>())
            {
              // Queen castle
              bs.castlingRights &= ~0b0010;
            }
            else if (endBitboard & castlingRookStartPos<!whiteTurn, true>())
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

  // Should not be here
  if (bs.teamBoards[1] == bs.teamBoards[2])
    throw std::invalid_argument("bs.teamBoards[1] == bs.teamBoards[2]");

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
  generatePawnAttacks<whiteTurn>(bs, attacks);

  //
  // KING
  //
  attacks |= m_kingAttacks[bs.kings[whiteTurn]];

  // Add to attacks
  bs.attacks = attacks;
}

// Kolla check, om det står något ivägen, xora bort och testa igen!
template<bool whiteTurn>
const void MoveGeneration::generatePinsBlocks(BoardState& bs)
{
  const uint8_t king = bs.kings[!whiteTurn];
  // const uint64_t king_bitboard = (1ULL << king);

  uint8_t checks = 0;
  uint64_t block_mask = 0;
  uint64_t pin_mask = 0;

  const uint64_t x_ray_diagonal =
      main_diagonals[(7 - (king >> 3) + (king & 7))] |
      anti_diagonals[(king >> 3) + (king & 7)];
  const uint64_t x_ray_straigt = ranks[king >> 3] | files[king & 7];

  uint64_t diagonal_threats = (bs.pieceBoards[0 + PIECE_OFFSET * whiteTurn] |
                               bs.pieceBoards[2 + PIECE_OFFSET * whiteTurn]) &
                              x_ray_diagonal;
  uint64_t straight_threats = (bs.pieceBoards[0 + PIECE_OFFSET * whiteTurn] |
                               bs.pieceBoards[1 + PIECE_OFFSET * whiteTurn]) &
                              x_ray_straigt;

  const uint64_t pawns = singlePawnAttack(king, !whiteTurn) & bs.pieceBoards[4 + PIECE_OFFSET * whiteTurn];
  const uint64_t knights = m_knightAttacks[king] & bs.pieceBoards[3 + PIECE_OFFSET * whiteTurn];

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
    for (int i = 1; i < 8; i++)
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
  constexpr uint64_t castleKingAttacks =
      whiteTurn ? 0b1100000ULL : (0b1100000ULL << 56);
  constexpr uint64_t castleQueenAttacks =
      whiteTurn ? 0b1100ULL : (0b1100ULL << 56);
  constexpr uint64_t castleQueenMoveSquares =
      whiteTurn ? 0b1110ULL : (0b1110ULL << 56);

  uint8_t castling = bs.castlingRights & (0b1100 - 0b1001 * whiteTurn);
  if ((castling & 0b0101) &&
      ((castleKingAttacks & (board | bs.attacks)) == 0))
  {
    bs.castlingRights |= CASTELING_K_CHECK;
  }
  if ((castling & 0b1010) && (((castleQueenAttacks & bs.attacks) == 0) &&
                              ((castleQueenMoveSquares & board) == 0)))
  {
    bs.castlingRights |= CASTELING_Q_CHECK;
  }
}

template<bool whiteTurn>
const void MoveGeneration::enPassantHelper(const BoardState& bs, MoveList& ml, uint64_t& epPosBitboard)
{
  uint64_t epPosAttacks = singlePawnAttack(bs.enPassant, whiteTurn) & bs.pieceBoards[9 - whiteTurn * 5];
  // Get king row and check if it is not 5 or 4 (white, black)
  const bool kingNotInDanger = !((bs.kings[!whiteTurn] >> 3) == (whiteTurn ? 4 : 3));
  unsigned long dest;
  while (epPosAttacks)
  {
    _BitScanForward64(&dest, epPosAttacks);
    if ((kingNotInDanger || !checkPinEP<whiteTurn>(bs, dest)) &&
        (!((1ULL << dest) & bs.pinnedSquares) || (epPosBitboard & bs.pinnedSquares & pinnedDiagonal(bs.kings[!whiteTurn], bs.enPassant))))
    {
      ml.add(dest | ((uint16_t)bs.enPassant) << 6 | moveModifiers::pawn | moveModifiers::EN_PESSANT_CAP);
    }
    epPosAttacks &= epPosAttacks - 1;
  }
}

template<bool whiteTurn>
const void MoveGeneration::promotionHelper(const BoardState& bs, MoveList& ml, uint64_t& promoting)
{
  unsigned long dest;
  while (promoting)
  {
    _BitScanForward64(&dest, promoting);
    const uint64_t startBitboard = (1ULL << dest);

    uint16_t promoPush = dest + (16 * whiteTurn - 8);
    const uint64_t promoPushBitboard = (1ULL << promoPush);

    if (promoPushBitboard & ~bs.teamBoards[0] & bs.blockMask)
    {
      if (!(startBitboard & bs.pinnedSquares) || ((startBitboard & bs.pinnedSquares) && (promoPushBitboard & bs.pinnedSquares)))
      {
        promoPush <<= 6;
        const uint32_t combo = dest | promoPush | moveModifiers::pawn;
        ml.add(combo | moveModifiers::PROMO_QUEEN);
        ml.add(combo | moveModifiers::PROMO_ROOK);
        ml.add(combo | moveModifiers::PROMO_BISHOP);
        ml.add(combo | moveModifiers::PROMO_KNIGHT);
      }
    }

    uint64_t promoAttacks = singlePawnAttack(!whiteTurn, dest) & bs.teamBoards[1 + whiteTurn] & bs.blockMask;
    unsigned long attackSquare;
    while (promoAttacks)
    {
      _BitScanForward64(&attackSquare, promoAttacks);
      const uint64_t attackSquareBitboard = 1ULL << attackSquare;
      // Unsure about order here
      if (!(startBitboard & bs.pinnedSquares) ||
          ((startBitboard & bs.pinnedSquares) &&
           (attackSquareBitboard & bs.pinnedSquares)))
      {
        attackSquare <<= 6;
        const uint32_t combo =
            dest | attackSquare | moveModifiers::pawn | moveModifiers::CAPTURE;
        ml.add(combo | moveModifiers::PROMO_QUEEN);
        ml.add(combo | moveModifiers::PROMO_ROOK);
        ml.add(combo | moveModifiers::PROMO_BISHOP);
        ml.add(combo | moveModifiers::PROMO_KNIGHT);
      }
      promoAttacks &= promoAttacks - 1;
    }
    promoting &= promoting - 1;
  }
}

////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////

template<piece::pieceType p>
void MoveGeneration::generateMultipleMoves(uint64_t board,
                                           uint64_t moves,
                                           uint32_t position,
                                           MoveList& ml)
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

template<bool pins>
void MoveGeneration::pawnBitScan(const BoardState& bs, MoveList& ml, uint64_t& pawns, const char dir, const uint32_t& move_type)
{
  unsigned long dest;
  while (pawns)
  {
    _BitScanForward64(&dest, pawns);
    if constexpr (pins)
    {
      const uint64_t startBitboard = 1ULL << (dest + dir);
      const uint64_t endBitboard = 1ULL << dest;
      // TODO not ? please!
      // GUI::printBitBoard(startBitboard);
      // GUI::printBitBoard(endBitboard);
      // GUI::printBitBoard(bs.pinnedSquares);
      // GUI::printBitBoard((((dir & 7) == 0) ? pinnedStraight(bs.kings[!bs.whiteTurn], dest) : pinnedDiagonal(bs.kings[!bs.whiteTurn], dest)));

      if (!(startBitboard & bs.pinnedSquares) || (endBitboard & bs.pinnedSquares & (((dir & 7) == 0) ? pinnedStraight(bs.kings[!bs.whiteTurn], dest) : pinnedDiagonal(bs.kings[!bs.whiteTurn], dest))))
      {
        ml.add((dest + dir) | dest << 6 | moveModifiers::pawn | move_type);
      }
    }
    else
    {
      ml.add((dest + dir) | dest << 6 | moveModifiers::pawn | move_type);
    }
    pawns &= pawns - 1;
  }
}

template<bool whiteTurn>
const bool MoveGeneration::checkPinEP(const BoardState& bs, const uint32_t start_pos)
{
  const uint8_t king = bs.kings[!whiteTurn];
  const uint64_t rank = ranks[king >> 3];
  const uint64_t startPosBB = (1ULL << start_pos);
  constexpr uint8_t offset = whiteTurn * PIECE_OFFSET;

  if (startPosBB & rank)
  {
    if (uint64_t sliders = (bs.pieceBoards[offset] | bs.pieceBoards[1 + offset]) & rank)
    {
      constexpr uint8_t back = 8 - whiteTurn * 16;
      const uint64_t board = bs.teamBoards[0] & ~startPosBB & ~(1ULL << (bs.enPassant + back));
      uint64_t attacks = 0;

      unsigned long slidePos;
      while (sliders)
      {
        _BitScanForward64(&slidePos, sliders);

        attacks |= m_rowAttacks_f(board, slidePos);
        sliders &= sliders - 1;
      }
      return attacks & (1ULL << king);
    }
    return 0;
  }
  return 0;
}

}  // namespace piece
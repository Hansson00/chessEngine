#include "../inc/makeMove.h"
#include <stdint.h>

/// TODO:
/// Note that hash wants value from 0-63
/// Make king cap

// Same as ep cap
template<bool whiteTurn, bool capture>
void makePieceMove(piece::BoardState& bs,
                   const piece::t_piece piece,
                   const uint8_t startSquare,
                   const uint8_t endSquare)
{
  constexpr uint8_t teamOffset = 2 - whiteTurn;
  constexpr uint8_t pieceOffset = !whiteTurn * 5;
  const uint8_t pieceType = (uint8_t)piece + pieceOffset;
  const uint64_t startBitboard = 1ULL << startSquare;
  const uint64_t endBitboard = 1ULL << endSquare;

  const uint64_t change = startBitboard | endBitboard;

  // bs.hash ^= m_zobristHash->epHash[(bs.enPassant & 7)] * (bs.enPassant != 0);
  bs.enPassant = 0;

  // bs.hash ^= m_zobristHash->pieceHashTable[pieceType][startSquare];
  bs.pieceBoards[pieceType] ^= change;
  // bs.hash ^= m_zobristHash->pieceHashTable[pieceType][endSquare];
  bs.teamBoards[teamOffset] ^= change;
  bs.teamBoards[0] ^= change;

  if constexpr (capture)
  {
    makeMoveCap<whiteTurn>(bs, endSquare, endBitboard);
  }
}

template<bool whiteTurn, bool capture>
void makePromoMove(piece::BoardState& bs,
                   const piece::t_piece promoPiece,
                   const uint8_t startSquare,
                   const uint8_t endSquare)
{
  constexpr uint8_t teamOffset = 2 - whiteTurn;
  constexpr uint8_t pawn = (uint8_t)piece::t_piece::t_Pawn + !whiteTurn * 5;
  constexpr uint8_t pieceOffset = !whiteTurn * 5;

  const uint8_t promoPieceType = (uint8_t)promoPiece + pieceOffset;
  const uint64_t startBitboard = 1ULL << startSquare;
  const uint64_t endBitboard = 1ULL << endSquare;

  bs.pieceCount[pawn]--;
  // bs.hash ^= m_zobristHash->pieceHashTable[pawn][startSquare];
  bs.pieceBoards[pawn] ^= startBitboard;
  bs.pieceBoards[promoPieceType] ^= endBitboard;
  // bs.hash ^= m_zobristHash->pieceHashTable[promoPieceType][endSquare];
  bs.pieceCount[promoPieceType]++;
  bs.teamBoards[teamOffset] ^= startBitboard | endBitboard;

  if constexpr (capture)
  {
    makeMoveCap<whiteTurn>(bs, endSquare, endBitboard);
  }
}

/// TODO: Fix castle hash and king cap
template<bool whiteTurn, bool castle, bool capture>
void makeKingMove(piece::BoardState& bs,
                  const uint8_t startSquare,
                  const uint8_t endSquare,
                  bool kingSideCastle)
{
  constexpr uint8_t teamOffset = 2 - whiteTurn;
  constexpr uint8_t kingOffset = !whiteTurn;
  constexpr uint8_t kingHashOffset = 10 + !whiteTurn;

  const uint64_t startBitboard = 1ULL << startSquare;
  const uint64_t endBitboard = 1ULL << endSquare;
  const uint64_t change = startBitboard | endBitboard;

  // bs.hash ^= m_zobristHash->epHash[(bs.enPassant & 7)] * (bs.enPassant != 0);
  bs.enPassant = 0;

  // bs.hash ^= m_zobristHash->pieceHashTable[kingHashOffset][bs.kings[kingOffset]];
  bs.kings[kingOffset] = endBitboard;
  // bs.hash ^= m_zobristHash->pieceHashTable[kingHashOffset][bs.kings[kingOffset]];
  bs.teamBoards[teamOffset] ^= change;

  if constexpr (castle)
  {
    constexpr uint64_t rookPosKingCastle = (0b10000000ULL | 0b100000ULL) << (56 * !whiteTurn);
    constexpr uint64_t rookPosQueenCastle = 0b1ULL | 0b1000ULL << (56 * !whiteTurn);
    constexpr uint8_t castlingFlags = whiteTurn ? ~0b0011 : ~0b1100;
    constexpr uint8_t rookOffset = (uint8_t)piece::t_piece::t_Rook + (5 * !whiteTurn);
    // Castling
    if (kingSideCastle)
    {
      bs.pieceBoards[rookOffset] ^= rookPosKingCastle;
      bs.teamBoards[teamOffset] ^= rookPosKingCastle;
      // These hashes are wrong
      // bs.hash ^= m_zobristHash->pieceHashTable[1][7];
      // bs.hash ^= m_zobristHash->pieceHashTable[1][5];
    }
    else
    {
      bs.pieceBoards[rookOffset] ^= rookPosQueenCastle;
      bs.teamBoards[teamOffset] ^= rookPosQueenCastle;
      // bs.hash ^= m_zobristHash->pieceHashTable[1][0];
      // bs.hash ^= m_zobristHash->pieceHashTable[1][3];
    }
    bs.castlingRights &= castlingFlags;
  }
  else if constexpr (capture)
  {
    makeMoveCap<whiteTurn>(bs, endSquare, endBitboard);
  }
}

template<bool whiteTurn>
void makeDubblePush(piece::BoardState& bs,
                    const uint8_t startSquare,
                    const uint8_t endSquare)
{
  constexpr uint8_t pieceType = (uint8_t)piece::t_piece::t_Pawn + (5 * !whiteTurn);
  constexpr uint8_t teamOffset = 2 - whiteTurn;
  constexpr uint8_t epWhiteTurn = (16 * whiteTurn) - 8;

  const uint64_t startBitboard = 1ULL << startSquare;
  const uint64_t endBitboard = 1ULL << endSquare;
  const uint64_t change = startBitboard | endBitboard;

  // bs.hash ^= m_zobristHash->epHash[(bs.enPassant & 7)] * (bs.enPassant != 0);
  bs.enPassant = endSquare - epWhiteTurn;
  // bs.hash ^= m_zobristHash->epHash[(bs.enPassant & 7)];

  // bs.hash ^= m_zobristHash->pieceHashTable[pieceType][startSquare];
  bs.pieceBoards[pieceType] ^= change;
  // bs.hash ^= m_zobristHash->pieceHashTable[pieceType][endSquare];

  bs.teamBoards[teamOffset] ^= change;
  bs.teamBoards[0] ^= change;
}

template<bool whiteTurn>
void makeMoveCap(piece::BoardState& bs,
                 const uint8_t endSquare,
                 const uint64_t endBitboard)
{
  constexpr uint8_t pieceStartOffset = (5 * whiteTurn);
  constexpr uint8_t pieceEndOffset = (5 + pieceStartOffset);
  constexpr uint8_t enemyTeamOffset = 1 + whiteTurn;

  for (uint8_t i = pieceStartOffset; i < pieceEndOffset; i++)
  {
    if (endBitboard & bs.pieceBoards[i])
    {
      // bs.hash ^= m_zobristHash->pieceHashTable[i][endSquare];
      bs.pieceCount[i]--;
      bs.pieceBoards[i] ^= endBitboard;
      bs.teamBoards[enemyTeamOffset] ^= endBitboard;
    }
  }
}
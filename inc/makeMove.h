#pragma once
#include "boardUtil.h"

template<bool whiteTurn, bool capture>
void makePieceMove(piece::BoardState& bs,
                   const piece::t_piece piece,
                   const uint8_t startSquare,
                   const uint8_t endSquare);

template<bool whiteTurn, bool capture>
void makePromoMove(piece::BoardState& bs,
                   const piece::t_piece promoPiece,
                   const uint8_t startSquare,
                   const uint8_t endSquare);

template<bool whiteTurn, bool castle, bool capture>
void makeKingMove(piece::BoardState& bs,
                  const uint8_t startSquare,
                  const uint8_t endSquare,
                  bool kingSideCastle);

template<bool whiteTurn>
void makeDubblePush(piece::BoardState& bs,
                    const uint8_t startSquare,
                    const uint8_t endSquare);

template<bool whiteTurn>
void makeMoveCap(piece::BoardState& bs,
                 const uint8_t endSquare,
                 const uint64_t endBitboard);
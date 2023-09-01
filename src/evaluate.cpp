#include "../inc/evaluate.h"
#include <stdint.h>
#include <intrin.h>

int evaluateMap(uint64_t bitBoard, const char* evaluationHeatMap)
{
  int evaluation = 0;
  unsigned long position = 0;

  while (bitBoard)
  {
    _BitScanForward64(&position, bitBoard);
    evaluation += evaluationHeatMap[position];
    bitBoard &= bitBoard - 1;
  }
  return evaluation;
}

const float Evaluate::evaluate(const piece::BoardState& bs)
{
  float eval = 0;

  // king

  if (bs.turns < 50)
  {
    eval += b_kingHeatmap[bs.kings[0]];
    eval -= w_kingHeatmap[bs.kings[1]];
  }
  else
  {
    eval += kingLateGame[bs.kings[0]];
    eval -= kingLateGame[bs.kings[1]];
  }

  // Queen
  // eval += evaluateMap(bs.pieceBoards[0], w_kingHeatmap);
  // eval += evaluateMap(bs.pieceBoards[5], b_kingHeatmap);

  // Rook
  if (bs.turns < 50)
  {
    eval += evaluateMap(bs.pieceBoards[1], b_rookHeatmap);
    eval -= evaluateMap(bs.pieceBoards[6], w_rookHeatmap);
  }
  else
  {
    eval += evaluateMap(bs.pieceBoards[1], rookLateGame);
    eval -= evaluateMap(bs.pieceBoards[6], rookLateGame);
  }
  // Bishop
  // eval += evaluateMap(bs.pieceBoards[2], w_rookHeatmap);
  // eval += evaluateMap(bs.pieceBoards[7], b_rookHeatmap);

  // Knight
  eval += evaluateMap(bs.pieceBoards[3], b_knightHeatmap);
  eval -= evaluateMap(bs.pieceBoards[8], w_knightHeatmap);

  // Pawn
  eval += evaluateMap(bs.pieceBoards[4], b_pawnHeatmap);
  eval -= evaluateMap(bs.pieceBoards[9], w_pawnHeatmap);

  eval *= 0.2f;

  // Piece value
  for (uint8_t i = 0; i < 5; i++)
  {
      eval += bs.pieceCount[i] * pieceValue[i];
      eval -= bs.pieceCount[i + 5] * pieceValue[i];
  }

  return eval;
}

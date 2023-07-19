#pragma once
#include "boardUtil.h"

class Evaluate
{
public:
  Evaluate() = default;
  ~Evaluate() = default;
  const int evaluate(const piece::BoardState& bs);

private:
  // Q, R, B, Kn, pawn
  const int pieceValue[5] = {10, 5, 3, 3, 1};

  const uint64_t centralWhiteSquares = 0x0004040400000000ULL | 0x0008080800000000ULL | 0x0010101000000000ULL | 0x0020202000000000ULL;
  const uint64_t centralBlackSquares = 0x000000004040400ULL | 0x000000008080800ULL | 0x0000000010101000ULL | 0x0000000020202000ULL;

  const char kingLateGame[64] = {
      -10, -10, -10, -10, -10, -10, -10, -10,
      -10, -5, -5, -5, -5, -5, -5, -10,
      -10, -5, 5, 5, 5, 5, -5, -10,
      -10, -5, 5, 5, 5, 5, -5, -10,
      -10, -5, 5, 5, 5, 5, -5, -10,
      -10, -5, 5, 5, 5, 5, -5, -10,
      -10, -5, -5, -5, -5, -5, -5, -10,
      -10, -10, -10, -10, -10, -10, -10, -10};

  const char empty[64] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0};

  const char w_kingHeatmap[64] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2,
      6, 9, 5, 0, 4, 0, 9, 6};

  const char b_kingHeatmap[64] = {
      6, 9, 5, 0, 4, 0, 9, 6,
      -2, -2, -2, -2, -2, -2, -2, -2,
      -2, -2, -2, -2, -2, -2, -2, -2,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0};

  const char w_pawnHeatmap[64] = {
      20, 20, 20, 20, 20, 20, 20, 20,
      10, 10, 10, 10, 10, 10, 10, 10,
      0, 3, -1, 5, 5, -1, 3, 0,
      0, 1, 4, 6, 6, 4, 1, 0,
      0, 1, 5, 7, 7, 5, 1, 0,
      1, 3, 2, 3, 3, 2, 3, 1,
      2, 1, 0, 0, 0, 0, 1, 2,
      0, 0, 0, 0, 0, 0, 0, 0};

  const char b_pawnHeatmap[64] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      2, 1, 0, 0, 0, 0, 1, 2,
      2, 3, 2, 3, 3, 2, 3, 2,
      0, 1, 5, 7, 7, 5, 1, 0,
      0, 1, 4, 6, 6, 4, 1, 0,
      0, 0, 3, 5, 5, 3, 0, 0,
      10, 10, 10, 10, 10, 10, 10, 10,
      20, 20, 20, 20, 20, 20, 20, 20};

  const char w_rookHeatmap[64] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      -2, 0, 4, 5, 5, 4, 0, -2};

  const char b_rookHeatmap[64] = {
      -2, 0, 4, 5, 5, 4, 0, -2,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0};
  const char rookLateGame[64] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 5, 5, 5, 5, 5, 5, 0,
      0, 5, 0, 0, 0, 0, 5, 0,
      0, 5, 0, 0, 0, 0, 5, 0,
      0, 5, 0, 0, 0, 0, 5, 0,
      0, 5, 0, 0, 0, 0, 5, 0,
      0, 5, 5, 5, 5, 5, 5, 0,
      0, 0, 0, 0, 0, 0, 0, 0};

  const char w_knightHeatmap[64] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 5, 5, 5, 5, 0, 0,
      0, 0, 5, 5, 5, 5, 0, 0,
      0, 0, 5, 5, 5, 5, 0, 0,
      0, 0, 5, 5, 5, 5, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0};

  const char b_knightHeatmap[64] = {
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 5, 5, 5, 5, 0, 0,
      0, 0, 5, 5, 5, 5, 0, 0,
      0, 0, 5, 5, 5, 5, 0, 0,
      0, 0, 5, 5, 5, 5, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0};
};
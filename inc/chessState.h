#pragma once
#include "boardUtil.h"
#include "moveGeneration.h"
#include "hash_table7.hpp"
#include "zobristHash.h"
#include "evaluate.h"
#include "stack"

class ChessState
{

public:
  emhash7::HashMap<uint64_t, std::pair<float, uint8_t>> moveHash;

  zobristHash::ZobristHash* m_zobristHash;
  piece::MoveGeneration* m_moveGenerator;
  Evaluate* m_evaluate;

  piece::BoardState m_bs;
  piece::MoveList m_ml;
  std::stack<piece::BoardState> m_prevStates;

};
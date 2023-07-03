
#pragma once
#include "../inc/boardUtil.h"
#include "../inc/gui.h"
#include "../inc/moveGeneration.h"

#include <stdint.h>
#include <memory>
#include <stack>
#include <string>

class Engine
{
public:
  Engine();
  ~Engine();

  void run();

private:
  // Input
  void engineInterface();
  static std::string getInput();

  // Gameplay
  void movePiece(std::string str);
  void undoMove();

  // Perft
  void perftCommand(std::string str);
  uint64_t perft(uint8_t depth, const piece::BoardState& bs);
  uint32_t search(uint8_t depth, const piece::BoardState& bs);

  // Tools
  void moveParserToFile(uint32_t move, std::ofstream& myFile);
  void movesParser();
  void moveParser(uint32_t move);
  void resetState();
  void tests();
  void setState(const char* str, piece::BoardState& bs);
  void initFenstring(piece::BoardState& bs, const char* str);

  // Members
  piece::MoveGeneration* m_moveGenerator;
  piece::BoardState m_bs;
  piece::MoveList m_ml;
  std::stack<piece::BoardState> m_prevStates;
};
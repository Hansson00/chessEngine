
#pragma once
#include "../inc/boardUtil.h"
#include "../inc/gui.h"
#include "../inc/moveGeneration.h"
#include "../inc/hash_table7.hpp"
#include "../inc/zobristHash.h"

#include <stdint.h>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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
  uint32_t search(uint8_t depth, const piece::BoardState& bs, uint16_t& hashHits);

  // Tools
  void moveParserToFile(uint32_t move, std::ofstream& myFile);
  void movesParser();
  void moveParser(uint32_t move);
  void resetState(std::string position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  void tests();
  void setState(const char* str, piece::BoardState& bs);
  void initFenstring(piece::BoardState& bs, const char* str);

  void findKey();

  // Members
  // TODO: Add std::pair!! hmm
  // emhash7::HashMap<uint64_t, uint64_t> moveHash;
  std::unordered_map<uint64_t, uint64_t> moveHash;

  zobristHash::ZobristHash* m_zobristHash;
  piece::MoveGeneration* m_moveGenerator;
  piece::BoardState m_bs;
  piece::MoveList m_ml;
  std::stack<piece::BoardState> m_prevStates;

  bool addBoardStates = false;
  uint64_t firstMiss = 0;
  std::vector<piece::BoardState*> boardStates;
};
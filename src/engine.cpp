#include "../inc/engine.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <intrin.h>
#include <iostream>
#include <utility>
#include <vector>
#include <stdint.h>
#include <math.h>

// #define PARSE_TO_FILE

Engine::Engine()
{
  GUI::m_setup();
  m_zobristHash = new zobristHash::ZobristHash();
  m_moveGenerator = new piece::MoveGeneration(m_zobristHash);
  m_evaluate = new Evaluate();
  resetState();

  std::cout << "Welcome to engine version 0.1. Use -h for help." << std::endl;
}

Engine::~Engine()
{
  delete (m_moveGenerator);
  delete (m_zobristHash);
  delete (m_evaluate);
}

void Engine::run()
{
  while (true)
  {
    engineInterface();
  }
}

////////////////////////////////////////////////////////////////
// Input
////////////////////////////////////////////////////////////////

void Engine::engineInterface()
{
  std::string str = getInput();
  std::string delimiter = " ";
  size_t temp = str.find(delimiter);
  std::string token = str.substr(0, temp);

  if (strcmp(token.c_str(), "perft") == 0)
  {
    perftCommand(str);
  }
  else if (strcmp(token.c_str(), "search") == 0)
  {
    if (m_ml.end != 0)
    {
      uint32_t move = startSearch(str);

      m_prevStates.push(m_bs);
      m_moveGenerator->makeMove(m_bs, move);
      m_ml.reset();
      m_moveGenerator->generatePossibleMoves(m_bs, m_ml);
      GUI::printToConsole(m_bs);
      if (m_ml.end == 0)
      {
        std::cout << "Check mate!" << std::endl;
      }
    }
  }
  else if (strcmp(token.c_str(), "move") == 0)
  {
    movePiece(str);
    GUI::printFenString(m_bs);
    std::cout << "Value: " << m_evaluate->evaluate(m_bs) << std::endl;
  }
  else if (strcmp(str.c_str(), "board") == 0)
  {
    GUI::printToConsole(m_bs);
    GUI::printFenString(m_bs);
    std::cout << "\nCorrect hash: " << m_zobristHash->zobristHash(m_bs) << std::endl;
    std::cout << "My hash     : " << m_bs.hash << std::endl;
  }
  else if (strcmp(str.c_str(), "board_mask") == 0)
  {
    GUI::printBitBoard(m_bs.teamBoards[0]);
  }
  else if (strcmp(str.c_str(), "white_mask") == 0)
  {
    GUI::printBitBoard(m_bs.teamBoards[1]);
  }
  else if (strcmp(str.c_str(), "black_mask") == 0)
  {
    GUI::printBitBoard(m_bs.teamBoards[2]);
  }
  else if (strcmp(str.c_str(), "attacks") == 0)
  {
    GUI::printBitBoard(m_bs.attacks);
  }
  else if (strcmp(str.c_str(), "pins") == 0)
  {
    GUI::printBitBoard(m_bs.pinnedSquares);
  }
  else if (strcmp(str.c_str(), "block") == 0)
  {
    GUI::printBitBoard(m_bs.blockMask);
  }
  else if (strcmp(str.c_str(), "castling") == 0)
  {
    GUI::printBitBoard(m_bs.castlingRights);
  }
  else if (strcmp(str.c_str(), "ep") == 0)
  {
    std::cout << (int)m_bs.enPassant << std::endl;
  }
  else if (strcmp(str.c_str(), "moves") == 0)
  {
    movesParser();
  }
  else if (strcmp(str.c_str(), "undo") == 0)
  {
    undoMove();
  }
  else if (strcmp(str.c_str(), "reset") == 0)
  {
    resetState();
  }
  else if (strcmp(str.c_str(), "tests") == 0)
  {
    tests();
  }
  else if (strcmp(str.c_str(), "findKey") == 0)
  {
    findKey();
  }
  else if (strcmp(token.c_str(), "fen") == 0)
  {
    resetState(str.substr(temp + 1, str.length()));
  }
}

void Engine::findKey()
{
  /*
  for (auto& state1 : boardStates)
  {
    for (auto& state2 : boardStates)
    {
      if (!(*state1 == *state2))
      {
        GUI::printToConsole(*state1);
        GUI::printFenString(*state1);

        GUI::printToConsole(*state2);
        GUI::printFenString(*state2);
      }
    }
  }
  */
}

std::string Engine::getInput()
{
  std::string answer;
  std::getline(std::cin, answer);
  return answer;
}

////////////////////////////////////////////////////////////////
// Gameplay
////////////////////////////////////////////////////////////////

uint32_t Engine::startSearch(std::string str)
{
  if (str.length() < 7)
  {
    std::cout << "Non-valid integer used" << std::endl;
    return 0;
  }
  std::string token = str.substr(7, str.length());
  for (const char& sub_str : token)
  {
    if (sub_str < '0' || sub_str > '9')
    {
      std::cout << "Non-valid integer used" << std::endl;
      return 0;
    }
  }
  uint8_t times = std::stoi(token);
  rootNegaMax(times, m_bs);

  std::pair<int, float>& bestMove = m_moveVector.front();

  for (auto& move : m_moveVector)
  {
    if (move.second > bestMove.second)
    {
      bestMove = move;
    }
  }

  std::cout << "Best move: ";
  moveParser(bestMove.first);
  std::cout << "\nScore: " << bestMove.second << std::endl;

  return bestMove.first;
}

void Engine::movePiece(std::string str)
{
  const std::string delimiter = " ";
  std::string token = str;

  while (token.length() > 5)
  {
    size_t temp = token.find(delimiter);
    token = token.substr(temp + 1, token.length() - 1);

    if (token[0] < 'a' || token[0] > 'h' || token[1] < '1' || token[1] > '8')
    {
      std::cout << "Non-valid move input used" << std::endl;
      return;
    }

    if (token[2] < 'a' || token[2] > 'h' || token[3] < '1' || token[3] > '8')
    {
      std::cout << "Non-valid move input used" << std::endl;
      return;
    }

    uint32_t start_move = token[0] - 'a' + (token[1] - '1') * 8;
    uint32_t end_move = token[2] - 'a' + (token[3] - '1') * 8;

    uint32_t move = start_move + (end_move << 6);

    // PROMO MOVE
    if (token.length() > 4)
    {
      if (token[4] != ' ')
      {
        switch (token[4])
        {
          case 'q':
            move |= piece::moveModifiers::PROMO_QUEEN;
            break;
          case 'r':
            move |= piece::moveModifiers::PROMO_ROOK;
            break;
          case 'b':
            move |= piece::moveModifiers::PROMO_BISHOP;
            break;
          case 'n':
            move |= piece::moveModifiers::PROMO_KNIGHT;
            break;
          default:
            std::cout << "Non-valid move input used" << std::endl;
            return;
        }
      }
    }

    for (int i = 0; i < m_ml.end; i++)
    {
      if ((m_ml.move[i] & (piece::moveModifiers::FROM_TO |
                           piece::moveModifiers::PROMO)) == move)
      {
        m_prevStates.push(m_bs);
        m_moveGenerator->makeMove(m_bs, m_ml.move[i]);
        m_ml.reset();
        m_moveGenerator->generatePossibleMoves(m_bs, m_ml);
        break;
      }
    }
  }
  GUI::printToConsole(m_bs);
}

void Engine::undoMove()
{
  m_bs = m_prevStates.top();
  m_prevStates.pop();
  m_ml.reset();
  m_moveGenerator->generatePossibleMoves(m_bs, m_ml);
  GUI::printToConsole(m_bs);
}

////////////////////////////////////////////////////////////////
// Search for moves
////////////////////////////////////////////////////////////////

void Engine::rootNegaMax(uint8_t depth, const piece::BoardState& bs)
{
  m_moveVector.clear();
  piece::BoardState current = bs;
  piece::MoveList ml;
  m_moveGenerator->generatePossibleMoves(current, ml);

  float a = -2000000;
  float b = 2000000;

  for (uint32_t i = 0; i < ml.end && i < 100; i++)
  {
    m_moveGenerator->makeMove(current, ml.move[i]);
    m_moveVector.emplace_back(std::make_pair(ml.move[i], -negaMax(depth - 1, current, (1 - bs.whiteTurn * 2), -b, -a)));
    current = bs;
  }
  moveHash.clear();
}

float Engine::negaMax(uint8_t depth, const piece::BoardState& bs, char color, float a, float b)
{
  if (depth == 0)
    return color * m_evaluate->evaluate(bs);

  float max = (-200000.0f) - depth;

  if ((depth > 1) && (moveHash.find(bs.hash) != moveHash.end()))
  {
    auto& pair = moveHash.at(bs.hash);
    if (pair.second >= depth)
      return pair.first;
  }
  piece::BoardState current = bs;
  piece::MoveList ml;
  m_moveGenerator->generatePossibleMoves(current, ml);

  for (uint32_t i = 0; i < ml.end && i < 100; i++)
  {
    m_moveGenerator->makeMove(current, ml.move[i]);
    float score = -negaMax(depth - 1, current, color, -b, -a);
    if (score > max)
    {
      max = score;
    }
    a = std::max(a, max);

    if (a >= b)
    {
      break;
    }

    current = bs;
  }

  if (ml.end == 0 && bs.numCheckers == 0)
  {
    max = 0;
  }
  if (depth > 1)
  {
    moveHash[bs.hash] = {max, depth};
  }
  return max;
}

////////////////////////////////////////////////////////////////
// Perft commands
////////////////////////////////////////////////////////////////

void Engine::perftCommand(std::string str)
{
  if (str.length() < 6)
  {
    std::cout << "Non-valid integer used" << std::endl;
    return;
  }

  std::string token = str.substr(6, str.length());

  for (const char& sub_str : token)
  {
    if (sub_str < '0' || sub_str > '9')
    {
      std::cout << "Non-valid integer used" << std::endl;
      return;
    }
  }
  uint8_t times = std::stoi(token);
  if (times > 8)
  {
    std::cout << "Maximum perft value supported is currently 7" << std::endl;
    return;
  }
  auto start = std::chrono::system_clock::now();
  uint64_t perftCount = perft(times, m_bs);
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_time = end - start;

  std::cout << "Time: " << elapsed_time.count() << "s" << std::endl;
  std::cout << "KN/s: "
            << static_cast<double>(perftCount) / elapsed_time.count() / 1000
            << std::endl;
}

uint64_t Engine::perft(uint8_t depth, const piece::BoardState& bs)
{
#ifdef PARSE_TO_FILE
  std::ofstream myfile;
  myfile.open("moves.txt");
#endif
  uint16_t hashHits = 0;
  uint64_t numPositions = 0;
  piece::BoardState current = bs;

  piece::MoveList ml;
  m_moveGenerator->generatePossibleMoves(current, ml);

  for (uint32_t i = 0; i < ml.end && i < 100; i++)
  {
    m_moveGenerator->makeMove(current, ml.move[i]);
    uint32_t part = search(depth - 1, current, hashHits);
    numPositions += part;
    current = bs;
#ifdef PARSE_TO_FILE
    moveParserToFile(ml.move[i], myfile);
    myfile << ": " << part << std::endl;

    moveParser(ml.move[i]);
    std::cout << ": " << part << std::endl;
#endif
  }
#ifdef PARSE_TO_FILE
  myfile.close();
#endif
  std::cout << "\nTotal: " << numPositions << std::endl;
  std::cout << "Hash hits: " << hashHits << std::endl;
  moveHash.clear();

  return numPositions;
}

uint32_t Engine::search(uint8_t depth, const piece::BoardState& bs, uint16_t& hashHits)
{
  if (depth == 0)
    return 1;

  // if (depth > 1)
  //{
  //   if (moveHash.find(bs.hash) != moveHash.end())
  //   {
  //     hashHits++;
  //     return moveHash.at(bs.hash);
  //   }
  // }

  uint32_t numPositions = 0;
  piece::BoardState current = bs;
  // Get legal moves for current position
  piece::MoveList ml;
  m_moveGenerator->generatePossibleMoves(current, ml);

  // retruns the amount of moves of the next level
  if (depth == 1)
    return ml.end;

  for (uint32_t i = 0; i < ml.end && i < 100; i++)
  {
    m_moveGenerator->makeMove(current, ml.move[i]);
    uint32_t part = search(depth - 1, current, hashHits);
    numPositions += part;
    current = bs;
  }

  // if (depth > 1)
  //{
  //   moveHash[bs.hash] = numPositions;
  // }

  return numPositions;
}

////////////////////////////////////////////////////////////////
// Tools
////////////////////////////////////////////////////////////////

void Engine::moveParserToFile(uint32_t move, std::ofstream& myFile)
{
  char row_from = (move & piece::moveModifiers::FROM) & 7;
  char col_from = (move & piece::moveModifiers::FROM) >> 3;

  char row_to = ((move & piece::moveModifiers::TO) >> 6) & 7;
  char col_to = ((move & piece::moveModifiers::TO) >> 6) >> 3;

  if (move & piece::moveModifiers::PROMO)
  {
    char promo_p;

    switch (move & piece::moveModifiers::PROMO)
    {
      case (piece::moveModifiers::PROMO_QUEEN):
        promo_p = 'q';
        break;
      case (piece::moveModifiers::PROMO_ROOK):
        promo_p = 'r';
        break;
      case (piece::moveModifiers::PROMO_BISHOP):
        promo_p = 'b';
        break;
      case (piece::moveModifiers::PROMO_KNIGHT):
        promo_p = 'n';
        break;

      default:
        break;
    }
    myFile << (char)('a' + row_from) << col_from + 1 << (char)('a' + row_to)
           << col_to + 1 << promo_p;
  }
  else
  {
    myFile << (char)('a' + row_from) << col_from + 1 << (char)('a' + row_to)
           << col_to + 1;
  }
}

void Engine::movesParser()
{
  std::cout << (int)(m_ml.end) << std::endl;
  for (int i = 0; i < m_ml.end; i++)
  {
    moveParser(m_ml.move[i]);
    std::cout << std::endl;
  }
}
void Engine::moveParser(uint32_t move)
{
  char row_from = (move & piece::moveModifiers::FROM) & 7;
  char col_from = (move & piece::moveModifiers::FROM) >> 3;

  char row_to = ((move & piece::moveModifiers::TO) >> 6) & 7;
  char col_to = ((move & piece::moveModifiers::TO) >> 6) >> 3;

  if (move & piece::moveModifiers::PROMO)
  {
    char promo_p;

    switch (move & piece::moveModifiers::PROMO)
    {
      case (piece::moveModifiers::PROMO_QUEEN):
        promo_p = 'q';
        break;
      case (piece::moveModifiers::PROMO_ROOK):
        promo_p = 'r';
        break;
      case (piece::moveModifiers::PROMO_BISHOP):
        promo_p = 'b';
        break;
      case (piece::moveModifiers::PROMO_KNIGHT):
        promo_p = 'n';
        break;

      default:
        break;
    }
    std::cout << (char)('a' + row_from) << col_from + 1 << (char)('a' + row_to)
              << col_to + 1 << promo_p;
  }
  else
  {
    std::cout << (char)('a' + row_from) << col_from + 1 << (char)('a' + row_to)
              << col_to + 1;
  }
}

void Engine::resetState(std::string position)
{
  while (m_prevStates.size())
  {
    m_prevStates.pop();
  }
  m_bs = {};
  initFenstring(m_bs, position.c_str());
  m_ml = {};

  m_moveGenerator->startUp(m_bs, m_ml);
}

void Engine::tests()
{
  bool fail = false;
  piece::BoardState bs;
  int testNr = 1;
  auto test = [&](const char* str,
                  uint64_t count, uint8_t perftLevel = 6) -> void {
    using namespace std;

    cout << "\nRunning test: " << testNr << endl;
    setState(str, bs);

    auto start = chrono::system_clock::now();
    uint64_t perftCount = perft(perftLevel, bs);
    auto end = chrono::system_clock::now();
    chrono::duration<double> elapsed_time = end - start;

    cout << "Time: " << elapsed_time.count() << "s" << endl;
    cout << "KN/s: "
         << static_cast<double>(perftCount) / elapsed_time.count() / 1000
         << endl;

    if (count != perftCount)
    {
      cout << "Test " << testNr
           << " failed, expected:" << count
           << " got: " << perftCount << endl
           << "Fen: " << str << endl;
      fail = true;
    }
    else
    {
      cout << "Test " << testNr << " success!" << endl;
    }
    testNr++;
  };

  test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 119060324ULL, 6);
  // test("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 8031647685ULL, 6);
  test("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 11030083ULL, 6);
  test("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 706045033ULL, 6);
  test("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ", 3048196529ULL, 6);

  test("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ", 6923051137ULL, 6);
  test("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 130459988ULL, 8);
  test("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 102503850ULL, 8);
  // test("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 1440467ULL, 6);
  test("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 73450134ULL, 8);
  test("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 91628014ULL, 8);

  test("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 1509218880ULL, 6);
  test("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 2010267707ULL, 6);
  test("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 905613447ULL, 8);
  test("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 197013195ULL, 7);
  test("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 397481663ULL, 9);

  test("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 153850274ULL, 9);
  test("K1k5/8/P7/8/8/8/8/8 w - - 0 1", 85822924ULL, 11);
  test("8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 173596091ULL, 10);
  test("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 104644508ULL, 7);

  if (!fail)
  {
    std::cout << "Current build cleared all tests" << std::endl;
  }
}

void Engine::setState(const char* str, piece::BoardState& bs)
{
  bs = {};
  initFenstring(bs, str);
  piece::MoveList ml = {};
  m_moveGenerator->startUp(bs, ml);
}

void Engine::initFenstring(piece::BoardState& bs, const char* str)
{
  enum pieces
  {
    q,
    r,
    b,
    n,
    p
  };
  unsigned int done = 0;

  uint8_t row = 7;
  uint8_t col = 0;
  uint64_t space = 1ULL << ((row * 8) + col);

  while (*str != 0)
  {
    switch (*str)
    {
      case ('K'):
        if (done)
          bs.castlingRights |= 0b1;
        else
        {
          // bs->§[0] |= space;
          unsigned long king;
          _BitScanForward64(&king, space);
          bs.kings[0] = king;
          bs.teamBoards[0] |= space;
          bs.teamBoards[1] |= space;
        }
        break;
      case ('Q'):
        if (done)
          bs.castlingRights |= 0b10;
        else
        {
          bs.pieceBoards[pieces::q] |= space;
          bs.pieceCount[pieces::q]++;
        }

        break;
      case ('R'):
        bs.pieceBoards[pieces::r] |= space;
        bs.pieceCount[pieces::r]++;
        break;
      case ('B'):
        bs.pieceBoards[pieces::b] |= space;
        bs.pieceCount[pieces::b]++;
        break;
      case ('N'):
        bs.pieceBoards[pieces::n] |= space;
        bs.pieceCount[pieces::n]++;
        break;
      case ('P'):
        bs.pieceBoards[pieces::p] |= space;
        bs.pieceCount[pieces::p]++;
        break;
      case ('k'):
        if (done)
          bs.castlingRights |= 0b100;
        else
        {
          // bs->pieceBoards[6] |= space;
          unsigned long king;
          _BitScanForward64(&king, space);
          bs.kings[1] = king;
          bs.teamBoards[0] |= space;
          bs.teamBoards[2] |= space;
        }
        break;
      case ('q'):
        if (done)
          bs.castlingRights |= 0b1000;
        else
        {
          bs.pieceBoards[pieces::q + 5] |= space;
          bs.pieceCount[pieces::q + 5]++;
        }
        break;
      case ('r'):
        bs.pieceBoards[pieces::r + 5] |= space;
        bs.pieceCount[pieces::r + 5]++;
        break;
      case ('b'):
        if (done)
          bs.whiteTurn = 0;
        else
        {
          bs.pieceBoards[pieces::b + 5] |= space;
          bs.pieceCount[pieces::b + 5]++;
        }
        break;
      case ('n'):
        bs.pieceBoards[pieces::n + 5] |= space;
        bs.pieceCount[pieces::n + 5]++;
        break;
      case ('p'):
        bs.pieceBoards[pieces::p + 5] |= space;
        bs.pieceCount[pieces::p + 5]++;
        break;
      case ('w'):
        bs.whiteTurn = 1;
        break;
      case (' '):
        done++;
        break;
      case ('/'):
        row--;
        col = 0;
        break;
      default:
        if (*str > 48 && *str < 58)  // If a number
          col += (*str - 49);
        break;
    }
    if (*str != '/')
      col++;
    str++;

    space = 1ULL << ((row * 8) + col);
  }
  for (int i = 0; i < 5; i++)
  {
    bs.teamBoards[0] |= bs.pieceBoards[i] | bs.pieceBoards[i + 5];
    bs.teamBoards[1] |= bs.pieceBoards[i];
    bs.teamBoards[2] |= bs.pieceBoards[i + 5];
  }

  bs.hash = m_zobristHash->zobristHash(bs);
}
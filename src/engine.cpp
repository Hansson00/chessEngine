#include "../inc/engine.h"

#include <chrono>
#include <fstream>
#include <intrin.h>
#include <iostream>
#include <stdint.h>

// #define PARSE_TO_FILE

Engine::Engine()
{
  GUI::m_setup();
  m_moveGenerator = new piece::MoveGeneration();
  resetState();

  std::cout << "Welcome to engine version 0.1. Use -h for help." << std::endl;
}

Engine::~Engine()
{
  delete (m_moveGenerator);
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
  else if (strcmp(token.c_str(), "move") == 0)
  {
    movePiece(str);
    GUI::printFenString(m_bs);
  }
  else if (strcmp(str.c_str(), "board") == 0)
  {
    GUI::printToConsole(m_bs);
    GUI::printFenString(m_bs);
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
  if (times > 7)
  {
    std::cout << "Maximum perft value supported is currently 7" << std::endl;
    return;
  }

  perft(times, m_bs);
}

uint64_t Engine::perft(uint8_t depth, const piece::BoardState& bs)
{
#ifdef PARSE_TO_FILE
  std::ofstream myfile;
  myfile.open("moves.txt");
#endif
  uint64_t num_positions = 0;
  piece::BoardState current = bs;

  piece::MoveList ml;
  m_moveGenerator->generatePossibleMoves(current, ml);

  for (uint32_t i = 0; i < ml.end && i < 100; i++)
  {
    m_moveGenerator->makeMove(current, ml.move[i]);
    uint32_t part = search(depth - 1, current);
    num_positions += part;
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
  std::cout << "\nTotal: ";
  std::cout << num_positions << std::endl;

  return num_positions;
}

uint32_t Engine::search(uint8_t depth, const piece::BoardState& bs)
{
  if (depth == 0)
    return 1;

  uint32_t num_positions = 0;
  piece::BoardState current = bs;

  // Get legal moves for current position
  piece::MoveList ml;
  m_moveGenerator->generatePossibleMoves(current, ml);

  if (depth == 1)
    return ml.end;

  for (uint32_t i = 0; i < ml.end && i < 100; i++)
  {
    m_moveGenerator->makeMove(current, ml.move[i]);
    uint32_t part = search(depth - 1, current);
    num_positions += part;
    current = bs;
  }
  return num_positions;
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

void Engine::resetState()
{
  m_bs = {};
  // initFenstring(m_bs, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  // initFenstring(m_bs, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq");
  initFenstring(m_bs, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ");
  m_ml = {};

  m_moveGenerator->startUp(m_bs, m_ml);
}

void Engine::tests()
{
  bool fail = false;
  piece::BoardState bs;
  auto test = [this, &bs, &fail](int testNr, const char* str,
                                 uint64_t count) -> void {
    using namespace std;

    cout << "\nRunning test: " << testNr << endl;
    setState(str, bs);

    auto start = chrono::system_clock::now();
    uint64_t perftCount = perft(6, bs);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_time = end - start;

    std::cout << "Time: " << elapsed_time.count() << "s" << endl;
    std::cout << "KN/s: "
              << static_cast<double>(perftCount) / elapsed_time.count() / 1000
              << endl;

    if (count != perftCount)
    {
      std::cout << "Test " << testNr << " failed, expected:" << count
                << " got: " << perftCount << std::endl;
      fail = true;
    }
    else
    {
      std::cout << "Test " << testNr << " success!" << std::endl;
    }
  };

  // TEST 1
  test(1, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
       119060324ULL);

  // TEST 2
  test(2, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
       8031647685ULL);

  // TEST 3
  test(3, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 11030083ULL);

  // TEST 4
  test(4, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
       706045033ULL);

  // TEST 5
  test(5, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ",
       89941194ULL);

  // TEST 6
  test(6,
       "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 "
       "10 ",
       6923051137ULL);

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
          bs.castlingRights |= 4;
        else
        {
          // bs->pieceBoards[0] |= space;
          unsigned long king;
          _BitScanForward64(&king, space);
          bs.kings[0] = king;
          bs.teamBoards[0] |= space;
          bs.teamBoards[1] |= space;
        }
        break;
      case ('Q'):
        if (done)
          bs.castlingRights |= 8;
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
          bs.castlingRights |= 1;
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
          bs.castlingRights |= 2;
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
          bs.pieceBoards[pieces::r + 5] |= space;
          bs.pieceCount[pieces::r + 5]++;
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
}
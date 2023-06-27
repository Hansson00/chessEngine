#include "../inc/engine.h"

#include <iostream>
#include <intrin.h>

Engine::Engine()
{
  m_moveGenerator = std::make_unique<piece::MoveGeneration>();
  resetState();

  std::cout << "Welcome to engine version 0.1. Use -h for help." << std::endl;
}

void Engine::run()
{
  GUI::m_setup();

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
      if ((m_ml.move[i] & (piece::moveModifiers::FROM_TO | piece::moveModifiers::PROMO)) == move)
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
  if (times > 6)
  {
    std::cout << "Maximum perft value supported is currently 6" << std::endl;
    return;
  }

  perft(times, m_bs);
}

uint32_t Engine::perft(uint8_t depth, const piece::BoardState& bs)
{
  // hash_hits = 0;
  uint32_t num_positions = 0;
  piece::BoardState current = bs;

  piece::MoveList ml;
  m_moveGenerator->generatePossibleMoves(current, ml);

  for (uint32_t i = 0; i < ml.end && i < 100; i++)
  {
    m_moveGenerator->makeMove(current, ml.move[i]);
    uint32_t part = search(depth - 1, current);
    num_positions += part;
    current = bs;
    moveParser(ml.move[i]);
    std::cout << ": " << part << std::endl;
  }
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
    std::cout << (char)('a' + row_from) << col_from + 1 << (char)('a' + row_to) << col_to + 1 << promo_p;
  }
  else
  {
    std::cout << (char)('a' + row_from) << col_from + 1 << (char)('a' + row_to) << col_to + 1;
  }
}

void Engine::resetState()
{
  m_bs = {};
  initFenstring(m_bs, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  // initFenstring(m_bs, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq");
  //  initFenstring(m_bs, "k7/8/K/3R4/8/8/8/8 w KQkq - ");
  m_ml = {};

  m_moveGenerator->startUp(m_bs, m_ml);
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
          bs.pieceBoards[pieces::q] |= space;
        break;
      case ('R'):
        bs.pieceBoards[pieces::r] |= space;
        break;
      case ('B'):
        bs.pieceBoards[pieces::b] |= space;
        break;
      case ('N'):
        bs.pieceBoards[pieces::n] |= space;
        break;
      case ('P'):
        bs.pieceBoards[pieces::p] |= space;
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
          bs.pieceBoards[pieces::q + 5] |= space;
        break;
      case ('r'):
        bs.pieceBoards[pieces::r + 5] |= space;
        break;
      case ('b'):
        if (done)
          bs.whiteTurn = 0;
        else
          bs.pieceBoards[pieces::b + 5] |= space;
        break;
      case ('n'):
        bs.pieceBoards[pieces::n + 5] |= space;
        break;
      case ('p'):
        bs.pieceBoards[pieces::p + 5] |= space;
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
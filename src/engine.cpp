#include "../inc/engine.h"

#include <iostream>
#include <memory>

Engine::Engine()
{
  m_moveGenerator = std::make_unique<piece::MoveGeneration>();
}

void Engine::run()
{
  GUI::m_setup();

  while (true)
  {
    engineInterface();
  }
}

void Engine::engineInterface()
{
  std::string str = getInput();
  std::string delimiter = " ";
  size_t temp = str.find(delimiter);
  std::string token = str.substr(0, temp);

  if (std::strcmp(token.c_str(), "perft") == 0)
  {
    perft_command(str);
  }
  if (std::strcmp(token.c_str(), "move") == 0)
  {
    move_piece(str);
    print_fen_string(current_state);
  }
  else if (std::strcmp(str.c_str(), "board") == 0)
  {
    gui->print_to_console(current_state);
    print_fen_string(current_state);
  }
  else if (std::strcmp(str.c_str(), "board_mask") == 0)
  {
    print_bit_board(current_state.team_boards[0]);
  }
  else if (std::strcmp(str.c_str(), "white_mask") == 0)
  {
    print_bit_board(current_state.team_boards[1]);
  }
  else if (std::strcmp(str.c_str(), "black_mask") == 0)
  {
    print_bit_board(current_state.team_boards[2]);
  }
  else if (std::strcmp(str.c_str(), "attacks") == 0)
  {
    print_bit_board(current_state.attacks);
  }
  else if (std::strcmp(str.c_str(), "pins") == 0)
  {
    print_bit_board(current_state.pinned_squares);
  }
  else if (std::strcmp(str.c_str(), "block") == 0)
  {
    print_bit_board(current_state.block_mask);
  }
  else if (std::strcmp(str.c_str(), "castling") == 0)
  {
    print_bit_board(current_state.castling_rights);
  }
  else if (std::strcmp(str.c_str(), "ep") == 0)
  {
    std::cout << (int)current_state.en_passant << std::endl;
  }
  else if (std::strcmp(str.c_str(), "moves") == 0)
  {
    moves_parser();
  }
  else if (std::strcmp(str.c_str(), "undo") == 0)
  {
    undo_move();
  }
  else if (std::strcmp(str.c_str(), "reset") == 0)
  {
    reset();
  }
}

std::string Engine::getInput()
{
  std::string answer;
  std::getline(std::cin, answer);
  return answer;
}

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
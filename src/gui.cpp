#include "../inc/gui.h"
#include <iostream>
#include "intrin.h"

char m_boardRepresentation[9][28] = {};

namespace GUI
{

void m_setup()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      m_boardRepresentation[i][j * 3] = '[';
      m_boardRepresentation[i][j * 3 + 1] = ' ';
      m_boardRepresentation[i][j * 3 + 2] = ']';
    }
    m_boardRepresentation[i][24] = ' ';
    m_boardRepresentation[i][25] = '1' + i;
    m_boardRepresentation[i][26] = '\n';
    m_boardRepresentation[i][27] = 0;
  }
  for (int i = 0; i < 8; i++)
  {
    m_boardRepresentation[8][i * 3] = ' ';
    m_boardRepresentation[8][i * 3 + 1] = 'a' + i;
    m_boardRepresentation[8][i * 3 + 2] = ' ';
  }
  m_boardRepresentation[8][24] = ' ';
  m_boardRepresentation[8][25] = ' ';
  m_boardRepresentation[8][26] = '\n';
  m_boardRepresentation[8][27] = 0;
}

void m_clearBoard()
{
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      m_boardRepresentation[i][j * 3 + 1] = ' ';
    }
  }
}

void m_setPieces(const piece::BoardState& bs)
{
  for (int i = 0; i < 10; i++)
  {
    uint64_t current_piece = bs.pieceBoards[i];
    for (int j = 0; current_piece; j++)
    {
      if (current_piece & 1)
      {
        m_boardRepresentation[j >> 3][(j & 7) * 3 + 1] = piece::piece[i];
      }
      current_piece >>= 1;
    }
  }

  m_boardRepresentation[bs.kings[0] >> 3][(bs.kings[0] & 7) * 3 + 1] = 'K';
  m_boardRepresentation[bs.kings[1] >> 3][(bs.kings[1] & 7) * 3 + 1] = 'k';
}

void m_printBoard()
{
  std::cout << m_boardRepresentation[7] << m_boardRepresentation[6] << m_boardRepresentation[5] << m_boardRepresentation[4]
            << m_boardRepresentation[3] << m_boardRepresentation[2] << m_boardRepresentation[1] << m_boardRepresentation[0] << m_boardRepresentation[8] << std::endl;
}

void moveVisualizer(uint32_t move)
{
  const char piece[6] = {'k', 'q', 'r', 'b', 'n', 'p'};

  char c_board[8 * 9 + 1] = {};  // xxxxxxxx + \n and 0 at the end

  unsigned long bit;

  _BitScanForward(&bit, ((move & piece::moveModifiers::ATTACKERS) >> 12));

  uint8_t from = move & piece::moveModifiers::FROM;
  uint8_t to = (move & piece::moveModifiers::TO) >> 6;

  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (i * 8 + j == from)
        c_board[(7 - i) * 9 + j] = piece[bit];
      else if (i * 8 + j == to)
        c_board[(7 - i) * 9 + j] = 'x';
      else
        c_board[(7 - i) * 9 + j] = '-';
    }
    c_board[(7 - i) * 9 + 8] = '\n';
  }

  c_board[8 * 9] = 0;

  std::cout << c_board;
}

void printToConsole(const piece::BoardState& bs)
{
  m_clearBoard();
  m_setPieces(bs);
  m_printBoard();
}

void printBitBoard(uint64_t board)
{
  m_clearBoard();
  for (int j = 0; board; j++)
  {
    if (board & 1)
    {
      m_boardRepresentation[j >> 3][(j & 7) * 3 + 1] = 'x';
    }
    board >>= 1;
  }
  m_printBoard();
}

void printFenString(const piece::BoardState& bs)
{
  m_clearBoard();
  m_setPieces(bs);

  std::cout << "position fen ";

  for (int i = 7; i > -1; i--)
  {
    int count = 0;
    for (int j = 0; j < 9; j++)
    {
      if (j == 8 && i != 0)
      {
        if (count)
        {
          std::cout << count;
          count = 0;
        }
        std::cout << '/';
      }
      else if (j == 8 && i == 0)
      {
        if (count)
        {
          std::cout << count;
          count = 0;
        }
      }
      else if (m_boardRepresentation[i][(j & 7) * 3 + 1] != ' ')
      {
        if (count)
        {
          std::cout << count;
          count = 0;
        }

        std::cout << m_boardRepresentation[i][(j & 7) * 3 + 1];
      }
      else
      {
        count++;
      }
    }
  }

  std::cout << ' ';

  if (bs.whiteTurn)
  {
    std::cout << "w ";
  }
  else
  {
    std::cout << "b ";
  }

  if (bs.castlingRights & 0b1)
  {
    std::cout << 'K';
  }
  if (bs.castlingRights & 0b10)
  {
    std::cout << 'Q';
  }
  if (bs.castlingRights & 0b100)
  {
    std::cout << 'k';
  }
  if (bs.castlingRights & 0b1000)
  {
    std::cout << 'q';
  }

  std::cout << std::endl;
}

}  // namespace GUI
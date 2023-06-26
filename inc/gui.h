#pragma once

#include "../inc/boardUtil.h"
namespace GUI
{

/*
 * @Brief Init board
 */
void m_setup();

/*
 * @Brief Clears the current board
 */
void m_clearBoard();

/*
 * @Brief Sets all pieces of a give state
 */
void m_setPieces(const piece::BoardState& bs);

/*
 * @Brief Prints the current of the char array
 */
void m_printBoard();

/*
 * @Brief Prints the start and end of a move
 */
void moveVisualizer(uint32_t move);

/*
 * @Brief Prints a move to the console
 */
void printToConsole(const piece::BoardState& bs);

/*
 * @Brief Prints a given bit board
 *
 * @Param[in] board to be visulized
 */
void printBitBoard(uint64_t board);

/*
 * @Brief Prints a fen string of the current position
 */
void printFenString(const piece::BoardState& bs);

}  // namespace GUI
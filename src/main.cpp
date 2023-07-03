#include <stdint.h>
#include <memory>
#include <iostream>
#include "../inc/magicalBitboards.h"
#include "../inc/gui.h"

#include "../inc/Engine.h"

int main()
{
  std::unique_ptr<Engine> engine = std::make_unique<Engine>();
  engine->run();

  // GUI::m_setup();
  //
  // magicalBits::MagicalBitboards* mb = new magicalBits::MagicalBitboards;
  //
  // uint64_t board = 0xFFFF;
  // GUI::printBitBoard(board);
  // uint64_t mask = mb->maskBishopAttacks(4) & board;
  //
  // GUI::printBitBoard(mask);
  // uint64_t occ = mb->setOccupancy(4, mb->countBits(mask), mask);
  // GUI::printBitBoard(occ);
  //  GUI::printBitBoard(mb->getBishopAttacks1(4, board));

  // mb->initMagicNumbers();
}

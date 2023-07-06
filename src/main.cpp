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
}

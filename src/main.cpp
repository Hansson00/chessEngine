#include <memory>
#include "../inc/Engine.h"
#include <iostream>
int main()
{
  std::unique_ptr<Engine> engine = std::make_unique<Engine>();
  engine->run();
}
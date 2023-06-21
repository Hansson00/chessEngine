#pragma once
#include <cinttypes>

namespace piece {
namespace helper {

class DiagonalMove
{
public:
  DiagonalMove() = default;
  ~DiagonalMove() = default;

private:
  template<bool whiteTurn>
  uint64_t antidiagonalAttacks(uint64_t board, uint8_t position);

  template<bool whiteTurn>
  uint64_t maindiagonalAttacks(uint64_t board, uint8_t position);
};

}  // namespace helper
}  // namespace piece
#pragma once

#include "slider.h"
#include "pieceHelper.h"

namespace piece {

class Rook : public helper::PieceHelper, private helper::Slider
{
};

}  // namespace piece
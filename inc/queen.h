#pragma once

#include "slider.h"
#include "pieceHelper.h"

namespace piece {

class Queen : public helper::PieceHelper, private helper::Slider
{
};

}  // namespace piece
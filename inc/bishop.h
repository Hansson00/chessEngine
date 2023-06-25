#pragma once

#include "slider.h"
#include "pieceHelper.h"

namespace piece
{
class Bishop : public helper::PieceHelper, private helper::Slider
{
};

}  // namespace piece
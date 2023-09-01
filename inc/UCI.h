
#pragma once

#include "string"
#include "boardUtil.h"
#include "chessState.h"

namespace UCI
{

  class UCI : private ChessState
  {
  public:
    UCI();
    ~UCI();

    void UCImain();

  private:
    std::string m_id = "id author Isak Hansson";
    std::string m_uciok = "uciok";
  };

}
#include "../inc/UCI.h"

namespace UCI
{

  UCI::UCI()
  {
    std::cout << m_id << std::endl;

    m_zobristHash = new zobristHash::ZobristHash();
    m_moveGenerator = new piece::MoveGeneration(m_zobristHash);
    m_evaluate = new Evaluate();

    std::cout << m_uciok << std::endl;
  }

  UCI::~UCI()
  {
  }

  void UCI::UCImain()
  {
    while (true)
    {

    }
  }

}

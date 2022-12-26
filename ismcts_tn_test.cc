#include "include/ismcts/sosolver.h"
#include "ismcts_tn.hh"
#include <iostream>

int main() {

  TwentyNine tn_game;
  std::cout << tn_game << std::endl;

  ISMCTS::SOSolver<TwentyNine::MoveType> solver;

  while (tn_game.m_tricksLeft > 0 && tn_game.m_currentTrick.size() < 4) {
    Card best_move = solver(tn_game);
    std::cout << "In state: " << tn_game << "move: " << best_move << std::endl;
    tn_game.doMove(best_move);
    std::cout << best_move << std::endl;
  }

  std::cout << tn_game.m_pointsScored[0] << std::endl;
  std::cout << tn_game.m_pointsScored[1] << std::endl;
  std::cout << tn_game.m_pointsScored[2] << std::endl;
  std::cout << tn_game.m_pointsScored[3] << std::endl;
  return 0;
}

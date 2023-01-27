#ifndef ISMCTS_TN_H_
#define ISMCTS_TN_H_

#include "bot.hh"
#include "card.h"
#include "include/ismcts/game.h"

#include <set>
#include <vector>

enum WinType {
  SAME_SUIT_GREATER_RANK, TRUMP_SUIT_GREATEST_CARD
};

enum MoveSetType {
  SAME_SUIT_WINNING = SAME_SUIT_GREATER_RANK,
  TRUMP_SUIT_WINNING = TRUMP_SUIT_GREATEST_CARD,
  SAME_SUIT_NOT_WINNING = 3,
  DIFFERENT_SUIT = 4
};

typedef struct WinData {
  Card winning_card;
  ISMCTS::POMGame<Card>::Player winning_player;
  WinType win_type;
} WinData;

class TwentyNine : public ISMCTS::POMGame<Card> {

public:
  explicit TwentyNine();
  virtual Clone cloneAndRandomise(Player observer) const override;
  virtual Player currentPlayer() const override;
  virtual std::vector<Player> players() const override;
  virtual std::vector<Card> validMoves() const override;
  virtual void doMove(Card const move) override;
  virtual double getResult(Player player) const override;
  friend std::ostream &operator<<(std::ostream &out, TwentyNine const &g);

public:
  using Hand = std::vector<Card>;
  using Play = std::pair<Player, Card>;

  unsigned int static constexpr s_deckSize{32};
  std::vector<Card> m_deck{s_deckSize};
  std::vector<Card> m_unknownCards;
  std::vector<Player> m_players{4};
  std::vector<Hand> m_playerCards;
  std::vector<Play> m_currentTrick;
  std::vector<unsigned> m_pointsScored;
  std::vector<unsigned> m_bids;

  unsigned m_tricksLeft{8};
  unsigned static constexpr m_numPlayers{4};
  Player m_player{0};
  Player m_dealer;
  Card::Suit m_trumpSuit;
  bool m_requestTrump = false;
  bool m_hasTrumpBeenRevealed = false;
  Player m_player_who_revealed_trump;
  size_t m_which_hand_the_trump_was_revealed_in;

  Player nextPlayer(Player p) const;
  void deal();
  void clear();
  void printVerbose() const;
  void finishRound();
  void finishTrick();
  unsigned calcPointsInTrick() const;
  Player roundWinner() const;
  Player trickWinner() const;
  bool canRevealTrump() const;

    void parse_playpayload(const PlayPayload&) ;
  WinData getWinData() const;
};

class PlayerState {
public:
  std::set<Card> m_cardDomain;
};

#endif // ISMCTS_TN_H_

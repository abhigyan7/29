#ifndef ISMCTS_TN_H_
#define ISMCTS_TN_H_

#include "bot.hh"
#include "card.h"
#include "include/ismcts/game.h"

#include <map>
#include <set>
#include <vector>

class CompoundMove {
public:
  Card card_to_play;
  bool reveal_trump;
  CompoundMove() = default;
  CompoundMove(const Card card) : card_to_play(card), reveal_trump(false) {}
  CompoundMove(const bool revealTrump) : reveal_trump(revealTrump) {}
  friend std::ostream &operator<<(std::ostream &out, CompoundMove const &m);
  bool operator==(CompoundMove const &) const;
  bool operator!=(CompoundMove const &other) const { return !(*this == other); }
  explicit operator int() const {
    if (this->reveal_trump)
      return 55;
    return int(card_to_play);
  }
  constexpr CompoundMove(Card::Rank rank, Card::Suit suit)
      : card_to_play(rank, suit), reveal_trump(false) {}
};

inline bool operator<(CompoundMove const &a, CompoundMove const &b) {
  return int(a) < int(b);
}

class TwentyNine : public ISMCTS::POMGame<CompoundMove> {

public:
  using Move = CompoundMove;
  explicit TwentyNine();
  virtual Clone cloneAndRandomise(Player observer) const override;
  virtual Player currentPlayer() const override;
  virtual std::vector<Player> players() const override;
  virtual std::vector<Move> validMoves() const override;
  virtual void doMove(Move const move) override;
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
  bool m_isTrumpSuitKnown = false;
  bool m_hasTrumpBeenRevealed = false;
  Player m_player_who_revealed_trump;
  size_t m_which_hand_the_trump_was_revealed_in;
  std::map<std::string, size_t> m_map_player_string_to_int_id;
  std::vector<std::set<Card>> m_players_possible_cards;
  float m_players_cards_probabilities[4][32];

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

  void parse_playpayload(const PlayPayload &);
};

#endif // ISMCTS_TN_H_

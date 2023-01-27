#include "ismcts_tn.hh"

#include "include/ismcts/utility.h"
#include "ismcts_tn.hh"
#include <algorithm>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <string>

inline int value(Card const &card) {
  std::map<Card::Rank, int> static const values{
      {Card::Ace, 1}, {Card::Seven, 0}, {Card::Eight, 0}, {Card::Nine, 2},
      {Card::Ten, 1}, {Card::Jack, 3},  {Card::Queen, 0}, {Card::King, 0}};
  return values.at(card.rank);
}

inline std::mt19937 &prng() {
  std::mt19937 static prng;
  return prng;
}

TwentyNine::TwentyNine() : m_tricksLeft(8), m_dealer(3) {
  for (unsigned i{0}; i < s_deckSize; ++i) {
    m_deck[i] = {Card::Rank(i / 4), Card::Suit(i % 4)};
  }
  std::cout << std::endl;
  m_players.resize(m_numPlayers);
  std::iota(m_players.begin(), m_players.end(), 0);
  m_playerCards.resize(m_numPlayers);
  m_pointsScored.resize(m_numPlayers, 0);
  m_currentTrick.clear();
  m_bids = {0, 1, 2, 3};

  // deal();
  m_trumpSuit = Card::Spades;
}

TwentyNine::Clone TwentyNine::cloneAndRandomise(Player observer) const {
  // DONE fix this
  // Randomize should shuffle unseedCards and then divide the required amount
  // to each player, note that each player might need different number of
  // cards depending on where in the hand they are
  auto clone = std::make_unique<TwentyNine>(*this);

  Hand unseenCards = m_unknownCards;
  // for (auto p : m_players) {
  //   if (p == observer)
  //     continue;
  //   auto const &hand = m_playerCards[p];
  //   unseenCards.insert(unseenCards.end(), hand.begin(), hand.end());
  // }

  std::shuffle(unseenCards.begin(), unseenCards.end(), prng());
  Player next_player = nextPlayer(observer);
  for (const auto &c : unseenCards) {
    if (next_player == observer) {
      next_player = nextPlayer(next_player);
    }
    clone->m_playerCards[next_player].push_back(c);
    next_player = nextPlayer(next_player);
  }
  return clone;
}

TwentyNine::Player TwentyNine::currentPlayer() const { return m_player; }

TwentyNine::Player TwentyNine::nextPlayer(Player p) const {
  return (p + 1) % m_numPlayers;
}

std::vector<TwentyNine::Player> TwentyNine::players() const {
  return m_players;
}

void TwentyNine::doMove(Card const move) {
  m_currentTrick.emplace_back(m_player, move);
  auto &hand = m_playerCards[m_player];
  auto const pos = std::find(hand.begin(), hand.end(), move);

  if (pos < hand.end())
    hand.erase(pos);

  m_player = nextPlayer(m_player);

  if (m_currentTrick.size() == m_players.size())
    finishTrick();
  else
    return;
  if (m_playerCards[m_player].empty())
    finishRound();
}

double TwentyNine::getResult(Player player) const {

  if (m_bids[player] == 0) {
    if (m_bids[nextPlayer(player)] > m_pointsScored[nextPlayer(player)])
      return 1.0;
    return 0.0;
  }

  if (m_bids[player] > m_pointsScored[player])
    return 0.0;

  return 1.0;
}

std::vector<Card> TwentyNine::validMoves() const {
  // DONE implement valid moves made of card throws only
  // TODO create a move+reveal trump kind of structure

  if (m_tricksLeft == 0)
    return std::vector<Card>();

  auto const &hand = m_playerCards[m_player];
  if (m_currentTrick.empty())
    return hand;

  Card winningCard;
  Hand currentTrickCards;
  Hand cardsInSuit;
  Hand winningCardsInHand;

  for (const auto &a : m_currentTrick)
    currentTrickCards.push_back(a.second);

  auto const leadCard = m_currentTrick.front().second;
  std::copy_if(hand.begin(), hand.end(), std::back_inserter(cardsInSuit),
               [&](auto const &c) { return c.suit == leadCard.suit; });

  winningCard = *std::max_element(
      currentTrickCards.begin(), currentTrickCards.end(),
      [&](auto const &c1, auto const &c2) { return value(c1) < value(c2); });

  std::copy_if(cardsInSuit.begin(), cardsInSuit.end(), std::back_inserter(winningCardsInHand),
               [&](auto const &c) { return value(c) > value(winningCard); });

  if (!m_hasTrumpBeenRevealed && !winningCardsInHand.empty()) {
    return winningCardsInHand;
  }
  if (!cardsInSuit.empty()) {
    return cardsInSuit;
  }

  if (m_hasTrumpBeenRevealed) {
    std::vector<Card> trumpCardsInCurrentTrick;
    std::copy_if(currentTrickCards.begin(), currentTrickCards.end(),
                 std::back_inserter(trumpCardsInCurrentTrick),
                 [&](auto const &c) { return c.suit == m_trumpSuit; });
    if (!trumpCardsInCurrentTrick.empty()) {
      winningCard = *std::max_element(trumpCardsInCurrentTrick.begin(),
                                      trumpCardsInCurrentTrick.end(),
                                      [&](auto const &c1, auto const &c2) {
                                        return value(c1) < value(c2);
                                      });

      std::copy_if(
          hand.begin(), hand.end(), std::back_inserter(winningCardsInHand),
          [&](auto const &c) { return value(c) > value(winningCard); });
      if (winningCardsInHand.empty())
        return hand;
      return winningCardsInHand;
    }
  }
  return hand;
}

void TwentyNine::clear() {
  for (auto &p : m_playerCards) {
    p.clear();
  }
  m_currentTrick.clear();
}

void TwentyNine::deal() {
  std::shuffle(m_deck.begin(), m_deck.end(), prng());

  auto pos = m_deck.begin();
  for (auto p : m_players) {
    auto &hand = m_playerCards[p];
    std::copy_n(pos, m_tricksLeft, std::back_inserter(hand));
    pos += m_tricksLeft;
  }

  // The rest of the cards are unknown to all players
  m_unknownCards.clear();
  std::copy(pos, m_deck.end(), std::back_inserter(m_unknownCards));
}

unsigned TwentyNine::calcPointsInTrick() const {
  unsigned ret = 0;
  for (auto card : m_currentTrick) {
    ret += value(card.second);
  }
  return ret;
}

TwentyNine::Player get_partner(const TwentyNine::Player player_in) {
  return (player_in + 2) % 4;
}

void TwentyNine::finishTrick() {
  auto const winner = trickWinner();
  m_pointsScored[winner] += calcPointsInTrick();
  m_pointsScored[get_partner(winner)] += calcPointsInTrick();
  m_currentTrick.clear();
  m_player = winner;
  m_tricksLeft--;
}

void TwentyNine::finishRound() {

  // calculate the winner
  // update the scored points
  // calculate the winner based on bids
  // deal

  // for (unsigned i = 0; i < 2; ++i) {
  //   m_pointsScored[i] = m_pointsScored[i] + m_pointsScored[get_partner(i)];
  //   m_pointsScored[get_partner(i)] = m_pointsScored[i];
  // }

  // deal();
}

TwentyNine::Player TwentyNine::trickWinner() const {
  auto winner = m_currentTrick.begin();
  // std::cout << winner->second << ", " ;
  for (auto p = winner + 1; p < m_currentTrick.end(); ++p) {
    auto const card = p->second;
    auto const winningCard = winner->second;
    // std::cout << card<< ", " ;
    if (card.suit == winningCard.suit) {
      if (value(card) > value(winningCard))
        winner = p;
    } else if (m_hasTrumpBeenRevealed && (card.suit == m_trumpSuit)) {
      winner = p;
    }
  }
  // std::cout << " winner = " << winner->first << std::endl;
  return winner->first;
}

std::ostream &operator<<(std::ostream &out, TwentyNine const &g) {
  out << "Card size: " << g.m_playerCards[g.m_player].size() << ", ";
  auto const player = g.m_player;
  auto const &hand = g.m_playerCards[player];
  out << "Round " << g.m_tricksLeft << " | P" << player << ": ";
  std::copy(hand.begin(), hand.end(), std::ostream_iterator<Card>(out, ","));
  out << " | Trump: " << g.m_trumpSuit << " | Trick: [";
  for (auto const &pair : g.m_currentTrick)
    out << pair.first << ":" << pair.second << ",";
  return out << "]";
}

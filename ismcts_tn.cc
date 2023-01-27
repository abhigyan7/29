#include "ismcts_tn.hh"

#include "bot.hh"
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

inline int move_value(Card const &card) {
  std::map<Card::Rank, int> static const values{
      {Card::Ace, 2}, {Card::Seven, 0}, {Card::Eight, 0}, {Card::Nine, 3},
      {Card::Ten, 1}, {Card::Jack, 4},  {Card::Queen, 0}, {Card::King, 0}};
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
  m_hasTrumpBeenRevealed = false;
  m_trumpSuit = Card::Spades;
}

TwentyNine::Clone TwentyNine::cloneAndRandomise(Player observer) const {
  auto clone = std::make_unique<TwentyNine>(*this);

  Hand unseenCards = m_unknownCards;

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

bool TwentyNine::canRevealTrump() const {

  if (m_hasTrumpBeenRevealed)
    return false;

  if (m_tricksLeft == 0)
    return false;

  auto const &hand = m_playerCards[m_player];
  if (m_currentTrick.empty())
    return false;

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

  std::copy_if(cardsInSuit.begin(), cardsInSuit.end(),
               std::back_inserter(winningCardsInHand),
               [&](auto const &c) { return value(c) > value(winningCard); });

  if (!m_hasTrumpBeenRevealed && cardsInSuit.empty())
    return true;
  return false;
}

WinData TwentyNine::getWinData() const {
  WinData ret;

  auto winner = m_currentTrick.begin();
  Card winning_card = m_currentTrick.begin()[0].second;
  WinType win_type = SAME_SUIT_GREATER_RANK;

  for (auto p = winner + 1; p < m_currentTrick.end(); ++p) {
    auto const card = p->second;
    auto const winningCard = winner->second;
    if (card.suit == winningCard.suit) {
      if (value(card) > value(winningCard)) {
        winner = p;
        win_type = SAME_SUIT_GREATER_RANK;
      } else if (m_hasTrumpBeenRevealed && (card.suit == m_trumpSuit)) {
        winner = p;
        win_type = TRUMP_SUIT_GREATEST_CARD;
      }
    }
  }

  ret.winning_player = winner->first;
  ret.winning_card = winning_card;
  ret.win_type = win_type;
  return ret;
}

bool does_card_win_in_a_hand(const std::vector<Card> hand, const Card card,
                             const Card::Suit lead_card_suit,
                             const bool has_trump_been_revealed,
                             const Card::Suit trump_suit) {
  return true;
}

std::vector<Card> TwentyNine::validMoves() const {

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

  std::copy_if(cardsInSuit.begin(), cardsInSuit.end(),
               std::back_inserter(winningCardsInHand),
               [&](auto const &c) { return value(c) > value(winningCard); });

  if (!m_hasTrumpBeenRevealed && !winningCardsInHand.empty()) {
    return winningCardsInHand;
  }
  if (!cardsInSuit.empty()) {
    return cardsInSuit;
  }

  std::vector<Card> trump_cards_in_hand;
  if (m_hasTrumpBeenRevealed) {
    std::vector<Card> trumpCardsInCurrentTrick;
    std::copy_if(currentTrickCards.begin(), currentTrickCards.end(),
                 std::back_inserter(trumpCardsInCurrentTrick),
                 [&](auto const &c) { return c.suit == m_trumpSuit; });
    std::copy_if(hand.begin(), hand.end(),
                 std::back_inserter(trump_cards_in_hand),
                 [&](auto const &c) { return c.suit == m_trumpSuit; });
    if (!trumpCardsInCurrentTrick.empty()) {
      winningCard = *std::max_element(trumpCardsInCurrentTrick.begin(),
                                      trumpCardsInCurrentTrick.end(),
                                      [&](auto const &c1, auto const &c2) {
                                        return value(c1) < value(c2);
                                      });

      winningCardsInHand.clear();
      std::copy_if(trump_cards_in_hand.begin(), trump_cards_in_hand.end(),
                   std::back_inserter(winningCardsInHand), [&](auto const &c) {
                     return value(c) > value(winningCard);
                   });
    }
  }
  if (!winningCardsInHand.empty())
    return winningCardsInHand;
  if (m_hasTrumpBeenRevealed && winningCardsInHand.empty() &&
      m_player_who_revealed_trump == m_player &&
      // i was here
      m_which_hand_the_trump_was_revealed_in == (9 - m_tricksLeft))
    if (!trump_cards_in_hand.empty())
      return trump_cards_in_hand;
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

void TwentyNine::finishRound() {}

TwentyNine::Player TwentyNine::trickWinner() const {
  auto winner = m_currentTrick.begin();
  for (auto p = winner + 1; p < m_currentTrick.end(); ++p) {
    auto const card = p->second;
    auto const winningCard = winner->second;
    if (card.suit == winningCard.suit) {
      if (value(card) > value(winningCard))
        winner = p;
    } else if (m_hasTrumpBeenRevealed && (card.suit == m_trumpSuit)) {
      winner = p;
    }
  }
  return winner->first;
}

void TwentyNine::parse_playpayload(const PlayPayload &payload) {

  std::map<std::string, size_t> map_player_string_to_int_id;

  std::set<Card> unknown_cards(m_deck.begin(), m_deck.end());

  // DONE clean this mess up, use the map we have above
  for (int i = 0; i < 4; ++i) {
    map_player_string_to_int_id[payload.player_ids[i]] = i;
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        if (payload.player_ids[i] == payload.teams[j].players[k]) {
          m_bids[i] = payload.teams[j].bid;
          m_pointsScored[i] = payload.teams[j].won;
        }
      }
    }
  }

  // std::cout << payload.cards.size() <<" cards in payload.cards\n";
  m_player = map_player_string_to_int_id[payload.player_id];
  // tngame.m_playerCards[tngame.m_player].clear();
  for (const auto &ccard : payload.cards) {
    Card _card = CCard_to_Card(ccard);
    m_playerCards[m_player].push_back(_card);
    unknown_cards.erase(_card);
  }
  // std::cout << tngame.m_playerCards[tngame.m_player].size() <<" cards in
  // hand.cards\n";

  int n_cards_in_trick = payload.played.size();

  TwentyNine::Player first_player_in_this_trick =
      (m_player + (4 - n_cards_in_trick)) % 4;
  TwentyNine::Player _player = first_player_in_this_trick;
  for (const auto &played_cards : payload.played) {
    Card _card = CCard_to_Card(played_cards);
    unknown_cards.erase(_card);
    m_currentTrick.push_back({_player, _card});
    _player = nextPlayer(_player);
  }

  m_hasTrumpBeenRevealed = false;
  if (!std::holds_alternative<bool>(payload.trumpRevealed)) {
    PlayerID player_id_who_revealed =
        std::get<PlayPayload::RevealedObject>(payload.trumpRevealed).player_id;
    m_player_who_revealed_trump =
        map_player_string_to_int_id[player_id_who_revealed];
    m_which_hand_the_trump_was_revealed_in =
        std::get<PlayPayload::RevealedObject>(payload.trumpRevealed).hand;
    if (std::holds_alternative<CSuit>(payload.trumpCSuit)) {
      m_trumpSuit = CSuit_to_Suit(std::get<CSuit>(payload.trumpCSuit));
      m_hasTrumpBeenRevealed = true;
    }
  }

  m_tricksLeft = 8;

  for (const auto &history_entry : payload.hand_history) {
    m_tricksLeft -= 1;
    for (const auto &played_ccard : history_entry.card) {
      Card _card = CCard_to_Card(played_ccard);
      unknown_cards.erase(_card);
    }
  }

  std::copy(unknown_cards.begin(), unknown_cards.end(),
            std::back_inserter(m_unknownCards));

  m_players = {0, 1, 2, 3};
}

const std::string spades_print_repr[] = {"C", "D", "H", "S"};

std::ostream &operator<<(std::ostream &out, TwentyNine const &g) {
  out << "Card size: " << g.m_playerCards[g.m_player].size() << ", ";
  auto const player = g.m_player;
  auto const &hand = g.m_playerCards[player];
  out << "Round " << 8 - g.m_tricksLeft << " | P" << player << ": ";
  std::copy(hand.begin(), hand.end(), std::ostream_iterator<Card>(out, ","));
  out << " | Trump: " << spades_print_repr[g.m_trumpSuit] << " | Trick: [";
  for (auto const &pair : g.m_currentTrick)
    out << pair.first << ":" << pair.second << ",";
  out << " ] | Player: " << g.m_player;
  out << " | Player who revealed trump: " << g.m_player_who_revealed_trump
      << ", In trick ";
  out << g.m_which_hand_the_trump_was_revealed_in;
  return out << "]";
}

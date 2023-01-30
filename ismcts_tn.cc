#include "ismcts_tn.hh"

#include "bot.hh"
#include "card.h"
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

inline int POINTS(Card const &card) {
  std::map<Card::Rank, int> static const values{
      {Card::Ace, 1}, {Card::Seven, 0}, {Card::Eight, 0}, {Card::Nine, 2},
      {Card::Ten, 1}, {Card::Jack, 3},  {Card::Queen, 0}, {Card::King, 0}};
  return values.at(card.rank);
}

inline int VALUE(Card const &card) {
  std::map<Card::Rank, int> static const values{
      {Card::Ace, 18}, {Card::Seven, 13}, {Card::Eight, 14}, {Card::Nine, 19},
      {Card::Ten, 17}, {Card::Jack, 20},  {Card::Queen, 15}, {Card::King, 16}};
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
      [&](auto const &c1, auto const &c2) { return VALUE(c1) < VALUE(c2); });

  std::copy_if(cardsInSuit.begin(), cardsInSuit.end(),
               std::back_inserter(winningCardsInHand),
               [&](auto const &c) { return VALUE(c) > VALUE(winningCard); });

  if (!m_hasTrumpBeenRevealed && cardsInSuit.empty())
    return true;
  return false;
}

bool does_card_win_in_a_hand(const std::vector<Card> hand, const Card card,
                             const Card::Suit lead_card_suit,
                             const bool has_trump_been_revealed,
                             const Card::Suit trump_suit) {
  return true;
}

std::vector<Card>
pureValidMoves(int tricksLeft, const std::vector<std::vector<Card>> playerCards,
               TwentyNine::Player player,
               std::vector<TwentyNine::Play> currentTrick,
               bool hasTrumpBeenRevealed, Card::Suit trumpSuit,
               TwentyNine::Player player_who_revealed_trump,
               int which_trick_trump_was_revealed_in) {

  using Hand = std::vector<Card>;
  if (tricksLeft == 0)
    return std::vector<Card>();

  const std::vector<Card> hand = playerCards[player];
  if (currentTrick.empty())
    return hand;

  Card winningCard;
  Hand currentTrickCards;
  Hand cardsInSuit;
  Hand winningCardsInHand;

  for (const auto &a : currentTrick)
    currentTrickCards.push_back(a.second);

  auto const leadCard = currentTrick.front().second;
  std::copy_if(hand.begin(), hand.end(), std::back_inserter(cardsInSuit),
               [&](auto const &c) { return c.suit == leadCard.suit; });

  winningCard = *std::max_element(
      currentTrickCards.begin(), currentTrickCards.end(),
      [&](auto const &c1, auto const &c2) { return VALUE(c1) < VALUE(c2); });

  std::copy_if(cardsInSuit.begin(), cardsInSuit.end(),
               std::back_inserter(winningCardsInHand),
               [&](auto const &c) { return VALUE(c) > VALUE(winningCard); });

  if (!hasTrumpBeenRevealed && !winningCardsInHand.empty()) {
    return winningCardsInHand;
  }
  if (!cardsInSuit.empty()) {
    return cardsInSuit;
  }

  std::vector<Card> trump_cards_in_hand;
  if (hasTrumpBeenRevealed) {
    std::vector<Card> trumpCardsInCurrentTrick;
    std::copy_if(currentTrickCards.begin(), currentTrickCards.end(),
                 std::back_inserter(trumpCardsInCurrentTrick),
                 [&](auto const &c) { return c.suit == trumpSuit; });
    std::copy_if(hand.begin(), hand.end(),
                 std::back_inserter(trump_cards_in_hand),
                 [&](auto const &c) { return c.suit == trumpSuit; });
    if (!trumpCardsInCurrentTrick.empty()) {
      winningCard = *std::max_element(trumpCardsInCurrentTrick.begin(),
                                      trumpCardsInCurrentTrick.end(),
                                      [&](auto const &c1, auto const &c2) {
                                        return VALUE(c1) < VALUE(c2);
                                      });

      winningCardsInHand.clear();
      std::copy_if(trump_cards_in_hand.begin(), trump_cards_in_hand.end(),
                   std::back_inserter(winningCardsInHand), [&](auto const &c) {
                     return VALUE(c) > VALUE(winningCard);
                   });
    }
  }
  if (!winningCardsInHand.empty())
    return winningCardsInHand;
  if (hasTrumpBeenRevealed && winningCardsInHand.empty() &&
      player_who_revealed_trump == player &&
      // i was here
      which_trick_trump_was_revealed_in == (9 - tricksLeft))
    if (!trump_cards_in_hand.empty())
      return trump_cards_in_hand;
  return hand;
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
      [&](auto const &c1, auto const &c2) { return VALUE(c1) < VALUE(c2); });

  std::copy_if(cardsInSuit.begin(), cardsInSuit.end(),
               std::back_inserter(winningCardsInHand),
               [&](auto const &c) { return VALUE(c) > VALUE(winningCard); });

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
                                        return VALUE(c1) < VALUE(c2);
                                      });

      winningCardsInHand.clear();
      std::copy_if(trump_cards_in_hand.begin(), trump_cards_in_hand.end(),
                   std::back_inserter(winningCardsInHand), [&](auto const &c) {
                     return VALUE(c) > VALUE(winningCard);
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
    ret += POINTS(card.second);
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
      if (VALUE(card) > VALUE(winningCard))
        winner = p;
    } else if (m_hasTrumpBeenRevealed && (card.suit == m_trumpSuit)) {
      winner = p;
    }
  }
  return winner->first;
}

int trickWinner(std::vector<Card> trick, bool has_trump_been_revealed,
                Card::Suit trump_suit) {
  Card winner = trick[0];
  int ret = 0;
  for (int i = 1; i < trick.size(); i++) {
    Card p = trick[i];
    if (p.suit == winner.suit) {
      if (VALUE(p) > VALUE(winner)) {
        winner = p;
        ret = i;
      }
    } else if (has_trump_been_revealed && (p.suit == trump_suit)) {
      winner = p;
      ret = i;
    }
  }
  return ret;
}

template <typename T>
inline std::vector<T> set_to_vec(const std::set<T> &set_in) {
  return std::vector<T>(set_in.begin(), set_in.end());
}

template <typename T>
inline std::set<T> vec_to_set(const std::vector<T> &vec_in) {
  return std::set<T>(vec_in.begin(), vec_in.end());
}

void TwentyNine::parse_playpayload(const PlayPayload &payload) {

  std::set<Card> unknown_cards(m_deck.begin(), m_deck.end());

  // DONE clean this mess up, use the map we have above
  for (int i = 0; i < 4; ++i) {
    m_map_player_string_to_int_id[payload.player_ids[i]] = i;
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        if (payload.player_ids[i] == payload.teams[j].players[k]) {
          m_bids[i] = payload.teams[j].bid;
          m_pointsScored[i] = payload.teams[j].won;
        }
      }
    }
  }

  std::vector<Card> my_remaining_cards;
  Player me = m_map_player_string_to_int_id[payload.player_id];
  m_player = m_map_player_string_to_int_id[payload.player_id];
  for (const auto &ccard : payload.cards) {
    Card _card = CCard_to_Card(ccard);
    m_playerCards[m_player].push_back(_card);
    my_remaining_cards.push_back(_card);
    unknown_cards.erase(_card);
  }

  int n_cards_in_trick = payload.played.size();

  std::vector<Play> currentTrick;

  TwentyNine::Player first_player_in_this_trick =
      (m_player + (4 - n_cards_in_trick)) % 4;
  TwentyNine::Player _player = first_player_in_this_trick;
  for (const auto &played_cards : payload.played) {
    Card _card = CCard_to_Card(played_cards);
    unknown_cards.erase(_card);
    currentTrick.push_back({_player, _card});
    _player = nextPlayer(_player);
  }

  m_hasTrumpBeenRevealed = false;
  if (!std::holds_alternative<bool>(payload.trumpRevealed)) {
    PlayerID player_id_who_revealed =
        std::get<PlayPayload::RevealedObject>(payload.trumpRevealed).player_id;
    m_player_who_revealed_trump =
        m_map_player_string_to_int_id[player_id_who_revealed];
    m_which_hand_the_trump_was_revealed_in =
        std::get<PlayPayload::RevealedObject>(payload.trumpRevealed).hand;
    if (std::holds_alternative<CSuit>(payload.trumpCSuit)) {
      m_trumpSuit = CSuit_to_Suit(std::get<CSuit>(payload.trumpCSuit));
      m_hasTrumpBeenRevealed = true;
    }
  }

  m_players = {0, 1, 2, 3};
  m_tricksLeft = 8;

  std::set<Card> deck_minus_mine(m_deck.begin(), m_deck.end());
  for (const auto &my_card : m_playerCards[me]) {
    deck_minus_mine.erase(my_card);
  }

  std::set<Card> my_cards_set = vec_to_set<Card>(m_playerCards[me]);
  std::vector<std::set<Card>> players_possible_cards{4};
  for (int _i = 0; _i < 4; _i++) {
    if (_i != m_player)
      players_possible_cards[_i] = deck_minus_mine;
    else
      players_possible_cards[_i] = my_cards_set;
  }

  if (payload.hand_history.size() == 0)
    m_player = first_player_in_this_trick;
  else
    m_player = m_map_player_string_to_int_id[payload.hand_history[0].initiator];

  std::vector<PlayPayload::HandHistoryEntry> hh = payload.hand_history;

  PlayPayload::HandHistoryEntry current_trick_shoehorned_handhistory;
  for (auto const &_c: currentTrick) {
    std::cout << "current trick: " << _c.second << ", ";
    current_trick_shoehorned_handhistory.card.push_back(Card_to_CCard(_c.second));
  }
  std::cout << std::endl;
  hh.push_back(current_trick_shoehorned_handhistory);

  for (const auto &history_entry : hh) {
    for (const auto &played_ccard : history_entry.card) {
      std::vector<Card> legal_moves;
      Card played_card = CCard_to_Card(played_ccard);
      std::cout << "Played card: " << played_card << std::endl;
      while (true) {
        m_playerCards[m_player] = set_to_vec(players_possible_cards[m_player]);
        legal_moves = validMoves();
        if (legal_moves.size() == 0)
          break;
        std::set<Card> legal_moves_set = vec_to_set(legal_moves);
        played_card = CCard_to_Card(played_ccard);
        if (legal_moves_set.count(played_card) != 0)
          break;
        for (const auto &legal_card_that_player_couldnt_play :
             legal_moves_set) {
          players_possible_cards[m_player].erase(
              legal_card_that_player_couldnt_play);
        }
      }
      doMove(played_card);
      unknown_cards.erase(played_card);
      players_possible_cards[0].erase(played_card);
      players_possible_cards[1].erase(played_card);
      players_possible_cards[2].erase(played_card);
      players_possible_cards[3].erase(played_card);
    }
  }

  players_possible_cards[me] = vec_to_set(my_remaining_cards);

  std::cout << std::endl << std::endl;
  std::cout << "---------------------------" << std::endl;

  std::cout << "Player 0 has: ";
  for (const auto &_c : players_possible_cards[0])
    std::cout << _c << ", ";
  std::cout << std::endl;

  std::cout << "Player 1 has: ";
  for (const auto &_c : players_possible_cards[1])
    std::cout << _c << ", ";
  std::cout << std::endl;

  std::cout << "Player 2 has: ";
  for (const auto &_c : players_possible_cards[2])
    std::cout << _c << ", ";
  std::cout << std::endl;

  std::cout << "Player 3 has: ";
  for (const auto &_c : players_possible_cards[3])
    std::cout << _c << ", ";
  std::cout << std::endl;

  std::cout << "Unknown cards: ";
  for (const auto &_c : unknown_cards)
    std::cout << _c << ", ";
  std::cout << std::endl;

  m_currentTrick = currentTrick;
  m_playerCards[0] = {};
  m_playerCards[1] = {};
  m_playerCards[2] = {};
  m_playerCards[3] = {};
  m_playerCards[me] = my_remaining_cards;
  std::copy(unknown_cards.begin(), unknown_cards.end(),
            std::back_inserter(m_unknownCards));
}

const std::string suits_print_repr[] = {"C", "D", "H", "S"};

std::ostream &operator<<(std::ostream &out, TwentyNine const &g) {
  out << "Card size: " << g.m_playerCards[g.m_player].size() << ", ";
  auto const player = g.m_player;
  auto const &hand = g.m_playerCards[player];
  out << "Round " << 8 - g.m_tricksLeft << " | P" << player << ": ";
  std::copy(hand.begin(), hand.end(), std::ostream_iterator<Card>(out, ","));
  out << " | Trump: " << suits_print_repr[g.m_trumpSuit] << " | Trick: [";
  for (auto const &pair : g.m_currentTrick)
    out << pair.first << ":" << pair.second << ",";
  out << " ] | Player: " << g.m_player;
  if (g.m_hasTrumpBeenRevealed) {
    out << " | Player who revealed trump: " << g.m_player_who_revealed_trump
        << ", In trick ";
    out << g.m_which_hand_the_trump_was_revealed_in;
  } else {
    out << " | Trump unrevealed.";
  }
  return out << "]";
}

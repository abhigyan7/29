#include "bot.hh"
#include "card.h"
#include "include/ismcts/mosolver.h"
#include "include/ismcts/sosolver.h"
#include "ismcts_tn.hh"
#include <algorithm>
#include <boost/asio/detail/handler_type_requirements.hpp>
#include <chrono>
#include <map>
#include <set>
#include <variant>

Card::Suit CSuit_to_Suit(CSuit csuit_in) { return (Card::Suit)(int)(csuit_in); }

Card CCard_to_Card(CCard card_in) {
  int CCard_rank_enum = (int)card_in.rank;
  int CCard_suit_enum = (int)card_in.suit;

  Card ret;
  ret.rank = (Card::Rank)(CCard_rank_enum - 7);
  ret.suit = (Card::Suit)CCard_suit_enum;
  return ret;
}

CCard Card_to_CCard(Card card_in) {
  int Card_rank_enum = (int)card_in.rank;
  int Card_suit_enum = (int)card_in.suit;

  CCard ret;
  ret.rank = (Rank)(Card_rank_enum + 7);
  ret.suit = (CSuit)Card_suit_enum;
  return ret;
}

std::ostream &operator<<(std::ostream &os, PlayPayload const &payload) {
  os << "Player ID : " << payload.player_id << std::endl;
  os << "Player IDs : ";
  for (auto const &player : payload.player_ids)
    os << player << "    ";

  os << "\nCCards \n";
  for (auto card : payload.cards)
    os << card << "   ";

  os << "\nPlayed : \n";
  for (auto card : payload.played)
    os << card << "  \n";

  os << "\nBid History : \n";
  for (auto const &entry : payload.bid_history) {
    os << "player    : " << entry.player_id << "\n"
       << "Bid value : " << entry.bid_value << std::endl;
  }

  // Hand history
  for (auto const &x : payload.hand_history) {
    os << "Initiator : " << x.initiator;
    os << "\nWinner   : " << x.winner;
    os << "\nCCards Played : ";
    for (auto card : x.card)
      os << card << "   ";
    os << std::endl;
  }
  return os;
}

static GameState game_instance;

GameState &GetGameInstance() { return game_instance; }

std::ostream &operator<<(std::ostream &os, const CCard &card) {
  return os << CCard::ToStr(card) << std::endl;
}

void InitGameInstance() {}

CSuit GameState::ChooseTrump(PlayerID myid, std::vector<PlayerID> player_ids,
                             std::vector<CCard> mycards, int32_t time_remaining,
                             std::vector<BidEntry> bid_history) {
  std::map<CSuit, int> occurances;
  occurances[SPADES] = 0;
  occurances[HEARTS] = 0;
  occurances[DIAMONDS] = 0;
  occurances[CLUBS] = 0;
  for (const auto &c : mycards) {
    occurances[c.suit] += 1;
  }

  int max_occ = 0;
  CSuit ret = mycards[3].suit;

  for (const auto &c : mycards) {
    if (occurances[c.suit] > max_occ) {
      max_occ = occurances[c.suit];
      ret = c.suit;
    }
  }

  return ret;
}

int GameState::Bid(PlayerID myid, std::vector<PlayerID> player_ids,
                   std::vector<CCard> mycards, int32_t time_remaining,
                   std::vector<BidEntry> bid_history,
                   BidState const &bid_state) {

  int max_bid_target = 16;

  std::map<CSuit, int> occurances;
  std::map<CSuit, int> bid_contribs;

  occurances[SPADES] = 0;
  occurances[HEARTS] = 0;
  occurances[DIAMONDS] = 0;
  occurances[CLUBS] = 0;

  for (const auto &c : mycards) {
    occurances[c.suit] += 1;
    if (c.rank == JACK) {
      bid_contribs[c.suit] += 1;
      max_bid_target += 1;
    }
  }

  int max_occ = 0;
  CSuit ret = mycards[3].suit;

  for (const auto &c : mycards) {
    if (occurances[c.suit] > max_occ) {
      max_occ = occurances[c.suit];
      ret = c.suit;
    }
  }

  max_bid_target += max_occ / 3;

  // Either bid or pass
  if (bid_history.empty())
    return 16;

  int max_bid = 0;
  std::map<PlayerID, int> bid_map;
  for (auto const &bid_entry : bid_history) {
    bid_map[bid_entry.player_id] = bid_entry.bid_value;
    if (bid_entry.bid_value > max_bid) {
      max_bid = bid_entry.bid_value;
    }
  }

  max_bid_target = max_bid_target > 18 ? 18 : max_bid_target;

  if (max_bid < max_bid_target) {
    if (max_bid == 0)
      return 16;
    return max_bid + 1;
  }
  return 0;
}

inline int value(Card const &card) {
  std::map<Card::Rank, int> static const values{
      {Card::Ace, 1}, {Card::Seven, 0}, {Card::Eight, 0}, {Card::Nine, 2},
      {Card::Ten, 1}, {Card::Jack, 3},  {Card::Queen, 0}, {Card::King, 0}};
  return values.at(card.rank);
}

PlayAction GameState::Play(PlayPayload payload) {

  TwentyNine tngame;
  tngame.clear();

  tngame.parse_playpayload(payload);

  auto valid_moves = tngame.validMoves();

  for (const auto valid_move : valid_moves) {
    std::cout << valid_move << ", ";
  }
  std::cout << std::endl;
  std::cout << tngame << std::endl;



  if (tngame.validMoves().size() == 1) {
    PlayAction p_action;
    p_action.played_card = Card_to_CCard(tngame.validMoves()[0].card_to_play);
    return p_action;
  }

  double offset_remaining_time = (double)payload.remaining_time -
                                 ((double)payload.cards.size() * 250.0 / 8.0);
  double time_to_search_for_s = 0.001 * offset_remaining_time * 0.3;
  if (time_to_search_for_s < 0.001)
    time_to_search_for_s = 0.001;
  if (payload.remaining_time == 0)
    time_to_search_for_s = 0.500;
  ISMCTS::SOSolver<TwentyNine::MoveType> solver{
      std::chrono::duration<double>(time_to_search_for_s)};

  // ISMCTS::SOSolver<TwentyNine::MoveType> solver{1000};

  PlayAction p_action;
  TwentyNine::Move best_move = solver(tngame);

  std::cout << "Move selected: " << best_move << std::endl;
  // std::cout << solver.currentTrees()[0]->treeToString() << std::endl;

#ifdef DEBUG
  std::cout << std::endl << std::endl;
  std::cout << "---------------------------" << std::endl;

  std::cout << "Player 0 has: ";
  for (const auto &_c : tngame.m_players_possible_cards[0])
    std::cout << _c << ", ";
  std::cout << std::endl;

  std::cout << "Player 1 has: ";
  for (const auto &_c : tngame.m_players_possible_cards[1])
    std::cout << _c << ", ";
  std::cout << std::endl;

  std::cout << "Player 2 has: ";
  for (const auto &_c : tngame.m_players_possible_cards[2])
    std::cout << _c << ", ";
  std::cout << std::endl;

  std::cout << "Player 3 has: ";
  for (const auto &_c : tngame.m_players_possible_cards[3])
    std::cout << _c << ", ";
  std::cout << std::endl;

  std::cout << "Unknown cards: ";
  for (const auto &_c : tngame.m_unknownCards)
    std::cout << _c << ", ";
  std::cout << std::endl;
#endif
  if (best_move.reveal_trump) {
    std::cout << "Revealing trump" << std::endl;
    p_action.action = PlayAction::RevealTrump;
    return p_action;
  }

  CCard selected_move = Card_to_CCard(best_move.card_to_play);

  p_action.action = PlayAction::PlayCCard;
  p_action.played_card = selected_move;
  return p_action;
}

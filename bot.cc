#include "bot.hh"
#include "card.h"
#include "include/ismcts/sosolver.h"
#include "include/ismcts/mosolver.h"
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

  occurances[SPADES] = 0;
  occurances[HEARTS] = 0;
  occurances[DIAMONDS] = 0;
  occurances[CLUBS] = 0;

  for (const auto &c : mycards) {
    occurances[c.suit] += 1;
    if (c.rank == JACK)
      max_bid_target += 1;
  }

  int max_occ = 0;
  CSuit ret = mycards[3].suit;

  for (const auto &c : mycards) {
    if (c.rank == JACK)
      max_bid_target += 1;
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
  for (auto const &bid_entry : bid_history)
  {
    bid_map[bid_entry.player_id] = bid_entry.bid_value;
    if (bid_entry.bid_value > max_bid) {
      max_bid = bid_entry.bid_value;
    }
  }

  if (max_bid < max_bid_target)
    return max_bid_target;
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

  std::cout << "GAME::::" << std::endl;
  std::cout << tngame << std::endl;

  if (tngame.validMoves().size() == 1)
  {
    PlayAction p_action;
    p_action.played_card = Card_to_CCard(tngame.validMoves()[0]);
    return p_action;
  }

  // double time_to_search_for_s = 0.001 * double(payload.remaining_time) * 0.2;
  // std::cout << "Searching for " << time_to_search_for_s << "seconds.\n";
  // ISMCTS::SOSolver<TwentyNine::MoveType> solver{std::chrono::duration<double>(time_to_search_for_s)};
  ISMCTS::SOSolver<TwentyNine::MoveType> solver{5000};

  if (tngame.canRevealTrump())
  {
    PlayAction p_action;
    p_action.action = PlayAction::RevealTrump;
    return p_action;
  }

  Card best_move = solver(tngame);
  // std::cout << *solver.currentTrees()[0] << std::endl;
  // for (const auto tree: solver.currentTrees()) {
  //   std::cout << *tree;
  // }

  std::cout << "Move selected: " << best_move << std::endl;

  CCard selected_move = Card_to_CCard(best_move);

  PlayAction p_action;
  p_action.action = PlayAction::PlayCCard;
  p_action.played_card = selected_move;
  return p_action;

  if (payload.played.empty()) {
    p_action.played_card = payload.cards[0];
    return p_action;
  }

  CSuit lead_suit = payload.played[0].suit;

  auto same_suit_filter = [=](CCard card) { return card.suit == lead_suit; };
  std::vector<CCard> same_suit_cards;
  std::copy_if(payload.cards.begin(), payload.cards.end(),
               std::back_inserter(same_suit_cards), same_suit_filter);
  if (same_suit_cards.empty()) {
    p_action.played_card = payload.cards[0];
    return p_action;
  }

  p_action.played_card = *same_suit_cards.begin();
  return p_action;
}

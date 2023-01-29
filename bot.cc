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
  // Either bid or pass
  if (bid_history.empty())
    return 16;
  return 0;
}

inline int value(Card const &card) {
  std::map<Card::Rank, int> static const values{
      {Card::Ace, 1}, {Card::Seven, 0}, {Card::Eight, 0}, {Card::Nine, 2},
      {Card::Ten, 1}, {Card::Jack, 3},  {Card::Queen, 0}, {Card::King, 0}};
  return values.at(card.rank);
}

std::vector<Card> validMoves(std::vector<Card> hand, std::vector<Card> trick,
                             Card::Suit trump_suit,
                             bool has_trump_been_revealed) {

  if (trick.empty())
    return hand;

  auto const leadCard = trick.front();
  std::vector<Card> cardsInSuit, winningCardsInHand;
  std::copy_if(hand.begin(), hand.end(), std::back_inserter(cardsInSuit),
               [&](auto const &c) { return c.suit == leadCard.suit; });

  std::vector<Card> currentTrickCards;
  for (const auto &a : trick)
    currentTrickCards.push_back(a);

  Card winningCard;
  std::vector<Card> trumpCardsInCurrentTrick;
  std::copy_if(currentTrickCards.begin(), currentTrickCards.end(),
               std::back_inserter(trumpCardsInCurrentTrick),
               [&](auto const &c) { return c.suit == trump_suit; });

  if (!trumpCardsInCurrentTrick.empty()) {
    winningCard = *std::max_element(
        trumpCardsInCurrentTrick.begin(), trumpCardsInCurrentTrick.end(),
        [&](auto const &c1, auto const &c2) { return value(c1) < value(c2); });
  } else if (!cardsInSuit.empty()) {

    winningCard = *std::max_element(
        cardsInSuit.begin(), cardsInSuit.end(),
        [&](auto const &c1, auto const &c2) { return value(c1) < value(c2); });
  } else {
    return hand;
  }

  if (!cardsInSuit.empty()) {

    std::vector<Card> cardsInTrumpSuit;
    std::copy_if(hand.begin(), hand.end(), std::back_inserter(cardsInSuit),
                 [&](auto const &c) { return c.suit == trump_suit; });
    if (cardsInTrumpSuit.empty()) {
      return hand;
    }

    std::copy_if(hand.begin(), hand.end(),
                 std::back_inserter(winningCardsInHand),
                 [&](auto const &c) { return value(c) > value(leadCard); });
    if (winningCardsInHand.empty())
      return hand;
    return winningCardsInHand;
  }
  return hand;
}

PlayAction GameState::Play(PlayPayload payload) {
  // std::cout << "Payload Received and parsed: \n" << payload << std::endl;

  TwentyNine tngame;
  tngame.clear();

  tngame.parse_playpayload(payload);
  if (tngame.validMoves().size() == 1)
  {
    PlayAction p_action;
    p_action.played_card = Card_to_CCard(tngame.validMoves()[0]);
    return p_action;
  }

  ISMCTS::SOSolver<TwentyNine::MoveType> solver{8000};
  // ISMCTS::SOSolver<TwentyNine::MoveType> solver{std::chrono::milliseconds((int)(payload.remaining_time * 0.2))};

  if (tngame.canRevealTrump())
  {
    PlayAction p_action;
    p_action.action = PlayAction::RevealTrump;
    return p_action;
  }

  Card best_move = solver(tngame);

  std::cout << "Move selected: " << best_move << std::endl;

  CCard selected_move = Card_to_CCard(best_move);

  std::cout << "TN GAME: ";
  std::cout << tngame;
  std::cout << std::endl;

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

#include "bot.hh"
#include "card.h"
#include "ismcts_tn.hh"
#include <algorithm>
#include <boost/asio/detail/handler_type_requirements.hpp>
#include <map>
#include <set>
#include <variant>


Card::Suit CSuit_to_Suit(CSuit csuit_in) {
  return (Card::Suit)(int)(csuit_in);
}

Card CCard_to_Card(CCard card_in)
{
  int CCard_rank_enum = (int)card_in.rank;
  int CCard_suit_enum = (int)card_in.suit;

  Card ret;
  ret.rank = (Card::Rank) (CCard_rank_enum-7);
  ret.suit = (Card::Suit) CCard_suit_enum;
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
  return CSuit(mycards[3].suit);
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

PlayAction GameState::Play(PlayPayload payload) {
  std::cout << "Payload Received and parsed: \n" << payload << std::endl;

  TwentyNine tngame;

  std::map<std::string, size_t> map_player_string_to_int_id;

  // TODO make sure this vec is populated before this line executes
  std::set<Card> unknown_cards(tngame.m_deck.begin(), tngame.m_deck.end());

  // TODO clean this mess up, use the map we have above
  for (int i = 0; i < 4; ++i) {
    map_player_string_to_int_id[payload.player_ids[i]] = i;
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        if (payload.player_ids[i] == payload.teams[j].players[k]) {
          tngame.m_bids[i] = payload.teams[j].bid;
          tngame.m_pointsScored[i] = payload.teams[j].won;
        }
      }
    }
  }

  for (const auto& ccard: payload.cards)
  {
    Card _card = CCard_to_Card(ccard);
    tngame.m_playerCards[tngame.m_player].push_back(_card);
    unknown_cards.erase(_card);
  }

  tngame.m_player = map_player_string_to_int_id[payload.player_id];

  int n_cards_in_trick = payload.played.size();

  TwentyNine::Player first_player_in_this_trick = (tngame.m_player + ( 4 - n_cards_in_trick )) % 4;
  TwentyNine::Player _player = first_player_in_this_trick;
  for (const auto& played_cards: payload.played)
  {
    Card _card = CCard_to_Card(played_cards);
    unknown_cards.erase(_card);
    tngame.m_currentTrick.push_back({_player, _card});
    _player = tngame.nextPlayer(_player);
  }


  if (std::holds_alternative<CSuit>(payload.trumpCSuit)) {
    tngame.m_trumpSuit = CSuit_to_Suit(std::get<CSuit>(payload.trumpCSuit));
  }

  for (const auto& history_entry: payload.hand_history)
  {
    for (const auto& played_ccard: history_entry.card)
    {
      Card _card = CCard_to_Card(played_ccard);
      unknown_cards.erase(_card);
    }
  }

  tngame.m_players = {0, 1, 2, 3};

  PlayAction p_action;
  p_action.action = PlayAction::PlayCCard;

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

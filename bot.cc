#include "bot.hh"

#include <algorithm>

std::ostream &operator<<(std::ostream &os, Card const &card) {
  return os << Card::ToStr(card) << std::endl;
}

std::ostream &operator<<(std::ostream &os, PlayPayload const &payload) {
  os << "Player ID : " << payload.player_id << std::endl;
  os << "Player IDs : ";
  for (auto const &player : payload.player_ids)
    os << player << "    ";

  os << "\nCards \n";
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
    os << "\nCards Played : ";
    for (auto card : x.card)
      os << card << "   ";
    os << std::endl;
  }
  return os;
}

static GameState game_instance;

GameState &GetGameInstance() { return game_instance; }

void InitGameInstance() {
}


Suit GameState::ChooseTrump(PlayerID myid, std::vector<PlayerID> player_ids,
                            std::vector<Card> mycards, int32_t time_remaining,
                            std::vector<BidEntry> bid_history) {
  return Suit(mycards[3].suit);
}

int GameState::Bid(PlayerID myid, std::vector<PlayerID> player_ids,
                   std::vector<Card> mycards, int32_t time_remaining,
                   std::vector<BidEntry> bid_history,
                   BidState const &bid_state) {
  // Either bid or pass
  if (bid_history.empty())
    return 16;
  return 0;
}

PlayAction GameState::Play(PlayPayload payload) {
  std::cout << "Payload Received and parsed: \n" << payload << std::endl;

  if (std::holds_alternative<PlayPayload::RevealedObject>(
          payload.trumpRevealed))
  {
  }

  PlayAction p_action;
  p_action.action = PlayAction::PlayCard;

  if (payload.played.empty()) {
    p_action.played_card = payload.cards[0];
    return p_action;
  }

  Suit lead_suit = payload.played[0].suit;

  auto same_suit_filter = [=](Card card) { return card.suit == lead_suit; };
  std::vector<Card> same_suit_cards;
  std::copy_if(payload.cards.begin(), payload.cards.end(),
               std::back_inserter(same_suit_cards), same_suit_filter);
  if (same_suit_cards.empty()) {
    p_action.played_card = payload.cards[0];
    return p_action;
  }

  p_action.played_card = *same_suit_cards.begin();
  return p_action;
}

#ifndef BOT_H_
#define BOT_H_

#pragma once
#include <cassert>
#include <iostream>
#include <variant>
#include <vector>
#include "card.h"

#define UNREACHABLE() __builtin_unreachable()

enum Rank { SEVEN = 7, EIGHT = 8, NINE, TEN, JACK, QUEEN, KING, ACE };

constexpr int32_t CCardValue(Rank rank) {
  constexpr int32_t card_value[] = {0, 0, 2, 1, 3, 0, 0, 1};
  return card_value[(int32_t)rank - (int32_t)Rank::SEVEN];
}

enum CSuit { CLUBS = 0, DIAMONDS, HEARTS, SPADES };

constexpr const char *CSuitToStr(CSuit suit) {
  switch (suit) {
  case SPADES:
    return "S";
  case DIAMONDS:
    return "D";
  case HEARTS:
    return "H";
  case CLUBS:
    return "C";
  default:
    UNREACHABLE();
  }
}

constexpr CSuit StrToCSuit(const char *suit) {
  switch (*suit) {
  case 'S':
    return SPADES;
  case 'D':
    return DIAMONDS;
  case 'H':
    return HEARTS;
  case 'C':
    return CLUBS;
  default:
    UNREACHABLE();
  }
}

class CCard {

public:
  Rank rank = ACE;
  CSuit suit = SPADES;

  CCard() = default;
  CCard(Rank rank, CSuit suit) : rank(rank), suit(suit) {}
  constexpr bool operator==(const CCard &other) const {
    return rank == other.rank && suit == other.suit;
  }
  constexpr bool operator!=(const CCard &other) const {
    return !(*this == other);
  }

  static std::string ToStr(CCard const &card) {
    std::string name(2, ' ');
    static const char *val = "789TJQK1";
    name[0] = val[(uint32_t)card.rank - (uint32_t)Rank::SEVEN];

    switch (card.suit) {
    case SPADES:
      name[1] = 'S';
      break;
    case DIAMONDS:
      name[1] = 'D';
      break;
    case HEARTS:
      name[1] = 'H';
      break;
    case CLUBS:
      name[1] = 'C';
      break;
    default:
      UNREACHABLE();
    }
    return name;
  }

  static CCard FromStr(const char *str) {
    CSuit suit;
    Rank rank;

    switch (str[1]) {
    case '7':
      rank = SEVEN;
      break;
    case '8':
      rank = EIGHT;
      break;
    case '9':
      rank = NINE;
      break;
    case 'T':
      rank = TEN;
      break;
    case 'J':
      rank = JACK;
      break;
    case 'Q':
      rank = QUEEN;
      break;
    case 'K':
      rank = KING;
      break;
    case '1':
      rank = ACE;
      break;
    default:
      UNREACHABLE();
    }

    switch (str[2]) {
    case 'D':
      suit = DIAMONDS;
      break;
    case 'S':
      suit = SPADES;
      break;
    case 'H':
      suit = HEARTS;
      break;
    case 'C':
      suit = CLUBS;
      break;
    default:
      UNREACHABLE();
    }
    return CCard(rank, suit);
  }
};

using PlayerID = std::string;

struct BidEntry {
  int32_t bid_value;
  PlayerID player_id;
};

struct BidState {
  BidEntry challenger;
  BidEntry defender;
};

struct PlayPayload {

  struct Teams {
    PlayerID players[2];
    int32_t bid;
    int32_t won;
  };

  struct HandHistoryEntry {
    PlayerID initiator;
    PlayerID winner;
    std::vector<CCard>
        card; // card played in that round in chronological order, they say
  };

  struct RevealedObject {
    int32_t hand; // hand at which the trump was revealed
    PlayerID player_id;
  };

  std::variant<bool, CSuit> trumpCSuit = false;
  std::variant<bool, RevealedObject> trumpRevealed = false;

  int32_t remaining_time = 0;
  PlayerID player_id;

  Teams teams[2];

  std::vector<PlayerID> player_ids;
  std::vector<CCard> cards;
  std::vector<CCard> played;
  std::vector<BidEntry> bid_history;

  std::vector<HandHistoryEntry> hand_history;

  PlayPayload() = default;

  PlayPayload(PlayPayload const &) = delete;
  PlayPayload(PlayPayload &&) = default;
  PlayPayload &operator=(PlayPayload const &) = delete;
  PlayPayload &operator=(PlayPayload &&) = default;
};

std::ostream &operator<<(std::ostream &os, PlayPayload const &payload);
std::ostream &operator<<(std::ostream &os, CCard const&);

struct PlayAction {
  enum Action {
    None = 0,
    RevealTrump = 1,
    PlayCCard = 2,
    RevealAndPlay = RevealTrump | PlayCCard
  };

  Action action = PlayCCard;
  CCard played_card;
};

struct GameState {
  // Keep any game related extra metadata here
  // like std::vector<CCard> seen_cards;

  // Time remaining is in milliseconds
  static CSuit ChooseTrump(PlayerID myid, std::vector<PlayerID> player_ids,
                          std::vector<CCard> mycards, int32_t time_remaining,
                          std::vector<BidEntry> bid_history);
  static int32_t Bid(PlayerID myid, std::vector<PlayerID> player_ids,
                     std::vector<CCard> mycards, int32_t time_remaining,
                     std::vector<BidEntry> bid_history,
                     BidState const &bid_state);

  PlayAction Play(PlayPayload payload);
};

GameState &GetGameInstance();
void InitGameInstance();
Card::Suit CSuit_to_Suit(CSuit csuit_in);
Card CCard_to_Card(CCard card_in);
CCard Card_to_CCard(Card card_in);

#endif // BOT_H_

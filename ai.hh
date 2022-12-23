#ifndef AI_H_
#define AI_H_

#include "bitutils.hh"
#include <cstdint>
#include <iostream>
#include <stdint.h>

typedef uint32_t Cards;
typedef uint32_t Card;
typedef uint32_t CardIndex;
typedef uint32_t Suit;

typedef Card Move;
typedef Cards Moves;

typedef uint32_t Player;

// clang-format off

const Cards DECK = UINT32_MAX;

const Suit SPADE = 0b11111111000000000000000000000000;
const Suit HEART = 0b11111111000000000000000000000000 >> 8;
const Suit DIAMOND = 0b11111111000000000000000000000000 >> 16;
const Suit CLUB = 0b11111111000000000000000000000000 >> 24;

const Cards JACKS  = 0b10000000100000001000000010000000;
const Cards NINES  = 0b01000000010000000100000001000000;
const Cards TENS   = 0b00100000001000000010000000100000;
const Cards ACES   = 0b00010000000100000001000000010000;
const Cards KINGS  = 0b00001000000010000000100000001000;
const Cards QUEENS = 0b00000100000001000000010000000100;
const Cards EIGHTS = 0b00000010000000100000001000000010;
const Cards SEVENS = 0b00000001000000010000000100000001;

const char CARD_STRINGS[][3] {
  "7C", "8C", "QC", "KC", "AC", "TC", "NC", "JC",
  "7D", "8D", "QD", "KD", "AD", "TD", "ND", "JD",
  "7H", "8H", "QH", "KH", "AH", "TH", "NH", "JH",
  "7S", "8S", "QS", "KS", "AS", "TS", "NS", "JS"
};

const Suit SUITS[] = {SPADE, HEART, DIAMOND, CLUB};

// clang-format on

inline Cards filter_cards_by_suit(Cards cards_in, Suit suit) {
  return cards_in & suit;
}

inline Suit get_suit(Card card_in) {
  if (card_in & SPADE)
    return SPADE;
  if (card_in & HEART)
    return HEART;
  if (card_in & DIAMOND)
    return DIAMOND;
  if (card_in & CLUB)
    return CLUB;
  return card_in;
}

inline uint32_t get_value(Card card_in) {
  if (card_in & JACKS)
    return 3;
  if (card_in & NINES)
    return 2;
  if (card_in & TENS)
    return 1;
  if (card_in & ACES)
    return 1;
  return 0;
}

void print_cards(Cards cards_in) {
  if (!cards_in) {
    std::cout << "Empty deck!";
    return;
  }
  while (true) {
    int x = bitutils::pop_next_index(cards_in);
    if (x == -1) {
      break;
    }
    std::cout << CARD_STRINGS[x];
    if (x == 0)
      break;
    std::cout << ", ";
  }
}

inline Card pick_card_at_msb(Cards cards_in) { return 1UL << LOG2(cards_in); }

inline Cards get_winning_cards(Cards cards_in_hand, Card card_to_win_over) {
  // assuming that the cards are of the same suit in both the inputs for now
  printf("Card to win over: ");
  print_cards(card_to_win_over);
  printf("\n");
  printf("cards in hand: ");
  print_cards(cards_in_hand);
  printf("\n");
  Cards cards_higher_than_card_to_win_over = ~(card_to_win_over - 1);
  printf("cards higher than card to win over: ");
  print_cards(cards_higher_than_card_to_win_over);
  printf("\n");
  cards_higher_than_card_to_win_over = filter_cards_by_suit(cards_higher_than_card_to_win_over, get_suit(card_to_win_over));
  return cards_in_hand & cards_higher_than_card_to_win_over;
}

inline Cards get_legal_moves(Card first_card, Cards cards_so_far_in_hand,
                             Cards cards_to_choose_move_from, Suit trump_suit,
                             bool has_trump_been_revealed,
                             bool i_am_the_bid_winner) {

  if (cards_so_far_in_hand == 0)
    return cards_to_choose_move_from;

  Suit first_card_suit = get_suit(first_card);

  if (trump_suit == 0) {
    trump_suit = SPADE;
  }

  Card highest_card = (cards_so_far_in_hand & trump_suit);
  if (highest_card == 0) {
    highest_card = (cards_so_far_in_hand & get_suit(first_card));
  }

  printf("Highest card: ");
  print_cards(highest_card);
  printf("\n");

  highest_card = pick_card_at_msb(highest_card);
  Cards cards_to_choose_from_that_are_of_the_same_suit_as_the_first_card =
      get_suit(first_card) & cards_to_choose_move_from;

  if (cards_to_choose_from_that_are_of_the_same_suit_as_the_first_card) {
    printf("Thing: ");
    print_cards(cards_to_choose_from_that_are_of_the_same_suit_as_the_first_card);
    printf("\n");
    Cards winning_cards = get_winning_cards(
        cards_to_choose_from_that_are_of_the_same_suit_as_the_first_card,
        highest_card);
    printf("Winning cards: ");
    print_cards(winning_cards);
    printf("\n");
    if (winning_cards) {
      return winning_cards;
    }
    return cards_to_choose_from_that_are_of_the_same_suit_as_the_first_card;
  }

  Cards cards_to_choose_from_that_are_of_the_trump_suit =
      trump_suit & cards_to_choose_move_from;

  if (cards_to_choose_from_that_are_of_the_trump_suit) {
    Cards winning_cards = get_winning_cards(cards_to_choose_from_that_are_of_the_trump_suit, highest_card);
    if (winning_cards) return winning_cards;
  }
  return cards_to_choose_move_from;
}

class Determinization {
public:
  Cards cards_each_player_has[4];
};

class PartiallyObservedGameNode {
public:
  Player me;
  int32_t hand_id;
  int32_t turn_in_this_hand;
  Player player_to_move;
  Cards cards_i_have_left;
  Cards cards_played_this_hand[4];
  Cards cards_each_players_might_have[4];
  Cards cards_each_players_wont_have[4];
  bool has_trump_been_revealed;
  Suit trump_suit;
  int8_t player_who_revealed_trump;
  Determinization determinization;
};

#endif // AI_H_

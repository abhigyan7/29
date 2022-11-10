#include <bit>
#include <iostream>
#include <stdint.h>

enum COLORS { SPADE, CLUB, DIAMOND, HEART };
enum VALUES { ACE, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING };

typedef uint32_t Card;
typedef uint32_t Cards;

const Cards DECK = 0b11111111111111111111111111111111;

const Cards SPADE_MASK = 0b11111111000000000000000000000000;
const Cards CLUB_MASK = 0b00000000111111110000000000000000;
const Cards DIAMOND_MASK = 0b00000000000000001111111100000000;
const Cards HEART_MASK = 0b11111111000000000000000011111111;

const Cards COLOR_MASKS[] = {SPADE_MASK, CLUB_MASK, DIAMOND_MASK, HEART_MASK};

// TODO check these masks
const Cards CARDS_GREATER_THAN_A = 0b00010100;
const Cards CARDS_GREATER_THAN_7 = 0b10111111;
const Cards CARDS_GREATER_THAN_8 = 0b10011111;
const Cards CARDS_GREATER_THAN_9 = 0b00000100;
const Cards CARDS_GREATER_THAN_T = 0b10010100;
const Cards CARDS_GREATER_THAN_J = 0b00000000;
const Cards CARDS_GREATER_THAN_Q = 0b10010101;
const Cards CARDS_GREATER_THAN_K = 0b10010100;

const Cards CARDS_GREATER_THAN[] = {CARDS_GREATER_THAN_A, CARDS_GREATER_THAN_7,
                                    CARDS_GREATER_THAN_8, CARDS_GREATER_THAN_9,
                                    CARDS_GREATER_THAN_T, CARDS_GREATER_THAN_J,
                                    CARDS_GREATER_THAN_Q, CARDS_GREATER_THAN_K};

Cards deck[32];

void populate_deck() {
  for (size_t i = 0; i < 32; i++) {
    deck[i] = 1 << i;
  }
}

const char CARD_STRINGS[][3] = {
    "AS", "7S", "8S", "9S", "TS", "JS", "QS", "KS", "AC", "7C", "8C",
    "9C", "TC", "JC", "QC", "KC", "AD", "7D", "8D", "9D", "TD", "JD",
    "QD", "KD", "AH", "7H", "8H", "9H", "TH", "JH", "QH", "KH",
};

void print_cards(const Card cards) {
  bool temp;
  for (size_t i = 0; i < 32; i++) {
    temp = (bool)((cards >> i) % 2);
    // std::cout << temp;
    if (temp) {
      std::cout << CARD_STRINGS[i];
      std::cout << ", ";
    }
  }
}

void randomize_deck(const Cards input_deck, Card *output_deck) {
  // TODO implement this
}

int main() {
  populate_deck();
  print_cards(DECK);
  std::cout << std::endl;

  Card randomized_deck[32];
  randomize_deck(DECK, randomized_deck);

  return 0;
}

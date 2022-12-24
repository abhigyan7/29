#include "ai.hh"

int main()
{
    Cards deck = DECK;
    println_cards(deck);

    Card nine_of_spades = SPADE & NINES;
    Card jack_of_spades = SPADE & JACKS;
    Card ten_of_spades = SPADE & TENS;
    Card ten_of_clubs = CLUB & TENS;
    Card king_of_spades = SPADE & KINGS;

    Cards hand = jack_of_spades | ten_of_clubs;
    println_cards(hand);
    println_cards(king_of_spades);

    Cards winning_cards = get_winning_cards(hand, jack_of_spades);
    printf("In legal moves:\n");
    Cards legal_moves = get_legal_moves(king_of_spades, king_of_spades, hand, 0, false, false);
    printf("Legal Moves ");
    println_cards(legal_moves);

    return 0;
}

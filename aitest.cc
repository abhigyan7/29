#include "ai.hh"

int main()
{
    Cards deck = DECK;
    print_cards(deck);
    printf("\n");

    Card nine_of_spades = SPADE & NINES;
    Card jack_of_spades = SPADE & JACKS;
    Card ten_of_spades = SPADE & TENS;
    Card ten_of_clubs = CLUB & TENS;
    Card king_of_spades = SPADE & KINGS;

    Cards hand = jack_of_spades | ten_of_clubs;
    print_cards(hand);
    printf("\n");
    print_cards(king_of_spades);
    printf("\n");

    Cards winning_cards = get_winning_cards(hand, jack_of_spades);
    printf("In legal moves:\n");
    Cards legal_moves = get_legal_moves(king_of_spades, king_of_spades, hand, 0, false, false);
    printf("Legal Moves ");
    print_cards(legal_moves);
    printf("\n");

    return 0;
}

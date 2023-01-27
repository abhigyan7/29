#include "bot.hh"
#include "ismcts_tn.hh"
#include "json.hpp"
#include <vector>

using namespace nlohmann;

void ParseCommon(json const &data, PlayerID &player_id,
                 std::vector<PlayerID> &player_ids, int32_t &remaining_time,
                 std::vector<CCard> &my_cards,
                 std::vector<BidEntry> &bid_history) {
  player_id = data["playerId"].get<PlayerID>();

  auto player_id_iter = data["playerIds"];
  for (auto const &id : player_id_iter)
    player_ids.push_back(id.get<PlayerID>());

  remaining_time = data["timeRemaining"].get<int32_t>();

  auto const &player_cards_iter = data["cards"];
  for (auto const &card : player_cards_iter)
    my_cards.push_back(CCard::FromStr(card.dump().c_str()));

  // Bid history is in the form of pair
  auto const &history_iter = data["bidHistory"];
  for (auto const &history : history_iter) {
    // Read first entry as playerID and second as int32
    PlayerID id = (*history.begin()).get<PlayerID>();
    int32_t bid_value = (*(history.begin() + 1)).get<int32_t>();
    bid_history.push_back(BidEntry{bid_value, id});
  }
}

int main(int argc, char** argv)
{
    TwentyNine tngame = TwentyNine();

        json data = json::parse(argv[1]);
        PlayPayload payload;
        ParseCommon(data, payload.player_id, payload.player_ids,
                    payload.remaining_time, payload.cards, payload.bid_history);

        auto &trump_suit = data["trumpSuit"];
        if (trump_suit.is_boolean())
          payload.trumpCSuit = trump_suit.get<bool>();
        else
          payload.trumpCSuit = StrToCSuit(trump_suit.dump().c_str() + 1);

        std::cout << "trump suit in payload: " << trump_suit.dump() << std::endl;

        auto &trump_reveal = data["trumpRevealed"];

        if (trump_reveal.is_boolean())
          payload.trumpRevealed = data["trumpRevealed"].get<bool>();
        else {
          // Parse as object
          auto iter = trump_reveal.begin();
          PlayPayload::RevealedObject object;
          object.hand = (*iter++).get<int32_t>();
          object.player_id = (*iter++).get<PlayerID>();
          payload.trumpRevealed = object;
        }

        auto const &player_cards_iter = data["played"];
        for (auto const &card : player_cards_iter)
          payload.played.push_back(CCard::FromStr(card.dump().c_str()));

        // Teams : teammate, win and bid
        auto const &teams_data = data["teams"];
        auto teams_iter = teams_data.begin();
        decltype(teams_iter) teams[] = {teams_iter, teams_iter + 1};
        for (uint32_t i = 0; i < 2; ++i) {
          // Nlohmann json will do the parsing for us
          payload.teams[i].won = (*teams[i])["won"].get<int32_t>();
          payload.teams[i].bid = (*teams[i])["bid"].get<int32_t>();

          auto const &teammates = (*teams[i])["players"];

          for (uint32_t j = 0; j < 2; ++j)
            payload.teams[i].players[j] =
                (*(teammates.begin() + j)).get<PlayerID>();
        }

        // history of every hand played before
        auto const &hands_history_iter = data["handsHistory"];
        for (auto const &hands : hands_history_iter) {
          // first is the initiator, third is the winner
          PlayPayload::HandHistoryEntry entry;
          entry.initiator = (*hands.begin()).get<PlayerID>();
          entry.winner = (*(hands.begin() + 2)).get<PlayerID>();

          // second is the card played in that round
          auto const &cards = *(hands.begin() + 1);
          for (auto const &card : cards)
            entry.card.push_back(CCard::FromStr(card.dump().c_str()));
          payload.hand_history.push_back(std::move(entry));
        }
    tngame.parse_playpayload(payload);
    std::cout << tngame.validMoves()[0] << std::endl;
    return 0;
}

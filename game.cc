#include "crow_all.h"
#include "bot.hh"
#include "json.hpp"

#include <iostream>
#include <string>
#include <utility>
#include <vector>

using json = nlohmann::json;

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

int main(int argc, char **argv) {

  crow::App<crow::CORSHandler> app;

  CROW_ROUTE(app, "/hi")([](){ return "{\"value\": \"hello\"}";});

  CROW_ROUTE(app, "/bid").methods(crow::HTTPMethod::Post)([](const crow::request &req) {
    json data = json::parse(req.body);

    int32_t remaining_time;
    PlayerID myid;
    std::vector<PlayerID> player_ids;
    player_ids.reserve(4);
    std::vector<CCard> my_cards;
    std::vector<BidEntry> bid_history;

    ParseCommon(data, myid, player_ids, remaining_time, my_cards, bid_history);

    // Parse challenger and defender
    BidState bid_state;
    auto const &state = data["bidState"];
    bid_state.defender.player_id = state["defenderId"].get<PlayerID>();
    bid_state.challenger.player_id = state["challengerId"].get<PlayerID>();
    bid_state.defender.bid_value = state["defenderBid"].get<int32_t>();
    bid_state.challenger.bid_value = state["challengerBid"].get<int32_t>();

    json send;
    send["bid"] =
        GameState::Bid(myid, std::move(player_ids), std::move(my_cards),
                       remaining_time, std::move(bid_history), bid_state);
    return crow::response(send.dump());
  });

  CROW_ROUTE(app, "/chooseTrump").methods(crow::HTTPMethod::Post)([](const crow::request &req) {

    json data = json::parse(req.body);

    int32_t remaining_time;
    PlayerID myid;
    std::vector<PlayerID> player_ids;
    player_ids.reserve(4);
    std::vector<CCard> my_cards;
    std::vector<BidEntry> bid_history;

    ParseCommon(data, myid, player_ids, remaining_time, my_cards, bid_history);

    json send;
    send["suit"] = CSuitToStr(
        GameState::ChooseTrump(myid, std::move(player_ids), std::move(my_cards),
                               remaining_time, std::move(bid_history)));
    return crow::response(send.dump());
  });

  CROW_ROUTE(app, "/play").methods(crow::HTTPMethod::Post)([](const crow::request &req) {
        json data = json::parse(req.body);
        PlayPayload payload;
        ParseCommon(data, payload.player_id, payload.player_ids,
                    payload.remaining_time, payload.cards, payload.bid_history);

        auto &trump_suit = data["trumpCSuit"];
        if (trump_suit.is_boolean())
          payload.trumpCSuit = trump_suit.get<bool>();
        else
          payload.trumpCSuit = StrToCSuit(trump_suit.dump().c_str() + 1);

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

        json send;
        auto play_action = GetGameInstance().Play(std::move(payload));
        if (play_action.action & PlayAction::RevealTrump)
          send["revealTrump"] = true;
        if (play_action.action & PlayAction::PlayCCard)
          send["card"] = CCard::ToStr(play_action.played_card);

        return crow::response(send.dump());
      });

  app.port(8001).loglevel(crow::LogLevel::Error).run();
  return 0;
}

# 29

Bots that play the card game 29. 

### TODOs
- [X] Incorporate better bidding strategy.
- [ ] MCTS correctness.
  - [X] cloneAndRandomize
  - [X] currentPlayer
  - [X] players
  - [ ] validMoves
    - [ ] Generate pairs and test against them
  - [ ] doMove
  - [ ] getResult
  - [ ] finishRound
  - [ ] finishTrick
  - [ ] calcPointsInTrick
  - [ ] roundWinner
  - [ ] trickWinner
- [ ] A and 10 arent the same value, make sure that's accounted for
- [ ] Domain restriction for cards based on what a player has played in history
- [X] Finish get_win_data 
- [ ] Parse and store total information
  - [X] Things like who revealed the trump and in which hand/round
  - [ ] Which suit i chose as trump

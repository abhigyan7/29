# 29

Bots that play the card game 29. 

### TODOs
- [X] Incorporate better bidding strategy.
- [ ] MCTS correctness.
  - [X] cloneAndRandomize
  - [X] currentPlayer
  - [X] players
  - [X] validMoves
    - [ ] Generate pairs and test against them
  - [X] doMove
  - [X] getResult
  - [X] finishRound
  - [X] finishTrick
  - [X] calcPointsInTrick
  - [X] roundWinner
  - [X] trickWinner
- [X] A and 10 arent the same value, make sure that's accounted for
- [X] Finish get_win_data 
- [X] Parse and store total information
  - [X] Things like who revealed the trump and in which hand/round
  - [X] Which suit i chose as trump
- [X] Make sure the legal moves gen around trump reveal and things like that is correct
- [X] If there is a single legal move, play that
- [X] Solve the post trump reveal K wins Q issue 29jan
- [X] Separate values and points
- [ ] Domain restriction for cards based on what a player has played in history

# Terminal Chess

This is a project I developed over the course of 2 weeks in late January/early February of 2024.
I originally wrote it inside a single c file, with the goals of: 
- minimizing lines of code/being efficient
- maximizing space efficiency
- implementing the rules of chess correctly

This project was meant to be a fun challenge for myself at the time.
I had basic knowledge of c, little knowledge of OOP, a good understanding of chess rules, and no idea where to start.
It's not representative of my current skills, since if I were to redo it now I would:
- Focus less on space efficiency and more on code readability/documentation
- Split functionality into different header files
- Adhere more to OOP principles

## Features

### Parsing & IO

- Loading a position from a [FEN string](https://en.wikipedia.org/wiki/Forsyth–Edwards_Notation).
- Parsing moves from [Algebraic notation](https://en.wikipedia.org/wiki/Algebraic_notation_(chess)).
  - A trailing + or # is ignored, so it is not necessary to know beforehand that a move was a check.
- If the given move was illegal, the program reprompts the user for another move.
- If either player enters "export", the current game position is printed as a [FEN string](https://en.wikipedia.org/wiki/Forsyth–Edwards_Notation).
- A game can be recorded to generate a [PGN](https://en.wikipedia.org/wiki/Portable_Game_Notation) file after tha game has ended.

### Gameplay

- A game of chess is played between 2 players.
- The game is always printed from white's perspective.
- The program ends when the game ends.

#### Ending the game

The game ends if any of the following occur:
  -  A player checkmates the other player.
  -  A player resigns by entering "resign".
  -  The game is drawn by entering "draw".
  -  The game is drawn by stalemate.
  -  The game is drawn by the [50 Move Rule](https://en.wikipedia.org/wiki/Fifty-move_rule).
  -  The game is drawn by repetition. NOTE: [Castling rights](https://en.wikipedia.org/wiki/Castling) are not taken into account.
  -  The game is drawn because there is insufficient material on the board for either player to checkmate the other player.

## Requirements/Compiling

There is a single c file.
It should compile with any compiler that supports at least c99.
The terminal in which you run the program should support unicode characters.

# Future plans

I plan to refactor this project in the near future, and maybe add some more features like a single-player mode against a chess engine.

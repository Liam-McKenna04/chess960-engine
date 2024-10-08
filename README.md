# Chess Game

This is a chess game implementation using C++ and SFML graphics library. It features a graphical user interface, player vs. player, and player vs. AI gameplay modes, including support for Chess960 (Fischer Random Chess).

## How to Run

1. Ensure you have SFML installed on your system.
2. Compile the game using the provided Makefile:
   ```
   make
   ```
3. Run the compiled executable:
   ```
   ./chess
   ```

## Gameplay

- On startup, you'll see a menu where you can choose player types for White and Black (Human or AI).
- You can also select the AI type: Random or Basic.
- There's an option to play standard chess or Chess960 (Fischer Random Chess).
- Click "Start Game" to begin.
- Click and drag pieces to make moves.
- The game will automatically detect checkmate, stalemate, and other draw conditions.

## Key Algorithms

1. **Move Generation**: The game uses a bitboard representation to efficiently generate all legal moves for each piece type.

2. **Check Detection**: Utilizes bitwise operations to quickly determine if a king is in check.

3. **Checkmate and Stalemate Detection**: After each move, the game checks if the opponent has any legal moves. If not, it's either checkmate (if the king is in check) or stalemate.

4. **AI Engines**:
   - Random Engine: Selects a random legal move.
   - Basic Engine: Uses a simple evaluation function and alpha beta pruning for move searching. Also uses transposition tables to not have to compute positions that are the same.

5. **Board Representation**: I use a hybrid approach with both a piece-centric (array) and square-centric (bitboard) representation for efficient move generation and board evaluation.

6. **FEN Parsing**: Implements Forsyth–Edwards Notation (FEN) parsing and chess960 game seeds to set up custom board positions.

7. **Chess960 Setup**: Implements the rules for generating a valid Chess960 starting position, ensuring correct piece placement. 

## Future Improvements

- Implement a more advanced AI using techniques like iterative deepening.
- Implement a game clock for timed matches.

## Dependencies

- SFML 2.5 or later
- C++17 compatible compiler

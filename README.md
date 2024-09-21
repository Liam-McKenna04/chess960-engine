# Chess Game

This is a chess game implementation using C++ and SFML graphics library. It features a graphical user interface, player vs. player, and player vs. AI gameplay modes.

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
- Click "Start Game" to begin.
- Click and drag pieces to make moves.
- The game will automatically detect checkmate, stalemate, and other draw conditions.

## Key Algorithms

1. **Move Generation**: The game uses a bitboard representation to efficiently generate all legal moves for each piece type.

2. **Check Detection**: Utilizes bitwise operations to quickly determine if a king is in check.

3. **Checkmate and Stalemate Detection**: After each move, the game checks if the opponent has any legal moves. If not, it's either checkmate (if the king is in check) or stalemate.

4. **AI Engines**:
   - Random Engine: Selects a random legal move.
   - Basic Engine: Uses a simple evaluation function to score board positions and selects the best immediate move (depth-1 search).

5. **Board Representation**: Uses a hybrid approach with both a piece-centric (array) and square-centric (bitboard) representation for efficient move generation and board evaluation.

6. **FEN Parsing**: Implements Forsyth–Edwards Notation (FEN) parsing to set up custom board positions.

## Future Improvements

- Implement a more advanced AI using techniques like alpha-beta pruning and iterative deepening.
- Add support for special rules like en passant and castling (if not already implemented).
- Implement a game clock for timed matches.

## Dependencies

- SFML 2.5 or later
- C++17 compatible compiler

## License

This project is open-source and available under the MIT License.
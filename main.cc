#include <SFML/Graphics.hpp>

#include "Piece.hpp"
const int BOARD_SIZE = 8;
const int SQUARE_SIZE = 80;

int size = 133;

int board[64] = {Piece::create(Piece::Rook, Piece::Black),
                 Piece::create(Piece::Knight, Piece::Black),
                 Piece::create(Piece::Bishop, Piece::Black),
                 Piece::create(Piece::Queen, Piece::Black),
                 Piece::create(Piece::King, Piece::Black),
                 Piece::create(Piece::Bishop, Piece::Black),
                 Piece::create(Piece::Knight, Piece::Black),
                 Piece::create(Piece::Rook, Piece::Black),
                 Piece::create(Piece::Pawn, Piece::Black),
                 Piece::create(Piece::Pawn, Piece::Black),
                 Piece::create(Piece::Pawn, Piece::Black),
                 Piece::create(Piece::Pawn, Piece::Black),
                 Piece::create(Piece::Pawn, Piece::Black),
                 Piece::create(Piece::Pawn, Piece::Black),
                 Piece::create(Piece::Pawn, Piece::Black),
                 Piece::create(Piece::Pawn, Piece::Black),
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::Empty,
                 Piece::create(Piece::Pawn, Piece::White),
                 Piece::create(Piece::Pawn, Piece::White),
                 Piece::create(Piece::Pawn, Piece::White),
                 Piece::create(Piece::Pawn, Piece::White),
                 Piece::create(Piece::Pawn, Piece::White),
                 Piece::create(Piece::Pawn, Piece::White),
                 Piece::create(Piece::Pawn, Piece::White),
                 Piece::create(Piece::Pawn, Piece::White),
                 Piece::create(Piece::Rook, Piece::White),
                 Piece::create(Piece::Knight, Piece::White),
                 Piece::create(Piece::Bishop, Piece::White),
                 Piece::create(Piece::Queen, Piece::White),
                 Piece::create(Piece::King, Piece::White),
                 Piece::create(Piece::Bishop, Piece::White),
                 Piece::create(Piece::Knight, Piece::White),
                 Piece::create(Piece::Rook, Piece::White)};
int main() {
    sf::RenderWindow window(
        sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE),
        "Chess Board");

    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));

    // Load Pieces
    sf::Texture pieces;
    pieces.loadFromFile("assets/Chess_Pieces_Sprite.png");
    sf::Sprite s(pieces);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        window.clear(sf::Color::White);

        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                square.setPosition(i * SQUARE_SIZE, j * SQUARE_SIZE);

                if ((i + j) % 2 == 0) {
                    square.setFillColor(
                        sf::Color(238, 238, 210));  // Light squares
                } else {
                    square.setFillColor(
                        sf::Color(118, 150, 86));  // Dark squares
                }

                window.draw(square);

                // Draw chess pieces
                int piece = board[j * 8 + i];
                if (piece != 0) {
                    int x = abs(piece) - 1;
                    int y = (piece > 0) ? 1 : 0;
                    s.setTextureRect(
                        sf::IntRect(x * size, y * size, size, size));
                    s.setScale(0.6, 0.6);
                    s.setPosition(i * SQUARE_SIZE, j * SQUARE_SIZE);
                    window.draw(s);
                }
            }
        }

        window.display();
    }

    return 0;
}
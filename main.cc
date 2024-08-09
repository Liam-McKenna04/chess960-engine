#include <SFML/Graphics.hpp>
#include <iostream>

#include "Piece.hpp"
const int BOARD_SIZE = 8;
const int SQUARE_SIZE = 80;
sf::Vector2f offset(0, 0);

int size = 133;

int board[64];

int main() {
    sf::RenderWindow window(
        sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE),
        "Chess Board");

    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));

    // Load Pieces
    sf::Texture pieces;
    pieces.loadFromFile("assets/Chess_Pieces_Sprite.png");
    sf::Sprite s(pieces);

    Piece::EPDToBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", board);

    while (window.isOpen()) {
        sf::Vector2i pos =
            sf::Mouse::getPosition(window) - sf::Vector2i(offset);

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            /////drag and drop///////
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int boxX = (pos.x / SQUARE_SIZE) + 1;
                    int boxY = (pos.y / SQUARE_SIZE) + 1;

                    std::cout << "x: " << boxX << ", y: " << boxY << std::endl;
                }
            }

            // if (event.type == sf::Event::MouseButtonReleased)
            //     if (event.key.code == sf::Mouse::Left) {
            //         isMove = false;
            //         Vector2f p =
            //             f[n].getPosition() + Vector2f(size / 2, size / 2);
            //         newPos = Vector2f(size * int(p.x / size),
            //                           size * int(p.y / size));
            //         str = toChessNote(oldPos) + toChessNote(newPos);
            //         move(str);
            //         if (oldPos != newPos) position += str + " ";
            //         f[n].setPosition(newPos);
            //     }
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
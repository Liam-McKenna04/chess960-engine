#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>

#include "Board.hpp"

const int BOARD_SIZE = 8;
const int SQUARE_SIZE = 80;
sf::Vector2f offset(0, 0);

const int BOX_SIZE = 133;

bool isInsideBoard(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

int main() {
    sf::RenderWindow window(
        sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE),
        "Chess Board");

    sf::RectangleShape square(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));

    // Load Pieces
    sf::Texture pieces;
    pieces.loadFromFile("assets/Chess_Pieces_Sprite.png");
    sf::Sprite s(pieces);

    Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    board.generateMoves();

    bool isMoving = false;
    int movingPiece = 0;
    sf::Vector2i movingPieceOrigin;
    sf::Vector2i clickedSquare(-1, -1);

    std::vector<int> targetSquares;

    sf::CircleShape targetIndicator(SQUARE_SIZE / 6);
    targetIndicator.setFillColor(sf::Color(128, 128, 128, 128));

    while (window.isOpen()) {
        sf::Vector2i mousePos =
            sf::Mouse::getPosition(window) - sf::Vector2i(offset);

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int boxX = (mousePos.x / SQUARE_SIZE);
                    int boxY = (mousePos.y / SQUARE_SIZE);
                    int startSquare = boxY * 8 + boxX;
                    if (isInsideBoard(boxX, boxY)) {
                        clickedSquare = sf::Vector2i(boxX, boxY);

                        if (!isMoving && board.square[startSquare] != 0) {
                            targetSquares.clear();

                            for (Move move : board.moves) {
                                if (move.startSquare == startSquare) {
                                    targetSquares.push_back(move.targetSquare);
                                }
                            }

                            isMoving = true;
                            movingPiece = board.square[startSquare];
                            movingPieceOrigin = sf::Vector2i(boxX, boxY);
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left && isMoving) {
                    int boxX = (mousePos.x / SQUARE_SIZE);
                    int boxY = (mousePos.y / SQUARE_SIZE);
                    int targetSquare = boxY * 8 + boxX;

                    if (isInsideBoard(boxX, boxY) &&
                        (movingPieceOrigin.y * 8 + movingPieceOrigin.x) !=
                            targetSquare &&
                        std::find(targetSquares.begin(), targetSquares.end(),
                                  targetSquare) != targetSquares.end()) {
                        // Find the corresponding move in board.moves
                        Move* selectedMove = nullptr;
                        for (const Move& move : board.moves) {
                            if (move.startSquare == (movingPieceOrigin.y * 8 +
                                                     movingPieceOrigin.x) &&
                                move.targetSquare == targetSquare) {
                                selectedMove = new Move(move);
                                break;
                            }
                        }

                        if (selectedMove) {
                            board.makeMove(*selectedMove);
                            delete selectedMove;
                        }
                    }
                    isMoving = false;
                    clickedSquare = sf::Vector2i(-1, -1);
                    targetSquares.clear();
                }
            }
        }

        window.clear(sf::Color::White);
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                square.setPosition(i * SQUARE_SIZE, j * SQUARE_SIZE);

                if (clickedSquare.x == i && clickedSquare.y == j) {
                    square.setFillColor(sf::Color(236, 126, 106));
                } else if (board.isLastMoveTile(j * 8 + i)) {
                    square.setFillColor(sf::Color(
                        255, 255, 0, 128));  // Light yellow for last move
                } else if ((i + j) % 2 == 0) {
                    square.setFillColor(
                        sf::Color(238, 238, 210));  // Light squares
                } else {
                    square.setFillColor(
                        sf::Color(118, 150, 86));  // Dark squares
                }

                window.draw(square);

                // Draw chess pieces
                int piece = board.square[j * 8 + i];
                if (piece != 0 && (!isMoving || (i != movingPieceOrigin.x ||
                                                 j != movingPieceOrigin.y))) {
                    int x = abs(piece) - 1;
                    int y = (piece > 0) ? 0 : 1;
                    s.setTextureRect(sf::IntRect(x * BOX_SIZE, y * BOX_SIZE,
                                                 BOX_SIZE, BOX_SIZE));
                    s.setScale(0.6, 0.6);
                    s.setPosition(i * SQUARE_SIZE, j * SQUARE_SIZE);
                    window.draw(s);
                }

                // Draw target indicators
                if (std::find(targetSquares.begin(), targetSquares.end(),
                              j * 8 + i) != targetSquares.end()) {
                    targetIndicator.setPosition(
                        i * SQUARE_SIZE + SQUARE_SIZE / 2 - SQUARE_SIZE / 6,
                        j * SQUARE_SIZE + SQUARE_SIZE / 2 - SQUARE_SIZE / 6);
                    window.draw(targetIndicator);
                }
            }
        }

        // Draw moving piece
        if (isMoving) {
            int x = abs(movingPiece) - 1;
            int y = (movingPiece > 0) ? 0 : 1;
            s.setTextureRect(
                sf::IntRect(x * BOX_SIZE, y * BOX_SIZE, BOX_SIZE, BOX_SIZE));
            s.setScale(0.6, 0.6);
            s.setPosition(mousePos.x - SQUARE_SIZE / 2,
                          mousePos.y - SQUARE_SIZE / 2);
            window.draw(s);
        }

        window.display();
    }

    return 0;
}

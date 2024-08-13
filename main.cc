#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include <memory>

#include "Board.hpp"

const int BOARD_SIZE = 8;
const int SQUARE_SIZE = 80;
const int PIECE_SPRITE_SIZE = 133;

bool isInsideBoard(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

class ChessGame {
   private:
    sf::RenderWindow window;
    sf::Texture piecesTexture;
    sf::Sprite pieceSprite;
    sf::RectangleShape square;
    sf::CircleShape targetIndicator;
    sf::RectangleShape promotionBackground;

    std::unique_ptr<Board> board;
    bool isMoving = false;
    int movingPiece = 0;
    sf::Vector2i movingPieceOrigin;
    sf::Vector2i clickedSquare{-1, -1};
    std::vector<int> targetSquares;

    bool showPromotionInterface = false;
    std::unique_ptr<Move> pendingPromotion;

    void loadAssets() {
        if (!piecesTexture.loadFromFile("assets/Chess_Pieces_Sprite.png")) {
            throw std::runtime_error("Failed to load piece sprite");
        }
        pieceSprite.setTexture(piecesTexture);
        pieceSprite.setScale(0.6f, 0.6f);

        square.setSize(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));

        targetIndicator.setRadius(SQUARE_SIZE / 6);
        targetIndicator.setFillColor(sf::Color(128, 128, 128, 128));

        promotionBackground.setSize(sf::Vector2f(SQUARE_SIZE * 4, SQUARE_SIZE));
        promotionBackground.setFillColor(sf::Color(200, 200, 200));
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                handleMousePress(event);
            } else if (event.type == sf::Event::MouseButtonReleased) {
                handleMouseRelease(event);
            }
        }
    }

    void handleMousePress(const sf::Event& event) {
        if (event.mouseButton.button != sf::Mouse::Left) return;

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        int boxX = mousePos.x / SQUARE_SIZE;
        int boxY = mousePos.y / SQUARE_SIZE;

        if (showPromotionInterface) {
            handlePromotionSelection(mousePos);
        } else if (isInsideBoard(boxX, boxY)) {
            handlePieceSelection(boxX, boxY);
        }
    }

    void handlePromotionSelection(const sf::Vector2i& mousePos) {
        if (mousePos.y >= 3 * SQUARE_SIZE && mousePos.y < 4 * SQUARE_SIZE &&
            mousePos.x >= 2 * SQUARE_SIZE && mousePos.x < 6 * SQUARE_SIZE) {
            int selectedPiece = (mousePos.x - 2 * SQUARE_SIZE) / SQUARE_SIZE;
            const int promotionPieces[4] = {Piece::Queen, Piece::Bishop,
                                            Piece::Knight, Piece::Rook};
            pendingPromotion->promotionPiece =
                promotionPieces[selectedPiece] * board->colorTurn;
            board->makeMove(*pendingPromotion);
            pendingPromotion.reset();
            showPromotionInterface = false;
        }
    }

    void handlePieceSelection(int boxX, int boxY) {
        clickedSquare = sf::Vector2i(boxX, boxY);
        int startSquare = boxY * 8 + boxX;

        if (!isMoving && board->square[startSquare] != 0) {
            targetSquares.clear();
            for (const Move& move : board->moves) {
                if (move.startSquare == startSquare) {
                    targetSquares.push_back(move.targetSquare);
                }
            }
            isMoving = true;
            movingPiece = board->square[startSquare];
            movingPieceOrigin = sf::Vector2i(boxX, boxY);
        }
    }

    void handleMouseRelease(const sf::Event& event) {
        if (event.mouseButton.button != sf::Mouse::Left || !isMoving) return;

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        int boxX = mousePos.x / SQUARE_SIZE;
        int boxY = mousePos.y / SQUARE_SIZE;
        int targetSquare = boxY * 8 + boxX;

        if (isInsideBoard(boxX, boxY) &&
            (movingPieceOrigin.y * 8 + movingPieceOrigin.x) != targetSquare &&
            std::find(targetSquares.begin(), targetSquares.end(),
                      targetSquare) != targetSquares.end()) {
            auto it = std::find_if(board->moves.begin(), board->moves.end(),
                                   [&](const Move& move) {
                                       return move.startSquare ==
                                                  (movingPieceOrigin.y * 8 +
                                                   movingPieceOrigin.x) &&
                                              move.targetSquare == targetSquare;
                                   });

            if (it != board->moves.end()) {
                if (it->promotionPiece != 0) {
                    showPromotionInterface = true;
                    pendingPromotion.reset(new Move(*it));
                } else {
                    board->makeMove(*it);
                }
            }
        }

        isMoving = false;
        clickedSquare = sf::Vector2i(-1, -1);
        targetSquares.clear();
    }

    void drawBoard() {
        for (int i = 0; i < BOARD_SIZE; ++i) {
            for (int j = 0; j < BOARD_SIZE; ++j) {
                square.setPosition(i * SQUARE_SIZE, j * SQUARE_SIZE);
                square.setFillColor(getBoardSquareColor(i, j));
                window.draw(square);

                drawPiece(i, j);
                drawTargetIndicator(i, j);
            }
        }
    }

    sf::Color getBoardSquareColor(int i, int j) {
        if (clickedSquare.x == i && clickedSquare.y == j) {
            return sf::Color(236, 126, 106);
        } else if (board->isLastMoveTile(j * 8 + i)) {
            return sf::Color(255, 255, 0, 128);
        } else if ((i + j) % 2 == 0) {
            return sf::Color(238, 238, 210);
        } else {
            return sf::Color(118, 150, 86);
        }
    }

    void drawPiece(int i, int j) {
        int piece = board->square[j * 8 + i];
        if (piece != 0 && (!isMoving || (i != movingPieceOrigin.x ||
                                         j != movingPieceOrigin.y))) {
            int x = abs(piece) - 1;
            int y = (piece > 0) ? 0 : 1;
            pieceSprite.setTextureRect(
                sf::IntRect(x * PIECE_SPRITE_SIZE, y * PIECE_SPRITE_SIZE,
                            PIECE_SPRITE_SIZE, PIECE_SPRITE_SIZE));
            pieceSprite.setPosition(i * SQUARE_SIZE, j * SQUARE_SIZE);
            window.draw(pieceSprite);
        }
    }

    void drawTargetIndicator(int i, int j) {
        if (std::find(targetSquares.begin(), targetSquares.end(), j * 8 + i) !=
            targetSquares.end()) {
            targetIndicator.setPosition(
                i * SQUARE_SIZE + SQUARE_SIZE / 2 - SQUARE_SIZE / 6,
                j * SQUARE_SIZE + SQUARE_SIZE / 2 - SQUARE_SIZE / 6);
            window.draw(targetIndicator);
        }
    }

    void drawMovingPiece() {
        if (isMoving) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            int x = abs(movingPiece) - 1;
            int y = (movingPiece > 0) ? 0 : 1;
            pieceSprite.setTextureRect(
                sf::IntRect(x * PIECE_SPRITE_SIZE, y * PIECE_SPRITE_SIZE,
                            PIECE_SPRITE_SIZE, PIECE_SPRITE_SIZE));
            pieceSprite.setPosition(mousePos.x - SQUARE_SIZE / 2,
                                    mousePos.y - SQUARE_SIZE / 2);
            window.draw(pieceSprite);
        }
    }

    void drawPromotionInterface() {
        if (!showPromotionInterface) return;

        promotionBackground.setPosition(2 * SQUARE_SIZE, 3 * SQUARE_SIZE);
        window.draw(promotionBackground);

        int color = (board->colorTurn == 1) ? 0 : 1;
        for (int i = 0; i < 4; ++i) {
            int piece = i + 1;  // Queen, Rook, Bishop, Knight
            pieceSprite.setTextureRect(sf::IntRect(
                piece * PIECE_SPRITE_SIZE, color * PIECE_SPRITE_SIZE,
                PIECE_SPRITE_SIZE, PIECE_SPRITE_SIZE));
            pieceSprite.setPosition((2 + i) * SQUARE_SIZE, 3 * SQUARE_SIZE);
            window.draw(pieceSprite);
        }
    }

   public:
    ChessGame()
        : window(
              sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE),
              "Chess Board") {
        loadAssets();
        board.reset(new Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"));
        board->generateMoves();
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();

            window.clear(sf::Color::White);
            drawBoard();
            drawMovingPiece();
            drawPromotionInterface();
            window.display();
        }
    }
};

int main() {
    try {
        ChessGame game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
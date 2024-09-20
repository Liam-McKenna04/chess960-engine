#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>

#include "Board.hpp"
#include "Piece.hpp"
#include "RandomEngine.hpp"
#include "BasicEngine.hpp"

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
    sf::Font font;
    sf::Text menuText;
    sf::RectangleShape startButton;
    sf::Text startButtonText;
    bool showMenu = true;

    std::unique_ptr<Board> board;
    bool isMoving = false;
    bool gameEnded = true;
    std::string winner = "";
    int movingPieceIndex = -1;
    sf::Vector2i movingPieceOrigin;
    sf::Vector2i clickedSquare{-1, -1};
    std::vector<int> targetSquares;

    bool showPromotionInterface = false;
    std::optional<Move> pendingPromotion;

    std::unique_ptr<Engine> whiteEngine;
    std::unique_ptr<Engine> blackEngine;

    sf::Text whiteSelectionText;
    sf::Text blackSelectionText;
    sf::RectangleShape whitePlayerButton;
    sf::RectangleShape whiteEngineButton;
    sf::RectangleShape blackPlayerButton;
    sf::RectangleShape blackEngineButton;
    sf::Text whitePlayerText;
    sf::Text whiteEngineText;
    sf::Text blackPlayerText;
    sf::Text blackEngineText;
    bool whiteIsEngine = false;
    bool blackIsEngine = true;

    bool isDraw = false;

    sf::Text engineSelectionText;
    sf::RectangleShape randomEngineButton;
    sf::RectangleShape basicEngineButton;
    sf::Text randomEngineText;
    sf::Text basicEngineText;
    bool useRandomEngine = true;

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

        if (!font.loadFromFile("assets/Lato-Regular.ttf")) {
            throw std::runtime_error("Failed to load font");
        }
        menuText.setFont(font);
        menuText.setCharacterSize(24);
        menuText.setFillColor(sf::Color::Black);

        startButton.setSize(sf::Vector2f(200, 50));
        startButton.setFillColor(sf::Color(100, 100, 100));
        startButtonText.setFont(font);
        startButtonText.setString("Start Game");
        startButtonText.setCharacterSize(20);
        startButtonText.setFillColor(sf::Color::White);

        // Setup new text and buttons
        whiteSelectionText.setFont(font);
        whiteSelectionText.setCharacterSize(20);
        whiteSelectionText.setFillColor(sf::Color::Black);
        whiteSelectionText.setString("White:");

        blackSelectionText.setFont(font);
        blackSelectionText.setCharacterSize(20);
        blackSelectionText.setFillColor(sf::Color::Black);
        blackSelectionText.setString("Black:");

        sf::Vector2f buttonSize(100, 40);
        whitePlayerButton.setSize(buttonSize);
        whiteEngineButton.setSize(buttonSize);
        blackPlayerButton.setSize(buttonSize);
        blackEngineButton.setSize(buttonSize);

        whitePlayerText.setFont(font);
        whitePlayerText.setCharacterSize(16);
        whitePlayerText.setString("Player");

        whiteEngineText.setFont(font);
        whiteEngineText.setCharacterSize(16);
        whiteEngineText.setString("Engine");

        blackPlayerText.setFont(font);
        blackPlayerText.setCharacterSize(16);
        blackPlayerText.setString("Player");

        blackEngineText.setFont(font);
        blackEngineText.setCharacterSize(16);
        blackEngineText.setString("Engine");

        // Setup engine selection text and buttons
        engineSelectionText.setFont(font);
        engineSelectionText.setCharacterSize(20);
        engineSelectionText.setFillColor(sf::Color::Black);
        engineSelectionText.setString("Engine:");

        randomEngineButton.setSize(buttonSize);
        basicEngineButton.setSize(buttonSize);

        randomEngineText.setFont(font);
        randomEngineText.setCharacterSize(16);
        randomEngineText.setString("Random");

        basicEngineText.setFont(font);
        basicEngineText.setCharacterSize(16);
        basicEngineText.setString("Basic");
    }

    void handleEvents() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (showMenu) {
                    handleMenuClick(event);
                } else {
                    handleMousePress(event);
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (!showMenu) {
                    handleMouseRelease(event);
                }
            }
        }
    }

    void handleMenuClick(const sf::Event& event) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        if (startButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            showMenu = false;
            resetBoard();
        } else if (whitePlayerButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            whiteIsEngine = false;
        } else if (whiteEngineButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            whiteIsEngine = true;
        } else if (blackPlayerButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            blackIsEngine = false;
        } else if (blackEngineButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            blackIsEngine = true;
        } else if (randomEngineButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            useRandomEngine = true;
        } else if (basicEngineButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
            useRandomEngine = false;
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
            int promotionPieceTypes[4] = {
                BasePieceType::Queen, BasePieceType::Rook,
                BasePieceType::Bishop, BasePieceType::Knight
            };

            if (pendingPromotion.has_value()) {
                int baseType = promotionPieceTypes[selectedPiece];
                pendingPromotion->promotionPiece = baseType + (board->colorTurn == 1 ? 0 : 6);
                board->makeMove(*pendingPromotion);
                pendingPromotion.reset();
                showPromotionInterface = false;
            }
        }
    }

    void handlePieceSelection(int boxX, int boxY) {
        clickedSquare = sf::Vector2i(boxX, boxY);
        int startSquare = boxY * 8 + boxX;
        int pieceIndex = board->getPieceAt(startSquare);

        if (!isMoving && pieceIndex != -1) {
            int pieceColor = pieceIndex < 6 ? 1 : -1;
            if (pieceColor != board->colorTurn) return;

            targetSquares.clear();
            for (const Move& move : board->moves) {
                if (move.startSquare == startSquare) {
                    targetSquares.push_back(move.targetSquare);
                }
            }
            if (!targetSquares.empty()) {
                isMoving = true;
                movingPieceIndex = pieceIndex;
                movingPieceOrigin = sf::Vector2i(boxX, boxY);
            }
        }
    }

    void afterMoveProcessing() {
        if (board->isCheckmate()) {
            gameEnded = true;
            winner = (board->colorTurn == 1) ? "Black" : "White";
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
                    pendingPromotion = *it;
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
        int squareIndex = j * 8 + i;
        int pieceIndex = board->getPieceAt(squareIndex);

        if (pieceIndex != -1 &&
            (!isMoving || (i != movingPieceOrigin.x || j != movingPieceOrigin.y))) {
            int basePieceType = pieceIndex % 6;
            int color = (pieceIndex < 6) ? 0 : 1;

            // Map basePieceType to asset index
            int assetIndex = basePieceType;

            pieceSprite.setTextureRect(
                sf::IntRect(assetIndex * PIECE_SPRITE_SIZE, color * PIECE_SPRITE_SIZE,
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
            int x = movingPieceIndex % 6;
            int y = (movingPieceIndex < 6) ? 0 : 1;
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
        // Promotion pieces in your asset's order: Queen, Rook, Bishop, Knight
        int promotionPieceTypes[4] = {
            BasePieceType::Queen, BasePieceType::Rook,
            BasePieceType::Bishop, BasePieceType::Knight
        };

        for (int i = 0; i < 4; ++i) {
            int basePieceType = promotionPieceTypes[i];
            int pieceIndex = basePieceType + (color == 0 ? 0 : 6);
            int textureX = basePieceType * PIECE_SPRITE_SIZE;

            pieceSprite.setTextureRect(sf::IntRect(
                textureX, color * PIECE_SPRITE_SIZE,
                PIECE_SPRITE_SIZE, PIECE_SPRITE_SIZE));
            pieceSprite.setPosition((2 + i) * SQUARE_SIZE, 3 * SQUARE_SIZE);
            window.draw(pieceSprite);
        }
    }

    void drawMenu() {
        window.clear(sf::Color::White);

        if (!winner.empty()) {
            menuText.setString(winner + " wins by checkmate!");
            menuText.setPosition(SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE / 2 - 150);
            window.draw(menuText);
        } else if (isDraw) {
            menuText.setString("The game is a draw!");
            menuText.setPosition(SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE / 2 - 150);
            window.draw(menuText);
        }

        // Draw white selection
        whiteSelectionText.setPosition(SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE / 2 - 50);
        window.draw(whiteSelectionText);

        whitePlayerButton.setPosition(SQUARE_SIZE + 100, BOARD_SIZE * SQUARE_SIZE / 2 - 50);
        whitePlayerButton.setFillColor(whiteIsEngine ? sf::Color(150, 150, 150) : sf::Color(100, 100, 100));
        window.draw(whitePlayerButton);
        whitePlayerText.setPosition(whitePlayerButton.getPosition().x + 25, whitePlayerButton.getPosition().y + 10);
        window.draw(whitePlayerText);

        whiteEngineButton.setPosition(SQUARE_SIZE + 220, BOARD_SIZE * SQUARE_SIZE / 2 - 50);
        whiteEngineButton.setFillColor(whiteIsEngine ? sf::Color(100, 100, 100) : sf::Color(150, 150, 150));
        window.draw(whiteEngineButton);
        whiteEngineText.setPosition(whiteEngineButton.getPosition().x + 25, whiteEngineButton.getPosition().y + 10);
        window.draw(whiteEngineText);

        // Draw black selection
        blackSelectionText.setPosition(SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE / 2);
        window.draw(blackSelectionText);

        blackPlayerButton.setPosition(SQUARE_SIZE + 100, BOARD_SIZE * SQUARE_SIZE / 2);
        blackPlayerButton.setFillColor(blackIsEngine ? sf::Color(150, 150, 150) : sf::Color(100, 100, 100));
        window.draw(blackPlayerButton);
        blackPlayerText.setPosition(blackPlayerButton.getPosition().x + 25, blackPlayerButton.getPosition().y + 10);
        window.draw(blackPlayerText);

        blackEngineButton.setPosition(SQUARE_SIZE + 220, BOARD_SIZE * SQUARE_SIZE / 2);
        blackEngineButton.setFillColor(blackIsEngine ? sf::Color(100, 100, 100) : sf::Color(150, 150, 150));
        window.draw(blackEngineButton);
        blackEngineText.setPosition(blackEngineButton.getPosition().x + 25, blackEngineButton.getPosition().y + 10);
        window.draw(blackEngineText);

        // Draw engine selection
        engineSelectionText.setPosition(SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE / 2 + 50);
        window.draw(engineSelectionText);

        randomEngineButton.setPosition(SQUARE_SIZE + 100, BOARD_SIZE * SQUARE_SIZE / 2 + 50);
        randomEngineButton.setFillColor(useRandomEngine ? sf::Color(100, 100, 100) : sf::Color(150, 150, 150));
        window.draw(randomEngineButton);
        randomEngineText.setPosition(randomEngineButton.getPosition().x + 25, randomEngineButton.getPosition().y + 10);
        window.draw(randomEngineText);

        basicEngineButton.setPosition(SQUARE_SIZE + 220, BOARD_SIZE * SQUARE_SIZE / 2 + 50);
        basicEngineButton.setFillColor(useRandomEngine ? sf::Color(150, 150, 150) : sf::Color(100, 100, 100));
        window.draw(basicEngineButton);
        basicEngineText.setPosition(basicEngineButton.getPosition().x + 30, basicEngineButton.getPosition().y + 10);
        window.draw(basicEngineText);

        // Draw start button
        startButton.setPosition((BOARD_SIZE * SQUARE_SIZE - 200) / 2, BOARD_SIZE * SQUARE_SIZE / 2 + 100);
        window.draw(startButton);

        startButtonText.setPosition(
            startButton.getPosition().x + (startButton.getSize().x - startButtonText.getGlobalBounds().width) / 2,
            startButton.getPosition().y + (startButton.getSize().y - startButtonText.getGlobalBounds().height) / 2
        );
        window.draw(startButtonText);

        window.display();
    }

    void makeEngineMove() {
        if ((board->colorTurn == 1 && whiteEngine) || (board->colorTurn == -1 && blackEngine)) {
            Engine* currentEngine = (board->colorTurn == 1) ? whiteEngine.get() : blackEngine.get();
            Move bestMove = currentEngine->getBestMove(*board);
            board->makeMove(bestMove);
            afterMoveProcessing();
        }
    }

   public:
    ChessGame()
        : window(
              sf::VideoMode(BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE),
              "Chess Game") {
        loadAssets();
        try {
            board = std::make_unique<Board>(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
            std::cout << "Board created successfully" << std::endl;
            std::cout << "Initial board state: " << board->boardToEPD()
                      << std::endl;

            board->generateMoves();
            std::cout << "Moves generated successfully" << std::endl;
            std::cout << "Number of legal moves: " << board->moves.size()
                      << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error during board initialization: " << e.what()
                      << std::endl;
            throw;
        }
    }

    void run() {
        while (window.isOpen()) {
            handleEvents();
            if (showMenu) {
                drawMenu();
            } else {
                checkGameState();
                if (!gameEnded && ((board->colorTurn == 1 && whiteEngine) || (board->colorTurn == -1 && blackEngine))) {
                    makeEngineMove();
                }
                window.clear(sf::Color::White);
                drawBoard();
                drawMovingPiece();
                drawPromotionInterface();
                window.display();
            }
        }
    }

    void checkGameState() {
        if (board->isCheckmate()) {
            winner = (board->colorTurn == 1) ? "Black" : "White";
            showMenu = true;
            gameEnded = true;
        } else if (board->isDraw()) {
            isDraw = true;
            showMenu = true;
            gameEnded = true;
        }
    }

    void resetBoard() {
        board = std::make_unique<Board>("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
        isMoving = false;
        gameEnded = false;
        movingPieceIndex = -1;
        clickedSquare = sf::Vector2i(-1, -1);
        targetSquares.clear();
        showPromotionInterface = false;
        pendingPromotion.reset();
        winner.clear();
        isDraw = false;

        // Set up engines based on selection
        if (whiteIsEngine) {
            setWhiteEngine(createSelectedEngine());
        } else {
            setWhiteEngine(nullptr);
        }

        if (blackIsEngine) {
            setBlackEngine(createSelectedEngine());
        } else {
            setBlackEngine(nullptr);
        }
    }

    void setWhiteEngine(std::unique_ptr<Engine> engine) {
        whiteEngine = std::move(engine);
    }

    void setBlackEngine(std::unique_ptr<Engine> engine) {
        blackEngine = std::move(engine);
    }

    std::unique_ptr<Engine> createSelectedEngine() {
        if (useRandomEngine) {
            return std::make_unique<RandomEngine>();
        } else {
            return std::make_unique<BasicEngine>();
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

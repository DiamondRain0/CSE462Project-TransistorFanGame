#ifndef GAMEUI_H
#define GAMEUI_H

#include <SFML/Graphics.hpp>
#include "Board.h"
#include <vector>
#include <string>
#include <iostream>

//Game States
enum AppState { INTRO, MENU, DIFFICULTY_SELECT, GAME, CREDITS, GAMEOVER };

//Turn Phases
enum TurnPhase { P2_SELECT_MOVE, P2_SELECT_REMOVE, AI_THINKING, P2_ANIMATING_ATTACK };

//--- ANIMATION HELPER ---
struct Animation {
    sf::Texture texture;
    int frameCount;
    float frameDuration;
    float timer;
    int currentFrame;
    int frameWidth;
    int frameHeight;
    bool loop;

    //Load with safety checks
    bool load(std::string path, int count, float duration, bool looping) {
        frameCount = count;
        frameDuration = duration;
        loop = looping;
        timer = 0;
        currentFrame = 0;

        if (!texture.loadFromFile(path)) {
            // std::cout << "Error loading " << path << ". Generating fallback..." << std::endl;
            return createFallback(count);
        }

        if (texture.getSize().x > sf::Texture::getMaximumSize()) {
             // std::cout << "Error: " << path << " is too wide! Generating fallback..." << std::endl;
             return createFallback(count);
        }

        frameWidth = texture.getSize().x / frameCount;
        frameHeight = texture.getSize().y;
        return true;
    }

    //Creates a placeholder if above fails
    bool createFallback(int count) {
        sf::Image img;
        int w = 80; 
        int h = 80;
        img.create(w * count, h, sf::Color::Transparent);
        texture.loadFromImage(img);
        frameWidth = w;
        frameHeight = h;
        return true;
    }

    void update(float dt) {
        timer += dt;
        if (timer >= frameDuration) {
            timer = 0;
            if (loop) currentFrame = (currentFrame + 1) % frameCount;
            else if (currentFrame < frameCount - 1) currentFrame++;
        }
    }

    void reset() { currentFrame = 0; timer = 0; }

    sf::IntRect getFrameRect() const {
        return sf::IntRect(currentFrame * frameWidth, 0, frameWidth, frameHeight);
    }
    
    bool isFinished() const { return !loop && currentFrame >= frameCount - 1; }
};

//--- MAIN UI ---
class GameUI {
private:
    sf::RenderWindow window;
    sf::Font font;
    Board board;
    AppState state;
    TurnPhase phase;
    std::string statusMsg;
    bool aiWon;
    
    //Difficulty
    int selectedDepth; 

    //Story
    sf::Texture storyTextures[3];
    std::string storyCaptions[3];
    int currentSlide;

    //Animation
    sf::Clock dtClock;
    Animation idleAnim;
    Animation attackAnim;
    bool isPlayerAttacking;
    Point pendingRemoval;

    //Helpers
    void drawButton(std::string text, float y, sf::Vector2i mousePos);
    void drawBoard(sf::Vector2i mousePos);
    void drawMenu(sf::Vector2i mousePos);
    void drawCredits(sf::Vector2i mousePos);
    void drawGameOver(sf::Vector2i mousePos);
    void drawIntro();
    void drawDifficultySelect(sf::Vector2i mousePos);
    
    void createPlaceholderStory(int index);

    //Core Logic
    void handleInput(sf::Event& event);
    void updateGame();

public:
    GameUI();
    void run();
};

#endif
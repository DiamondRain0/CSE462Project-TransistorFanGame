#include "../include/GameUI.h"
#include "../include/AI.h"
#include "../include/Theme.h"
#include <iostream>
#include <cmath>

using namespace sf;
using namespace std;

GameUI::GameUI() {
    ContextSettings settings;
    settings.antialiasingLevel = 8;
    window.create(VideoMode(WIN_WIDTH, WIN_HEIGHT), "TRANSISTOR: RECURSION", Style::Default, settings);
    window.setFramerateLimit(60);
    
    //Font Loading
    if (!font.loadFromFile("Moonhouse.ttf")) {
        if (!font.loadFromFile("arial.ttf")) cout << "Error: Font not found." << endl;
    }

    //Story Loading
    if(!storyTextures[0].loadFromFile("story1.jpg")) createPlaceholderStory(0);
    if(!storyTextures[1].loadFromFile("story2.png")) {
        if(!storyTextures[1].loadFromFile("story2.jpg")) createPlaceholderStory(1);
    }
    if(!storyTextures[2].loadFromFile("story3.png")) {
        if(!storyTextures[2].loadFromFile("story3.jpg")) createPlaceholderStory(2);
    }

    storyCaptions[0] = "CLOUDBANK IS SILENT.\n\nThe Process has begun its recursion loop.\nThe city's geometry is being rewritten.";
    storyCaptions[1] = "THE WIELDER.\n\nYou possess the Transistor.\nIt is the only key to the system.";
    storyCaptions[2] = "THE FUNCTION.\n\nDefeat the Process in the Grid.\nTrapping it is the only way to stop the deletion.";

    //Animation Loading
    idleAnim.load("p2_idle.png", 8, 0.1f, true);
    attackAnim.load("p2_attack.png", 45, 0.01f, false);
    isPlayerAttacking = false;

    state = INTRO;
    currentSlide = 0;
    phase = AI_THINKING;
    statusMsg = "Waiting for User Input...";
    selectedDepth = 2; 
}

void GameUI::createPlaceholderStory(int index) {
    Image img; img.create(600, 400, Color(20, 20, 20));
    storyTextures[index].loadFromImage(img);
}

void GameUI::run() {
    while (window.isOpen()) {
        float dt = dtClock.restart().asSeconds();
        
        if (isPlayerAttacking) attackAnim.update(dt);
        idleAnim.update(dt); //Updates idle animation, it always plays

        Event event;
        while (window.pollEvent(event)) handleInput(event);
        
        updateGame();
        
        window.clear(Theme::BG);
        Vector2i mousePos = Mouse::getPosition(window);

        if (state == INTRO) drawIntro();
        else if (state == MENU) drawMenu(mousePos);
        else if (state == DIFFICULTY_SELECT) drawDifficultySelect(mousePos);
        else if (state == CREDITS) drawCredits(mousePos);
        else if (state == GAME) drawBoard(mousePos);
        else if (state == GAMEOVER) drawGameOver(mousePos);
        
        window.display();
    }
}

void GameUI::handleInput(Event& event) {
    if (event.type == Event::Closed) window.close();
    if (phase == P2_ANIMATING_ATTACK) return;

    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
        Vector2i mousePos = Mouse::getPosition(window);
        float centerX = (WIN_WIDTH - Theme::BTN_WIDTH) / 2;

        if (state == INTRO) {
            currentSlide++;
            if (currentSlide >= 3) state = MENU;
        }

        else if (state == MENU) {
            if (FloatRect(centerX, 250, Theme::BTN_WIDTH, Theme::BTN_HEIGHT).contains(mousePos.x, mousePos.y)) state = DIFFICULTY_SELECT;
            else if (FloatRect(centerX, 350, Theme::BTN_WIDTH, Theme::BTN_HEIGHT).contains(mousePos.x, mousePos.y)) state = CREDITS;
        }
        else if (state == DIFFICULTY_SELECT) {
            if (FloatRect(centerX, 200, Theme::BTN_WIDTH, Theme::BTN_HEIGHT).contains(mousePos.x, mousePos.y)) selectedDepth = 1;
            else if (FloatRect(centerX, 300, Theme::BTN_WIDTH, Theme::BTN_HEIGHT).contains(mousePos.x, mousePos.y)) selectedDepth = 2;
            else if (FloatRect(centerX, 400, Theme::BTN_WIDTH, Theme::BTN_HEIGHT).contains(mousePos.x, mousePos.y)) selectedDepth = 3;
            else if (FloatRect(centerX, 500, Theme::BTN_WIDTH, Theme::BTN_HEIGHT).contains(mousePos.x, mousePos.y)) selectedDepth = 4;
            
            if (mousePos.x >= centerX && mousePos.x <= centerX + Theme::BTN_WIDTH && mousePos.y >= 200 && mousePos.y <= 550) {
                 state = GAME; board = Board(); phase = AI_THINKING;
            }
        }
        else if (state == CREDITS || state == GAMEOVER) {
            if (FloatRect(centerX, 500, Theme::BTN_WIDTH, Theme::BTN_HEIGHT).contains(mousePos.x, mousePos.y)) state = MENU;
        }
        else if (state == GAME && phase != AI_THINKING) {
            int gridPixelSize = BOARD_SIZE * CELL_SIZE;
            int startX = (WIN_WIDTH - gridPixelSize) / 2;
            int startY = (WIN_HEIGHT - gridPixelSize) / 2 + 20;

            int c = (mousePos.x - startX) / CELL_SIZE;
            int r = (mousePos.y - startY) / CELL_SIZE;

            if (board.isValid(r, c)) {
                if (phase == P2_SELECT_MOVE) {
                    if (board.canMoveTo(r, c)) {
                        if (abs(r - board.p2Pos.r) <= 1 && abs(c - board.p2Pos.c) <= 1) {
                            board.grid[board.p2Pos.r][board.p2Pos.c] = EMPTY;
                            board.grid[r][c] = P2_PIECE;
                            board.p2Pos = {r, c};
                            phase = P2_SELECT_REMOVE;
                            statusMsg = "Select Sector to Format()";
                        }
                    }
                } else if (phase == P2_SELECT_REMOVE) {
                    if (board.grid[r][c] == EMPTY) {
                        pendingRemoval = {r, c};
                        isPlayerAttacking = true;
                        attackAnim.reset();
                        phase = P2_ANIMATING_ATTACK;
                        statusMsg = "Executing Delete()...";
                    }
                }
            }
        }
    }
}

void GameUI::updateGame() {
    if (state == GAME && phase == P2_ANIMATING_ATTACK) {
        if (attackAnim.isFinished()) {
            board.grid[pendingRemoval.r][pendingRemoval.c] = REMOVED; 
            isPlayerAttacking = false;
            phase = AI_THINKING;
            statusMsg = "The Process is Calculating...";
        }
        return;
    }

    if (state == GAME && phase == AI_THINKING) {
        if (board.hasNoMoves(true)) {
            state = GAMEOVER; aiWon = false;
        } else {
            window.clear(Theme::BG);
            drawBoard(Mouse::getPosition(window));
            window.display();

            TurnMove best = getBestMove(board, selectedDepth);
            
            board.grid[board.p1Pos.r][board.p1Pos.c] = EMPTY;
            board.p1Pos = best.moveDest;
            board.grid[board.p1Pos.r][board.p1Pos.c] = P1_PIECE;
            board.grid[best.removeLoc.r][best.removeLoc.c] = REMOVED;
            
            phase = P2_SELECT_MOVE;
            statusMsg = "User Turn: Execute Move()";
            
            if (board.hasNoMoves(false)) {
                state = GAMEOVER; aiWon = true;
            }
        }
    }
}

//--- DRAWING FUNCTIONS ---

void GameUI::drawIntro() {
    Sprite sprite;
    if (storyTextures[currentSlide].getSize().x > 0) {
        sprite.setTexture(storyTextures[currentSlide]);
        float maxW = 700.0f, maxH = 400.0f;
        FloatRect b = sprite.getLocalBounds();
        float scale = min(maxW / b.width, maxH / b.height);
        sprite.setScale(scale, scale);
        sprite.setPosition((WIN_WIDTH - b.width*scale)/2, 50); 
        window.draw(sprite);
    } 

    Text txt(storyCaptions[currentSlide], font, 24);
    txt.setFillColor(Theme::TEXT_MAIN); 
    FloatRect tr = txt.getLocalBounds();
    txt.setOrigin(tr.left + tr.width/2.0f, 0);
    txt.setPosition(WIN_WIDTH/2, 480); 
    window.draw(txt);

    Text hint(">> CLICK TO EXECUTE >>", font, 16);
    hint.setFillColor(Theme::TEXT_SUB);
    hint.setPosition(WIN_WIDTH - 250, WIN_HEIGHT - 50);
    window.draw(hint);
}

void GameUI::drawButton(string text, float y, Vector2i mousePos) {
    float x = (WIN_WIDTH - Theme::BTN_WIDTH) / 2;
    RectangleShape btn(Vector2f(Theme::BTN_WIDTH, Theme::BTN_HEIGHT));
    btn.setPosition(x, y);
    
    Color outline = Theme::GRID_LINES;
    Color fill = Theme::BTN_IDLE;
    if (text == "INFINITE LOOP") outline = Color::Red;

    if (btn.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
        fill = Theme::BTN_HOVER;
        if (text == "INFINITE LOOP") fill = Color(60, 0, 0);
        outline = (text == "INFINITE LOOP") ? Color::Red : Theme::TEXT_MAIN;
        btn.setOutlineThickness(2);
    } else {
        btn.setOutlineThickness(1);
    }

    btn.setFillColor(fill);
    btn.setOutlineColor(outline);
    window.draw(btn);

    Text txt(text, font, 22); 
    txt.setFillColor((text == "INFINITE LOOP") ? Color::Red : Theme::TEXT_MAIN);
    FloatRect tr = txt.getLocalBounds();
    txt.setOrigin(tr.left + tr.width/2.0f, tr.top + tr.height/2.0f);
    txt.setPosition(x + Theme::BTN_WIDTH/2, y + Theme::BTN_HEIGHT/2);
    window.draw(txt);
}

void GameUI::drawMenu(Vector2i mousePos) {
    Text title("TRANSISTOR", font, 50);
    title.setFillColor(Theme::TEXT_MAIN);
    FloatRect tr = title.getLocalBounds();
    title.setOrigin(tr.left + tr.width/2.0f, 0);
    title.setPosition(WIN_WIDTH/2, 80);
    window.draw(title);

    Text sub("RECURSION", font, 35);
    sub.setFillColor(Theme::TEXT_SUB);
    FloatRect sr = sub.getLocalBounds();
    sub.setOrigin(sr.left + sr.width/2.0f, 0);
    sub.setPosition(WIN_WIDTH/2, 140);
    window.draw(sub);
    
    drawButton("INITIALIZE()", 250, mousePos);
    drawButton("ARCHIVES()", 350, mousePos);
}

void GameUI::drawDifficultySelect(Vector2i mousePos) {
    Text title("SELECT THREAD DEPTH", font, 30);
    title.setFillColor(Theme::TEXT_SUB);
    FloatRect tr = title.getLocalBounds();
    title.setOrigin(tr.left + tr.width/2.0f, 0);
    title.setPosition(WIN_WIDTH/2, 100);
    window.draw(title);

    drawButton("RECURSION 1 (Easy)", 200, mousePos);
    drawButton("RECURSION 2 (Med)", 300, mousePos);
    drawButton("RECURSION 3 (Hard)", 400, mousePos);
    drawButton("INFINITE LOOP", 500, mousePos);
}

void GameUI::drawCredits(Vector2i mousePos) {
    Text title("ARCHIVES", font, 40);
    title.setPosition((WIN_WIDTH-200)/2, 100);
    title.setFillColor(Theme::TEXT_MAIN);
    window.draw(title);
    Text names("The Camerata Member:\nEylul AKAR\n\nLibrary:\nCSE462 Term Project", font, 24);
    names.setFillColor(Theme::TEXT_SUB);
    names.setPosition(WIN_WIDTH/2 - 100, 200);
    window.draw(names);
    drawButton("RETURN()", 500, mousePos);
}

void GameUI::drawGameOver(Vector2i mousePos) {
    Text title("PROCESS TERMINATED", font, 45);
    FloatRect tr = title.getLocalBounds();
    title.setOrigin(tr.left + tr.width/2.0f, 0);
    title.setPosition(WIN_WIDTH/2, 150);
    
    if (aiWon) {
        title.setFillColor(Theme::TEXT_SUB);
        Text res("THE PROCESS PREVAILS", font, 25);
        res.setPosition(WIN_WIDTH/2 - 150, 250); res.setFillColor(Color::White); window.draw(res);
    } else {
        title.setFillColor(Theme::PLAYER_PIECE);
        Text res("RECURSION BROKEN. YOU WIN.", font, 25);
        res.setPosition(WIN_WIDTH/2 - 180, 250); res.setFillColor(Theme::TEXT_MAIN); window.draw(res);
    }
    window.draw(title);
    drawButton("MAIN MENU()", 500, mousePos);
}

void GameUI::drawBoard(Vector2i mousePos) {
    int gridPixelSize = BOARD_SIZE * CELL_SIZE;
    int startX = (WIN_WIDTH - gridPixelSize) / 2;
    int startY = (WIN_HEIGHT - gridPixelSize) / 2 + 20;

    //Board Background
    RectangleShape boardBg(Vector2f(gridPixelSize + 10, gridPixelSize + 10));
    boardBg.setPosition(startX - 5, startY - 5);
    boardBg.setFillColor(Color(10, 15, 20)); 
    boardBg.setOutlineColor(Theme::GRID_LINES);
    boardBg.setOutlineThickness(2);
    window.draw(boardBg);

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            float x = startX + c * CELL_SIZE;
            float y = startY + r * CELL_SIZE;
            
            RectangleShape cell(Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
            cell.setPosition(x + 1, y + 1); 
            
            bool isValidMove = false;
            if (phase == P2_SELECT_MOVE && board.grid[r][c] == EMPTY) {
                if (abs(r - board.p2Pos.r) <= 1 && abs(c - board.p2Pos.c) <= 1) isValidMove = true;
            }

            if (board.grid[r][c] == REMOVED) {
                cell.setFillColor(Theme::REMOVED);
                cell.setOutlineThickness(0);
            } else {
                if (FloatRect(x, y, CELL_SIZE, CELL_SIZE).contains(mousePos.x, mousePos.y)) 
                     cell.setFillColor(Color(40, 60, 65));
                else cell.setFillColor(Color::Transparent);
                cell.setOutlineColor(Theme::GRID_LINES);
                cell.setOutlineThickness(1);
            }
            if (isValidMove) {
                cell.setOutlineColor(Theme::TEXT_MAIN);
                cell.setOutlineThickness(2);
                cell.setFillColor(Color(40, 40, 20));
            }
            window.draw(cell);
            
            //Draw Animation On Target Tile
            if (isPlayerAttacking && r == pendingRemoval.r && c == pendingRemoval.c) {
                if (attackAnim.texture.getSize().x > 0) {
                    Sprite s;
                    s.setTexture(attackAnim.texture);
                    s.setTextureRect(attackAnim.getFrameRect());
                    
                    float scale = min((float)CELL_SIZE/s.getLocalBounds().width, (float)CELL_SIZE/s.getLocalBounds().height);
                    s.setScale(scale, scale);
                    s.setOrigin(s.getLocalBounds().width/2, s.getLocalBounds().height/2);
                    s.setPosition(x + CELL_SIZE/2, y + CELL_SIZE/2);
                    
                    window.draw(s);
                } else {
                    //Fallback visual for attack
                    RectangleShape flash(Vector2f(CELL_SIZE, CELL_SIZE));
                    flash.setPosition(x, y);
                    flash.setFillColor(Color(255, 50, 50, 150));
                    window.draw(flash);
                }
            }

            //--- DRAW PIECES ---
            if (board.grid[r][c] == P1_PIECE) { 
                RectangleShape p(Vector2f(CELL_SIZE/2, CELL_SIZE/2));
                p.setOrigin(CELL_SIZE/4, CELL_SIZE/4);
                p.setPosition(x + CELL_SIZE/2, y + CELL_SIZE/2);
                p.setFillColor(Theme::AI_PIECE);
                p.setOutlineColor(Theme::AI_OUTLINE);
                p.setOutlineThickness(2);
                p.setRotation(45);
                window.draw(p);
            } else if (board.grid[r][c] == P2_PIECE) { 
                //PLAYER
                Sprite p;
                bool useSprite = false;

                //Idle Animation
                if (idleAnim.texture.getSize().x > 0) {
                    p.setTexture(idleAnim.texture);
                    p.setTextureRect(idleAnim.getFrameRect());
                    useSprite = true;
                }

                if (useSprite) {
                    float scale = min((float)CELL_SIZE/p.getLocalBounds().width, (float)CELL_SIZE/p.getLocalBounds().height) * 0.8f;
                    p.setScale(scale, scale);
                    p.setOrigin(p.getLocalBounds().width/2, p.getLocalBounds().height/2);
                    p.setPosition(x + CELL_SIZE/2, y + CELL_SIZE/2);
                    window.draw(p);
                } else {
                    CircleShape c(CELL_SIZE / 3);
                    c.setOrigin(CELL_SIZE / 3, CELL_SIZE / 3);
                    c.setPosition(x + CELL_SIZE/2, y + CELL_SIZE/2);
                    c.setFillColor(Theme::PLAYER_PIECE);
                    c.setOutlineColor(Theme::PLAYER_GLOW);
                    c.setOutlineThickness(3);
                    window.draw(c);
                }
            }
        }
    }

    for(int i=0; i<BOARD_SIZE; i++) {
        Text t(string(1, 'a'+i), font, 16); t.setFillColor(Theme::TEXT_SUB);
        t.setPosition(startX - 20, startY + i*CELL_SIZE + 30); window.draw(t);
        Text n(to_string(i+1), font, 16); n.setFillColor(Theme::TEXT_SUB);
        n.setPosition(startX + i*CELL_SIZE + 35, startY - 25); window.draw(n);
    }

    Text st(statusMsg, font, 20); st.setFillColor(Theme::TEXT_MAIN);
    FloatRect sr = st.getLocalBounds(); st.setOrigin(sr.left + sr.width/2, 0);
    st.setPosition(WIN_WIDTH/2, WIN_HEIGHT - 35);
    window.draw(st);
}
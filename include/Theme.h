#ifndef THEME_H
#define THEME_H

#include <SFML/Graphics.hpp>

//--- TRANSISTOR COLOR PALETTE ---
namespace Theme {
    //Dark Background-Deep Teal/Black
    const sf::Color BG(15, 25, 30);            
    
    //Grid Lines-Faint Tech Teal
    const sf::Color GRID_LINES(40, 80, 80);    
    
    //AI-Stark White/Pale Blue
    const sf::Color AI_PIECE(220, 240, 255);   
    const sf::Color AI_OUTLINE(0, 255, 255);   

    //Red(Human)-Transistor Gold & Red
    const sf::Color PLAYER_PIECE(255, 60, 60); 
    const sf::Color PLAYER_GLOW(255, 200, 0);  

    //Void/Removed squares-Glitchy Dark
    const sf::Color REMOVED(35, 40, 42);        
    
    //Text-Gold/Amber
    const sf::Color TEXT_MAIN(255, 215, 100);  
    const sf::Color TEXT_SUB(0, 200, 180);     
    
    //Button States
    const sf::Color BTN_IDLE(20, 30, 35);
    const sf::Color BTN_HOVER(50, 70, 70);

    //Layout
    const float BTN_WIDTH = 260;
    const float BTN_HEIGHT = 50;
}

#endif
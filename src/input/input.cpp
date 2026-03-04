#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

#include "input/input.h"
#include "input/keystate.h"
#include "core/window.h"
#include "graphics/menu.h"
#include "core/game.h"

void keyboard(unsigned char key, int, int)
{
    const GameState state = gameGetState();

    // ESC sai do jogo
    if (key == 27)
        std::exit(0);

    // --- MENU INICIAL ---
    if (state == GameState::MENU_INICIAL)
    {
        if (key == 13)
        {
            // Mais seguro para prazo: inicia direto
            gameStartNewGame();
        }
        return;
    }

    // --- GAME OVER ---
    if (state == GameState::GAME_OVER)
    {
        if (key == 13)
        {
            gameReloadLevel();
            gameReset();
            gameSetState(GameState::JOGANDO);
        }
        return;
    }

    // --- VITÓRIA ---
    if (state == GameState::VITORIA)
    {
        if (key == 13)
        {
            gameSetState(GameState::MENU_INICIAL);
        }
        return;
    }

    // --- PAUSE ---
    if (state == GameState::PAUSADO)
    {
        if (key == 'p' || key == 'P')
        {
            gameSetState(GameState::JOGANDO);
        }
        return;
    }

    // --- JOGANDO ---
    if (state == GameState::JOGANDO)
    {
        if (key == 'p' || key == 'P')
        {
            gameSetState(GameState::PAUSADO);
            keyW = keyA = keyS = keyD = false;
            return;
        }

        switch (key)
        {
        case 'w':
        case 'W':
            keyW = true;
            break;
        case 's':
        case 'S':
            keyS = true;
            break;
        case 'a':
        case 'A':
            keyA = true;
            break;
        case 'd':
        case 'D':
            keyD = true;
            break;
        case 'r':
        case 'R':
            playerTryReload();
            break;
        }
    }

    if (state == GameState::VITORIA)
{
    if (key == 13)
    {
        gameSetState(GameState::MENU_INICIAL);
    }
    return;
}
}

void keyboardUp(unsigned char key, int, int)
{
    switch (key)
    {
    case 'w':
    case 'W':
        keyW = false;
        break;
    case 's':
    case 'S':
        keyS = false;
        break;
    case 'a':
    case 'A':
        keyA = false;
        break;
    case 'd':
    case 'D':
        keyD = false;
        break;
    }

    if ((key == 13 || key == '\r') && (glutGetModifiers() & GLUT_ACTIVE_ALT))
    {
        altFullScreen();
    }
}

void mouseClick(int button, int state, int x, int y)
{
    (void)x; (void)y;

    if (gameGetState() != GameState::JOGANDO) return;
    if (state != GLUT_DOWN) return;

    if (button == GLUT_LEFT_BUTTON)
        playerTryAttack();
    else if (button == GLUT_RIGHT_BUTTON)
        playerTryAltAttack();
}
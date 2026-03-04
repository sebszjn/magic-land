#pragma once
#include <GL/glew.h>

struct GameAssets
{
    // texturas
    GLuint texMenuBG = 0;
    GLuint texChao = 0;
    GLuint texParede = 0;
    GLuint texSangue = 0;
    GLuint texLava = 0;
    GLuint texChaoInterno = 0;
    GLuint texParedeInterna = 0;
    GLuint texTeto = 0;

    GLuint texHealthOverlay = 0;
    GLuint texHealth = 0;
    GLuint texAmmo = 0;

    // itens
    GLuint texKey = 0;
    GLuint texSpecial = 0;

    // NOVO: portal/saida (tile X)
    GLuint texPortal = 0;

    GLuint texGunDefault = 0;
    GLuint texGunFire1 = 0;
    GLuint texGunFire2 = 0;
    GLuint texGunReload1 = 0;
    GLuint texGunReload2 = 0;
    GLuint texDamage = 0;
    GLuint texGunHUD = 0;
    GLuint texHudFundo = 0;

    GLuint texEnemies[5]       = {0, 0, 0, 0, 0};
    GLuint texEnemiesRage[5]   = {0, 0, 0, 0, 0};
    GLuint texEnemiesDamage[5] = {0, 0, 0, 0, 0};

    // shaders
    GLuint progSangue = 0;
    GLuint progLava = 0;

    // skies
    GLuint texSkydome = 0;
    GLuint texSkydome1 = 0;
    GLuint texSkydome2 = 0;
    GLuint texSkydome3 = 0;
};

bool loadAssets(GameAssets &a);
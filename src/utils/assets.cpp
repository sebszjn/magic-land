#include "utils/assets.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include <cstdio>

bool loadAssets(GameAssets &a)
{
    a.texMenuBG = carregaTextura("assets/menu_bg.png");

    a.texChao = carregaTextura("assets/181.png");
    a.texParede = carregaTextura("assets/091.png");
    a.texSangue = carregaTextura("assets/016.png");
    a.texLava = carregaTextura("assets/179.png");
    a.texChaoInterno = carregaTextura("assets/100.png");
    a.texParedeInterna = carregaTextura("assets/060.png");
    a.texTeto = carregaTextura("assets/081.png");

    a.progSangue = criaShader("shaders/blood.vert", "shaders/blood.frag");
    a.progLava = criaShader("shaders/lava.vert", "shaders/lava.frag");

    // --- INIMIGOS ---
    a.texEnemies[0] = carregaTextura("assets/enemies/enemy.png");
    a.texEnemiesRage[0] = carregaTextura("assets/enemies/enemyRage.png");
    a.texEnemiesDamage[0] = carregaTextura("assets/enemies/enemyRageDamage.png");

    a.texEnemies[1] = carregaTextura("assets/enemies/enemy2.png");
    a.texEnemiesRage[1] = carregaTextura("assets/enemies/enemyRage2.png");
    a.texEnemiesDamage[1] = carregaTextura("assets/enemies/enemyRageDamage2.png");

    a.texEnemies[2] = carregaTextura("assets/enemies/enemy3.png");
    a.texEnemiesRage[2] = carregaTextura("assets/enemies/enemyRage3.png");
    a.texEnemiesDamage[2] = carregaTextura("assets/enemies/enemyRageDamage3.png");

    a.texEnemies[3] = carregaTextura("assets/enemies/enemy4.png");
    a.texEnemiesRage[3] = carregaTextura("assets/enemies/enemyRage4.png");
    a.texEnemiesDamage[3] = carregaTextura("assets/enemies/enemyRageDamage4.png");

    a.texEnemies[4] = carregaTextura("assets/enemies/enemy5.png");
    a.texEnemiesRage[4] = carregaTextura("assets/enemies/enemyRage5.png");
    a.texEnemiesDamage[4] = carregaTextura("assets/enemies/enemyRageDamage5.png");

    // HUD / overlays
    a.texHealthOverlay = carregaTextura("assets/heal.png");
    a.texGunDefault = carregaTextura("assets/gun_default.png");
    a.texGunFire1 = carregaTextura("assets/gun_fire1.png");
    a.texGunFire2 = carregaTextura("assets/gun_fire2.png");
    a.texGunReload1 = carregaTextura("assets/gun_reload1.png");
    a.texGunReload2 = carregaTextura("assets/gun_reload2.png");
    a.texDamage = carregaTextura("assets/damage.png");

    // itens base
    a.texHealth = carregaTextura("assets/health.png");
    a.texAmmo = carregaTextura("assets/066.png");

    // sky
    a.texSkydome1 = carregaTextura("assets/Va4wUMQ.png");
    a.texSkydome2 = carregaTextura("assets/sky2.png");
    a.texSkydome3 = carregaTextura("assets/sky3.png");
    a.texSkydome = a.texSkydome1; // default

    a.texGunHUD = carregaTextura("assets/Shotgun.png");
    a.texHudFundo = carregaTextura("assets/088.png");

    // NOVO: itens/portal
    a.texKey = carregaTextura("assets/chave.png");         // <-- coloque seu png da key aqui
    a.texSpecial = carregaTextura("assets/special.png"); // <-- item da fase 2
    a.texPortal = carregaTextura("assets/999.png");      // <-- textura do portal/saida

    if (!a.texChao || !a.texKey || !a.texSpecial || !a.texPortal || !a.texParede || !a.texSangue || !a.texLava || !a.progSangue ||
        !a.progLava || !a.texHealth || !a.texGunDefault || !a.texGunFire1 ||
        !a.texGunFire2 || !a.texSkydome1 || !a.texGunReload1 || !a.texGunReload2 ||
        !a.texDamage || !a.texAmmo || !a.texHealthOverlay || !a.texEnemies[0] ||
        !a.texEnemiesRage[0] || !a.texEnemiesDamage[0] || !a.texEnemies[1] ||
        !a.texEnemiesRage[1] || !a.texEnemiesDamage[1] || !a.texEnemies[2] ||
        !a.texEnemiesRage[2] || !a.texEnemiesDamage[2] || !a.texGunHUD || !a.texHudFundo || !a.texMenuBG)
    {
        std::printf("ERRO: falha ao carregar algum asset (textura/shader).\n");
        return false;
    }

    // Se você ainda não tiver as imagens, não vai travar o jogo:
    // if (!a.texKey)
    //     std::printf("[ASSETS] AVISO: nao carregou assets/key.png (KEY vai usar fallback)\n");
    // if (!a.texSpecial)
    //     std::printf("[ASSETS] AVISO: nao carregou assets/special.png (SPECIAL vai usar fallback)\n");
    // if (!a.texSkydome2)
    //     std::printf("[ASSETS] AVISO: nao carregou assets/sky2.png\n");
    // if (!a.texSkydome3)
    //     std::printf("[ASSETS] AVISO: nao carregou assets/sky3.png\n");

    return true;
}
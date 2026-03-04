#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "core/game_enums.h"
#include "core/game_state.h"
#include "core/game.h"

#include "level/level.h"

#include "core/camera.h"
#include "input/input.h"
#include "input/keystate.h"

#include "graphics/drawlevel.h"
#include "graphics/skybox.h"
#include "graphics/hud.h"
#include "graphics/menu.h"
#include "graphics/lighting.h"

#include "core/movement.h"
#include "core/player.h"
#include "core/entities.h"

#include "audio/audio_system.h"

#include "utils/assets.h"
#include "core/config.h"

#include "core/window.h"

#include <GL/glew.h>
#include <GL/glut.h>

static HudTextures gHudTex;
static GameContext g;

constexpr int MAX_MAGAZINE = 12;

// --- Assets / Level ---
static GameAssets gAssets;
Level gLevel;
static AudioSystem gAudioSys;

GameContext &gameContext() { return g; }
AudioSystem &gameAudio() { return gAudioSys; }
Level &gameLevel() { return gLevel; }

GameState gameGetState() { return g.state; }
void gameSetState(GameState s) { g.state = s; }

// =========================
//  SISTEMA DE FASES
// =========================
static const char *LEVEL_PATHS[3] = {
    "maps/level1.txt",
    "maps/level2.txt",
    "maps/level3.txt"};

static int gLevelIndex = 0;

static void applySkyForCurrentLevel()
{
    // 0->level1, 1->level2, 2->level3
    if (gLevelIndex == 0)
        g.r.texSkydome = gAssets.texSkydome1;
    else if (gLevelIndex == 1)
        g.r.texSkydome = gAssets.texSkydome2;
    else
        g.r.texSkydome = gAssets.texSkydome3;
}

static int countAliveEnemies()
{
    int alive = 0;
    for (const auto &e : gLevel.enemies)
        if (e.state != STATE_DEAD)
            alive++;
    return alive;
}

// Fallback: se por algum motivo loadLevel não setar hasExit/exitX/exitZ,
// achamos o 'X' varrendo o mapData. Isso salva tempo e evita travar no requisito.
static bool findExitFallback(float &outX, float &outZ)
{
    const auto &data = gLevel.map.data();
    const int H = gLevel.map.getHeight();

    for (int z = 0; z < H; ++z)
    {
        const std::string &row = data[z];
        for (int x = 0; x < (int)row.size(); ++x)
        {
            if (row[x] == 'X')
            {
                gLevel.metrics.tileCenter(x, z, outX, outZ);
                return true;
            }
        }
    }
    return false;
}

static bool loadCurrentLevel()
{
    const char *path = LEVEL_PATHS[gLevelIndex];

    if (!loadLevel(gLevel, path, GameConfig::TILE_SIZE))
    {
        std::printf("[LEVEL] ERRO: loadLevel falhou: %s\n", path);
        return false;
    }

    applySkyForCurrentLevel();

    g.r.texPortal = gAssets.texPortal;

    // Recarrega o ataque alternativo SOMENTE ao trocar de fase:
    if (g.player.altUnlocked)
        g.player.altAmmo = 5;

    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;

    // Reconfigura áudio para o novo level (ambiente + fontes)
    audioInit(gAudioSys, gLevel);

    // Debug leve (não spamma)
    std::printf("[LEVEL] Carregado %s | enemies=%d items=%d\n",
                path, (int)gLevel.enemies.size(), (int)gLevel.items.size());

    // Se não tiver saída (ou não setou), tenta achar via fallback
    if (!gLevel.hasExit)
    {
        float ex, ez;
        if (findExitFallback(ex, ez))
        {
            gLevel.hasExit = true;
            gLevel.exitX = ex;
            gLevel.exitZ = ez;
            std::printf("[LEVEL] Fallback: achou X em (%.2f, %.2f)\n", gLevel.exitX, gLevel.exitZ);
        }
        else
        {
            std::printf("[LEVEL] AVISO: nao achou 'X' neste mapa.\n");
        }
    }
    else
    {
        std::printf("[LEVEL] Saida X em (%.2f, %.2f)\n", gLevel.exitX, gLevel.exitZ);
    }

    return true;
}

// Funções públicas (chamadas no input)
void gameStartNewGame()
{
    gLevelIndex = 0;
    if (!loadCurrentLevel())
        return;

    gameReset();
    g.state = GameState::JOGANDO;
}

void gameReloadLevel()
{
    loadCurrentLevel();
}

void gameTogglePause()
{
    if (g.state == GameState::JOGANDO)
        g.state = GameState::PAUSADO;
    else if (g.state == GameState::PAUSADO)
        g.state = GameState::JOGANDO;
}

// --- INIT ---
bool gameInit(const char * /*mapPath*/)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    setupSunLightOnce();
    setupIndoorLightOnce();

    if (!loadAssets(gAssets))
        return false;

    g.r.texChao = gAssets.texChao;
    g.r.texParede = gAssets.texParede;
    g.r.texSangue = gAssets.texSangue;
    g.r.texLava = gAssets.texLava;
    g.r.texChaoInterno = gAssets.texChaoInterno;
    g.r.texParedeInterna = gAssets.texParedeInterna;
    g.r.texTeto = gAssets.texTeto;

    applySkyForCurrentLevel(); // (vai setar o do level 1 no init)

    g.r.texMenuBG = gAssets.texMenuBG;
    gHudTex.texHudFundo = gAssets.texHudFundo;
    gHudTex.texGunHUD = gAssets.texGunHUD;

    gHudTex.texGunDefault = gAssets.texGunDefault;
    gHudTex.texGunFire1 = gAssets.texGunFire1;
    gHudTex.texGunFire2 = gAssets.texGunFire2;
    gHudTex.texGunReload1 = gAssets.texGunReload1;
    gHudTex.texGunReload2 = gAssets.texGunReload2;

    gHudTex.texDamage = gAssets.texDamage;
    gHudTex.texHealthOverlay = gAssets.texHealthOverlay;
    g.r.texKey = gAssets.texKey;

    for (int i = 0; i < 5; i++)
    {
        g.r.texEnemies[i] = gAssets.texEnemies[i];
        g.r.texEnemiesRage[i] = gAssets.texEnemiesRage[i];
        g.r.texEnemiesDamage[i] = gAssets.texEnemiesDamage[i];
    }

    g.r.texHealth = gAssets.texHealth;
    g.r.texAmmo = gAssets.texAmmo;
    g.r.texKey = gAssets.texKey;
    g.r.texSpecial = gAssets.texSpecial;
    g.r.texPortal = gAssets.texPortal;

    g.r.progSangue = gAssets.progSangue;
    g.r.progLava = gAssets.progLava;

    // carrega level 1
    gLevelIndex = 0;
    if (!loadCurrentLevel())
        return false;

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutSetCursor(GLUT_CURSOR_NONE);

    g.state = GameState::MENU_INICIAL;
    g.time = 0.0f;
    g.player = PlayerState{};
    g.weapon = WeaponAnim{};

    return true;
}

// Reinicia o jogo (stats + respawn do player)
void gameReset()
{
    g.player.health = 100;
    g.player.currentAmmo = 12;
    g.player.reserveAmmo = 25;

    g.player.damageAlpha = 0.0f;
    g.player.healthAlpha = 0.0f;

    // NOVO: chaves sempre recomeçam na fase
    g.player.keys = 0;

    g.weapon.state = WeaponState::W_IDLE;
    g.weapon.timer = 0.0f;

    applySpawn(gLevel, camX, camZ);
}

void gameUpdate(float dt)
{
    g.time += dt;

    // Se não estiver jogando, não roda lógica
    if (g.state != GameState::JOGANDO)
        return;

    atualizaMovimento();

    // Listener de áudio
    AudioListener L;
    L.pos = {camX, camY, camZ};
    {
        float ry = yaw * 3.14159f / 180.0f;
        float rp = pitch * 3.14159f / 180.0f;
        L.forward = {cosf(rp) * sinf(ry), sinf(rp), -cosf(rp) * cosf(ry)};
    }
    L.up = {0.0f, 1.0f, 0.0f};
    L.vel = {0.0f, 0.0f, 0.0f};

    bool moving = (keyW || keyA || keyS || keyD);
    audioUpdate(gAudioSys, gLevel, L, dt, moving, g.player.health);

    // Overlays
    if (g.player.damageAlpha > 0.0f)
    {
        g.player.damageAlpha -= dt * 0.5f;
        if (g.player.damageAlpha < 0.0f)
            g.player.damageAlpha = 0.0f;
    }
    if (g.player.healthAlpha > 0.0f)
    {
        g.player.healthAlpha -= dt * 0.9f;
        if (g.player.healthAlpha < 0.0f)
            g.player.healthAlpha = 0.0f;
    }

    updateEntities(dt);
    updateWeaponAnim(dt);

    // =========================
    // PROGRESSÃO DE FASE
    // Agora: precisa ter 3 chaves para atravessar o 'X'
    // (se você quiser exigir matar todos também, descomente o "cleared")
    // =========================
    if (gLevel.hasExit)
    {
        const int alive = countAliveEnemies();
        const bool hasKeys = (g.player.keys >= 3);

        // Se quiser exigir matar todos + 3 chaves, use isso:
        // const bool cleared = (alive == 0);
        // if (hasKeys && cleared)

        if (hasKeys)
        {
            float dx = camX - gLevel.exitX;
            float dz = camZ - gLevel.exitZ;
            float dist2 = dx * dx + dz * dz;

            const float r = GameConfig::TILE_SIZE * 1.2f;

            if (dist2 <= r * r)
            {
                gLevelIndex++;

                if (gLevelIndex >= 3)
                {
                    g.state = GameState::VITORIA;
                }
                else
                {
                    if (loadCurrentLevel())
                    {
                        gameReset(); // ideal: aqui você zera keys e recarrega altAmmo na troca de fase
                    }
                }
            }
        }
    }
    // GAME OVER
    if (g.player.health <= 0)
    {
        g.state = GameState::GAME_OVER;
        g.player.damageAlpha = 1.0f;
    }
}

// Função auxiliar para desenhar o mundo 3D
static void drawWorld3D()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    float radYaw = yaw * 3.14159265f / 180.0f;
    float radPitch = pitch * 3.14159265f / 180.0f;
    float dirX = cosf(radPitch) * sinf(radYaw);
    float dirY = sinf(radPitch);
    float dirZ = -cosf(radPitch) * cosf(radYaw);
    gluLookAt(camX, camY, camZ, camX + dirX, camY + dirY, camZ + dirZ, 0.0f, 1.0f, 0.0f);

    setSunDirectionEachFrame();
    drawSkydome(camX, camY, camZ, g.r);
    drawLevel(gLevel.map, camX, camZ, dirX, dirZ, g.r, g.time);
    drawEntities(gLevel.enemies, gLevel.items, camX, camZ, dirX, dirZ, g.r);
}

void gameRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 🔥 Garante que textura do menu nunca seja perdida
    g.r.texMenuBG = gAssets.texMenuBG;

    // 🔥 Reseta shader antes de qualquer coisa 2D
    glUseProgram(0);

    HudState hs;
    hs.playerHealth = g.player.health;
    hs.currentAmmo = g.player.currentAmmo;
    hs.reserveAmmo = g.player.reserveAmmo;
    hs.damageAlpha = g.player.damageAlpha;
    hs.healthAlpha = g.player.healthAlpha;
    hs.weaponState = g.weapon.state;

    if (g.state == GameState::MENU_INICIAL)
    {
        // Menu inicial só 2D
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        menuRender(janelaW, janelaH, g.time,
                   "",
                   "Pressione ENTER para Jogar",
                   g.r);
    }
    else if (g.state == GameState::GAME_OVER)
    {
        drawWorld3D();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        menuRender(janelaW, janelaH, g.time,
                   "GAME OVER",
                   "Pressione ENTER para Reiniciar",
                   g.r);
    }
    else if (g.state == GameState::VITORIA)
    {
        drawWorld3D();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        menuRender(janelaW, janelaH, g.time,
                   "VOCE VENCEU!",
                   "Pressione ENTER para voltar ao menu",
                   g.r);
    }
    else if (g.state == GameState::PAUSADO)
    {
        drawWorld3D();

        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        pauseMenuRender(janelaW, janelaH, g.time);
    }
    else // JOGANDO
    {
        drawWorld3D();

        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true);

        menuMeltRenderOverlay(janelaW, janelaH, g.time);
    }

    glutSwapBuffers();
}
#pragma once
#include <vector>

// Configurações da IA
const float ENEMY_SPEED = 2.5f;
const float ENEMY_VIEW_DIST = 15.0f;
const float ENEMY_ATTACK_DIST = 1.5f;

enum EnemyState
{
    STATE_IDLE,
    STATE_CHASE,
    STATE_ATTACK,
    STATE_DEAD
};

struct Enemy
{
    int type;
    float x, z;
    float hp;
    EnemyState state;
    float startX, startZ;

    float respawnTimer;

    int animFrame;
    float animTimer;

    float attackCooldown;
    float hurtTimer;

    // NOVO
    bool isBoss = false;
};

enum ItemType
{
    ITEM_HEALTH,
    ITEM_AMMO,

    // NOVO
    ITEM_KEY,        // dropa do inimigo
    ITEM_SPECIAL     // pega na fase 2 e libera ataque alternativo
};

struct Item
{
    float x, z;
    ItemType type;
    bool active;
    float respawnTimer;
};

bool isWalkable(float x, float z);
void updateEntities(float dt);
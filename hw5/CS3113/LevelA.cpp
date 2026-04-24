#include "LevelA.h"

LevelA::LevelA()                                      : Scene { {0.0f}, nullptr   } {}
LevelA::LevelA(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelA::~LevelA() { shutdown(); }

void LevelA::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.playerHit   = false;

    mGameState.bgm        = gBgm;
    mGameState.jumpSound  = gJumpSound;
    mGameState.stepSound  = gStepSound;
    mGameState.stompSound = gStompSound;
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);

    // tileset.png is laid out as 8 columns x 31 rows of 16x16 tiles.
    // Level data contains 1-based atlas indices directly; `solidTiles`
    // lists which atlas indices block movement (walls + furniture).
    // Only the perimeter wall blocks movement; furniture tiles are treated
    // as hazards (handled in update()) so touching them costs a life.
    // Bathroom wall + bathtub block movement. Blood decor is cosmetic.
    std::unordered_set<unsigned int> solidTiles = {
        9,
        185, 186, 187,
        193, 194, 195,
        201, 202, 203,
    };
    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,
        (unsigned int *) mLevelData,
        "assets/game/tileset.png",
        TILE_DIMENSION,
        8, 31,
        mOrigin,
        solidTiles,
        6 // base floor = gray tile
    );

    // character.png is a 10×20 atlas of 16×16 sprites. Row 5 (1-indexed,
    // so row index 4 → base frame 40), per-column layout:
    //   40 idle-down | 41 idle-up | 42 idle-right | 43 empty
    //   44,45 walk-down | 46,47 walk-up | 48,49 walk-right
    // LEFT reuses the right-facing frames and is mirrored by Entity::render.
    // Each walk cycle interleaves the idle pose for a natural step beat.
    std::map<Direction, std::vector<int>> playerAnim = {
        {DOWN,  { 40, 44, 40, 45 }},
        {UP,    { 41, 46, 41, 47 }},
        {LEFT,  { 42, 48, 42, 49 }},
        {RIGHT, { 42, 48, 42, 49 }},
    };

    float mapLeft = mGameState.map->getLeftBoundary();
    float mapTop  = mGameState.map->getTopBoundary();

    mGameState.xochitl = new Entity(
        { mapLeft + 3.0f * TILE_DIMENSION, mapTop + 6.5f * TILE_DIMENSION },
        { 64.0f, 64.0f },
        "assets/game/character.png",
        ATLAS,
        { 20, 10 },
        playerAnim,
        PLAYER
    );
    mGameState.xochitl->setColliderDimensions({ 24.0f, 20.0f });
    mGameState.xochitl->setColliderOffset({ 0.0f, 14.0f });
    // Top-down: no gravity
    mGameState.xochitl->setAcceleration({ 0.0f, 0.0f });
    mGameState.xochitl->setSpeed(200);

    // 3 scripted hazards (bob/sweep/orbit around anchors) + 1 follower.
    mGameState.enemyCount = ENEMY_COUNT;
    mGameState.enemies    = new Entity*[ENEMY_COUNT];

    struct HazardSpec {
        const char *texturePath;
        float col, row;
    };
    HazardSpec specs[HAZARD_COUNT] = {
        { "assets/game/job.png",       8.5f, 4.5f }, // vertical bob
        { "assets/game/job.png",      13.5f, 5.5f }, // horizontal sweep
        { "assets/game/leetcode.png", 17.5f, 4.5f }, // circular orbit
    };

    for (int i = 0; i < HAZARD_COUNT; i++)
    {
        Vector2 anchor = {
            mapLeft + specs[i].col * TILE_DIMENSION,
            mapTop  + specs[i].row * TILE_DIMENSION
        };
        mHazardAnchors[i] = anchor;

        mGameState.enemies[i] = new Entity(
            anchor,
            { TILE_DIMENSION, TILE_DIMENSION },
            specs[i].texturePath,
            NPC
        );
        mGameState.enemies[i]->setColliderDimensions({ TILE_DIMENSION * 0.6f, TILE_DIMENSION * 0.6f });
        mGameState.enemies[i]->setColliderOffset({ 0.0f, 0.0f });
        mGameState.enemies[i]->setAcceleration({ 0.0f, 0.0f });
        mGameState.enemies[i]->setSpeed(0);
    }

    // FOLLOWER — john_sterling chases the player in 2D via Entity::AIFollow.
    mGameState.enemies[HAZARD_COUNT] = new Entity(
        { mapLeft + 18.0f * TILE_DIMENSION, mapTop + 7.5f * TILE_DIMENSION },
        { TILE_DIMENSION, TILE_DIMENSION },
        "assets/game/john_sterling.png",
        NPC
    );
    mGameState.enemies[HAZARD_COUNT]->setColliderDimensions({ TILE_DIMENSION * 0.55f, TILE_DIMENSION * 0.55f });
    mGameState.enemies[HAZARD_COUNT]->setColliderOffset({ 0.0f, 0.0f });
    mGameState.enemies[HAZARD_COUNT]->setAcceleration({ 0.0f, 0.0f });
    mGameState.enemies[HAZARD_COUNT]->setAIType(FOLLOWER);
    mGameState.enemies[HAZARD_COUNT]->setAIState(IDLE);
    mGameState.enemies[HAZARD_COUNT]->setSpeed(110);

    std::map<Direction, std::vector<int>> endpointAnim = {
        {DOWN,  {5}},
        {LEFT,  {5}},
        {UP,    {5}},
        {RIGHT, {5}},
    };
    float endCol = LEVEL_WIDTH - 1.5f;
    mGameState.endpoint = new Entity(
        { mapLeft + endCol * TILE_DIMENSION, mapTop + 6.5f * TILE_DIMENSION },
        { TILE_DIMENSION, TILE_DIMENSION },
        "assets/game/world_tileset.png",
        ATLAS,
        { 3, 2 },
        endpointAnim,
        EMPTY
    );
    mGameState.endpoint->setColliderDimensions({ TILE_DIMENSION, TILE_DIMENSION});
}

void LevelA::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);
    mGameState.playerHit = false;

    mGameState.xochitl->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

    // Drive moving hazards: 0=vertical bob, 1=horizontal sweep, 2=circle.
    mHazardTime += deltaTime;
    const float bobAmp   = 35.0f;
    const float sweepAmp = 60.0f;
    const float orbitR   = 35.0f;
    const float omega    = 2.2f;

    if (mGameState.enemyCount >= HAZARD_COUNT && mGameState.enemies != nullptr)
    {
        Vector2 a0 = mHazardAnchors[0];
        mGameState.enemies[0]->setPosition({ a0.x, a0.y + bobAmp * sinf(omega * mHazardTime) });

        Vector2 a1 = mHazardAnchors[1];
        mGameState.enemies[1]->setPosition({ a1.x + sweepAmp * sinf(omega * mHazardTime), a1.y });

        Vector2 a2 = mHazardAnchors[2];
        mGameState.enemies[2]->setPosition({
            a2.x + orbitR * cosf(omega * mHazardTime),
            a2.y + orbitR * sinf(omega * mHazardTime)
        });
    }

    // Follower (index HAZARD_COUNT) runs the regular AI update path.
    for (int i = HAZARD_COUNT; i < mGameState.enemyCount; i++)
        mGameState.enemies[i]->update(deltaTime, mGameState.xochitl,
                                      mGameState.map, nullptr, 0);

    // Touching a moving hazard adds cortisol AND pushes the player out
    // so the hazards act as solid, damaging obstacles.
    for (int i = 0; i < mGameState.enemyCount; i++)
    {
        if (!mGameState.enemies[i]->isActive()) continue;
        if (mGameState.xochitl->checkCollisionWith(mGameState.enemies[i]))
        {
            mGameState.playerHit = true;
            mGameState.xochitl->resolveAgainst(mGameState.enemies[i]);
        }
    }

    // Bathtub tiles are solid AND damaging — edge-probe so pushing against
    // the tub costs cortisol.
    static const std::unordered_set<unsigned int> bathtubTiles = {
        185, 186, 187,
        193, 194, 195,
        201, 202, 203,
    };
    Vector2 cc = {
        mGameState.xochitl->getPosition().x + mGameState.xochitl->getColliderOffset().x,
        mGameState.xochitl->getPosition().y + mGameState.xochitl->getColliderOffset().y
    };
    float halfW = mGameState.xochitl->getColliderDimensions().x / 2.0f;
    float halfH = mGameState.xochitl->getColliderDimensions().y / 2.0f;
    const float EPS = 2.0f;
    Vector2 probes[4] = {
        { cc.x,                cc.y - halfH - EPS },
        { cc.x,                cc.y + halfH + EPS },
        { cc.x - halfW - EPS,  cc.y               },
        { cc.x + halfW + EPS,  cc.y               },
    };
    for (int i = 0; i < 4; i++)
    {
        if (bathtubTiles.count(mGameState.map->getTileAt(probes[i])))
        {
            mGameState.playerHit = true;
            break;
        }
    }

    if (mGameState.xochitl->checkCollisionWith(mGameState.endpoint))
        mGameState.nextSceneID = 2; // LevelB
}

void LevelA::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
    mGameState.map->render();
    mGameState.endpoint->render();
    for (int i = 0; i < mGameState.enemyCount; i++)
        mGameState.enemies[i]->render();
    mGameState.xochitl->render();
}

void LevelA::shutdown()
{
    delete mGameState.xochitl;
    mGameState.xochitl = nullptr;

    if (mGameState.enemies != nullptr)
    {
        for (int i = 0; i < mGameState.enemyCount; i++)
        {
            delete mGameState.enemies[i];
            mGameState.enemies[i] = nullptr;
        }
        delete[] mGameState.enemies;
    }
    mGameState.enemies = nullptr;

    delete mGameState.endpoint;
    mGameState.endpoint = nullptr;

    delete mGameState.map;
    mGameState.map = nullptr;

    StopMusicStream(mGameState.bgm);
}

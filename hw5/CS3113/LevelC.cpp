#include "LevelC.h"

LevelC::LevelC()                                      : Scene { {0.0f}, nullptr   } {}
LevelC::LevelC(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelC::~LevelC() { shutdown(); }

void LevelC::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.playerHit   = false;

    mGameState.bgm        = gBgm;
    mGameState.jumpSound  = gJumpSound;
    mGameState.stepSound  = gStepSound;
    mGameState.stompSound = gStompSound;
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);

    std::unordered_set<unsigned int> solidTiles = { 8 };
    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,
        (unsigned int *) mLevelData,
        "assets/game/tileset.png",
        TILE_DIMENSION,
        8, 31,
        mOrigin,
        solidTiles,
        5 // base floor
    );

    std::map<Direction, std::vector<int>> playerAnim = {
        {DOWN,  { 40, 44, 40, 45 }},
        {UP,    { 41, 46, 41, 47 }},
        {LEFT,  { 42, 48, 42, 49 }},
        {RIGHT, { 42, 48, 42, 49 }},
    };

    float mapLeft = mGameState.map->getLeftBoundary();
    float mapTop  = mGameState.map->getTopBoundary();

    mGameState.xochitl = new Entity(
        { mapLeft + 2.0f * TILE_DIMENSION, mapTop + 2.5f * TILE_DIMENSION },
        { 64.0f, 64.0f },
        "assets/game/character.png",
        ATLAS,
        { 20, 10 },
        playerAnim,
        PLAYER
    );
    mGameState.xochitl->setColliderDimensions({ 24.0f, 20.0f });
    mGameState.xochitl->setColliderOffset({ 0.0f, 14.0f });
    mGameState.xochitl->setAcceleration({ 0.0f, 0.0f });
    mGameState.xochitl->setSpeed(200);

    // 3 scripted hazards + 1 follower.
    mGameState.enemyCount = ENEMY_COUNT;
    mGameState.enemies    = new Entity*[ENEMY_COUNT];

    struct HazardSpec { const char *texturePath; float col, row; int motion; };
    HazardSpec specs[HAZARD_COUNT] = {
        // LEFT column
        { "assets/game/job.png",       3.0f,  6.0f, 0 }, // bob
        // MIDDLE
        { "assets/game/job.png",      15.0f,  8.0f, 1 }, // sweep
        // ENTIRE BOTTOM row — 4 entities spread across cols
        { "assets/game/job.png",       5.0f, 13.0f, 0 }, // bob
        { "assets/game/leetcode.png", 12.0f, 13.0f, 2 }, // orbit
        { "assets/game/job.png",      20.0f, 13.0f, 1 }, // sweep
        { "assets/game/leetcode.png", 25.0f, 13.0f, 2 }, // orbit
    };

    for (int i = 0; i < HAZARD_COUNT; i++)
    {
        Vector2 anchor = {
            mapLeft + specs[i].col * TILE_DIMENSION,
            mapTop  + specs[i].row * TILE_DIMENSION
        };
        mHazardAnchors[i] = anchor;
        mHazardMotions[i] = specs[i].motion;

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

    mGameState.enemies[HAZARD_COUNT] = new Entity(
        { mapLeft + 22.0f * TILE_DIMENSION, mapTop + 3.5f * TILE_DIMENSION },
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
    mGameState.endpoint = new Entity(
        { mapLeft + (LEVEL_WIDTH  - 2.5f) * TILE_DIMENSION,
          mapTop  + (LEVEL_HEIGHT - 1.5f) * TILE_DIMENSION },
        { TILE_DIMENSION, TILE_DIMENSION },
        "assets/game/world_tileset.png",
        ATLAS,
        { 3, 2 },
        endpointAnim,
        EMPTY
    );
    mGameState.endpoint->setColliderDimensions({ TILE_DIMENSION, TILE_DIMENSION });
}

void LevelC::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);
    mGameState.playerHit = false;

    mGameState.xochitl->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

    // Drive the 3 scripted hazards (bob / sweep / orbit) around anchors.
    mHazardTime += deltaTime;
    const float bobAmp   = 35.0f;
    const float sweepAmp = 60.0f;
    const float orbitR   = 35.0f;
    const float omega    = 2.0f;

    for (int i = 0; i < HAZARD_COUNT && mGameState.enemies != nullptr; i++)
    {
        Vector2 a = mHazardAnchors[i];
        switch (mHazardMotions[i])
        {
            case 0: // bob
                mGameState.enemies[i]->setPosition({ a.x,
                    a.y + bobAmp * sinf(omega * mHazardTime) });
                break;
            case 1: // sweep
                mGameState.enemies[i]->setPosition({
                    a.x + sweepAmp * sinf(omega * mHazardTime), a.y });
                break;
            case 2: // orbit
                mGameState.enemies[i]->setPosition({
                    a.x + orbitR * cosf(omega * mHazardTime),
                    a.y + orbitR * sinf(omega * mHazardTime) });
                break;
        }
    }

    // Follower (index HAZARD_COUNT) runs the regular AI update path.
    for (int i = HAZARD_COUNT; i < mGameState.enemyCount; i++)
        mGameState.enemies[i]->update(deltaTime, mGameState.xochitl,
                                      mGameState.map, nullptr, 0);

    // Top-down: enemy contact hurts the player and pushes them back out,
    // so enemies act as damaging solid obstacles.
    for (int i = 0; i < mGameState.enemyCount; i++)
    {
        if (!mGameState.enemies[i]->isActive()) continue;
        if (mGameState.xochitl->checkCollisionWith(mGameState.enemies[i]))
        {
            mGameState.playerHit = true;
            mGameState.xochitl->resolveAgainst(mGameState.enemies[i]);
        }
    }

    // Walking onto trash decor (tiles 24, 29, 30) costs cortisol.
    static const std::unordered_set<unsigned int> trashTiles = { 24, 29, 30 };
    Vector2 cc = {
        mGameState.xochitl->getPosition().x + mGameState.xochitl->getColliderOffset().x,
        mGameState.xochitl->getPosition().y + mGameState.xochitl->getColliderOffset().y
    };
    if (trashTiles.count(mGameState.map->getTileAt(cc)))
        mGameState.playerHit = true;

    // Reached the endpoint -> WIN
    if (mGameState.xochitl->checkCollisionWith(mGameState.endpoint))
        mGameState.nextSceneID = 4; // WinScene
}

void LevelC::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
    mGameState.map->render();
    mGameState.endpoint->render();
    for (int i = 0; i < mGameState.enemyCount; i++)
    {
        mGameState.enemies[i]->render();
    }
    mGameState.xochitl->render();
}

void LevelC::shutdown()
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

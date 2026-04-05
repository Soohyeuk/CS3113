#include "LevelB.h"

LevelB::LevelB()                                      : Scene { {0.0f}, nullptr   } {}
LevelB::LevelB(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelB::~LevelB() { shutdown(); }

void LevelB::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.playerHit   = false;

    mGameState.bgm        = LoadMusicStream("assets/game/levelb.ogg");
    mGameState.jumpSound  = LoadSound("assets/game/jumo_sound.wav");
    mGameState.stepSound  = LoadSound("assets/game/step_sound.wav");
    mGameState.stompSound = LoadSound("assets/game/kill_sound.wav");
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);
    mGameState.audioLoaded = true;

    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT,
        (unsigned int *) mLevelData,
        "assets/game/world_tileset.png",
        TILE_DIMENSION,
        2, 3,
        mOrigin
    );


    std::map<Direction, std::vector<int>> playerAnim = {
        {DOWN,  {  0,  1,  2,  3,  4,  5,  6,  7 }},
        {LEFT,  {  8,  9, 10, 11, 12, 13, 14, 15 }},
        {UP,    { 24, 25, 26, 27, 28, 29, 30, 31 }},
        {RIGHT, { 40, 41, 42, 43, 44, 45, 46, 47 }},
    };

    float sizeRatio = 40.0f / 64.0f;
    float mapLeft = mGameState.map->getLeftBoundary();
    float mapTop  = mGameState.map->getTopBoundary();

    mGameState.xochitl = new Entity(
        { mapLeft + 1.5f * TILE_DIMENSION, mapTop + 2.0f * TILE_DIMENSION },
        { 250.0f * sizeRatio, 250.0f },
        "assets/game/IDLE.png",
        ATLAS,
        { 1, 10 },
        playerAnim,
        PLAYER
    );
    mGameState.xochitl->configurePlayerAnimationSet(
        "assets/game/IDLE.png", { 1, 10 }, 10,
        "assets/game/RUN.png",  { 1, 16 }, 16
    );
    mGameState.xochitl->setJumpingPower(500.0f);
    mGameState.xochitl->setColliderDimensions({
        mGameState.xochitl->getScale().x / 3.2f,
        mGameState.xochitl->getScale().y / 2.8f
    });
    mGameState.xochitl->setColliderOffset({
        0.0f,
        45.0f
    });
    mGameState.xochitl->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
    mGameState.xochitl->setSpeed(200);

    // ---- ENEMIES ----
    mGameState.enemyCount = ENEMY_COUNT;
    mGameState.enemies    = new Entity*[ENEMY_COUNT];

    std::map<Direction, std::vector<int>> slimeAnim = {
        {DOWN,  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }},
        {LEFT,  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }},
        {UP,    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }},
        {RIGHT, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }},
    };

    //enemy 0: wanderer (green slime)
    mGameState.enemies[0] = new Entity();
    mGameState.enemies[0]->setPosition({ mapLeft + 3.0f * TILE_DIMENSION, mapTop + 3.0f * TILE_DIMENSION });
    mGameState.enemies[0]->setScale({ 50.0f, 50.0f });
    mGameState.enemies[0]->setColliderDimensions({ 30.0f, 24.0f });
    mGameState.enemies[0]->setColliderOffset({ 0.0f, 13.0f });
    mGameState.enemies[0]->setTexture("assets/game/slime_green.png");
    mGameState.enemies[0]->setTextureType(ATLAS);
    mGameState.enemies[0]->setSpriteSheetDimensions({ 3, 4 });
    mGameState.enemies[0]->setAnimationAtlas(slimeAnim);
    mGameState.enemies[0]->setFrameSpeed(10);
    mGameState.enemies[0]->setEntityType(NPC);
    mGameState.enemies[0]->setAIType(WANDERER);
    mGameState.enemies[0]->setAIState(WALKING);
    mGameState.enemies[0]->setDirection(LEFT);
    mGameState.enemies[0]->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
    mGameState.enemies[0]->setSpeed(100);

    //enemy 1: follower (purple slime)
    mGameState.enemies[1] = new Entity();
    mGameState.enemies[1]->setPosition({ mapLeft + 16.0f * TILE_DIMENSION, mapTop + 3.0f * TILE_DIMENSION });
    mGameState.enemies[1]->setScale({ 50.0f, 50.0f });
    mGameState.enemies[1]->setColliderDimensions({ 30.0f, 24.0f });
    mGameState.enemies[1]->setColliderOffset({ 0.0f, 13.0f });
    mGameState.enemies[1]->setTexture("assets/game/slime_purple.png");
    mGameState.enemies[1]->setTextureType(ATLAS);
    mGameState.enemies[1]->setSpriteSheetDimensions({ 3, 4 });
    mGameState.enemies[1]->setAnimationAtlas(slimeAnim);
    mGameState.enemies[1]->setFrameSpeed(10);
    mGameState.enemies[1]->setEntityType(NPC);
    mGameState.enemies[1]->setAIType(FOLLOWER);
    mGameState.enemies[1]->setAIState(IDLE);
    mGameState.enemies[1]->setDirection(LEFT);
    mGameState.enemies[1]->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
    mGameState.enemies[1]->setSpeed(120);

    std::map<Direction, std::vector<int>> endpointAnim = {
        {DOWN,  {5}},
        {LEFT,  {5}},
        {UP,    {5}},
        {RIGHT, {5}},
    };
    float endCol = LEVEL_WIDTH - 1.5f;
    mGameState.endpoint = new Entity(
        { mapLeft + endCol * TILE_DIMENSION, mapTop + 4.5f * TILE_DIMENSION },
        { TILE_DIMENSION, TILE_DIMENSION },
        "assets/game/world_tileset.png",
        ATLAS,
        { 3, 2 },
        endpointAnim,
        NONE
    );
    mGameState.endpoint->setColliderDimensions({ TILE_DIMENSION, TILE_DIMENSION });
}

void LevelB::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);

    mGameState.xochitl->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

    for (int i = 0; i < mGameState.enemyCount; i++)
        mGameState.enemies[i]->update(deltaTime, mGameState.xochitl, mGameState.map, nullptr, 0);

    for (int i = 0; i < mGameState.enemyCount; i++)
    {
        if (!mGameState.enemies[i]->isActive()) continue;
        if (mGameState.xochitl->checkCollisionWith(mGameState.enemies[i]))
        {
            if (mGameState.xochitl->getPosition().y < mGameState.enemies[i]->getPosition().y &&
                mGameState.xochitl->getVelocity().y > 0)
            {
                mGameState.enemies[i]->deactivate();
                mGameState.xochitl->jump();
                PlaySound(mGameState.stompSound);
            }
            else
            {
                mGameState.playerHit = true;
            }
        }
    }

    if (mGameState.xochitl->getPosition().y > FALL_THRESHOLD)
        mGameState.playerHit = true;

    // Reached the endpoint
    if (mGameState.xochitl->checkCollisionWith(mGameState.endpoint))
        mGameState.nextSceneID = 3; // LevelC
}

void LevelB::render()
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

void LevelB::shutdown()
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

    if (mGameState.audioLoaded)
    {
        UnloadMusicStream(mGameState.bgm);
        UnloadSound(mGameState.jumpSound);
        UnloadSound(mGameState.stepSound);
        UnloadSound(mGameState.stompSound);
        mGameState.audioLoaded = false;
    }
}

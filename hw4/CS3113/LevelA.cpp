#include "LevelA.h"

LevelA::LevelA()                                      : Scene { {0.0f}, nullptr   } {}
LevelA::LevelA(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LevelA::~LevelA() { shutdown(); }

void LevelA::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.playerHit   = false;

    mGameState.bgm        = LoadMusicStream("assets/game/levela.ogg");
    mGameState.jumpSound  = LoadSound("assets/game/jumo_sound.wav");
    mGameState.stepSound  = LoadSound("assets/game/step_sound.wav");
    mGameState.stompSound = LoadSound("assets/game/kill_sound.wav");
    SetMusicVolume(mGameState.bgm, 0.33f);
    PlayMusicStream(mGameState.bgm);
    mGameState.audioLoaded = true;

    // world_tileset.png is arranged as 2 columns x 3 rows
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
    std::map<Direction, std::vector<int>> slimeAnim = {
        {DOWN,  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }},
        {LEFT,  { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }},
        {UP,    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }},
        {RIGHT, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }},
    };

    mGameState.enemyCount = ENEMY_COUNT;
    mGameState.enemies    = new Entity*[ENEMY_COUNT];

    mGameState.enemies[0] = new Entity(
        { mapLeft + 4.0f * TILE_DIMENSION, mapTop + 3.0f * TILE_DIMENSION },
        { 50.0f, 50.0f },
        "assets/game/slime_green.png",
        ATLAS, { 3, 4 }, slimeAnim, NPC
    );

    //for level a,b,c you will see a lot of this instead of initializing enemies with the constructor
    //it's simply bc i can't keep up with the parameters lol, so i just initialized using setters for readability
    mGameState.enemies[0]->setColliderDimensions({ 30.0f, 24.0f });
    mGameState.enemies[0]->setColliderOffset({ 0.0f, 13.0f });
    mGameState.enemies[0]->setAIType(WANDERER);
    mGameState.enemies[0]->setAIState(WALKING);
    mGameState.enemies[0]->setDirection(LEFT);
    mGameState.enemies[0]->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
    mGameState.enemies[0]->setSpeed(80);

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
    mGameState.endpoint->setColliderDimensions({ TILE_DIMENSION, TILE_DIMENSION});
}

void LevelA::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);

    mGameState.xochitl->update(deltaTime, nullptr, mGameState.map, nullptr, 0);

    for (int i = 0; i < mGameState.enemyCount; i++)
        mGameState.enemies[i]->update(deltaTime, mGameState.xochitl, mGameState.map, nullptr, 0);

    // Player-enemy collision
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

    // Fall off map
    if (mGameState.xochitl->getPosition().y > FALL_THRESHOLD)
        mGameState.playerHit = true;

    // Reached the endpoint
    if (mGameState.xochitl->checkCollisionWith(mGameState.endpoint))
        mGameState.nextSceneID = 2; // LevelB
}

void LevelA::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));
    mGameState.map->render();
    mGameState.endpoint->render();
    for (int i = 0; i < mGameState.enemyCount; i++)
    {
        mGameState.enemies[i]->render();
    }
    mGameState.xochitl->render();
    // mGameState.xochitl->displayCollider();
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

    if (mGameState.audioLoaded)
    {
        UnloadMusicStream(mGameState.bgm);
        UnloadSound(mGameState.jumpSound);
        UnloadSound(mGameState.stepSound);
        UnloadSound(mGameState.stompSound);
        mGameState.audioLoaded = false;
    }
}

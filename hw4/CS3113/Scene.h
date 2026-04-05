#include "Entity.h"

#ifndef SCENE_H
#define SCENE_H

struct GameState
{
    Entity *xochitl  = nullptr;
    Entity **enemies = nullptr;  // array of pointers
    Entity *endpoint = nullptr;
    int enemyCount   = 0;

    Map *map = nullptr;

    Music bgm       = {0};
    Sound jumpSound = {0};
    Sound stepSound = {0};
    Sound stompSound= {0};
    bool audioLoaded = false;  // guard for safe unloading

    int nextSceneID  = -1;
    bool playerHit   = false;
};

class Scene
{
protected:
    GameState mGameState;
    Vector2 mOrigin;
    const char *mBGColourHexCode = "#000000";

public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);

    virtual void initialise() = 0;
    virtual void processInput();
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;

    GameState&  getState()                   { return mGameState;       }
    Vector2     getOrigin()          const   { return mOrigin;          }
    const char* getBGColourHexCode() const   { return mBGColourHexCode; }
};

#endif

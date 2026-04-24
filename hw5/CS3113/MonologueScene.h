#include "Scene.h"

#ifndef MONOLOGUE_SCENE_H
#define MONOLOGUE_SCENE_H

class MonologueScene : public Scene
{
private:
    std::vector<std::string> mLines;
    int                      mCurrentLine;
    int                      mAfterSceneID;
    Texture2D                mCharacterTex;
    bool                     mTexLoaded;

public:
    MonologueScene();
    MonologueScene(Vector2 origin, const char *bgHexCode);
    ~MonologueScene();

    void configure(std::vector<std::string> lines, int afterSceneID);

    void initialise() override;
    void processInput() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif

#include "Entity.h"

Entity::Entity() : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f},
                   mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
                   mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderOffset {0.0f, 0.0f},
                   mIdleTexture {0}, mRunTexture {0},
                   mIdleSpriteSheetDimensions {1.0f, 1.0f},
                   mRunSpriteSheetDimensions {1.0f, 1.0f},
                   mTexture {0}, mTextureType {SINGLE}, mAngle {0.0f},
                   mSpriteSheetDimensions {}, mDirection {RIGHT},
                   mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0},
                   mSpeed {DEFAULT_SPEED}, mEntityType {NONE},
                   mAIType {WANDERER}, mAIState {IDLE} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
    EntityType entityType) : mPosition {position}, mVelocity {0.0f, 0.0f},
    mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f},
    mColliderDimensions {scale}, mColliderOffset {0.0f, 0.0f},
    mIdleTexture {0}, mRunTexture {0},
    mIdleSpriteSheetDimensions {1.0f, 1.0f},
    mRunSpriteSheetDimensions {1.0f, 1.0f},
    mTexture {LoadTexture(textureFilepath)},
    mTextureType {SINGLE}, mDirection {RIGHT}, mAnimationAtlas {{}},
    mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED},
    mAngle {0.0f}, mEntityType {entityType},
    mAIType {WANDERER}, mAIState {IDLE} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
        TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction,
        std::vector<int>> animationAtlas, EntityType entityType) :
        mPosition {position}, mVelocity {0.0f, 0.0f},
        mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
        mColliderDimensions {scale}, mColliderOffset {0.0f, 0.0f},
        mIdleTexture {0}, mRunTexture {0},
        mIdleSpriteSheetDimensions {1.0f, 1.0f},
        mRunSpriteSheetDimensions {1.0f, 1.0f},
        mTexture {LoadTexture(textureFilepath)},
        mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
        mAnimationIndices {animationAtlas.at(RIGHT)},
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f },
        mSpeed { DEFAULT_SPEED }, mEntityType {entityType},
        mAIType {WANDERER}, mAIState {IDLE} { }

Entity::~Entity()
{
    if (mUsesPlayerAnimationSet)
    {
        if (mIdleTexture.id != 0) UnloadTexture(mIdleTexture);
        if (mRunTexture.id  != 0) UnloadTexture(mRunTexture);
    }
    else if (mTexture.id != 0)
    {
        UnloadTexture(mTexture);
    }
};

//was created to manage multiple sprite sheets for the play character
void Entity::configurePlayerAnimationSet(const char *idleTexturePath,
    Vector2 idleSpriteSheetDimensions, int idleFrameCount,
    const char *runTexturePath, Vector2 runSpriteSheetDimensions,
    int runFrameCount)
{
    if (mUsesPlayerAnimationSet)
    {
        if (mIdleTexture.id != 0) UnloadTexture(mIdleTexture);
        if (mRunTexture.id  != 0) UnloadTexture(mRunTexture);
    }
    else if (mTexture.id != 0)
    {
        UnloadTexture(mTexture);
    }

    mIdleTexture = LoadTexture(idleTexturePath);
    mRunTexture  = LoadTexture(runTexturePath);

    mIdleSpriteSheetDimensions = idleSpriteSheetDimensions;
    mRunSpriteSheetDimensions  = runSpriteSheetDimensions;

    mIdleAnimationIndices.clear();
    for (int i = 0; i < idleFrameCount; i++) mIdleAnimationIndices.push_back(i);

    mRunAnimationIndices.clear();
    for (int i = 0; i < runFrameCount; i++) mRunAnimationIndices.push_back(i);

    mUsesPlayerAnimationSet = true;
    mPlayerAnimState = PLAYER_IDLE;
    mTextureType = ATLAS;
    mTexture = mIdleTexture;
    mSpriteSheetDimensions = mIdleSpriteSheetDimensions;
    mAnimationIndices = mIdleAnimationIndices;
    mCurrentFrameIndex = 0;
}

void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            float yDistance = fabs((mPosition.y + mColliderOffset.y) -
                                   (collidableEntity->mPosition.y + collidableEntity->mColliderOffset.y));
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) -
                              (collidableEntity->mColliderDimensions.y / 2.0f));

            if (mVelocity.y > 0)
            {
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;
            } else if (mVelocity.y < 0)
            {
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;

                if (collidableEntity->mEntityType == BLOCK)
                    collidableEntity->deactivate();
            }
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            float yDistance = fabs((mPosition.y + mColliderOffset.y) -
                                   (collidableEntity->mPosition.y + collidableEntity->mColliderOffset.y));
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - (collidableEntity->mColliderDimensions.y / 2.0f));

            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs((mPosition.x + mColliderOffset.x) -
                                   (collidableEntity->mPosition.x + collidableEntity->mColliderOffset.x));
            float xOverlap  = fabs(xDistance - (mColliderDimensions.x / 2.0f) - (collidableEntity->mColliderDimensions.x / 2.0f));

            if (mVelocity.x > 0) {
                mPosition.x     -= xOverlap;
                mVelocity.x      = 0;
                mIsCollidingRight = true;
            } else if (mVelocity.x < 0) {
                mPosition.x    += xOverlap;
                mVelocity.x     = 0;
                mIsCollidingLeft = true;
            }
        }
    }
}

void Entity::checkCollisionY(Map *map)
{
    if (map == nullptr) return;

    Vector2 colliderCentre = { mPosition.x + mColliderOffset.x, mPosition.y + mColliderOffset.y };

    Vector2 topCentreProbe    = { colliderCentre.x, colliderCentre.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topLeftProbe      = { colliderCentre.x - (mColliderDimensions.x / 2.0f), colliderCentre.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topRightProbe     = { colliderCentre.x + (mColliderDimensions.x / 2.0f), colliderCentre.y - (mColliderDimensions.y / 2.0f) };

    Vector2 bottomCentreProbe = { colliderCentre.x, colliderCentre.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomLeftProbe   = { colliderCentre.x - (mColliderDimensions.x / 2.0f), colliderCentre.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomRightProbe  = { colliderCentre.x + (mColliderDimensions.x / 2.0f), colliderCentre.y + (mColliderDimensions.y / 2.0f) };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ABOVE (jumping upward)
    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap)) && mVelocity.y < 0.0f)
    {
        mPosition.y += yOverlap;
        mVelocity.y  = 0.0f;
        mIsCollidingTop = true;
    }

    // COLLISION BELOW (falling downward)
    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
         map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap)   ||
         map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap)) && mVelocity.y > 0.0f)
    {
        mPosition.y -= yOverlap;
        mVelocity.y  = 0.0f;
        mIsCollidingBottom = true;
    }
}

void Entity::checkCollisionX(Map *map)
{
    if (map == nullptr) return;

    Vector2 colliderCentre = { mPosition.x + mColliderOffset.x, mPosition.y + mColliderOffset.y };

    Vector2 leftCentreProbe   = { colliderCentre.x - (mColliderDimensions.x / 2.0f), colliderCentre.y };
    Vector2 rightCentreProbe  = { colliderCentre.x + (mColliderDimensions.x / 2.0f), colliderCentre.y };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    if (map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap)
         && mVelocity.x > 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x -= xOverlap * 1.01f;
        mVelocity.x  = 0.0f;
        mIsCollidingRight = true;
    }

    if (map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap)
         && mVelocity.x < 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x += xOverlap * 1.01f;
        mVelocity.x  = 0.0f;
        mIsCollidingLeft = true;
    }
}

bool Entity::isColliding(Entity *other) const
{
    if (!other->isActive() || other == this) return false;

    float xDistance = fabs((mPosition.x + mColliderOffset.x) -
                           (other->mPosition.x + other->mColliderOffset.x)) -
        ((mColliderDimensions.x + other->mColliderDimensions.x) / 2.0f);
    float yDistance = fabs((mPosition.y + mColliderOffset.y) -
                           (other->mPosition.y + other->mColliderOffset.y)) -
        ((mColliderDimensions.y + other->mColliderDimensions.y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void Entity::animate(float deltaTime)
{
    if (!(mEntityType == PLAYER && mUsesPlayerAnimationSet))
        mAnimationIndices = mAnimationAtlas.at(mDirection);

    mAnimationTime += deltaTime;
    float framesPerSecond = 1.0f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::AIWander(Map *map)
{
    // Reverse direction on wall collision (flags from previous frame)
    if (mIsCollidingLeft)  mDirection = RIGHT;
    if (mIsCollidingRight) mDirection = LEFT;

    // Edge detection: check if there's ground ahead
    if (map != nullptr)
    {
        constexpr float EDGE_LOOKAHEAD = 2.0f;
        constexpr float GROUND_PROBE_DEPTH = 1.0f;
        Vector2 colliderCentre = { mPosition.x + mColliderOffset.x, mPosition.y + mColliderOffset.y };

        float aheadX;
        if (mDirection == LEFT)
            aheadX = colliderCentre.x - (mColliderDimensions.x / 2.0f + EDGE_LOOKAHEAD);
        else
            aheadX = colliderCentre.x + (mColliderDimensions.x / 2.0f + EDGE_LOOKAHEAD);

        float belowY = colliderCentre.y + mColliderDimensions.y / 2.0f + GROUND_PROBE_DEPTH;

        float xO = 0, yO = 0;
        if (!map->isSolidTileAt({aheadX, belowY}, &xO, &yO))
        {
            // No ground ahead, reverse
            if (mDirection == LEFT) mDirection = RIGHT;
            else                    mDirection = LEFT;
        }
    }

    if (mDirection == LEFT) moveLeft();
    else                    moveRight();
}

void Entity::AIFollow(Entity *target)
{
    switch (mAIState)
    {
    case IDLE:
        if (Vector2Distance(mPosition, target->getPosition()) < 300.0f)
            mAIState = WALKING;
        break;

    case WALKING:
        if (mPosition.x > target->getPosition().x) moveLeft();
        else                                        moveRight();
        break;

    default:
        break;
    }
}

void Entity::AIFlyer()
{
    mPatrolTimer += 1.0f / 60.0f;

    // Horizontal: bounce using timer
    if (fmod(mPatrolTimer, 5.0f) < 2.5f) mMovement.x = -1;
    else                                   mMovement.x =  1;

    // Vertical: oscillate faster
    if (fmod(mPatrolTimer, 3.0f) < 1.5f) mMovement.y = -1;
    else                                   mMovement.y =  1;

    // Set direction for sprite flipping
    mDirection = (mMovement.x < 0) ? LEFT : RIGHT;
}

void Entity::AIActivate(Entity *target, Map *map)
{
    switch (mAIType)
    {
    case WANDERER:
        AIWander(map);
        break;

    case FOLLOWER:
        AIFollow(target);
        break;

    case FLYER:
        AIFlyer();
        break;

    default:
        break;
    }
}

void Entity::update(float deltaTime, Entity *player, Map *map,
    Entity *collidableEntities, int collisionCheckCount)
{
    if (mEntityStatus == INACTIVE) return;

    if (mEntityType == NPC) AIActivate(player, map);

    resetColliderFlags();

    mVelocity.x = mMovement.x * mSpeed;

    // Flyers control their own vertical movement (no gravity)
    if (mEntityType == NPC && mAIType == FLYER)
    {
        mVelocity.y = mMovement.y * mSpeed;
    }
    else
    {
        mVelocity.x += mAcceleration.x * deltaTime;
        mVelocity.y += mAcceleration.y * deltaTime;
    }

    // ––––– JUMPING ––––– //
    if (mIsJumping)
    {
        mIsJumping = false;
        mVelocity.y -= mJumpingPower;
    }

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);
    // Flyers skip map collision
    if (!(mEntityType == NPC && mAIType == FLYER)) checkCollisionY(map);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);
    if (!(mEntityType == NPC && mAIType == FLYER)) checkCollisionX(map);

    if (mEntityType == PLAYER && mUsesPlayerAnimationSet)
    {
        PlayerAnimState nextState = PLAYER_IDLE;
        if (fabs(mMovement.x) > 0.0f) nextState = PLAYER_RUN;

        if (nextState != mPlayerAnimState)
        {
            mPlayerAnimState = nextState;
            mCurrentFrameIndex = 0;
            mAnimationTime = 0.0f;
        }

        switch (mPlayerAnimState)
        {
        case PLAYER_RUN:
            mTexture = mRunTexture;
            mSpriteSheetDimensions = mRunSpriteSheetDimensions;
            mAnimationIndices = mRunAnimationIndices;
            break;

        case PLAYER_IDLE:
        default:
            mTexture = mIdleTexture;
            mSpriteSheetDimensions = mIdleSpriteSheetDimensions;
            mAnimationIndices = mIdleAnimationIndices;
            break;
        }

        if (!mAnimationIndices.empty())
            animate(deltaTime);

        return;
    }

    if (mTextureType == ATLAS && GetLength(mMovement) != 0)
    {
        // NPCs animate whenever moving; player only when on ground
        if (mEntityType == NPC || mIsCollidingBottom)
            animate(deltaTime);
    }
}

void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;

    Rectangle textureArea;

    switch (mTextureType)
    {
        case SINGLE:
            textureArea = {
                0.0f, 0.0f,
                static_cast<float>(mTexture.width),
                static_cast<float>(mTexture.height)
            };
            // Flip sprite when facing left
            if (mDirection == LEFT) textureArea.width = -textureArea.width;
            break;
        case ATLAS:
            textureArea = getUVRectangle(
                &mTexture,
                mAnimationIndices[mCurrentFrameIndex],
                mSpriteSheetDimensions.x,
                mSpriteSheetDimensions.y
            );
            // Flip sprite when facing left
            if (mDirection == LEFT) textureArea.width = -textureArea.width;
            break;

        default: break;
    }

    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    DrawTexturePro(
        mTexture,
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );
}

void Entity::displayCollider()
{
    Rectangle colliderBox = {
        mPosition.x + mColliderOffset.x - mColliderDimensions.x / 2.0f,
        mPosition.y + mColliderOffset.y - mColliderDimensions.y / 2.0f,
        mColliderDimensions.x,
        mColliderDimensions.y
    };

    DrawRectangleLines(
        colliderBox.x,
        colliderBox.y,
        colliderBox.width,
        colliderBox.height,
        GREEN
    );
}

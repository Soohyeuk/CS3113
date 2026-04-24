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
                   mSpeed {DEFAULT_SPEED}, mEntityType {EMPTY},
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

// Top-down wall collision (symmetric on both axes).
void Entity::checkCollisionY(Map *map)
{
    if (map == nullptr) return;

    Vector2 c = { mPosition.x + mColliderOffset.x, mPosition.y + mColliderOffset.y };
    float halfW = mColliderDimensions.x / 2.0f;
    float halfH = mColliderDimensions.y / 2.0f;

    Vector2 topProbe    = { c.x, c.y - halfH };
    Vector2 bottomProbe = { c.x, c.y + halfH };

    float xO = 0.0f, yO = 0.0f;

    if (map->isSolidTileAt(topProbe, &xO, &yO) && mVelocity.y < 0.0f)
    {
        mPosition.y    += yO;
        mVelocity.y     = 0.0f;
        mIsCollidingTop = true;
    }
    if (map->isSolidTileAt(bottomProbe, &xO, &yO) && mVelocity.y > 0.0f)
    {
        mPosition.y       -= yO;
        mVelocity.y        = 0.0f;
        mIsCollidingBottom = true;
    }
}

void Entity::checkCollisionX(Map *map)
{
    if (map == nullptr) return;

    Vector2 c = { mPosition.x + mColliderOffset.x, mPosition.y + mColliderOffset.y };
    float halfW = mColliderDimensions.x / 2.0f;
    float halfH = mColliderDimensions.y / 2.0f;

    Vector2 leftProbe  = { c.x - halfW, c.y };
    Vector2 rightProbe = { c.x + halfW, c.y };

    float xO = 0.0f, yO = 0.0f;

    if (map->isSolidTileAt(rightProbe, &xO, &yO) && mVelocity.x > 0.0f)
    {
        mPosition.x      -= xO;
        mVelocity.x       = 0.0f;
        mIsCollidingRight = true;
    }
    if (map->isSolidTileAt(leftProbe, &xO, &yO) && mVelocity.x < 0.0f)
    {
        mPosition.x     += xO;
        mVelocity.x      = 0.0f;
        mIsCollidingLeft = true;
    }
}

// Push this entity out of `other` along the smaller-overlap axis.
// Used for "damage-and-block" hazards that aren't part of the tilemap.
void Entity::resolveAgainst(Entity *other)
{
    if (other == nullptr || other == this) return;
    if (!other->isActive() || mEntityStatus == INACTIVE) return;
    if (!isColliding(other)) return;

    float ax = mPosition.x + mColliderOffset.x;
    float ay = mPosition.y + mColliderOffset.y;
    float bx = other->mPosition.x + other->mColliderOffset.x;
    float by = other->mPosition.y + other->mColliderOffset.y;

    float xOverlap = (mColliderDimensions.x + other->mColliderDimensions.x) / 2.0f - fabs(ax - bx);
    float yOverlap = (mColliderDimensions.y + other->mColliderDimensions.y) / 2.0f - fabs(ay - by);

    if (xOverlap <= 0.0f || yOverlap <= 0.0f) return;

    if (xOverlap < yOverlap)
    {
        if (ax < bx) { mPosition.x -= xOverlap; mIsCollidingRight = true; }
        else         { mPosition.x += xOverlap; mIsCollidingLeft  = true; }
        mVelocity.x = 0.0f;
    }
    else
    {
        if (ay < by) { mPosition.y -= yOverlap; mIsCollidingBottom = true; }
        else         { mPosition.y += yOverlap; mIsCollidingTop    = true; }
        mVelocity.y = 0.0f;
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
    (void)map;
    // Top-down: simple horizontal bounce on wall collision
    if (mIsCollidingLeft)  mDirection = RIGHT;
    if (mIsCollidingRight) mDirection = LEFT;

    if (mDirection == LEFT) moveLeft();
    else                    moveRight();
}

void Entity::AIFollow(Entity *target)
{
    switch (mAIState)
    {
    case IDLE:
        if (Vector2Distance(mPosition, target->getPosition()) < 400.0f)
            mAIState = WALKING;
        break;

    case WALKING:
    {
        // 2D pursuit — set x and y movement toward the target so the
        // follower tracks the player in a top-down maze, not just L/R.
        float dx = target->getPosition().x - mPosition.x;
        float dy = target->getPosition().y - mPosition.y;
        resetMovement();
        if (fabs(dx) > 4.0f) { if (dx > 0) moveRight(); else moveLeft(); }
        if (fabs(dy) > 4.0f) { if (dy > 0) moveDown();  else moveUp();   }
        if (GetLength(mMovement) > 1.0f) normaliseMovement();
        break;
    }

    default:
        break;
    }
}

void Entity::AIFlyer()
{
    // Flyer was a platformer-only behaviour; reuse as a 2-axis patroller.
    mPatrolTimer += 1.0f / 60.0f;
    if (fmod(mPatrolTimer, 5.0f) < 2.5f) mMovement.x = -1;
    else                                 mMovement.x =  1;
    if (fmod(mPatrolTimer, 3.0f) < 1.5f) mMovement.y = -1;
    else                                 mMovement.y =  1;
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

    // Top-down: both axes driven directly by movement input
    mVelocity.x = mMovement.x * mSpeed;
    mVelocity.y = mMovement.y * mSpeed;

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);
    checkCollisionY(map);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);
    checkCollisionX(map);

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

    if (mTextureType == ATLAS)
    {
        if (GetLength(mMovement) != 0)
        {
            animate(deltaTime);
        }
        else if (mEntityType == PLAYER)
        {
            // Idle: snap to the first frame of the current direction.
            if (!mAnimationIndices.empty()) mCurrentFrameIndex = 0;
            mAnimationTime = 0.0f;
        }
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

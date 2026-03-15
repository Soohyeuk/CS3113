#include "Entity.h"

Entity::Entity() : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f},
                   mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
                   mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderOffset {0.0f, 0.0f},
                   mTexture {NULL}, mTextureType {SINGLE}, mAngle {0.0f},
                   mSpriteSheetDimensions {}, mDirection {RIGHT},
                   mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0},
                   mEntityType {NONE} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
    EntityType entityType) : mPosition {position}, mVelocity {0.0f, 0.0f},
    mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f},
    mColliderDimensions {scale}, mColliderOffset {0.0f, 0.0f},
    mTexture {LoadTexture(textureFilepath)},
    mTextureType {SINGLE}, mDirection {RIGHT}, mAnimationAtlas {{}},
    mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED},
    mAngle {0.0f}, mEntityType {entityType} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
        TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction,
        std::vector<int>> animationAtlas, EntityType entityType) :
        mPosition {position}, mVelocity {0.0f, 0.0f},
        mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
        mColliderDimensions {scale}, mColliderOffset {0.0f, 0.0f},
        mTexture {LoadTexture(textureFilepath)},
        mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
        mAnimationIndices {animationAtlas.at(RIGHT)},
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f },
        mSpeed { DEFAULT_SPEED }, mEntityType {entityType} { }

Entity::~Entity() { UnloadTexture(mTexture); };

void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            float myY = mPosition.y + mColliderOffset.y;
            float otherY = collidableEntity->mPosition.y + collidableEntity->mColliderOffset.y;
            float yDistance = fabs(myY - otherY);
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
            float myY = mPosition.y + mColliderOffset.y;
            float otherY = collidableEntity->mPosition.y + collidableEntity->mColliderOffset.y;
            float yDistance = fabs(myY - otherY);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - (collidableEntity->mColliderDimensions.y / 2.0f));

            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float myX = mPosition.x + mColliderOffset.x;
            float otherX = collidableEntity->mPosition.x + collidableEntity->mColliderOffset.x;
            float xDistance = fabs(myX - otherX);
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

bool Entity::isColliding(Entity *other) const
{
    if (!other->isActive() || other == this) return false;

    float myX = mPosition.x + mColliderOffset.x;
    float myY = mPosition.y + mColliderOffset.y;
    float otherX = other->mPosition.x + other->mColliderOffset.x;
    float otherY = other->mPosition.y + other->mColliderOffset.y;

    float xDistance = fabs(myX - otherX) -
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(myY - otherY) -
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void Entity::animate(float deltaTime)
{
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

void Entity::update(float deltaTime, Entity *collidableEntities,
    int collisionCheckCount)
{
    if (mEntityStatus == INACTIVE) return;

    resetColliderFlags();

    if (mEntityType != PLAYER)
        mVelocity.x = mMovement.x * mSpeed;

    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    if (mIsJumping)
    {
        mIsJumping = false;
        mVelocity.y -= mJumpingPower;
    }

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);

    if (mTextureType == ATLAS && GetLength(mMovement) != 0 && mIsCollidingBottom)
        animate(deltaTime);
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
            break;
        case ATLAS:
            textureArea = getUVRectangle(
                &mTexture,
                mAnimationIndices[mCurrentFrameIndex],
                mSpriteSheetDimensions.x,
                mSpriteSheetDimensions.y
            );

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

    // displayCollider();
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

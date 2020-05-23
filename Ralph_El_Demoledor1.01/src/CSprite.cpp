#include "../include/CSprite.h"

CSprite::CSprite()
{
    mFrame = 0;
    mX= 0;
    mY= 0;
    mOldX= 0;
    mOldY= 0;
    mAnimating= false;
    mDrawn= 0;
    mSpeed= 0;
    mLastupdate= 0;
    mSpriteBase=new CSpriteBase;
    mBackreplacement=NULL;
    mScreen=NULL;
}

int CSprite::init(char *dir, SDL_Surface *screen)
{
    mSpriteBase->init(dir);

    if (mSpriteBase->mBuilt)
    {
        if (mSpriteBase->mNumframes>1)
            mAnimating=true;
        mBackreplacement = SDL_DisplayFormat(mSpriteBase->mAnim[0].image);
    }
    mScreen = screen;
    return 0;
}

void CSprite::clearBG()
{
    if (mDrawn==1)
    {
        SDL_Rect dest;
        dest.x = (int)mOldX;
        dest.y = (int)mOldY;
        dest.w = mSpriteBase->mW;
        dest.h = mSpriteBase->mH;
        SDL_BlitSurface(mBackreplacement, NULL, mScreen, &dest);
    }
}

void CSprite::updateBG()
{
    SDL_Rect srcrect;
    srcrect.x = (int)mX;
    srcrect.y = (int)mY;
    srcrect.w = mSpriteBase->mW;
    srcrect.h = mSpriteBase->mH;
    mOldX=mX;
    mOldY=mY;
    SDL_BlitSurface(mScreen, &srcrect, mBackreplacement, NULL);
}

void CSprite::draw()
{

    if (mAnimating)
    {
        if (mLastupdate+mSpriteBase->mAnim[mFrame].pause*mSpeed<SDL_GetTicks())
        {

            mFrame++;
            if (mFrame>mSpriteBase->mNumframes-1) mFrame=0;
            mLastupdate = SDL_GetTicks();
        }
    }

    if (mDrawn==0) mDrawn=1;

    SDL_Rect dest;
    dest.x = (int)mX;
    dest.y = (int)mY;
    SDL_BlitSurface(mSpriteBase->mAnim[mFrame].image, NULL, mScreen, &dest);
}

// Mios
void CSprite::drawMoving()
{

    if (mLastupdate+mSpriteBase->mAnim[mFrame].pause*mSpeed<SDL_GetTicks())
    {

        mFrame++;
        if (mFrame>mSpriteBase->mNumframes-1) mFrame=0;
        mLastupdate = SDL_GetTicks();
    }

    if (mDrawn==0) mDrawn=1;

    SDL_Rect dest;
    dest.x = (int)mX;
    dest.y = (int)mY;
    SDL_BlitSurface(mSpriteBase->mAnim[mFrame].image, NULL, mScreen, &dest);
}

/* ************************************************** */
/* Las siguientes funciones toman la pantalla como una
   matriz, en la cual se encuentra situado el sprite
   las variables xoff e yoff indican el desplazamiento
   que debe hacer el sprite para posicionarse
   correctamente en la escena. Por ejemplo el sprite
   de Ralph es mas grande que el resto y su posicion
   relativa tiene que ser ajustada
/* ************************************************** */

// Define la coordenada del objeto en Edificio(VENTANA,PISO)
void CSprite::setCoord(int x, int y, int xoff, int yoff)
{
    int relX= START_POS_X, relY = START_POS_Y;

    if (x < 0) x = 0;

    if (x > VENTANA_PISO -1 ) x =  VENTANA_PISO -1;

    if (y < 0) y = 0;

    relX += x * MOVE_X;
    relY -= y * MOVE_Y;

    this->set(relX + xoff, relY + yoff);
}

// Obtiene la coordenada del objeto en Edificio(VENTANA,PISO)
int CSprite::coordX()
{
    return (mX - START_POS_X) / MOVE_X;
}

// Obtiene la coordenada del objeto en Edificio(VENTANA,PISO)
int CSprite::coordY()
{
    return (START_POS_Y - mY) / MOVE_Y;
}
void CSprite::setFrame(int nr)
{
    mFrame = nr;
}
int  CSprite::getFrame()
{
    return mFrame;
}

void CSprite::setSpeed(float nr)
{
    mSpeed = nr;
}
float CSprite::getSpeed()
{
    return mSpeed;
}

void CSprite::toggleAnim()
{
    mAnimating = !mAnimating;
}
void CSprite::startAnim()
{
    mAnimating = true;
}
void CSprite::stopAnim()
{
    mAnimating = false;
}
void CSprite::rewind()
{
    mFrame = 0;
}
void CSprite::xadd(int nr)
{
    mX+=nr;
}
void CSprite::yadd(int nr)
{
    mY+=nr;
}
void CSprite::xset(int nr)
{
    mX=nr;
}
void CSprite::yset(int nr)
{
    mY=nr;
}
void CSprite::set(int xx, int yy)
{
    mX=xx;
    mY=yy;
}
int CSprite::getX()
{
    return mX;
}
int CSprite::getY()
{
    return mY;
}

int CSprite::getw()
{
    return mSpriteBase->mAnim[0].image->w;
}
int CSprite::geth()
{
    return mSpriteBase->mAnim[0].image->h;
}

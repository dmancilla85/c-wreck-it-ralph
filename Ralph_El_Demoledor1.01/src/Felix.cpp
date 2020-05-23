#include "../include/Felix.h"

Felix::Felix()
{
    //ctor
    power = false;
    dead = false;
    fixing = false;
    jumping = false;
    herido = false;
    sonido = false;
    startPower = 0;
    xOffset = 5;
    yOffset = 0;
    lastMove = 0;
    vidas = VIDAS_DEFAULT;
    puntaje = 0;
    jump = NULL;
    fix = NULL;
    hurt = NULL;
    death = NULL;
    powOn = NULL;
    powOff = NULL;

}
void Felix::setSonido(bool sonidoState)
{
    // Sonidos de accion
    if (sonidoState)
    {
        sonido=true;
        jump = Mix_LoadWAV( "data/sound/jumpsound.wav" );
        fix = Mix_LoadWAV( "data/sound/fixit.wav" );
        hurt = Mix_LoadWAV( "data/sound/bewoop.wav" );
        death = Mix_LoadWAV( "data/sound/loselifesound.wav" );
        powOn = Mix_LoadWAV( "data/sound/piesound.wav" );
        powOff = Mix_LoadWAV( "data/sound/powerdown.wav" );
    }

}
int Felix::init(SDL_Surface *screen, int mode)
{

    switch(mode)
    {
    case 2:
        sprite.init("data/felix2", screen);
        break;
    case 1:
        sprite.init(SPRITE_FELIX, screen);
        break;
    }

    sprite.stopAnim(); // Yo controlo la animacion
    return 1;
}

void Felix::draw()
{
    int newMove = SDL_GetTicks();

    if (power)
        sprite.setFrame( FLX_POWER );
    else
        sprite.setFrame( FLX_QUIETO);

    if(jumping)
        sprite.setFrame( FLX_SALTANDO );

    if(fixing)
       sprite.setFrame( FLX_REPARANDO );


    if(herido)
        sprite.setFrame( FLX_HERIDO );

    if(dead)
    {
        sprite.setFrame( FLX_MUERTO );
        if( sprite.getY() < START_POS_Y +  WINDOW_LY )
            sprite.yadd(6);
    }

    // Controla la duracion del frame "saltando"
    if(jumping && (newMove - lastMove) > 25 )
    {
        jumping = false;
        if(sonido) Mix_PlayChannel( -1, jump, 0 );
        lastMove = SDL_GetTicks();
    }

    // Controla la duracion del frame "herido"
    if(herido && newMove - lastMove > 25  )
    {
        herido = false;
        lastMove = SDL_GetTicks();
    }

    // Controla la duracion del frame "reparando"
    if(fixing && newMove - lastMove > 100 )
    {
        fixing = false;
        if(sonido) Mix_PlayChannel( -1, fix, 0 );
        lastMove = SDL_GetTicks();
    }

    sprite.draw();
}

void Felix::pierdeVida()
{
    vidas--;

    // Sonido de herida
    if(vidas > 0)
    {
        if(sonido) Mix_PlayChannel( -1, hurt, 0 );
        herido = true;
    }

    if( !dead && vidas <= 0 ) muere();
}

void Felix::muere()
{
    dead = true;
    if(sonido)  Mix_PlayChannel( -1, death, 0 );
}
// Definir posicion relativa en matriz
void Felix::set(int x, int y)
{
    sprite.setCoord(x, y, xOffset, yOffset);
}
int  Felix::getX()
{
    return sprite.coordX();
}
int Felix::getY()
{
    return sprite.coordY();
}

// Definir desplazamiento
void Felix::setOffset(int x, int y)
{
    xOffset= x;
    yOffset= y;
}

// Manejo de vidas
void Felix::setVida(int vi)
{
    vidas = vi;
}
void Felix::ganaVida()
{
    vidas < VIDAS_MAXIMO ? vidas++ : vidas = VIDAS_MAXIMO;
}
int Felix::getVida()
{
    return vidas;
}

// Reset vidas (y define en estado QUIETO)
void Felix::resetVida()
{
    vidas = VIDAS_DEFAULT;
    dead = false;
    sprite.setFrame (FLX_QUIETO);
}
// Manejo de puntajes
void Felix::resetPuntaje()
{
    puntaje = 0;
}
void Felix::anotarPuntaje(int puntos)
{
    puntaje += puntos;
}
int Felix::getPuntaje()
{
    return puntaje;
}
// Saber si impacto contra el objetivo
bool Felix::isFixing()
{
    return fixing;
}
void Felix::setFixing(bool sh)
{
    fixing = sh;
}
// Saber si impacto contra el objetivo
bool Felix::isJumping()
{
    return jumping;
}
void Felix::setJumping(bool sh)
{
    jumping = sh;
}

// Saber si impacto contra el objetivo
bool Felix::isDead()
{
    return dead;
}
void Felix::setDead(bool sh)
{
    dead = sh;
}
// Indica que tiene el poder especial
bool Felix::gotPower()
{
    return power;
}
void Felix::setPower(bool pow)
{
    power = pow;

    if(power)
        if(sonido) Mix_PlayChannel( -1, powOn, 0 );
        else if(sonido) Mix_PlayChannel( -1, powOff, 0 );
    startPower = SDL_GetTicks();
}

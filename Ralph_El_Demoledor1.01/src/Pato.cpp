#include "../include/Pato.h"

Pato::Pato()
{
    //ctor
    lastMove = SDL_GetTicks();
    sprite.xset(0);
    xOffset = 0;
    yOffset = -15;
    shock = false;
    coefDif = COEF_NORMAL;
    start = false;
}

int Pato::init(SDL_Surface *screen)
{
    sprite.init(SPRITE_PATO, screen);
    sprite.startAnim();
    sprite.setCoord(0, PISOS-1, xOffset, yOffset);
    return 1;
}

void Pato::draw()
{
    int newMove = SDL_GetTicks();
    int alea, alea2; // variables aleatorias

    if (newMove - lastMove > 5 * coefDif)
        lastMove = SDL_GetTicks();

    sprite.draw();
}

// Definir posicion relativa en matriz
void Pato::set(int x, int y)
{
    sprite.setCoord(x, y, xOffset, yOffset);
}

void Pato::setPos(int x, int y)
{
    /*    setAltura(y);
        sprite.xset(x);*/
    setAltura(x);
    sprite.xset(y);
}

// Definir desplazamiento
void Pato::setOffset(int x, int y)
{
    xOffset= x;
    yOffset= y;
}

// Definir altura (piso del edificio)
void Pato::setAltura(int y)
{
    sprite.yset(START_POS_Y - y * MOVE_Y);
}

// Definir dificultad de juego
void Pato::setDificultad(dificultad dif)
{
    if (dif == FACIL) coefDif = COEF_FACIL;
    if (dif == MEDIO) coefDif = COEF_NORMAL;
    if (dif == DIFICIL) coefDif = COEF_DIFICIL;
}

// Definir coeficiente de dificultad
void Pato::setCoefDif(float coef)
{
    coefDif = coef;
}

void Pato::addCoefDif(float coef)
{
    coefDif += coef;
}

float Pato::getCoefDif()
{
    return coefDif;
}

int Pato::getX()
{
    return sprite.coordX();
}

int Pato::getY()
{
    return sprite.coordY();
}

// Saber si impacto contra el objetivo
bool Pato::isView()
{
    return shock;
}

// Indicar que impacto contra el objetivo
void Pato::setView(bool sh)
{
    shock = sh;
}

// Saber si comenzo a moverse
bool Pato::isStarted()
{
    return start;
}

// Indicar que se esta moviendo
void Pato::setStart(bool st)
{
    start = st;
}


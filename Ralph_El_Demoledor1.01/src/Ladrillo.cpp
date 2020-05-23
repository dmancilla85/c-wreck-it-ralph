#include "../include/Ladrillo.h"

Ladrillo::Ladrillo() {
    //ctor
    lastMove = SDL_GetTicks();
    shock = false;
    coefDif = COEF_NORMAL;
    xOffset = 0;
    coefDif = COEF_NORMAL;
    yOffset = -10;
}

int Ladrillo::init(SDL_Surface *screen, int x, int y) {
    sprite.init(SPRITE_LADRILLO, screen);
    sprite.startAnim();
    sprite.setCoord(x,y, 10, 20);
    return 1;
}

void Ladrillo::draw() {
    int newMove = SDL_GetTicks();

    // El tiempo, coefdif y el yadd regulan la velocidad de caìda
    if(newMove - lastMove > 10 * coefDif ) {
        //sprite.yadd(3);
        lastMove = SDL_GetTicks();
    }
    sprite.draw();
}

// Definir posicion relativa en matriz
void Ladrillo::set(int x, int y) {
    sprite.setCoord(x, y, 10, yOffset);
}
// Definir posicion relativa en matriz
void Ladrillo::setPos(int x, int y) {
    sprite.setCoord(x, PISOS, 10, yOffset);
    sprite.yset(y);
}

// Definir desplazamiento
void Ladrillo::setOffset(int x, int y) {
    xOffset= x;
    yOffset= y;
}

// Definir dificultad de juego
void Ladrillo::setDificultad(dificultad dif) {
    if(dif == FACIL) coefDif = COEF_FACIL;
    if(dif == MEDIO) coefDif = COEF_NORMAL;
    if(dif == DIFICIL) coefDif = COEF_DIFICIL;
}
// Definir coeficiente de dificultad
void Ladrillo::setCoefDif(float coef) {
    coefDif = coef;
}
void Ladrillo::addCoefDif(float coef) {
    coefDif += coef;
}
float Ladrillo::getCoefDif() {
    return coefDif;
}

// Obtener coordenadas relativas
int  Ladrillo::getX() {
    return sprite.coordX();
}
int  Ladrillo::getY() {
    return sprite.coordY();
}

// Saber si impacto contra el objetivo
bool Ladrillo::isShocked() {
    return shock;
}

// Indicar que impacto contra el objetivo
void Ladrillo::setShock(bool sh) {
    shock = sh;
}

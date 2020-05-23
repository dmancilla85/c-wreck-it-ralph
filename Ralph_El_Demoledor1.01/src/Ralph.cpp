#include "../include/Ralph.h"

Ralph::Ralph() {
    //ctor
    lastMove = 0;
    xOffset = -20;
    yOffset = -54;
    attacking = false;
  //  coefDif = COEF_NORMAL;
    set(1, PISOS );
}

int Ralph::init(SDL_Surface *screen) {
    sprite.init(SPRITE_RALPH,screen);
    sprite.stopAnim(); // Deshab. animacion automatica
    // Cargar sonido
    attack = Mix_LoadWAV( "data/sound/smashsound.wav" );

    return  1;
}

// Dibujar a Ralph en pantalla
void Ralph::draw(Edificio &edificio) {

    long newMove = SDL_GetTicks();

	 if(attacking) {
        sprite.setFrame(1);
    } else {
        sprite.setFrame(0);
    }

    // actua una vez transcurrido el t. aleatorio
    if(attacking && newMove - lastMove > 600 ) {
        lastMove = newMove;

        // Activa/desactiva modo ataque
        attacking = !attacking;
		    Mix_PlayChannel( -1, attack, 0 ); // Sonido de ataque
    }
    sprite.draw();
}

// Definir posicion relativa en matriz
void Ralph::set(int x, int y) {
    sprite.setCoord(x, y, xOffset, yOffset);
}

// Definir desplazamiento
void Ralph::setOffset(int x, int y) {
    xOffset= x;
    yOffset= y;
}

// Definir altura (piso del edificio)
void Ralph::setAltura(int y) {
    sprite.yset(START_POS_Y - y * MOVE_Y);
}

// Definir dificultad de juego
/*void Ralph::setDificultad(dificultad dif) {
    if(dif == FACIL) coefDif = COEF_FACIL;
    else if(dif == MEDIO) coefDif = COEF_NORMAL;
    else if(dif == DIFICIL) coefDif = COEF_DIFICIL;
}*/
// Definir coeficiente de dificultad
/*void Ralph::setCoefDif(float coef) {
    coefDif = coef;
}*/
/*void Ralph::addCoefDif(float coef) {
    coefDif += coef;
}*/
/*float Ralph::getCoefDif() {
    return coefDif;
}*/
/*int  Ralph::getX() {
    return sprite.coordX();
}
int  Ralph::getY() {
    return sprite.coordY();
}
*/
// Esta atacando (si/no)
void Ralph::setAttacking(bool at) {
    attacking = at;
}
bool Ralph::isAttacking() {
    return attacking;
}

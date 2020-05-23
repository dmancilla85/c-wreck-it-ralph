#ifndef EDIFICIO_H
#define EDIFICIO_H

#include "CSprite.h"

class Ventana
{
public:

    Ventana()
    {
        tipoVentana = 0;
        estaOcupado = false;
        esEspecial = false;
    };
    virtual ~Ventana() {}
    bool 	estaOcupado;
    bool 	esEspecial;
    CSprite 	imagen;
    short  tipoVentana; // Normal, rota, muy rota, especial
};

class Edificio
{
public:
    Edificio();
    virtual ~Edificio() {}
    int  inicializar(SDL_Surface *screen, SDL_Surface **bmp,int mapaDeJuego[PISOS][VENTANA_PISO]);
    void update();
    void init(SDL_Surface *screen);
    int  getNivel();
    void aumentaNivel();
    void setNivel(int niv);
    void reduceNivel();
    // Define que tipo de ventana debe ser
    void setTipoVentana(int x, int y, short tipo);
    // OBtiene  tipo de ventana
    short  getTipoVentana(int x, int y);
    // Saber si una posicion puede ser ocupada
   // void setOcupado(int x, int y);
    //bool getOcupado(int x, int y);
    // Indicar que la posicion esta vacia
    //void setLibre(int x, int y);
    // Contar ventanas no reparadas
   // int	 contarVentanasRotas();

protected:
    std::array <Ventana, VENTANA_PISO*PISOS > ventanas;
    unsigned short ventana_nivel[4][VENTANA_PISO * PISOS];
    int  nivel;
    float win_h, win_w;
    SDL_Surface *bmp1, *bmp2, *bmp3;
};

#endif

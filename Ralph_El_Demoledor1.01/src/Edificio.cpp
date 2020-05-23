#include "../include/Edificio.h"

Edificio::Edificio()
{
    nivel = 0;

    // Cargo fondo de pantalla
    bmp1=IMG_Load("data/background.png");
    bmp2=IMG_Load("data/background2.png");
    bmp3=IMG_Load("data/background3.png");
}

void Edificio::init(SDL_Surface *screen)
{
    // Recorro el array e inicializo las ventanas
    for(int y = 0; y < PISOS; y++)
    {
        for(int x = 0; x < VENTANA_PISO; x++)
        {
            ventanas[y * VENTANA_PISO + x].imagen.init(SPRITE_VENTANA, screen);
            ventanas[y * VENTANA_PISO + x].imagen.stopAnim();
            win_w = ventanas[y * VENTANA_PISO + x].imagen.getw();
            win_h = ventanas[y * VENTANA_PISO + x].imagen.geth();
            // Seteo la posición del ladrilo
            ventanas[y*VENTANA_PISO+x].imagen.set(START_POS_X + x * (win_w + SPACE_BETWEEN_X),
                                                  START_POS_Y - y *(win_h + SPACE_BETWEEN_Y));
        }
    }

}

int Edificio::inicializar(SDL_Surface *screen,SDL_Surface **bmp,
                          int mapaDeJuego[PISOS][VENTANA_PISO])
{
    // Cargo fondo de pantalla
    if(nivel == 0)
        *bmp=bmp1;
    else if(nivel == 1)
        *bmp=bmp2;
    else if(nivel > 2)
        *bmp=bmp3;

    /*   if(nivel >= 4)
           nivel = 3;

       if(nivel <= 0)
           nivel = 0;
    */
    // Recorro el array e inicializo las ventanas
    for(int y = 0; y < PISOS; y++)
    {
        for(int x = 0; x < VENTANA_PISO; x++)
        {
            // Defino tipoVentana, esta variable es la que indica si una ventana fue reparada
            // cuando se carga se le asigna el valor calculado aleatoriamente
            ventanas[y * VENTANA_PISO + x].tipoVentana = mapaDeJuego[y][x]; //ventana_nivel[nivel][y * VENTANA_PISO + x];

            // Por defecto no hay ninguna ocupada
            //ventanas[y * VENTANA_PISO + x].estaOcupado = false;


        }
    }

    return true;
}

void Edificio::update()
{
    // Recorro el array y actualizo las ventanas
    for(int i = 0; i < VENTANA_PISO*PISOS; i++ )
    {
        ventanas[i].imagen.setFrame(ventanas[i].tipoVentana);
        ventanas[i].imagen.draw();
    }
}

// Verifico si se cumple condicion de victoria
/*int	 Edificio::contarVentanasRotas()
{
    int cuenta = 0;

    for(int i = 0; i < PISOS*VENTANA_PISO; i++)
        if( ventanas[i].tipoVentana == WIN_BROKEN || ventanas[i].tipoVentana == WIN_BROKEN2)
            cuenta++;

    return cuenta;
}
 */
int  Edificio::getNivel()
{
    return nivel;
}
void Edificio::aumentaNivel()
{
    nivel++;
}
void Edificio::setNivel(int niv)
{
    nivel = niv;
}
void Edificio::reduceNivel()
{
    nivel--;
}
// Define que tipo de ventana debe ser
void Edificio::setTipoVentana(int x, int y, short tipo)
{
    ventanas[y * VENTANA_PISO + x].tipoVentana = tipo;
}
// OBtiene  tipo de ventana
short  Edificio::getTipoVentana(int x, int y)
{
    return ventanas[y * VENTANA_PISO + x].tipoVentana;
}

// Saber si una posicion puede ser ocupada
/*void Edificio::setOcupado(int x, int y)
{
    ventanas[y * VENTANA_PISO + x].estaOcupado = true;
}*/
/*bool Edificio::getOcupado(int x, int y)
{
    return ventanas[y * VENTANA_PISO + x].estaOcupado;
}*/

// Indicar que la posicion esta vacia
/*void Edificio::setLibre(int x, int y)
{
    ventanas[y * VENTANA_PISO + x].estaOcupado = false;
}*/


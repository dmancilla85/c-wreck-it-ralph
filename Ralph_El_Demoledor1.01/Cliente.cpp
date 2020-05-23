#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <pthread.h>
#include <signal.h>
#include <X11/Xlib.h>

#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL.h>
#include "include/variables.h"
#include "include/Felix.h"
#include "include/Ralph.h"
#include "include/Ladrillo.h"
#include "include/Pato.h"
#include "include/Edificio.h"

#define MAXDATASIZE 255
#define ARCHIVODECOFIGURACION "ParametrosCliente.conf"

using namespace std;

struct posicion
{
    int x,y,NumJugador;
    char nombreJugador[20];
};

/** Estructura Para setear los datos de comunicacion**/
struct parametrosConfiguracionCliente
{
    int port;
    char ip[20];
    char left[10];
    char right[10];
    char up[10];
    char down[10];
    char fix[10];
};

/************** Variables globales *******************/
int         socketAddress, finDelTorneo = 1, finalizarComunicacion = 1, finalizarGraficos = 1, finalizarTeclado = 1;
posicion    miFelix;
posicion    otroFelix;
int         gameState = INTRO;
int         cantVidas, nroJugador, nivel=0;
char        nombreOponente[20], nombreGanador[20];
bool 		sonidoOn = true;
bool	 	fullScreen = false;
Felix 		felix, felix2;
// Hilos
pthread_t       hiloComunicacion,
hiloTeclado,
hiloGraficar;

// Semaforos
pthread_mutex_t mutexTeclado,
mutexPaquete,
mutexGameState,
mutexGraficar;

/*0 se puede pasar; M=hay marquesina en esa posicion*/
int         mapaDeObstaculos[PISOS][VENTANA_PISO]= {{0,0,0,0,0},{0,0,0,0,0},{0,0,1,0,0}};
int         mapaDeJuego[PISOS][VENTANA_PISO]= {{1,2,4,2,0},{1,3,1,2,0},{1,0,3,0,1}};
int         mapaDeJuegoOriginal[PISOS][VENTANA_PISO]= {{1,2,4,2,0},{1,3,1,2,0},{1,0,3,0,1}};

// Declaramos las variables que vamos a usar para el buffer de entrada y salida
char            buffSalida[MAXDATASIZE],
buffEntrada[MAXDATASIZE]; //buffer para mandar msj al server
// Parametros de configuracion
parametrosConfiguracionCliente parmCliente;


/************** Declaracion de funciones *******************/
int     validarMovimiento(const char);
int     cargarConfiguracion(void); //Funcion para cargar configuracion
void    *rutinaComunicacion(void *);
void    *rutinaTeclado(void *);
void    *rutinaGraficar(void *);
void    finalizarCliente(int);
void    servidorFinalizado(int socketAddress);
int     validarPuerto(char *parametro);
int     verificarIP(char *ip);
void    setearPosiciones();

vector<string> split(char *str, char *sep);


/****************** Main **********************************/
int main(int argc, char *argv[])
{

    struct hostent *he;
    struct sockaddr_in server;

    signal(SIGINT, finalizarCliente);

    if (argc > 1)
    {
        // Este parámetro se va a usar para activar el sonido
        if (strcmp(argv[1], "-nosound") == 0)
        {
            sonidoOn = false;
            puts("Sonido desactivado.");
        }
		  else 
		  // Este parámetro se va a usar para activar pantalla completa
        if (strcmp(argv[1], "-fullscreen") == 0)
        {
            fullScreen = true;
            puts("Modo pantalla completa.");
        }
        else
            puts("Error. Paramétro incorrecto. Ingrese -nosound para desactivar el sonido. ");
    }

    cout<<"Ingrese su nombre: ";
    fgets(miFelix.nombreJugador, 20, stdin);
    cout<<"Su nombre durante el juego sera: "<<miFelix.nombreJugador<<endl;

    if ( cargarConfiguracion() == 0)
    {
        cerr << "Ocurrio un error al leer el archivo de configuracion."<<endl;
        exit(EXIT_FAILURE);
    }

    if ((he=gethostbyname(parmCliente.ip))==NULL)
    {
        cerr<<"gethostbyname() error."<<endl;
        exit(EXIT_FAILURE);
    }

    if ((socketAddress=socket(AF_INET, SOCK_STREAM, 0))==-1)
    {
        cerr<<"socket() error."<<endl;
        exit(EXIT_FAILURE);
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(parmCliente.port);
    server.sin_addr = *((struct in_addr *)he->h_addr);

    bzero(&(server.sin_zero),8);

    if (connect(socketAddress, (struct sockaddr *)&server, sizeof(struct sockaddr))==-1)
    {
        cerr<<"connect() error."<<endl;
        exit(EXIT_FAILURE);
    }

    /** Envio un mensaje al servidor con mi nombre **/
    strcpy(buffSalida, miFelix.nombreJugador);
    send(socketAddress, buffSalida , MAXDATASIZE, 0);
    printf("Enviando el nombre: %s", buffSalida);

    /* CREAMOS EL HILO PARA LA COMUNICACION */
    if (( pthread_create(&hiloComunicacion, NULL, rutinaComunicacion, (void *) &socketAddress))<0)
    {
        cerr <<"Error al crear el thread de comunicacion."<<endl;
        exit(EXIT_FAILURE);
    }

    /* CREAMOS EL HILO PARA LA CAPTURA DEL TECLADO */
    if ((pthread_create(&hiloTeclado, NULL, rutinaTeclado,(void *) &socketAddress))<0)
    {
        cout <<"error al crear el thread de captura de teclado ."<<endl;
        exit(EXIT_FAILURE);
    }

    /* CREAMOS EL HILO PARA GRAFICAR */
    if ((pthread_create(&hiloGraficar, NULL, rutinaGraficar,(void *) &socketAddress))<0)
    {
        cout <<"error al crear el thread para graficar."<<endl;
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init (&mutexTeclado, NULL);
    pthread_mutex_init (&mutexGraficar, NULL);
    pthread_mutex_lock (&mutexTeclado);
    pthread_mutex_lock (&mutexGraficar);


    while (finDelTorneo)
        sleep(3);

    if (pthread_kill(hiloComunicacion,0)==0)
    {
        if (pthread_cancel(hiloComunicacion)!=0)
            cout<<"Error al cancelar el hiloComunicacion."<<endl;
        pthread_join(hiloComunicacion, NULL);
    }

    if (pthread_kill(hiloGraficar,0)==0)
    {
        if (pthread_cancel(hiloGraficar)!=0)
            cout<<"Error al cancelar el hiloGraficar."<<endl;
        pthread_join(hiloGraficar, NULL);
    }

    if (pthread_kill(hiloTeclado,0)==0)
    {
        if (pthread_cancel(hiloTeclado)!=0)
            cout<<"Error al cancelar el hiloGraficar."<<endl;
        pthread_join(hiloTeclado, NULL);
    }

    if (pthread_mutex_destroy(&mutexTeclado)!=0)
        cerr<<"Error al destruir el mutexTeclado."<<endl;

    if (pthread_mutex_destroy(&mutexGraficar)!=0)
        cerr<<"Error al destruir el mutexGraficar."<<endl;

    if (pthread_mutex_destroy(&mutexPaquete)!=0)
        cerr<<"Error al destruir el mutexPaquete."<<endl;

    close(socketAddress);
    exit(EXIT_SUCCESS);
}
/** Fin del main **/

/** HILO QUE SE USA PARA RECIBIR LOS MENSAJES QUE MANDA EL SERVIDOR **/
void *rutinaComunicacion(void * params)
{

    int numBytes;
    while (finalizarComunicacion)
        if ((numBytes=recv(socketAddress,buffEntrada,MAXDATASIZE,0)) > 0)
        {
            buffEntrada[numBytes]='\0';
            //de aca salta al hilo graficar
            // Ganador
            int x;
            if ( strstr(buffEntrada, "ep") != 0)
            {
                sscanf(buffEntrada, "ep %d", &x);
                if (x == miFelix.NumJugador) gameState = VICTORIA ;
                if (x == otroFelix.NumJugador) gameState = GAME_OVER;

                switch (x)
                {
                case 0:
                    gameState = EMPATE;
                    break;
                case -1:
                    gameState = CRASH_GAME;
                    break;
                case -9:
                    gameState = CRASH;
                    break;
                }

            }
            else if (strstr(buffEntrada,"ft") != 0 )   // GANADOR - FIN DEL TORNEO
            {
                sscanf(buffEntrada, "ft %s", nombreGanador);
                gameState = FINAL;
            }
            else if (!strcmp(buffEntrada, "END"))
                servidorFinalizado(socketAddress);
            else if(!strcmp(buffEntrada, "rd"))
            {
            //Si el rival se desconecta lo graficamos muerto
            strcat(otroFelix.nombreJugador,"(DESC)");
            otroFelix.x = otroFelix.y = -1;
            felix2.setVida(0);
            felix2.muere();
            /*AGREGADO LUIS*/
            }
            pthread_mutex_unlock (&mutexGraficar);
        }
    return NULL;
}

/************ HILO QUE SE USA PARA CAPTURAR EL TECLADO Y ENVIARSELO AL SERVIDOR **********************/
void *rutinaTeclado(void * params)
{

    sleep(1);
    SDL_Event evento;
    char *key;

    while (finalizarTeclado )
    {
        if (SDL_WaitEvent( &evento ) > 0 )   //¿Se ha producido un evento?
        {
            if (evento.type == SDL_KEYDOWN)   // ¿Ha pulsado una tecla?
            {
                if (evento.key.keysym.sym==SDLK_ESCAPE)   //presiono la tecla ESC
                {
                    strcpy(buffSalida,"END");
                    finalizarCliente(0); //finaliza el cliente
                }

                key = SDL_GetKeyName(evento.key.keysym.sym);

                if (gameState == INTRO || gameState == VICTORIA || gameState == GAME_OVER
                        || gameState == EMPATE)
                {
                    if (!strcmp("space", key) /*&& !isReady*/)
                    {
                        send(socketAddress, "ready", MAXDATASIZE, 0);
                        puts("En espera..");
                    }

                    continue;
                }

                // CAPTURO LA TECLA PARA MOVERSE PARA ARRIBA
                if (strcmp(parmCliente.up, key)==0)
                {
                    if ( validarMovimiento('U'))
                        sprintf(buffSalida,"mf %c",'U'); //seteo el valor de la tecla que vamos a enviar
                }

                // CAPTURO LA TECLA PARA MOVERSE PARA ABAJO
                if (strcmp(parmCliente.down, key)==0)
                {
                    if ( validarMovimiento('D'))
                        sprintf(buffSalida,"mf %c",'D'); //seteo el valor de la tecla que vamos a enviar
                }

                // CAPTURO LA TECLA PARA MOVERSE PARA LA IZQUIERDA
                if (strcmp(parmCliente.left, key)==0)
                {
                    if ( validarMovimiento('L'))
                        sprintf(buffSalida,"mf %c",'L'); //seteo el valor de la tecla que vamos a enviar
                }

                // CAPTURO LA TECLA PARA MOVERSE PARA LA DERECHA
                if (strcmp(parmCliente.right, key)==0)
                {
                    if ( validarMovimiento('R'))
                        sprintf(buffSalida,"mf %c",'R'); //seteo el valor de la tecla que vamos a enviar
                }

                if (strcmp(parmCliente.fix, key)==0)
                {
                    if ( validarMovimiento('A'))
                        sprintf(buffSalida,"av %d %d",miFelix.y,miFelix.x);
                }

                usleep(300);

                if (strlen(buffSalida) != 0)
                {
                    send(socketAddress, buffSalida , MAXDATASIZE, 0);
                    strcpy(buffSalida,"");
                }

            }
            else if (evento.type == SDL_QUIT)  //presiono la tecla ESC
            {
                strcpy(buffSalida,"END");
                cout <<"tecla"<<endl;
                finalizarCliente(0); //finaliza el cliente
            }
        }
    }
    return NULL;
}
/************** HILO QUE SE USA PARA GRAFICAR EL JUEGO ****************************/
void *rutinaGraficar(void * params)
{
    SDL_Surface *screen;

    void        *ret = NULL;
    long  		someTicks = SDL_GetTicks();;
    char  		marcadorPuntaje[30];
    bool 		ending = false, hayTorta = false;
    SDL_Rect 	dstrect;
    SDL_Rect 	rectGameState;
    Edificio 	edificio;

    Ralph 		ralph;
    Pato  		pato[PISOS];
    CSprite  	torta;
    Ladrillo	ladrillo[10];
    CSprite 	marcadorVida[VIDAS_MAXIMO];
    CSprite 	marcadorVida2[VIDAS_MAXIMO];
    // Pone el texto en pantalla.
    SDL_Rect    destinoRect;
    SDL_Rect    destinoRect2;
    SDL_Rect    destinoRect3;
    SDL_Rect    destinoRect4;
    SDL_Surface *text,
    *text2,
    *text3,
    *text4;
    char        auxi[10];
    SDL_Color text_color = {255, 95, 0};
    char msj[100];
    int x, y, z;
    //Keep track of the current frame
    int frame = 0;
    //Whether or not to cap the frame rate
    bool cap = true;
    //The frame rate regulator
    long fpsTicks, fpsT;
    bool cr = false;

    printf("Inicializando SDL... ");
    XInitThreads();

    if ( SDL_Init( SDL_INIT_VIDEO ||  SDL_INIT_AUDIO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        exit(EXIT_FAILURE);
    }
    printf(" Listo.\n");

    //Initialize SDL_mixer
    if (sonidoOn)
    {
        printf("Inicializando placa de sonido... ");

        if ( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
        {
            printf( "Imposible iniciar SDL Audio: %s\n", SDL_GetError() );
            sonidoOn=false;
        }
        printf(" Listo.\n");
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    // create a new window
    printf("Definiendo configuración de pantalla... ");

	 if(!fullScreen)
    	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_PALETTE_16,
                              	SDL_HWSURFACE|SDL_DOUBLEBUF);
	 else
		screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_PALETTE_16,
                              	SDL_HWSURFACE|SDL_FULLSCREEN|SDL_DOUBLEBUF);
		
    if ( !screen )
    {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // definir icono y nombre de pantalla
    SDL_WM_SetIcon(SDL_LoadBMP("data/ralph_ico.bmp"), NULL);
    SDL_WM_SetCaption("(TP4 SO) Fix-it Felix Jr.", "Fix-it Felix Jr.");
    printf(" Listo.\n");

    printf("Cargando imagenes de fondo... ");

    SDL_Surface* intro = IMG_Load("data/intro.png");
    if (!intro) printf("Unable to load bitmap: %s\n", SDL_GetError());

    SDL_Surface* start_screen = IMG_Load("data/start_screen.png");
    if (!start_screen) printf("Unable to load bitmap: %s\n", SDL_GetError());

    SDL_Surface* crash_game = IMG_Load("data/finAbruptoPartida.png");
    if (!crash_game) printf("Unable to load bitmap: %s\n", SDL_GetError());

    SDL_Surface* waiting = IMG_Load("data/waiting.png");
    if (!waiting) printf("Unable to load bitmap: %s\n", SDL_GetError());

    SDL_Surface* loading = IMG_Load("data/loading.png");
    if (!waiting) printf("Unable to load bitmap: %s\n", SDL_GetError());

    SDL_Surface* victoria = IMG_Load("data/victoria.png");
    if (!victoria) printf("Unable to load bitmap: %s\n", SDL_GetError());

    SDL_Surface* empate = IMG_Load("data/empate.png");
    if (!empate) printf("Unable to load bitmap: %s\n", SDL_GetError());

    SDL_Surface* gameover = IMG_Load("data/gameover.png");
    if (!gameover) printf("Unable to load bitmap: %s\n", SDL_GetError());

    printf(" Listo.\n");

    // Inicializa SDL_ttf.
    if (TTF_Init() != 0)
    {
        cerr << "TTF_Init() Failed: " << TTF_GetError() << endl;
        SDL_Quit();
        exit(1);
    }
    // Carga una fuente. Tiene que estar en la misma carpeta.
    TTF_Font *font=NULL;
    if (!font) font = TTF_OpenFont("data/fonts/FreeSans.ttf", 16);
    if (font == NULL)
    {
        cerr << "TTF_OpenFont() Failed: " << TTF_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        exit(1);
    }

    //NOmbres de los jugadores.
    destinoRect.x=80;
    destinoRect.y=70;
    destinoRect2.x=550;
    destinoRect2.y=70;
    //Puntajes de los jugadores.
    destinoRect3.x=80;
    destinoRect3.y=90;
    destinoRect4.x=550;
    destinoRect4.y=90;

    /* COMPONENTES DEL JUEGO */
    printf("Cargando componentes del juego... ");
    SDL_Surface *bmp=IMG_Load("data/background.png");

    // Inicio sprites de ladrillos
    for (int i = 0; i < VENTANA_PISO; i++)
        ladrillo[i].init(screen, i, PISOS-1);

    ralph.init(screen);
    felix.setSonido(sonidoOn);
    felix2.setSonido(sonidoOn);
    for (int i = 0; i < PISOS; i++)
        pato[i].init(screen);

    torta.init(SPRITE_TARTA, screen);

    dstrect.x = (screen->w - bmp->w) / 2;
    dstrect.y = (screen->h - bmp-> h) / 2;
    rectGameState.x =0;
    rectGameState.y =0;
    printf(" Listo.\n");

    puts("Comienza el juego...");
    vector<string> comandosSeparados;

    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(start_screen, 0, screen, &rectGameState);
    SDL_Flip(screen);
    sleep(3);

    SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(waiting, 0, screen, &rectGameState);
    SDL_Flip(screen);
    //sleep(2);

    edificio.init(screen);

    while (finalizarGraficos)
    {
        // Recibo mensaje
        pthread_mutex_lock (&mutexGraficar);
        SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, 0, 0, 0));

        if (strstr(buffEntrada,"cv") != 0 )
        {
            printf("Cargando cantidad de vidas\n");
            fflush(stdout);
            miFelix.x = miFelix.y = otroFelix.x = otroFelix.y = 0;
            sscanf(buffEntrada, "cv %d %s %d", &cantVidas, nombreOponente, &nroJugador);
            sscanf(buffEntrada, "nj %d", &nroJugador);
            miFelix.NumJugador = nroJugador;
            otroFelix.NumJugador = (nroJugador == 1) ? 2 : 1;
            strcpy(otroFelix.nombreJugador, nombreOponente);

            text = TTF_RenderText_Solid(font, miFelix.nombreJugador, text_color);
            text2 = TTF_RenderText_Solid(font, otroFelix.nombreJugador, text_color);
            sprintf(auxi,"%d",0);
            text3 = TTF_RenderText_Solid(font,(const char*)(& auxi), text_color);
            sprintf(auxi,"%d",0);
            text4 = TTF_RenderText_Solid(font,(const char*)(& auxi), text_color);


            if (miFelix.NumJugador == 1)
                SDL_WM_SetCaption("(TP4 SO) Fix-it Felix Jr. - Jugador 1", "Fix-it Felix Jr.");
            else
                SDL_WM_SetCaption("(TP4 SO) Fix-it Felix Jr. - Jugador 2", "Fix-it Felix Jr.");

            // Colores de vidas en pantalla
            for (int i=0; i < VIDAS_MAXIMO; i++)
            {
                marcadorVida[i].setFrame(miFelix.NumJugador - 1);
                marcadorVida2[i].setFrame(otroFelix.NumJugador - 1);

                // Inicio marcadores de vidas
                if (miFelix.NumJugador == 1)
                {
                    marcadorVida [i].init ("data/lives", screen);
                    marcadorVida [i].xset (0 + (50*(i+1)));
                    marcadorVida[i].yset(30);

                    marcadorVida2[i].init ("data/lives", screen);
                    marcadorVida2[i].xset (SCREEN_WIDTH -(50*(i+1)));
                    marcadorVida2[i].yset(30);

                    marcadorVida[i].stopAnim();
                    marcadorVida2[i].stopAnim();
                }
                else
                {
                    marcadorVida [i].init ("data/lives", screen);
                    marcadorVida [i].xset (SCREEN_WIDTH -(50*(i+1)));
                    marcadorVida[i].yset(30);

                    marcadorVida2[i].init ("data/lives", screen);
                    marcadorVida2[i].xset (0 + (50*(i+1)));
                    marcadorVida2[i].yset(30);

                    marcadorVida[i].stopAnim();
                    marcadorVida2[i].stopAnim();
                }
            }
            felix.init(screen, miFelix.NumJugador);
            felix2.init(screen, otroFelix.NumJugador);
            felix.resetPuntaje();
            felix.resetVida();
            felix2.resetPuntaje();
            felix2.resetVida();
            edificio.setNivel(0);
            nivel = 0;
            ending = false;
            gameState = INTRO;

            someTicks = SDL_GetTicks();
            /*AGREGADO LUIS*/
        }


        switch (gameState)
        {
            // ********************************************
        case INTRO:

            SDL_BlitSurface(intro, 0, screen, &rectGameState);
            if (SDL_GetTicks() - someTicks > 2000)
            {
                gameState = LOADING_LEVEL;
                pthread_mutex_unlock(&mutexGraficar);
                someTicks = SDL_GetTicks();
            }
            break;

        case WAITING:

            break;
            // ********************************************
        case LOADING_LEVEL:

            SDL_BlitSurface(loading, 0, screen, &rectGameState);
            /*AGREGADO LUIS*/
            setearPosiciones();
	    
            

            if(miFelix.x !=-1 && miFelix.y != -1)felix.set(miFelix.x, miFelix.y);
            if(otroFelix.x !=-1 && otroFelix.y != -1)felix2.set(otroFelix.x, otroFelix.y);
            /*AGREGADO LUIS*/

            if (SDL_GetTicks() - someTicks > 3000)
            {
                gameState = IN_GAME;
                someTicks = SDL_GetTicks();

                for (int i=0; i<PISOS; i++)
                    for (int x=0; x<VENTANA_PISO; x++)
                        mapaDeJuego[i][x] = mapaDeJuegoOriginal[i][x];

                if (nivel > 0)
                {
                    mapaDeJuego[0][2] = 1;
                }
                edificio.inicializar(screen, &bmp, mapaDeJuego);

                if (nivel > 2)
                    mapaDeObstaculos[1][1]=mapaDeObstaculos[1][3]=1;
                else
                    mapaDeObstaculos[1][1]=mapaDeObstaculos[1][3]=0;

            break;
            // ********************************************
        case IN_GAME:

            SDL_BlitSurface(bmp, 0, screen, &dstrect);
            /*AGREGADO LUIS*/
            if (miFelix.NumJugador == 1)
            {
                text2 = TTF_RenderText_Solid(font, otroFelix.nombreJugador, text_color);
                SDL_BlitSurface(text, NULL, screen, &destinoRect);
                SDL_BlitSurface(text2, NULL, screen, &destinoRect2);
                SDL_BlitSurface(text3, NULL, screen, &destinoRect3);
                SDL_BlitSurface(text4, NULL, screen, &destinoRect4);
            }
            else
            {
                text2 = TTF_RenderText_Solid(font, otroFelix.nombreJugador, text_color);
                SDL_BlitSurface(text, NULL, screen, &destinoRect2);
                SDL_BlitSurface(text2, NULL, screen, &destinoRect);
                SDL_BlitSurface(text3, NULL, screen, &destinoRect4);
                SDL_BlitSurface(text4, NULL, screen, &destinoRect3);
                /*AGREGADO LUIS*/
            }
            edificio.update();

            comandosSeparados = split(buffEntrada, "|");
            // **********************************
            // AREA DE DIBUJO
            // **********************************

            for (unsigned int i = 0; i < comandosSeparados.size() && gameState == IN_GAME; i++)
            {
                strcpy(msj, comandosSeparados[i].c_str());	

                //moverFelix %d %d
                if (strstr(msj,"mf") != 0)
                {
                    sscanf(msj, MOVER_FELIX_PMSG, &z, &y, &x);
                    // Jugador 1 o 2?
                    if ( z == miFelix.NumJugador)
                    {
                        if (!felix.isDead())
                        {
                            miFelix.x = x;
                            miFelix.y = y;
                            felix.set(x, y);
                            if (!felix.isJumping())
                                felix.setJumping(true);
                        }
                    }
                    else
                    {
                        if (!felix2.isDead())
                        {
                            otroFelix.x = x;
                            otroFelix.y = y;
                            felix2.set(x, y);
                            if (!felix2.isJumping())
                                felix2.setJumping(true);
                        }
                    }
                }
                else if (strstr(msj,"if") != 0)      //impactoFelix %d
                {
                    sscanf(msj, IMPACTO_FELIX_PMSG, &x);
                    if (x == miFelix.NumJugador)
                    {
                        felix.pierdeVida();
                        if (felix.getVida() <= 0)
                        {
                            miFelix.x=-1;
                            miFelix.y=-1;
                        }
                    }
                    else
                    {
                        felix2.pierdeVida();
                        if (felix2.getVida() <= 0)
                        {
                            otroFelix.x=-1;
                            otroFelix.y=-1;
                        }
                    }
                }
                else if (strstr(msj,"dg") != 0)    //dibujarGaviota %d %d
                {
                    sscanf(msj, DIBUJAR_GAVIOTA_PMSG, &x, &y);
                    pato[x].setPos(x, y);
                    pato[x].setView(true);
                }
                else if (strstr(msj,"cf") != 0)    //cambiarEstadoFelix %d %d
                {
                    sscanf(msj, CAMBIAR_ESTADO_FELIX_PMSG, &x, &y);

                    if (x == miFelix.NumJugador)
                        felix.setPower(y);
                    else
                        felix2.setPower(y);
                }
                else if (strstr(msj,"dt") != 0)     //dibujarTorta %d %d
                {
                    sscanf(msj, DIBUJAR_TORTA_PMSG, &x, &y);
                    if (y > PISOS - 1) y = PISOS - 1;
                    torta.setCoord(y, x, 0, 0);
                    torta.yadd(30); // Ajusto posicion
                    torta.xadd(10); // Ajusto posicion
                    hayTorta = true;
                }
                else if (strstr(msj,"et") != 0)      //eliminarTorta %d %d
                {
                    sscanf(msj, ELIMINAR_TORTA_PMSG, &x, &y);
                    hayTorta = false;
                }
                else if (strstr(msj,"av") != 0)  // Felix está reparando las ventanas arreglarVentana %d, %d, %d
                {
                    sscanf(msj, ARREGLAR_VENTANA_PMSG, &z, &y, &x);

                    short aux = edificio.getTipoVentana(x, y);

                    if ( z == miFelix.NumJugador)
                    {
                        felix.setFixing(true);

                        edificio.setTipoVentana(x, y, aux-1);
                        felix.anotarPuntaje( PUNTO_X_VENTANA );
                        sprintf(auxi,"%d",felix.getPuntaje());
                        /*AGREGADO LUIS*/
                        text3 = TTF_RenderText_Solid(font,(const char*)(& auxi), text_color);
                    }
                    else
                    {
                        mapaDeJuego[otroFelix.y][otroFelix.x]--;
                        felix2.setFixing(true);
                        edificio.setTipoVentana(x, y, aux-1);
                        felix2.anotarPuntaje( PUNTO_X_VENTANA );
                        sprintf(auxi,"%d",felix2.getPuntaje() );
                        /*AGREGADO LUIS*/
                        text4 = TTF_RenderText_Solid(font,(const char*)(& auxi), text_color);
                    }
                    edificio.update();
                }
                else if (strstr(msj,"mr") != 0)  // Ataque aleatorio de Ralph hacia Felix moverRalph %d
                {
                    sscanf(msj, MOVER_RALPH_PMSG, &x);
                    ralph.set( x, PISOS);
                }
                else if (strstr(msj,"ra") != 0)
                {
                    ralph.setAttacking(true);
                }
                else if (strstr(msj,"dp") != 0)
                {
                    sscanf(msj, DIBUJAR_PROYECTIL_PMSG, &x, &y);
                    // Cada x va a ser para un ladrillo distinto
                    ladrillo[x].setPos(x, y);
                    ladrillo[x].draw();
                }
                else if ( strstr(msj, "sn") != 0)  // Checkear fin de nivel
                {
                    nivel++;
                    gameState = LOADING_LEVEL;
                    edificio.aumentaNivel() ;
                    /*AGREGADO LUIS*/
                    felix.setPower(0);
                    felix2.setPower(0);
                    /*AGREGADO LUIS*/
                    if (felix.getVida() > 0 && edificio.getNivel() % 3 == 0)
                        felix.anotarPuntaje(PUNTO_X_NIVEL);

                    if (felix2.getVida() > 0 && edificio.getNivel() % 3 == 0)
                        felix2.anotarPuntaje(PUNTO_X_NIVEL);
                    someTicks = SDL_GetTicks();
                    pthread_mutex_unlock(&mutexGraficar);
                }
            }

            if (hayTorta)
                torta.draw();

            // Marcador de vidas
            for (int i=0; i < felix.getVida (); i++)
                marcadorVida[i].draw();

            // Marcador de vidas del otro jugador
            for (int i=0; i < felix2.getVida (); i++)
                marcadorVida2[i].draw();

            // Felix
            felix.draw();
            felix2.draw();
            // Ralph
            ralph.draw(edificio);

            // Patos!
            for (int i = 0; i < PISOS; i++)
            {
                if (pato[i].isView()) pato[i].draw();
                pato[i].setView(false);
            }

			 }

            break;

        case GAME_OVER:
				SDL_Delay(1000);
            SDL_BlitSurface(gameover, 0, screen, &rectGameState);
            break;

        case VICTORIA:
   			SDL_Delay(1000);
				SDL_BlitSurface(victoria, 0, screen, &rectGameState);
            break;

        case EMPATE:
				SDL_Delay(1000);
           	SDL_BlitSurface(empate, 0, screen, &rectGameState);
            break;

        case CRASH:
        {
            SDL_Surface* crash = IMG_Load("data/finAbruptoServer.png");
            if (!crash) printf("Unable to load bitmap: %s\n", SDL_GetError());
            SDL_BlitSurface(crash, 0, screen, &rectGameState);

            if ( !cr )
                cr = true;

            if ( cr)
            {
                sleep(3);
                finalizarCliente(1);
            }

            break;
        }


        case CRASH_GAME:
            SDL_BlitSurface(crash_game, 0, screen, &rectGameState);
            break;

        case FINAL:
        {
            // Aca se dibuja el nombre del ganador, hay que cambiar el BMP
            SDL_Surface* finale = IMG_Load("data/FinTorneo.png");
            if (!finale)
                printf("Unable to load bitmap: %s\n", SDL_GetError());
            destinoRect.x=320;
            destinoRect.y=240;
            text = TTF_RenderText_Solid(font, nombreGanador, text_color);
            sleep(3);
            SDL_BlitSurface(finale, 0, screen, &rectGameState);
            SDL_BlitSurface(text, NULL, screen, &destinoRect);
            break;
        }

        }

        fpsT = SDL_GetTicks() - fpsTicks;

        //If we want to cap the frame rate
        if ( ( cap ) && ( fpsT < 1000 / FRAMES_PER_SECOND ) )
        {
            //Sleep the remaining frame time
            SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - fpsT );
        }

        // finally, update the screen :)
        SDL_Flip(screen);

         if (gameState==GAME_OVER||gameState==VICTORIA||gameState==EMPATE||gameState==CRASH||gameState==CRASH_GAME)
        {
            sleep(3);
            SDL_BlitSurface(waiting, 0, screen, &rectGameState);
            SDL_Flip(screen);
        }

    }

    // Limpieza de memoria
    puts("Liberando memoria...");
    puts("Finalizado.");

    // free loaded bitmap
    SDL_FreeSurface(intro);
    SDL_FreeSurface(crash_game);
    SDL_FreeSurface(waiting);
    SDL_FreeSurface(loading);
    SDL_FreeSurface(victoria);
    SDL_FreeSurface(empate);
    SDL_FreeSurface(gameover);
    SDL_FreeSurface(bmp);
    SDL_FreeSurface(screen);
    // all is well ;)
    printf("Exited clean");

    return ret;
}



/************************** Funcion cargarConfiguracion ***********************/
/* funcion que lee el archivo de configuracion y modifica la estructura administrativa */
int cargarConfiguracion(void)
{

    ifstream archivo( ARCHIVODECOFIGURACION, ifstream::in);
    if ( ! archivo )
    {
        cerr << "Erro al intentar abrir el archivo de configuracion."<<endl;
        archivo.close();
        return 0;
    }

    char descripcion[20],dato[20];
    archivo >> descripcion >> dato;
    while ( ! archivo.eof() )
    {
        if (strcmp(descripcion,"PORT") == 0)
            if (validarPuerto(dato) > 0)
                parmCliente.port = atoi(dato);
            else
            {
                cout <<"El puerto ingresado no es valido."<<endl;
                archivo.close();
                return 0;
            }
        else if (strcmp(descripcion,"IP") == 0)
        {
            strcpy(parmCliente.ip, dato);

        }
        else if ( strcmp(descripcion,"FIX") == 0)
            if ( strcmp(dato,"SDLK_QUIT") != 0 )
                strcpy(parmCliente.fix,dato);
            else
            {
                cout <<"La tecla FIX ingresada no es valida."<<endl;
                archivo.close();
                return 0;
            }
        else if (strcmp(descripcion,"LEFT") == 0)
            if ( strcmp(dato,"SDLK_QUIT") != 0 )
                strcpy(parmCliente.left, dato);
            else
            {
                cout <<"La tecla LEFT ingresada no es valida."<<endl;
                archivo.close();
                return 0;
            }
        else if (strcmp(descripcion,"RIGHT") == 0)
            if ( strcmp(dato,"SDLK_QUIT") != 0 )
                strcpy(parmCliente.right, dato);
            else
            {
                cout <<"La tecla RIGHT ingresada no es valida."<<endl;
                archivo.close();
                return 0;
            }
        else if (strcmp(descripcion,"UP") == 0)
            if ( strcmp(dato,"SDLK_QUIT") != 0 )
                strcpy(parmCliente.up, dato);
            else
            {
                cout <<"La tecla UP ingresada no es valida."<<endl;
                archivo.close();
                return 0;
            }
        else if (strcmp(descripcion,"DOWN") == 0)
            if ( strcmp(dato,"SDLK_QUIT") != 0 )
                strcpy(parmCliente.down, dato);
            else
            {
                cout <<"La tecla DOWN ingresada no es valida."<<endl;
                archivo.close();
                return 0;
            }
        else
        {
            cerr<<"La opcion leida es icorrecta." << descripcion <<endl;
            return 0;
        }

        archivo >> descripcion >> dato;
    }
    archivo.close();
    return 1;
}

/******************* Funcion validarMovimientos ****************************/
int validarMovimiento(const char mov)
{

    if ( mov == 'L' && miFelix.x > 0 && !(miFelix.x-1 == otroFelix.x && miFelix.y == otroFelix.y))
    {
        miFelix.x--;
        return 1; //se puede mover a la izquierda
    }
    else if ( mov == 'R' && miFelix.x < VENTANA_PISO-1 && !(miFelix.x+1 == otroFelix.x && miFelix.y == otroFelix.y))
    {
        miFelix.x++;
        return 1; //se puede mover a la izquierda
    }
    else if ( mov == 'A' && (mapaDeJuego[miFelix.y][miFelix.x] == 2 || mapaDeJuego[miFelix.y][miFelix.x] == 3))
    {
        mapaDeJuego[miFelix.y][miFelix.x]--;
        return 1; //Puede arreglar.
    }
    else if ( mov == 'U' && miFelix.y < PISOS-1 && !(miFelix.x == otroFelix.x && miFelix.y+1 == otroFelix.y))
    {
        if (nivel)
        {
            if ( mapaDeObstaculos[miFelix.y+1][miFelix.x] == 0)
            {
                miFelix.y++;
                return 1; //se puede mover a la izquierda
            }
        }
        else
        {
            miFelix.y++;
            return 1; //se puede mover a la izquierda
        }
    }
    else if ( mov == 'D' && miFelix.y > 0 && !(miFelix.x == otroFelix.x && miFelix.y-1 == otroFelix.y))
    {
        if (nivel)
        {
            if ( mapaDeObstaculos[miFelix.y][miFelix.x] == 0)
            {
                miFelix.y--;
                return 1; //se puede mover a la izquierda
            }
        }
        else
        {
            miFelix.y--;
            return 1; //se puede mover a la izquierda
        }
    }
    return 0;

}

/************************** Funcion servidorFinalizado ***********************/
void servidorFinalizado(int socketAddress)
{
    cerr<<"El servidor ha finalizado."<<endl;
    sleep(1);
    close(socketAddress);
    exit(EXIT_SUCCESS);
}
/************************** Funcion validarPuerto ***********************/
int validarPuerto(char *parametro)
{
    int i, limite = strlen(parametro) ;

    for (i=0; i < limite; i++  )
        if ( !isdigit(parametro[i]))
            return 0;

    if (strchr(parametro, '.') || strchr(parametro, ',') )  //Valido que no sea un numero decimal.
        return 0;

    if (atoi (parametro) <= 0)        //Valido que no sea un numero negativo.
        return 0;

    return 1;
}


/************************** Funcion compruebaIP ***********************/
//Función para comprobar la validez de la dir IP
int verificarIP(char *ip)
{
    unsigned int num1, num2, num3, num4; /* los 4 octetos */
    if (sscanf(ip, "%d.%d.%d.%d", &num1, &num2, &num3, &num4) == 4)
        if (num1 < 0xFF && num2 < 0xFF && num3 < 0xFF && num4 < 0xFF)
            return 1;
        else
            return 0;
    else
        return 0;
    return 0;
}
/************************** Funcion setearPosiciones ***********************/
/*Funcion para setear las posiciones iniciales de los jugadores en la matriz
  que luego usaremos para validar los movimientos*/
void setearPosiciones()
{

    if (miFelix.NumJugador == 1)
    {
        if ( otroFelix.x !=-1 && otroFelix.y != -1 )
        {
            otroFelix.x = 4;
            otroFelix.y = 0;
        }
        if ( miFelix.x !=-1 && miFelix.y != -1 )
        {
            miFelix.x = 0;
            miFelix.y = 0;
        }

    }
    else
    {
        if ( otroFelix.x !=-1 && otroFelix.y != -1 )
        {
            otroFelix.x = 0;
            otroFelix.y = 0;
        }
        if ( miFelix.x !=-1 && miFelix.y != -1 )
        {
            miFelix.x = 4;
            miFelix.y = 0;
        }
    }
}
/************************** Funcion finalizarCliente ***********************/
void finalizarCliente(int signo)
{
    send(socketAddress, "END" , MAXDATASIZE, 0);
    sleep(1);
    puts("Cliente finalizado.");
    close(socketAddress);
    exit(0);
}

// Split de strings
vector<string> split(char *str, char *sep)
{
    char* current;
    vector<std::string> arr;
    current=strtok(str, sep);
    while (current != NULL)
    {
        arr.push_back(current);
        current=strtok(NULL, sep);
    }
    return arr;
}



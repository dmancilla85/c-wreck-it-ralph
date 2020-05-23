all:Torneo Partida Cliente

Torneo: Torneo.o
	g++ -o Torneo Torneo.o -lSDL -lSDL_image -lSDL_ttf  -lpthread -lrt 

Torneo.o: Torneo.cpp
	g++ -c Torneo.cpp
	
Partida: Partida.o
	g++ -std=gnu++0x -o Partida Partida.o -lpthread -lrt -lSDL 

Partida.o: Partida.cpp
	g++ -std=gnu++0x -fno-stack-protector -c Partida.cpp	
objetos = Cliente.o Felix.o Ralph.o Ladrillo.o Edificio.o Pato.o CSprite.o CSpriteBase.o

cflags = -std=c++11 -w -ggdb -lm -pthread 
cflags2 = -std=gnu++0x -w -ggdb -lm -pthread
	
Cliente: $(objetos)

	g++ $(cflags2) -o Cliente $(objetos) -lSDL -lSDL_mixer -lSDL_image -lSDL_ttf -lX11

Cliente.o: Cliente.cpp include/variables.h include/Felix.h include/Ralph.h include/Ladrillo.h
	g++ $(cflags2) -c Cliente.cpp	
				
Felix.o: 	src/Felix.cpp include/Felix.h include/Edificio.h include/variables.h
	g++ $(cflags2) -c src/Felix.cpp
	
Pato.o: 	src/Pato.cpp include/Pato.h include/CSprite.h include/variables.h
	g++ $(cflags2) -c src/Pato.cpp

Edificio.o: 	src/Edificio.cpp include/Edificio.h include/CSprite.h include/variables.h
	g++ $(cflags2) -c src/Edificio.cpp	
				
Ralph.o: 	src/Ralph.cpp include/Ralph.h include/Edificio.h include/variables.h
	g++ $(cflags2) -c src/Ralph.cpp
				
Ladrillo.o: src/Ladrillo.cpp include/Ladrillo.h include/CSprite.h include/variables.h
	g++ $(cflags2) -c src/Ladrillo.cpp
				
CSprite.o:	src/CSprite.cpp include/CSprite.h include/CSpriteBase.h include/variables.h
	g++ $(cflags2) -c src/CSprite.cpp
				
CSpriteBase.o: src/CSpriteBase.cpp include/CSpriteBase.h include/variables.h
	g++ $(cflags2) -c src/CSpriteBase.cpp

.PHONY: clean

clean:
	rm -f *.o Cliente Partida Torneo

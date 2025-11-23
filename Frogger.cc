#include <esat/window.h>
#include <esat/draw.h>
#include <esat/input.h>
#include <esat/time.h>
#include <esat/sprite.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/* Enums */
enum Direccion {
  ARRIBA,
  DERECHA,
  ABAJO,
  IZQUIERDA
};

/* STRUCTS */
struct PuntoCoord{
    float x = 0, y = 0;
};

struct Colision{
    // P1 Tambien sirve como ubicación
    PuntoCoord P1;
    PuntoCoord P2;
};

struct Sprite{
    esat::SpriteHandle imagen;
    Colision collider = {0.0f,0.0f};
    // tipoAnimacion -> El set de animaciones a usar (Fila en spritesheet);
    // indiceAnimacion -> Secuencia de la animación a usar (Columna en spritesheet);
    unsigned char tipoAnimacion = 0, indiceAnimacion = 0;
};


/* FIN STRUCTS */

/* GLOBALES */
// Constantes
//-- Tamaños ventanas
const int VENTANA_X = 588, VENTANA_Y = 720;

//-- Fuentes
const unsigned char ALTURA_FUENTE = 19;

//-- FPS
const unsigned char FPS=60;

//-- UI

//-- Fin UI

// Fin Constantes


// Variables
//-- Tiempos para FPS
double current_time,last_time;


//-- UI


//-- Fin UI


//-- SpriteSheets
//Recursos/Imagenes/Sprites

//-- Fin SpriteSheets


// Fin Variables
/* FIN GLOBALES */

/* FUNCIONALIDADES */
// UTILS
void ControlFPS(){
    do{
        current_time = esat::Time();
    }while((current_time-last_time)<=1000.0/FPS);
}

//-- Randoms
    // Genera nueva semilla en base al 
    // ms del reloj del sistema
void GenerarSemillaAleatoria(){
    srand(time(NULL)); 
}
//Genera un número del 0 al límite indicado
int GenerarNumeroAleatorio(int limite){
    return (rand()%limite);
}
//-- Fin Randoms

//-- Arrays
int GetIndiceArray(int fila, int columnasTotales, int columna){
    return((fila*columnasTotales)+columna);
}

int GetFilaIndice(int indice, int columnasTotales){
    return(indice/columnasTotales);
}

int GetColumnaIndice(int indice, int columnasTotales){
    return(indice%columnasTotales);
}
//-- Fin Arrays


/* FIN FUNCIONALIDADES*/

int esat::main(int argc, char **argv) {
    GenerarSemillaAleatoria();

    esat::WindowInit(VENTANA_X,VENTANA_Y);
    WindowSetMouseVisibility(true);

    //Declaración de la fuente

    
    while(esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {
        last_time = esat::Time(); 

        //Inicio dibujado de pantalla
        esat::DrawBegin();
        esat::DrawClear(0,0,0);


        esat::DrawEnd();      
        esat::WindowFrame();
        //Fin dibujado de pantalla
        
        ControlFPS();
    }

    esat::WindowDestroy();
    return 0;  
}


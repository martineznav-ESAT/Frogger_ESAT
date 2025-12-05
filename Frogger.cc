#include <esat/window.h>
#include <esat/draw.h>
#include <esat/input.h>
#include <esat/time.h>
#include <esat/sprite.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/* Enums */
enum Pantalla {
    INTRO,
    PUNTUACIONES,
    INSERT,
    RANKING,
    START,
    JUEGO
};

enum Direccion {
    ARRIBA,
    DERECHA,
    ABAJO,
    IZQUIERDA
};

enum Align_H{
    LEFT,
    CENT,
    RIGHT,
};
enum Align_V{
    TOP,
    MID,
    BOT
};

enum FilaRio{
    FILA_RIO_0,
    FILA_RIO_1,
    FILA_RIO_2,
    FILA_RIO_3,
    FILA_RIO_4,
    FILA_RIO_5,
    FILA_RIO_T
};

// Facilita la selección del indiceAnimacion 
// de los vehiculos para dibujar el vehiculo deaseado
enum TipoVehiculo {
    COCHE_AMARILLO,
    TRACTOR,
    COCHE_ROSA,
    COCHE_BLANCO,
    CAMION_FRONT,
    CAMION_BACK
};

/* STRUCTS */
struct PuntoCoord{
    float x = 0, y = 0;
};

struct Color{
    unsigned char r, g, b, a = 255;
};

struct Collider{
    // P1 Tambien sirve como ubicación
    PuntoCoord P1 = {0.0f,0.0f};
    PuntoCoord P2 = {10.0f,10.0f};
};

struct SpriteSheet{
    //SpriteSheet
    esat::SpriteHandle spriteSheet;
    //Filas y columnas del SpriteSheet
    unsigned char tiposAnim, indicesAnim;
    //Total Coordenadas X,Y de acceso al sprite deseado
    unsigned char coordsAnim;
    unsigned char totalCoordsAnim;
    //Tamaño de los sprites
    int spriteWidth,spriteHeight;
};

struct Sprite{
    esat::SpriteHandle imagen;
    Collider collider;
    // tipoAnimacion -> El set de indiceAnimacion a usar (Fila en spritesheet);
    // indiceAnimacion -> Secuencia de la animación a usar (Columna en spritesheet);
    unsigned char tipoAnimacion = 0, indiceAnimacion = 0;
    bool isVisible = true, isActive = true; //isActive indica si está dispuesto a comprobar colisiones
};

// Contiene información sobre el control de tiempo de una animación
// duracion     -> La duración total de la animacion antes de reiniciarse/finalizar
// velocidad    -> El tiempo que tarda en avanzar de frame. Se calcula dividiendo la duracion entre la cantidad de frames de la animacion 
// temporizador -> Utilizado por la funcion HacerCadaX/HacerDuranteX. Contiene la información del reloj de sistema necesaria para realizar correctamente el contador. Importante igualar a last_time una única vez antes de usar el contador
struct Animacion{
    // La duración total de la animacion antes de reiniciarse/finalizar
    float duracion; 
    // El tiempo que tarda en avanzar de frame. Se calcula dividiendo la duracion entre la cantidad de frames de la animacion 
    float velocidad; 
    // Utilizado por la funcion HacerCadaX/HacerDuranteX. Contiene la información del reloj de sistema necesaria para realizar correctamente el contador. Importante igualar a last_time una única vez antes de usar el contador
    float temporizador = 0;
};


struct Rana{
	Direccion direccion;
	Sprite sprite;
    bool isJumping = false;
    // Se encarga de conservar la coordenada final en la
    // que debe aterrizar la rana para asegurar su correcto movimiento
    Collider finSalto;
    int distanciaSalto;
    Animacion animSalto;
};

struct Jugador{
    Rana ranaJugador;
    int puntuacion = 0, vidas = 0, nivelActual = 1;
    Animacion animMuerte;
};

struct Vehiculo{
    TipoVehiculo tipoVehiculo;
    Direccion direccion;
	Sprite sprite;
    float velocidadMovimiento;
};

struct Troncodrilo{
    Direccion direccion;
	Sprite sprite;
    float velocidadMovimiento;
    // Si no es un tronco, será un cocodrilo
    bool isTronco = true;
};

struct Tortuga{
    Direccion direccion;
	Sprite sprite;
    float velocidadMovimiento;
    // Indica si la tortuga tiene la posibilidad de hundirse o no
    bool isSumergible = true, isSumergiendo = true;
    Animacion animSumergir;
};

/* FIN STRUCTS */

/* GLOBALES */
//-- Tamaños ventanas
const int VENTANA_X = 672, VENTANA_Y = 768;
const int SPRITE_SIZE = 48;
const int VENTANA_COLUMNAS = VENTANA_X/SPRITE_SIZE +1, VENTANA_FILAS = VENTANA_Y/SPRITE_SIZE;

//-- FPS
const unsigned char FPS=60;
double current_time,last_time;

//-- Prototipado
bool areCollidersVisible = false;

//-- SpriteSheets y arrays de coordenadas del mismo
// Estos arrays no se incluyen en el propio spriteSheet para poder inicializar su tamaño
// evitando problemas de acceso a memoria al no tener la capacidad de usar gestión dinámica de memoria
SpriteSheet animMuerteSpriteSheet;
int animMuerteSpriteSheet_Coords[14]; 

SpriteSheet ranaBaseSpriteSheet;
SpriteSheet ranaRosaSpriteSheet;
SpriteSheet ranaRojaSpriteSheet;
int ranasSpriteSheet_Coords[16];

SpriteSheet vehiculosSpriteSheet;
int vehiculosSpriteSheet_Coords[12];

SpriteSheet tortugaSpriteSheet;
int tortugaSpriteSheet_Coords[12];

SpriteSheet troncoSpriteSheet;
int troncoSpriteSheet_Coords[6];

SpriteSheet zonaFinalSpriteSheet;
int zonaFinalSpriteSheet_Coords[8];

SpriteSheet ranaFinSpriteSheet;
int ranaFinSpriteSheet_Coords[4];

SpriteSheet tituloSpriteSheet;
int tituloSpriteSheet_Coords[10];

SpriteSheet ranaIntroSpriteSheet;
int ranaIntroSpriteSheet_Coords[4];

//-- Sprites | Declaración de los handles cuyos sprites: 
//  -Siempre serán iguales (Sin animación ni acceso multiple como los vehiculos)
//  -No necesitan collider
esat::SpriteHandle arbustoSprite;
esat::SpriteHandle indicadorNivelSprite;
esat::SpriteHandle vidaSprite;
esat::SpriteHandle copySprite;

//-- UI
const unsigned char FONT_SIZE = 24;
const unsigned char maxScoreDigits = 5, maxCreditDigits = 2;
int hiScore = 0, credits = 0;

const unsigned char maxLetrasTitulo = 7;
int letraActual = 0;
Rana letrasTitulo[maxLetrasTitulo];

const unsigned char maxRankingScores = 5;
int rankingScores[maxRankingScores] = {0,0,0,0,0};

Pantalla pantallaActual = INTRO;
Animacion animIntro = {37000,1000,0};
Animacion animPuntuaciones = {10000,1000,0};
Animacion animRanking = {3000,1000,0};
Animacion animInsert = {3000,1000,0};
Animacion animBarrido = {3000,1000,0};

// Duracion, velocidad de avance y temporizador del 
// contador para que el jugador llegue a una zona final 
Animacion cronometro;

//-- Jugadores
const unsigned char maxJugadores = 2;
unsigned char jugadorActual = 0;
Jugador jugadores[maxJugadores];

//-- Obstáculos
const int filasCarretera = 5;
const int maxCamiones = 8, maxCochesBlancos = 4, maxCochesAmarillos = 4;
const int maxCochesRosas = 5, maxTractores = 4;
Vehiculo camiones[maxCamiones], cochesBlancos[maxCochesBlancos], cochesAmarillos[maxCochesAmarillos]; 
Vehiculo cochesRosas[maxCochesRosas], tractores[maxTractores];

Tortuga tortugas_1[VENTANA_COLUMNAS], tortugas_2[VENTANA_COLUMNAS];
Troncodrilo troncos_1[VENTANA_COLUMNAS], troncos_2[VENTANA_COLUMNAS], troncos_3[VENTANA_COLUMNAS];

const int maxZonasFinales = 28;
Sprite zonasFinales[maxZonasFinales];

const int maxRanasFinales = 5;
Sprite ranasFinales[maxRanasFinales];

//-- Estructuras
const int tamanyoFilaArbustos = 14;

// Fin Variables
/* FIN GLOBALES */

/* FUNCIONALIDADES */
void ControlFPS(){
    do{
        current_time = esat::Time();
    }while((current_time-last_time)<=1000.0/FPS);
}

// Devuelve el valor del contador del temporizador proporcionado en milisegundos
//  temp = temporizador del que se quiere obtener el contador
//  unidad = unidades en las que se quiere recuperar el contador. Por defecto lo devuelve en milisegundos. Como referencia, si unidad es 1000 entonces lo devolverá en segundos
float GetContadorFromTemp(float temp, int unidad = 1){
    return (last_time - temp)/unidad;
}

// Actua como "trigger" donde:
// Si el tiempo actual de ejecución menos el temporizador pasado 
// por parametro es mayor a 'x' entonces permitirá acceso 
// (Todo en milesimas de segundo)
bool HacerCadaX(float *temp, float x){
    bool isAccesible = false;
    // printf("LOG--- Valores funcion HacerCadaX()\n");
    // printf("LOG--- T.Actual    Temp    Contador    Limite\n");
    // printf("LOG--- %14.10f | %14.10f | %14.10f |%14.10f\n",last_time,(*temp), last_time - (*temp),x);
    isAccesible = GetContadorFromTemp((*temp)) > x;
    if(isAccesible){
        *temp = last_time;
    }
    return isAccesible;
}

bool HacerDuranteX(float *temp, float x){
    return(!HacerCadaX(&(*temp),x));
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

//-- Sprites
// Necesita un SpriteSheet con todos los parametros completados a excepción de spriteSheetCoords. 
// Calcula e inicializa en spriteSheetCoords las coordenadas X e Y de cada Sprite 
// dentro del SpriteSheet para uso futuro durante ejecución
void InicializarCoordsSpriteSheet(SpriteSheet *spriteSheet, int spriteSheetCoords[]){
    for(int i = 0; i < (*spriteSheet).tiposAnim; i++){
        for(int j = 0, k = 0; j < (*spriteSheet).coordsAnim; j+=2, k++){
            //Coordenada X
            spriteSheetCoords[GetIndiceArray(i,(*spriteSheet).coordsAnim,j)] = (*spriteSheet).spriteWidth*k;
            //Coordenada Y
            spriteSheetCoords[GetIndiceArray(i,(*spriteSheet).coordsAnim,j+1)] = (*spriteSheet).spriteHeight*i;
        }
    }
}


//Dada una coordenada actual y una esperada final, devuelve false si es difetente
bool ComprobarPosicionFinal(Collider actual, Collider final){
    return (
        actual.P1.x == final.P1.x && actual.P2.x == final.P2.x &&
        actual.P1.y == final.P1.y && actual.P2.y == final.P2.y
    );
}

bool ComprobarSalidaVentanaSprite(Sprite *sprite, Direccion direccion){
    bool isOut = false;
    switch(direccion){
        case ARRIBA:
            isOut = (*sprite).collider.P2.y < 0;
        break;
        case DERECHA:
            isOut = (*sprite).collider.P1.x > VENTANA_X;
        break;
        case ABAJO:
            isOut = (*sprite).collider.P1.y > VENTANA_Y;
        break;
        case IZQUIERDA:
            isOut = (*sprite).collider.P2.x < 0;
        break;
    }

    return isOut;
}

// Se guardan en sus structs correspondientes para facilitar su uso y legibilidad durante el proyecto
// Los archivos de hojas de sprite en .png se ubican en ./Recursos/Imagenes/SpriteSheets/*.png
void InicializarSpriteSheet(char rutaSpriteSheetPNG[], SpriteSheet *spriteSheet, int spriteSheet_Coords[], int tiposAnim, int indicesAnim, bool ignoreCoords = false){
    (*spriteSheet).spriteSheet = esat::SpriteFromFile(rutaSpriteSheetPNG);
    (*spriteSheet).tiposAnim = tiposAnim;
    (*spriteSheet).indicesAnim = indicesAnim;
    (*spriteSheet).coordsAnim = (*spriteSheet).indicesAnim * 2;
    (*spriteSheet).totalCoordsAnim = (*spriteSheet).tiposAnim * (*spriteSheet).coordsAnim;
    (*spriteSheet).spriteWidth = (esat::SpriteWidth((*spriteSheet).spriteSheet) / (*spriteSheet).indicesAnim);
    (*spriteSheet).spriteHeight = (esat::SpriteHeight((*spriteSheet).spriteSheet) / (*spriteSheet).tiposAnim);
    if(!ignoreCoords){
        InicializarCoordsSpriteSheet(&(*spriteSheet), spriteSheet_Coords);
    }
}

// Inicializa todos los valores de todos los SpriteSheets para ser utilizables durante el resto de la ejecución del programa
void InicializarSpriteSheets(){
    //Inicializa el SpriteSheet de la animación de muerte del jugador
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/AnimMuerteSpriteSheet.png",
        &animMuerteSpriteSheet,
        animMuerteSpriteSheet_Coords,
        1,
        7
    );

    // Inicializa los SpriteSheets de las ranas en todos sus colores y el array de sus coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaBaseSpriteSheet.png",
        &ranaBaseSpriteSheet,
        ranasSpriteSheet_Coords,
        4,
        2
    );
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaRosaSpriteSheet.png",
        &ranaRosaSpriteSheet,
        ranasSpriteSheet_Coords,
        4,
        2,
        true
    );
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaRojaSpriteSheet.png",
        &ranaRojaSpriteSheet,
        ranasSpriteSheet_Coords,
        4,
        2,
        true
    );

    // Inicializa el SpriteSheet de los vehiculos y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/VehiculosSpriteSheet.png",
        &vehiculosSpriteSheet,
        vehiculosSpriteSheet_Coords,
        1,
        6
    );

    // Inicializa el SpriteSheet de las tortugas y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/TortugaSpriteSheet.png",
        &tortugaSpriteSheet,
        tortugaSpriteSheet_Coords,
        1,
        6
    );

    // Inicializa el SpriteSheet de los troncos y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/TroncoSpriteSheet.png",
        &troncoSpriteSheet,
        troncoSpriteSheet_Coords,
        1,
        3
    );

    // Inicializa el SpriteSheet de la zona final y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/ZonaFinalSpriteSheet.png",
        &zonaFinalSpriteSheet,
        zonaFinalSpriteSheet_Coords,
        1,
        4
    );

    // Inicializa el SpriteSheet de la rana que aparece al llegar a una zona final y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaFinSpriteSheet.png",
        &ranaFinSpriteSheet,
        ranaFinSpriteSheet_Coords,
        1,
        2
    );

    // Inicializa el SpriteSheet del titulo del juego y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/TituloSpriteSheet.png",
        &tituloSpriteSheet,
        tituloSpriteSheet_Coords,
        1,
        5
    );

    // Inicializa el SpriteSheet de las ranas que aparecen en la intro del juego
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaIntroSpriteSheet.png",
        &ranaIntroSpriteSheet,
        ranaIntroSpriteSheet_Coords,
        1,
        2
    );
}

void InicializarSprites(){
    arbustoSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/ArbustoSprite.png");
    indicadorNivelSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/IndicadorNivelSprite.png");
    vidaSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/VidaSprite.png");
    copySprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/CopySprite.png");
}

//*** MANEJO DE SPRITES/SPRITESHEETS ***/
// Recupera el sprite del tipoAnimacion(Fila) e indiceAnimacion(Columna) de una spriteSheet
esat::SpriteHandle GetSpriteFromSheet(SpriteSheet *spriteSheet, int spriteSheetCoords[], int tipoAnimacion, int indiceAnimacion){
    return (esat::SubSprite(
        (*spriteSheet).spriteSheet,
        spriteSheetCoords[GetIndiceArray(tipoAnimacion,(*spriteSheet).coordsAnim,indiceAnimacion)],
        spriteSheetCoords[GetIndiceArray(tipoAnimacion,(*spriteSheet).coordsAnim,indiceAnimacion+1)],
        (*spriteSheet).spriteWidth,
        (*spriteSheet).spriteHeight
    ));
}

// ** USAR SOLO CUANDO SE DESEA CAMBIAR DE SPRITE NO PARA DIBUJAR UNA IMAGEN DE FORMA REGULAR **
// Dada una estructura SpriteSheet y una estructura Sprite, actualiza la imagen
// del Sprite en base al SpriteSheet y el tipoAnimacion(Fila) e indiceAnimacion(Columna) que 
// tiene el Sprite y lo dibuja
//
// Mediante la variable local buffer, se asegura de liberar la imagen previa que tenia asignada el Sprite
// para prevernir leaks de memoria
void ActualizarSprite(SpriteSheet *spriteSheet, int spriteSheetCoords[], Sprite *sprite){
    esat::SpriteHandle buffer = (*sprite).imagen;
    (*sprite).imagen = GetSpriteFromSheet(&(*spriteSheet), spriteSheetCoords, (*sprite).tipoAnimacion,(*sprite).indiceAnimacion);

    if(buffer != NULL){
        esat::SpriteRelease(buffer);
    }
}

// ** USAR SOLO CUANDO SE DESEA AVANZAR EL INDICE DE ANIMACIÓN NO PARA ACTUALIZAR EL SPRITE O DIBUJARLO DE FORMA REGULAR **
// Dada una estructura SpriteSheet y una estructura Sprite.
// Avanza el indice de animación del sprite en +2 para que el siguiente Sprite que guarde sea el correspondiente
// al siguiente en su animación. Si el indice es mayor del total de coordenadas, lo reinicia 0 para volver a comenzar la animación
void AvanzarSpriteAnimado(SpriteSheet *spriteSheet, int spriteSheetCoords[], Sprite *sprite){
    // Avanza indice de animación. 
    (*sprite).indiceAnimacion += 2;
    // Si despues de sumar 2, el indice es mayor o igual a las columnas totales de coordenadas
    // de animacion, lo instancia a 0 para reiniciar la animacion
    if((*sprite).indiceAnimacion >= (*spriteSheet).coordsAnim){
        (*sprite).indiceAnimacion = 0;
    }
}

// Dada una estructura SpriteSheet y una estructura Sprite.
// Retrocede el indice de animación del sprite en -2 para que el siguiente Sprite que guarde sea el correspondiente
// al anterior en su animación. Si el indice es menor de 0, lo reinicia al total de coordenadas para volver a comenzar la animación
void RetrocederSpriteAnimado(SpriteSheet *spriteSheet, int spriteSheetCoords[], Sprite *sprite){
    // Avanza indice de animación. 
    (*sprite).indiceAnimacion -= 2;
    // Si despues de restar 2, el indice es menor de 0
    // lo instancia a la ultima coordenada de animacion -1 para reiniciar la animacion a la inversa
    if((*sprite).indiceAnimacion < 0){
        (*sprite).indiceAnimacion = (*spriteSheet).coordsAnim-1;
    }
}

void RellocateSprite(Sprite *sprite, PuntoCoord nuevaUbicacion){
    (*sprite).collider.P1 = nuevaUbicacion;
    (*sprite).collider.P2 = {nuevaUbicacion.x + esat::SpriteWidth((*sprite).imagen), nuevaUbicacion.y + esat::SpriteHeight((*sprite).imagen)};
}

// Ubica el Sprite en el borde opuesto de la dirección que se indica ya que se asume que se ha escapado
// por ese borde y se desea reubicar en el lugar contrario
//   - *collider        -> El collider a actualizar. El P1 de un collider indica también el punto de inicio de dibujado
//   - direccion        -> Indica en que dirección se moverá
void RellocateSpriteOnBorderEscape(Sprite *sprite, Direccion direccion){
    PuntoCoord nuevaUbicacion;
    switch(direccion){
        case ARRIBA:
            nuevaUbicacion = {(*sprite).collider.P1.x,VENTANA_Y};
        break;
        case DERECHA:
            nuevaUbicacion = {0.0f-esat::SpriteWidth((*sprite).imagen),(*sprite).collider.P1.y};
        break;
        case ABAJO:
            nuevaUbicacion = {(*sprite).collider.P1.x,0.0f-esat::SpriteWidth((*sprite).imagen)};
        break;
        case IZQUIERDA:
            nuevaUbicacion = {VENTANA_X,(*sprite).collider.P1.y};
        break;
    }
    RellocateSprite(&(*sprite), nuevaUbicacion);
}

// Actualiza las coordenadas de un Collider
//   - *collider -> El collider a actualizar. El P1 de un collider indica también el punto de inicio de dibujado
//   - direccion -> Indica en que dirección se moverá
//   - velocidad -> Indica la cantidad de pixeles que se moverá el collider en la direccion indicada
void MoveCollider(Collider *collider, Direccion direccion, float velocidad){
    switch(direccion){
        case ARRIBA:
            (*collider).P1.y -= velocidad;
            (*collider).P2.y -= velocidad;
        break;
        case DERECHA:
            (*collider).P1.x += velocidad;
            (*collider).P2.x += velocidad;
        break;
        case ABAJO:
            (*collider).P1.y += velocidad;
            (*collider).P2.y += velocidad;
        break;
        case IZQUIERDA:
            (*collider).P1.x -= velocidad;
            (*collider).P2.x -= velocidad;
        break;
    }
}

//*** MANEJO DE INICIALIZACIÓN DE OBJETOS ***/
//TO_DO GESTION DE INFORMACIÓN EN BASE AL NIVEL

// Inicializa los sprites del rótulo del juego que aparecen en las pantallas de INTRO, PUNTUACION y RANKING asegurandose de que aparecen en la ubicacion correcta
// El parametro booleano indica si debe inicializarse en base a: 
//  -INTRO. Ubicando los sprites como ranas fuera de la pantalla
//  -PUNTUACION/RANKING Ubicando los sprites como el rótulo
void InicializarTitulo(bool isIntro = false){
    letraActual = 0;

    for(int i = 0; i < maxLetrasTitulo; i++){
        letrasTitulo[i].sprite.isVisible = true;
        letrasTitulo[i].sprite.isActive = false;
        letrasTitulo[i].direccion = IZQUIERDA;
        letrasTitulo[i].animSalto.temporizador = last_time;

        if(isIntro){
            letrasTitulo[i].sprite.tipoAnimacion = 0;
            letrasTitulo[i].sprite.indiceAnimacion = 2;
            letrasTitulo[i].sprite.collider = {
                {VENTANA_X,SPRITE_SIZE*9},
                {VENTANA_X+SPRITE_SIZE,SPRITE_SIZE*10}
            };

            if(i == 0){
                letrasTitulo[i].finSalto = {
                    {(i+2.0f)*SPRITE_SIZE,SPRITE_SIZE*9},
                    {(i+3.0f)*SPRITE_SIZE,SPRITE_SIZE*10}
                };
            }else{
                letrasTitulo[i].finSalto = {
                    {letrasTitulo[i-1].finSalto.P2.x+(SPRITE_SIZE/2),letrasTitulo[i-1].finSalto.P1.y},
                    {letrasTitulo[i-1].finSalto.P2.x+((SPRITE_SIZE/2)+SPRITE_SIZE),letrasTitulo[i-1].finSalto.P2.y},
                };
            }

            letrasTitulo[i].distanciaSalto = letrasTitulo[i].sprite.collider.P1.x-letrasTitulo[i].finSalto.P1.x;
            letrasTitulo[i].animSalto.velocidad = SPRITE_SIZE/2;

            if(i < 3){
                letrasTitulo[i].animSalto.duracion = 6000;
            }else{
                if(i < 5){
                    letrasTitulo[i].animSalto.duracion = 3000;
                }else{
                    letrasTitulo[i].animSalto.duracion = 2000;
                }
            }

            ActualizarSprite(&ranaIntroSpriteSheet,ranaIntroSpriteSheet_Coords,&letrasTitulo[i].sprite);
        }else{
            letrasTitulo[i].sprite.tipoAnimacion = 0;

            if(i == 0){
                letrasTitulo[i].sprite.collider = {
                    {(i+2.0f)*SPRITE_SIZE,SPRITE_SIZE*3},
                    {(i+3.0f)*SPRITE_SIZE,SPRITE_SIZE*4}
                };
            }else{
                letrasTitulo[i].sprite.collider = {
                    {letrasTitulo[i-1].sprite.collider.P2.x+(SPRITE_SIZE/2),letrasTitulo[i-1].sprite.collider.P1.y},
                    {letrasTitulo[i-1].sprite.collider.P2.x+((SPRITE_SIZE/2)+SPRITE_SIZE),letrasTitulo[i-1].sprite.collider.P2.y},
                };
            }

            switch(i){
                case 0:
                    //AsignarLetra F
                    letrasTitulo[i].sprite.indiceAnimacion = 0;
                break;
                case 1:
                case 6:
                    //AsignarLetra R
                    letrasTitulo[i].sprite.indiceAnimacion = 2;    
                break;
                case 2:
                    //AsignarLetra O
                    letrasTitulo[i].sprite.indiceAnimacion = 4;
                break;
                case 3:
                case 4:
                    //AsignarLetra G
                    letrasTitulo[i].sprite.indiceAnimacion = 6;
                break;
                case 5:
                    //AsignarLetra E
                    letrasTitulo[i].sprite.indiceAnimacion = 8;
                break;
            }
            ActualizarSprite(&tituloSpriteSheet,tituloSpriteSheet_Coords,&letrasTitulo[i].sprite);
        }
    }
}   

//Comprueba si el grupo de obstaculos puede generarse sin problemas en su fila correspondiente
bool CabeGrupoObstaculos(int posicionGrupo, int columna, int longitud){
    return posicionGrupo == 0 && columna + longitud < VENTANA_COLUMNAS || posicionGrupo != 0 && columna + (longitud - posicionGrupo) < VENTANA_COLUMNAS;
}

// Inicializar valores de las ranas que aparecen al llegar a una zona final donde:
// filaRio      -> Indica la fila del rio en la que se debe ubicar en Y
void InicializarRanasFinales(FilaRio filaRio){
    int posicionZona_X;
    for(int i = 0; i < maxRanasFinales; i++){
        ranasFinales[i].tipoAnimacion = 0;
        ranasFinales[i].indiceAnimacion = 0;
        ranasFinales[i].isVisible = false;
        ranasFinales[i].isActive = true;

        // Recupera la posición libre de la zona final
        posicionZona_X = (i*6+1)*zonaFinalSpriteSheet.spriteWidth;
        ranasFinales[i].collider.P1 = {((float)posicionZona_X),((VENTANA_Y-(SPRITE_SIZE*9))-(((float)filaRio)*SPRITE_SIZE))};
        ranasFinales[i].collider.P2 = {((float)posicionZona_X+ranaFinSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*8))-(((float)filaRio)*SPRITE_SIZE))};
        
        ActualizarSprite(&ranaFinSpriteSheet, ranaFinSpriteSheet_Coords, &ranasFinales[i]);
    }
}

// Inicializar valores de las zonas finales donde:
// filaRio   -> Indica la fila del rio en la que está para ubicarse en Y
void InicializarZonasFinales(FilaRio filaRio){
    int contador = 0;
    for(int i = 0; i < maxZonasFinales; i++){
        zonasFinales[i].collider.P1 = {((float)i*zonaFinalSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*9))-(((float)filaRio+0.5f)*SPRITE_SIZE))}; // se ajusta 0.5f en la fila para coincidir adecuadamente al no medir el sprite 48px de alto
        zonasFinales[i].collider.P2 = {((float)(i+1)*zonaFinalSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*8))-(((float)filaRio)*SPRITE_SIZE))};
        zonasFinales[i].tipoAnimacion = 0;
        zonasFinales[i].isVisible = true;
        switch (contador){
        case 0:
            zonasFinales[i].indiceAnimacion = 0;
            zonasFinales[i].isActive = true;
            break;
        case 1:
        case 2:
            zonasFinales[i].indiceAnimacion = 2;
            zonasFinales[i].isActive = false;
            break;
        case 3:
            zonasFinales[i].indiceAnimacion = 4;
            zonasFinales[i].isActive = true;
            break;
        case 4:
        case 5:
            zonasFinales[i].indiceAnimacion = 6;
            zonasFinales[i].isActive = true;
            break;
        }
        ActualizarSprite(&zonaFinalSpriteSheet, zonaFinalSpriteSheet_Coords, &zonasFinales[i]);

        ++contador %= 6;
    }
}

// Inicializar valores de una fila de tortugas donde:
// longitud  -> Indica el tamaño de cada grupo de tortugas
// margen    -> Indica la cantidad de espaciado entre grupos de la fila (Tortugas Inactivas)
// velocidad -> Indica la velocidad a la que se moveran las tortugas
// filaRio   -> Indica la fila del rio en la que está para ubicarse en Y
void InicializarFilaTortugas(Tortuga tortugas[], int longitud, int margen, float velocidad, FilaRio filaRio){
    // alternator indica cuando debe inicializar en base a la longitud o al margen (por defecto empieza por la longitud)
    bool alternator = true;
    int contador = 0;
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        tortugas[i].direccion = IZQUIERDA;
        tortugas[i].velocidadMovimiento = velocidad;
        tortugas[i].sprite.collider.P1 = {((float)i*tortugaSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*9))-((float)filaRio*tortugaSpriteSheet.spriteHeight))};
        tortugas[i].sprite.collider.P2 = {((float)(i+1)*tortugaSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*8))-((float)filaRio*tortugaSpriteSheet.spriteHeight))};
        tortugas[i].sprite.tipoAnimacion = 0;
        tortugas[i].sprite.indiceAnimacion = 0;
        tortugas[i].isSumergiendo = true;

        tortugas[i].animSumergir.duracion = 1500;
        tortugas[i].animSumergir.temporizador = 0;
        tortugas[i].animSumergir.velocidad = tortugas[i].animSumergir.duracion/tortugaSpriteSheet.indicesAnim;

        if(alternator && longitud > 0 && CabeGrupoObstaculos(contador,i,longitud)){
            //Inicializar grupo
            tortugas[i].sprite.isVisible = true;
            tortugas[i].sprite.isActive = true;

            // Hace que el primer grupo de tortugas sea sumergible
            if(i < longitud){
                tortugas[i].isSumergible = true;
            }else{
                tortugas[i].isSumergible = false;
            }
            ++contador %= longitud;
            if(contador == 0){
                alternator = !alternator;
            }
        }else{
            //Inicializar "margen"
            tortugas[i].sprite.isVisible = false;
            tortugas[i].sprite.isActive = false;

            ++contador %= margen;
            if(contador == 0){
                alternator = !alternator;
            }
        }
        ActualizarSprite(&tortugaSpriteSheet,tortugaSpriteSheet_Coords,&tortugas[i].sprite);
    }
}

// Inicializar valores de una fila de troncodrilos donde:
// longitud  -> Indica el tamaño del tronco
// margen    -> Indica la cantidad de espaciado entre troncos
// velocidad -> Indica la velocidad a la que se moveran los troncodrilos
// filaRio   -> Indica la fila del rio en la que está para ubicarse en Y
void InicializarFilaTroncodrilos(Troncodrilo troncodrilos[], int longitud, int margen, float velocidad, FilaRio filaRio){
    // alternator indica cuando debe inicializar en base a la longitud o al margen (por defecto empieza por la longitud)
    bool alternator = true;
    int contador = 0;
    
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        troncodrilos[i].direccion = DERECHA;
        troncodrilos[i].velocidadMovimiento = velocidad;
        troncodrilos[i].sprite.collider.P1 = {((float)i*troncoSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*9))-((float)filaRio*troncoSpriteSheet.spriteHeight))};
        troncodrilos[i].sprite.collider.P2 = {((float)(i+1)*troncoSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*8))-((float)filaRio*troncoSpriteSheet.spriteHeight))};
        troncodrilos[i].sprite.tipoAnimacion = 0;

        if(alternator && longitud > 0 && CabeGrupoObstaculos(contador,i,longitud)){
            //Inicializar grupo
            troncodrilos[i].sprite.isVisible = true;
            troncodrilos[i].sprite.isActive = true;

            // Construye los troncos
            if(contador == 0){
                troncodrilos[i].sprite.indiceAnimacion = 0;
            }else{
                if(contador == longitud-1){
                    troncodrilos[i].sprite.indiceAnimacion = troncoSpriteSheet.coordsAnim-2;
                }else{
                    troncodrilos[i].sprite.indiceAnimacion = 2;
                }
            }
            
            ++contador %= longitud;
            if(contador == 0){
                alternator = !alternator;
            }
        }else{
            //Inicializar "margen"
            troncodrilos[i].sprite.isVisible = false;
            troncodrilos[i].sprite.isActive = false;

            ++contador %= margen;
            if(contador == 0){
                alternator = !alternator;
            }
        }
        ActualizarSprite(&troncoSpriteSheet,troncoSpriteSheet_Coords,&troncodrilos[i].sprite);
    }
}

// Instancia los valores de la fila correspondiente
void InicializarFilaRio(FilaRio filaRio){
    switch (filaRio){
        case FILA_RIO_5:
            InicializarZonasFinales(filaRio);
            InicializarRanasFinales(filaRio);
        break;
        case FILA_RIO_4:
            InicializarFilaTroncodrilos(troncos_3, 4, 2, 2, filaRio);
        break;
        case FILA_RIO_3:
            InicializarFilaTortugas(tortugas_2, 2, 2, 1, filaRio);
        break;
        case FILA_RIO_2:
            InicializarFilaTroncodrilos(troncos_2, 5, 2, 2, filaRio);
        break;
        case FILA_RIO_1:
            InicializarFilaTroncodrilos(troncos_1, 3, 2, 2, filaRio);
        break;
        case FILA_RIO_0:
            InicializarFilaTortugas(tortugas_1, 3, 1, 1, filaRio);
        break;
    }
}

// Instancia los valores por defecto de los elementos de cada fila del rio al inicio de un nivel
void InicializarRio(){
    for(int i = 0 ; i < ((int) FILA_RIO_T); i++){
        InicializarFilaRio((FilaRio) i);
    }
}


// Instancia los valores de un vehiculo
// *vehiculo            -> Vehiculo a instanciar
// tipo                 -> Indica que tipo de vehiculo
// direccion            -> Establece la dirección en la que se está moviendo
// fila                 -> Fila de la pantalla (contando desde abajo) en la que se va a dibujar (coord -> Y)
// posicionX            -> Posición de X del punto 1 del sprite del vehiculo
// velocidadMovimiento  -> Velocidad a la que se moverá el vehiculo
// margen               -> Indica el espacio adicional en X en funcion de la dirección
void InicializarVehiculo(
    Vehiculo *vehiculo, TipoVehiculo tipo, Direccion direccion,
    int fila, float posicionX, 
    float velocidadMovimiento, 
    float margen
){
    margen = vehiculosSpriteSheet.spriteWidth * margen;

    (*vehiculo).direccion = direccion;
    (*vehiculo).sprite.tipoAnimacion = 0;
    (*vehiculo).sprite.isVisible = true;
    (*vehiculo).sprite.isActive = true;
    (*vehiculo).velocidadMovimiento = velocidadMovimiento;
    
    (*vehiculo).sprite.collider.P1 = {posicionX, ((float)(VENTANA_Y-(vehiculosSpriteSheet.spriteHeight*fila)))};
    (*vehiculo).sprite.collider.P2 = {posicionX + vehiculosSpriteSheet.spriteWidth, ((float) VENTANA_Y-(vehiculosSpriteSheet.spriteHeight*(fila-1)))};
    
    switch(direccion){
        case DERECHA:
            (*vehiculo).sprite.collider.P1.x -= margen;
            (*vehiculo).sprite.collider.P2.x -= margen;
        break;
        case IZQUIERDA:
            (*vehiculo).sprite.collider.P1.x += margen;
            (*vehiculo).sprite.collider.P2.x += margen;
        break;
    }
    
    (*vehiculo).sprite.indiceAnimacion = tipo*2;

    ActualizarSprite(&vehiculosSpriteSheet,vehiculosSpriteSheet_Coords,&(*vehiculo).sprite);  
}

// Instancia los valores por defecto de los vehiculos al inicio de un nivel
void InicializarVehiculos(){
    // Se usa maxCamiones como límite ya que es el array de los obstaculos de carretera mas largo
    // Luego internamente se comprueba que el indice sea valido para cada columna (cantidad de vehiculos)
    for(int i = 0; i < maxCamiones; i++){
        // InicializarCochesAmarillos
        if(i < maxCochesAmarillos){
            InicializarVehiculo(
                &cochesAmarillos[i],
                COCHE_AMARILLO,
                IZQUIERDA,
                // Fila 3 empezando desde abajo
                3,
                // Si es el primero, aparece en el borde, si no detras del ultimo + el margen que se le asigne
                i == 0 ?  
                0: 
                cochesAmarillos[i-1].sprite.collider.P1.x+vehiculosSpriteSheet.spriteWidth, 
                //Velocidad
                1,
                // Si es el primero no se le asigna margen.
                i == 0 ? 0 : 2
            );
        }

        // InicializarTractores
        if(i < maxTractores){
            InicializarVehiculo(
                &tractores[i],
                TRACTOR,
                DERECHA,
                4,
                i == 0 ? 
                VENTANA_X-vehiculosSpriteSheet.spriteWidth : 
                tractores[i-1].sprite.collider.P1.x-vehiculosSpriteSheet.spriteWidth,
                1,
                i == 0 ? 0 : 2
            );
        }

        // InicializarCochesRosas
        if(i < maxCochesRosas){
            InicializarVehiculo(
                &cochesRosas[i],
                COCHE_ROSA,
                IZQUIERDA,
                5,
                i == 0 ?  
                0: 
                cochesRosas[i-1].sprite.collider.P1.x+vehiculosSpriteSheet.spriteWidth, 
                1,
                i == 0 ? 0 : 2
            );
        }
        
        // InicializarCochesBlancos
        if(i < maxCochesBlancos){
            InicializarVehiculo(
                &cochesBlancos[i],
                COCHE_BLANCO,
                DERECHA,
                6,
                i == 0 ? 
                VENTANA_X-vehiculosSpriteSheet.spriteWidth : 
                cochesBlancos[i-1].sprite.collider.P1.x-vehiculosSpriteSheet.spriteWidth, 
                1,
                i == 0 ? 0 : 2
            );
        }

        // InicializarCamiones
        InicializarVehiculo(
            &camiones[i],
            i%2 == 0 ? CAMION_FRONT : CAMION_BACK,
            IZQUIERDA,
            7,
            i == 0 ?  
            0: 
            camiones[i-1].sprite.collider.P1.x+vehiculosSpriteSheet.spriteWidth, 
            1,
            // Si es el primero no se le asigna margen.
            // Si no es el primero, tampoco se le asigna margen si se quiere instanciar la parte trasera del camion
            i == 0 ? 0 : i%2==0 ? 2 : 0
        );
    }
}

// Instancia los valores por defecto del jugador
// Pensado para utilizarse después de cada muerte para ubicar al jugador en su posicion inicial
void SpawnJugador(Jugador *jugador){
    (*jugador).ranaJugador.sprite.isActive = true;
    (*jugador).ranaJugador.direccion = ARRIBA;
    (*jugador).ranaJugador.sprite.tipoAnimacion = 0;
    (*jugador).ranaJugador.sprite.indiceAnimacion = 0;
    (*jugador).ranaJugador.sprite.collider.P1 = {
        VENTANA_X/2,
        (float)(VENTANA_Y-ranaBaseSpriteSheet.spriteHeight*8)
    };
    (*jugador).ranaJugador.sprite.collider.P2 = {
        (*jugador).ranaJugador.sprite.collider.P1.x + ranaBaseSpriteSheet.spriteWidth,
        (*jugador).ranaJugador.sprite.collider.P1.y + ranaBaseSpriteSheet.spriteHeight
    };
    (*jugador).ranaJugador.isJumping = false;
    (*jugador).ranaJugador.finSalto = (*jugador).ranaJugador.sprite.collider; 
    (*jugador).ranaJugador.distanciaSalto = ranaBaseSpriteSheet.spriteHeight;
    (*jugador).ranaJugador.animSalto.duracion = 100;
    (*jugador).ranaJugador.animSalto.velocidad = ((*jugador).ranaJugador.distanciaSalto/((*jugador).ranaJugador.animSalto.duracion/(1000.0f/FPS)));
    (*jugador).ranaJugador.animSalto.temporizador = 0;
    ActualizarSprite(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*jugador).ranaJugador.sprite);
}

// Instancia los valores por defecto del jugador
// Pensado para utilizarse al inicio de cada partida
void InicializarJugador(){
    //Propiedades del jugador al inicio de la partida
    jugadores[jugadorActual].ranaJugador.sprite.isActive = true;
    jugadores[jugadorActual].puntuacion = false;
    jugadores[jugadorActual].vidas = 3;
    jugadores[jugadorActual].nivelActual = 1;

    jugadores[jugadorActual].animMuerte.duracion = 2000;
    jugadores[jugadorActual].animMuerte.temporizador = 0;
    jugadores[jugadorActual].animMuerte.velocidad = jugadores[jugadorActual].animMuerte.duracion/animMuerteSpriteSheet.indicesAnim;

    //Spawn Rana en posición inicial del jugador
    SpawnJugador(&jugadores[jugadorActual]);
}

void InicializarCronometro(){
    cronometro.duracion = 30000;
    cronometro.temporizador = last_time;
    cronometro.velocidad = 500;
}

void InicializarNivel(){
    // Inicialización de cosas interactivas durante la ejecución del nivel
    InicializarRio();
    InicializarVehiculos();
    InicializarJugador();
    InicializarCronometro();
}

//*** INICIO DE ACCIONES DE LOS OBJETOS Y JUGADORES***//

// Dada una rana con posicion de finSalto, comprueba si puede realizar ese salto
// El comportamiento cambiará en función de si se indica que es una rana de jugador o bonus
bool IsSaltoRanaPosible(Rana rana, bool isPlayer = true){
    if(isPlayer){
        return (
            rana.finSalto.P1.x >=0 && rana.finSalto.P2.x <= VENTANA_X &&
            rana.finSalto.P1.y >= ranaBaseSpriteSheet.spriteHeight*2 && rana.finSalto.P2.y <= VENTANA_Y-ranaBaseSpriteSheet.spriteHeight
        );
    }else{
        // TO_DO IA RANA
        return false;
    }
}
void IniciarSaltoRana(Rana *rana, Direccion newDireccion){
    (*rana).direccion = newDireccion;
    // CALCULA LA POSICION FINAL DONDE DEBE ATERRIZAR
    switch((*rana).direccion){
        case ARRIBA:
            (*rana).finSalto.P1.y = (*rana).sprite.collider.P1.y-(*rana).distanciaSalto;
            (*rana).finSalto.P2.y = (*rana).sprite.collider.P2.y-(*rana).distanciaSalto;
            (*rana).finSalto.P1.x = (*rana).sprite.collider.P1.x;
            (*rana).finSalto.P2.x = (*rana).sprite.collider.P2.x;
        break;
        case DERECHA:
            (*rana).finSalto.P1.x = (*rana).sprite.collider.P1.x+(*rana).distanciaSalto;
            (*rana).finSalto.P2.x = (*rana).sprite.collider.P2.x+(*rana).distanciaSalto;
            (*rana).finSalto.P1.y = (*rana).sprite.collider.P1.y;
            (*rana).finSalto.P2.y = (*rana).sprite.collider.P2.y;
        break;
        case ABAJO:
            (*rana).finSalto.P1.y = (*rana).sprite.collider.P1.y+(*rana).distanciaSalto;
            (*rana).finSalto.P2.y = (*rana).sprite.collider.P2.y+(*rana).distanciaSalto;
            (*rana).finSalto.P1.x = (*rana).sprite.collider.P1.x;
            (*rana).finSalto.P2.x = (*rana).sprite.collider.P2.x;
        break;
        case IZQUIERDA:
            (*rana).finSalto.P1.x = (*rana).sprite.collider.P1.x-(*rana).distanciaSalto;
            (*rana).finSalto.P2.x = (*rana).sprite.collider.P2.x-(*rana).distanciaSalto;
            (*rana).finSalto.P1.y = (*rana).sprite.collider.P1.y;
            (*rana).finSalto.P2.y = (*rana).sprite.collider.P2.y;
        break;
    }

    // Si detecta que la posición final de salto, excede el borde de la pantalla de juego, 
    // no saltará
    if(IsSaltoRanaPosible(*rana)){
        (*rana).isJumping = true;
        (*rana).animSalto.temporizador = last_time;
        (*rana).sprite.tipoAnimacion = newDireccion;
        AvanzarSpriteAnimado(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
        ActualizarSprite(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
    }else{
        (*rana).finSalto = (*rana).sprite.collider;
    }
}

void MatarJugador(Jugador *jugador){
    (*jugador).ranaJugador.sprite.isActive = false;
    (*jugador).ranaJugador.sprite.tipoAnimacion = 0;
    (*jugador).ranaJugador.sprite.indiceAnimacion = 0;
    (*jugador).animMuerte.temporizador = last_time;
    ActualizarSprite(&animMuerteSpriteSheet, animMuerteSpriteSheet_Coords, &(*jugador).ranaJugador.sprite);
}

//*** DETECCIÓN INPUT DEL JUGADOR ***//
void DetectarControles(){
    if(pantallaActual == JUEGO){
        // CONTROLES RANA_J1
        // El jugador podrá realizar acciones siempre que:
        //   - No esté saltando
        if(!jugadores[0].ranaJugador.isJumping && jugadores[jugadorActual].ranaJugador.sprite.isActive){
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Up)){
                IniciarSaltoRana(&jugadores[0].ranaJugador, ARRIBA);
            }
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Right)){
                IniciarSaltoRana(&jugadores[0].ranaJugador, DERECHA);
            }
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Down)){
                IniciarSaltoRana(&jugadores[0].ranaJugador, ABAJO);
            }
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Left)){
                IniciarSaltoRana(&jugadores[0].ranaJugador, IZQUIERDA);
            }
        }
    }

    //PROTOTIPADO
    if(esat::IsKeyDown('1')){
        printf("INTRO\n");
        InicializarTitulo(true);
        animIntro.temporizador=last_time;
        pantallaActual = INTRO;
    }
    if(esat::IsKeyDown('2')){
        printf("PUNTUACIONES\n");
        InicializarTitulo();
        animPuntuaciones.temporizador=last_time;
        pantallaActual = PUNTUACIONES;
    }
    if(esat::IsKeyDown('3')){
        printf("RANKING\n");
        InicializarTitulo();
        animRanking.temporizador=last_time;
        pantallaActual = RANKING;
    }
    if(esat::IsKeyDown('4')){
        printf("INSERT\n");
        animInsert.temporizador=last_time;
        pantallaActual = INSERT;
    }
    if(esat::IsKeyDown('5')){
        printf("START\n");
        pantallaActual = START;
    }
    if(esat::IsKeyDown('6')){
        printf("JUEGO\n");
        pantallaActual = JUEGO;
        InicializarNivel();
    }
    //Alterna la visualización de los colliders
    if(esat::IsSpecialKeyDown(esat::kSpecialKey_Alt)){
        areCollidersVisible = !areCollidersVisible;
        if(areCollidersVisible){
            printf("SHOWING COLLIDERS\n");
        }else{
            printf("HIDDEN COLLIDERS\n");
        }
    }
}

float GetFilaPantallaSprite(Sprite s){
    return (VENTANA_Y-s.collider.P1.y)/ranaBaseSpriteSheet.spriteHeight;
}
float GetColumnaPantallaSprite(Sprite s){
    return s.collider.P1.x/ranaBaseSpriteSheet.spriteWidth;
}

/*** FUNCIONES DE ACTUALIZACIÓN DE ESTADO DEL JUEGO ***/

/** COLISIONES **/
// Dados 2 collider, comprueba si hay colisión entre ellos
bool DetectarColision(Collider C1, Collider C2) {
  return (C1.P2.x >= C2.P1.x) &&
         (C1.P1.x <= C2.P2.x) &&
         (C1.P2.y >= C2.P1.y) &&
         (C1.P1.y <= C2.P2.y);
}

// Dados 2 collider, siendo el primero de un jugador, ajusta el radio de colision 
// y comprueba si hay colisión entre ellos
bool DetectarColisionJugador(Jugador player, Collider object_C) {
  return (player.ranaJugador.sprite.collider.P2.x-6 > object_C.P1.x) &&
         (player.ranaJugador.sprite.collider.P1.x+6 < object_C.P2.x) &&
         (player.ranaJugador.sprite.collider.P2.y-6 > object_C.P1.y) &&
         (player.ranaJugador.sprite.collider.P1.y+6 < object_C.P2.y);
}

// Si está saltando, comprueba si se choca con la pared.
// Si consigue aterrizar, se asegura que haga contacto una zona desocupada
//      Zona Ocupada -> Mismo efecto que chocar con la pared
//      Zona Desocupada -> Suma de puntuacion y comprobar si se ha superado el nivel, ejecutando unos ventos u otros
void DetectarColisionZonaFinal(Jugador *jugador){
    int victoria = 0;
    if((*jugador).ranaJugador.isJumping){
        for(int i = 0; i < maxZonasFinales; i++){
            if(zonasFinales[i].isActive && DetectarColisionJugador((*jugador), zonasFinales[i].collider)){
                MatarJugador(&(*jugador));
            }
        }
    }else{
        for(int i = 0; i < maxRanasFinales; i++){
            if(DetectarColisionJugador((*jugador), ranasFinales[i].collider)){
                if(ranasFinales[i].isVisible){
                    MatarJugador(&(*jugador));
                }else{
                    ranasFinales[i].isVisible = true;
                    (*jugador).puntuacion += 200;
                    InicializarCronometro();
                    SpawnJugador(&(*jugador));
                }
            }

            if(ranasFinales[i].isVisible){
                victoria++;
            }
        }
        printf("VICTORIA %d\n",victoria);

        if(victoria == maxRanasFinales){
            // TO_DO
            // AvanzarNivel();
        }
    }
}

// Comprueba si puede posarse sobre un troncodrilo
void DetectarColisionJugadorFilaTroncodrilos(Jugador *jugador, Troncodrilo troncodrilos[]){
    int indiceActual = 0;

    while(!DetectarColisionJugador(*jugador,troncodrilos[indiceActual].sprite.collider)){
        indiceActual++;
    };

    if((*jugador).ranaJugador.isJumping){
        switch((*jugador).ranaJugador.direccion){
            case DERECHA:
                
            break;
            case IZQUIERDA:

            break;
        }
    }else{
        if(troncodrilos[indiceActual].sprite.isActive){
            MoveCollider(
                &(*jugador).ranaJugador.sprite.collider, 
                (Direccion) troncodrilos[indiceActual].direccion, 
                troncodrilos[indiceActual].velocidadMovimiento
            );
        }else{
            MatarJugador(&(*jugador));
        }
    }
}

// Comprueba si puede posarse sobre una tortuga
void DetectarColisionJugadorFilaTortugas(Jugador *jugador, Tortuga tortugas[]){
    int indiceActual = 0;

    while(!DetectarColisionJugador(*jugador,tortugas[indiceActual].sprite.collider)){
        indiceActual++;
    };

    if((*jugador).ranaJugador.isJumping){
        switch((*jugador).ranaJugador.direccion){
            case DERECHA:
                
            break;
            case IZQUIERDA:

            break;
        }
    }else{
        if(tortugas[indiceActual].sprite.isActive){
            MoveCollider(
                &(*jugador).ranaJugador.sprite.collider, 
                (Direccion) tortugas[indiceActual].direccion, 
                tortugas[indiceActual].velocidadMovimiento
            );
        }else{
            MatarJugador(&(*jugador));
        }
    }
}

// Todos los vehiculos actuan de la misma forma contra la rana:
// La rana es atropellada y muere en el acto, activando todos los 
// eventos que deban ocurrir cuando esta fallece
void DetectarColisionJugadorFilaVehiculos(Jugador *jugador, Vehiculo vehiculos[], int totalVehiculos){
    for(int i = 0; i < totalVehiculos; i++){
        if(DetectarColisionJugador((*jugador),vehiculos[i].sprite.collider)){
            MatarJugador(&(*jugador));
        }
    }
}

// Dado un sprite y una fila, devuelve verdadero si debería comprobar la fila pasada por parametro y sus adyacentes al estar el sprite ubicado en una de ellas
bool ComprobarFilaActualYAdyacentes(Sprite sprite, int fila){
    // return(GetFilaPantallaSprite(sprite) == fila);
    return(GetFilaPantallaSprite(sprite) >= fila-0.9 && GetFilaPantallaSprite(sprite) <= fila+0.9);
}

// En función de donde se ubica la rana del jugador, comprueba las colisiones de la fila de la pantalla donde se encuentra y
// de las adyacentes debido a sus posibles movimientos futuros.
// Esto permite no tener que comprobar las colisiones de absolutamente todos los obstaculos
void ComprobarColisionesJugador(Jugador *jugador){
    //Recorre las filas con posibles interacciones con el jugador si está activo
    if((*jugador).ranaJugador.sprite.isActive){
        if((*jugador).ranaJugador.sprite.collider.P1.x < 0 || (*jugador).ranaJugador.sprite.collider.P2.x > VENTANA_X){
            MatarJugador(&(*jugador));
        }

        for(int filaComprobar = 3; filaComprobar < VENTANA_Y/ranaBaseSpriteSheet.spriteHeight - 1; filaComprobar++){
            if(ComprobarFilaActualYAdyacentes((*jugador).ranaJugador.sprite,filaComprobar)){
                // Si es una fila que en la que se encuentra el jugador o es adyacente, comprueba las colisiones 
                // relacionadas con el jugador correspondientes
                switch(filaComprobar){
                    //FINAL
                    case 14:
                        // printf("FINAL\n");
                        DetectarColisionZonaFinal(&(*jugador));
                    break;
                    // RIO
                    case 13:
                        // printf("FILA_RIO_4\n");
                        DetectarColisionJugadorFilaTroncodrilos(&(*jugador),troncos_3);
                    break;
                    case 12:
                        // printf("FILA_RIO_3\n");
                        DetectarColisionJugadorFilaTortugas(&(*jugador),tortugas_2);
                    break;
                    case 11:
                        // printf("FILA_RIO_2\n");
                        DetectarColisionJugadorFilaTroncodrilos(&(*jugador),troncos_2);
                    break;
                    case 10:
                        // printf("FILA_RIO_1\n");
                        DetectarColisionJugadorFilaTroncodrilos(&(*jugador),troncos_1);
                    break;
                    case 9:
                        // printf("FILA_RIO_0\n");
                        DetectarColisionJugadorFilaTortugas(&(*jugador),tortugas_1);
                    break;

                    //CARRETERA
                    case 7:
                        // printf("CAMION\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),camiones, maxCamiones);
                    break;
                    case 6:
                        // printf("COCHE BLANCO\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),cochesBlancos, maxCochesBlancos);
                    break;
                    case 5:
                        // printf("COCHE ROSA\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),cochesRosas, maxCochesRosas);
                    break;
                    case 4:
                        // printf("TRACTOR\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),tractores, maxTractores);
                    break;
                    case 3:
                        // printf("COCHE AMARILLO\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),cochesAmarillos, maxCochesAmarillos);
                    break;
                }
            }
        }
        // printf("\n");
    }

}

/** ESTADOS **/

// Cambia los valores de posicion de un obstaculo en función de los parametros indicados
// Pensado para el movimiento de obstaculos en movimiento como los vehiculos o los troncos
void ActualizarMovimientoObstaculo(Sprite *sprite, Direccion direccion, float velocidad){
    if (ComprobarSalidaVentanaSprite(&(*sprite), direccion)){
        RellocateSpriteOnBorderEscape(&(*sprite), direccion);
    }else{
        MoveCollider(&(*sprite).collider, direccion, velocidad);
    }
}

//Comprueba si la tortuga se está sumergiendo o no (Direccion derecha o inversa de la animacion y deteccion de colision) 
void ComprobarSumersionTortuga(Tortuga *tortuga){
    int preSumersionCoord = (tortugaSpriteSheet.coordsAnim/2)-2;
    int postSumersionCoord = tortugaSpriteSheet.coordsAnim/2;
    int ultimaCoord = tortugaSpriteSheet.coordsAnim-2;

    // Si está en indice 0, cambia el sentido de la animacion y no comprueba nada mas
    if((*tortuga).sprite.indiceAnimacion == 0){
        (*tortuga).isSumergiendo = true;
    }else {
        // Si está en el frame previo a hundirse...
        if((*tortuga).sprite.indiceAnimacion == preSumersionCoord){
            // Si la tortuga no se puede hundir, entonces simplemente cambia el sentido de  la animacion
            if(!(*tortuga).isSumergible){
                    (*tortuga).isSumergiendo = false;
            }else {
            // Si es una tortuga de las que pueden hundirse y está saliendo del agua, entonces activamos la colision para que sea tangible
                if(!(*tortuga).isSumergiendo){
                    (*tortuga).sprite.isActive = true;
                }
            }
        }else {
            //Si está en el frame justo despúes de hundirse y se está sumergiendo, desactivamos su tangibilidad
            if((*tortuga).isSumergiendo && (*tortuga).sprite.indiceAnimacion == postSumersionCoord){
                (*tortuga).sprite.isActive = false;
            }else{
                //Por último, si está en el último indice de animación. Cambia el sentido de la animacion para que vuelva a emerger
                if((*tortuga).sprite.indiceAnimacion == ultimaCoord){
                    (*tortuga).isSumergiendo = false;
                }
            }
        }
    }
}

// Actualiza el estado de un obstaculo (En funcion del tipo de obstaculo puede variar el comportamiento)
void ActualizarEstadoObstaculo(Tortuga *obstaculo){
    ActualizarMovimientoObstaculo(&(*obstaculo).sprite,(*obstaculo).direccion,(*obstaculo).velocidadMovimiento);
    
    //Animacion de la tortuga
    if(HacerCadaX(&(*obstaculo).animSumergir.temporizador,(*obstaculo).animSumergir.velocidad) && (*obstaculo).sprite.isVisible){
        ComprobarSumersionTortuga(&(*obstaculo));
        //En funcion de si la tortuga se está sumergiendo o no, entonces avanza o retrocede el frame de animacion
        if((*obstaculo).isSumergiendo){
            AvanzarSpriteAnimado(&tortugaSpriteSheet,tortugaSpriteSheet_Coords,&(*obstaculo).sprite);
        }else{
            RetrocederSpriteAnimado(&tortugaSpriteSheet,tortugaSpriteSheet_Coords,&(*obstaculo).sprite);
        }
    }
    ActualizarSprite(&tortugaSpriteSheet,tortugaSpriteSheet_Coords,&(*obstaculo).sprite);

}

void ActualizarEstadoObstaculo(Troncodrilo *obstaculo){
    ActualizarMovimientoObstaculo(&(*obstaculo).sprite,(*obstaculo).direccion,(*obstaculo).velocidadMovimiento);
    ActualizarSprite(&troncoSpriteSheet,troncoSpriteSheet_Coords,&(*obstaculo).sprite);
}
void ActualizarEstadoObstaculo(Vehiculo *obstaculo){
    ActualizarMovimientoObstaculo(&(*obstaculo).sprite,(*obstaculo).direccion,(*obstaculo).velocidadMovimiento);
    ActualizarSprite(&vehiculosSpriteSheet,vehiculosSpriteSheet_Coords,&(*obstaculo).sprite);
}

//Recorre todos los obstaculos de la fila de un rio y actualiza sus estados (En funcion del tipo de obstaculo puede variar el comportamiento)
void ActualizarEstadoFilaRio(Tortuga array[]){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        ActualizarEstadoObstaculo(&array[i]);
    }
}
void ActualizarEstadoFilaRio(Troncodrilo array[]){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        ActualizarEstadoObstaculo(&array[i]);
    }
}

void ActualizarEstadoRio(){
    for(int i = 0 ; i < ((int) FILA_RIO_T); i++){
        switch ((FilaRio) i){
            case FILA_RIO_0:
                ActualizarEstadoFilaRio(tortugas_1);
            break;
            case FILA_RIO_3:
                ActualizarEstadoFilaRio(tortugas_2);
            break;
            case FILA_RIO_1:
                ActualizarEstadoFilaRio(troncos_1);
            break;
            case FILA_RIO_2:
                ActualizarEstadoFilaRio(troncos_2);
            break;
            case FILA_RIO_4:
                ActualizarEstadoFilaRio(troncos_3);
            break;

        }
    }
}

void ActualizarEstadoVehiculos(){
    for(int i = 0; i < maxCamiones; i++){
        if(i < maxCochesAmarillos){
            ActualizarEstadoObstaculo(&cochesAmarillos[i]);
        }

        if(i < maxTractores){
            ActualizarEstadoObstaculo(&tractores[i]);
        }

        if(i < maxCochesRosas){
            ActualizarEstadoObstaculo(&cochesRosas[i]);
        }
        
        if(i < maxCochesBlancos){
            ActualizarEstadoObstaculo(&cochesBlancos[i]);
        }

        // Actualiza el estado de los camiones
        if(i%2 != 0 && camiones[i].sprite.collider.P2.x < 0){
            camiones[i-1].sprite.collider.P1.x = VENTANA_X;
            camiones[i-1].sprite.collider.P2.x = VENTANA_X+vehiculosSpriteSheet.spriteWidth;
            camiones[i].sprite.collider.P1.x = camiones[i-1].sprite.collider.P2.x;
            camiones[i].sprite.collider.P2.x = camiones[i-1].sprite.collider.P2.x+vehiculosSpriteSheet.spriteWidth;
        }else{
            MoveCollider(&camiones[i].sprite.collider,camiones[i].direccion,camiones[i].velocidadMovimiento);
        }
        ActualizarSprite(&vehiculosSpriteSheet,vehiculosSpriteSheet_Coords,&camiones[i].sprite);
    }
}

//Actualización del estado de la animacion de muerte del jugador y sus consecuencias
void ActualizarMuerteRanaJugador(Jugador *jugador){
    if(HacerCadaX(&(*jugador).animMuerte.temporizador, (*jugador).animMuerte.velocidad)){
        //Fin animacion muerte
        if((*jugador).ranaJugador.sprite.indiceAnimacion >= animMuerteSpriteSheet.totalCoordsAnim-2){
            (*jugador).vidas--;
            if((*jugador).vidas <= 0){
                if(credits>0){
                    pantallaActual = START;
                }else{
                    animInsert.temporizador = last_time;
                    pantallaActual = INSERT;
                }
            }else{
                SpawnJugador(&(*jugador));
            }
        }else{
            AvanzarSpriteAnimado(&animMuerteSpriteSheet, animMuerteSpriteSheet_Coords, &(*jugador).ranaJugador.sprite);
            ActualizarSprite(&animMuerteSpriteSheet, animMuerteSpriteSheet_Coords, &(*jugador).ranaJugador.sprite);
        }
    }
}

void ActualizarSaltoRana(Rana *rana){
    // Durante la duración del salto, irá avanzando su velocidad en cada iteración distanciaSalto/duracionSalto
    if(HacerDuranteX(&(*rana).animSalto.temporizador,(*rana).animSalto.duracion) && !ComprobarPosicionFinal((*rana).sprite.collider, (*rana).finSalto)){
        MoveCollider(&(*rana).sprite.collider, (*rana).direccion, (*rana).animSalto.velocidad);
    }else{
        (*rana).isJumping = false;
        (*rana).sprite.collider = (*rana).finSalto;
        (*rana).sprite.indiceAnimacion = 0;
        ActualizarSprite(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
    }
}

void ActualizarEstadoJugador(){
    ComprobarColisionesJugador(&jugadores[jugadorActual]);

    if(!jugadores[jugadorActual].ranaJugador.sprite.isActive){
        //Si la rana del jugador está muerta...
        ActualizarMuerteRanaJugador(&jugadores[jugadorActual]);
    }else{
        //En caso de que no...
        //Si la rana está saltando actualizará el estado del salto
        if(jugadores[jugadorActual].ranaJugador.isJumping){
            ActualizarSaltoRana(&jugadores[jugadorActual].ranaJugador);
        }
    }
}

float CalcularTiempoPorAvance(float duracion, float distanciaTotal, float velocidad){
    // printf("duracion distanciaTotal velocidad | %.2f %.2f %.2f\n", duracion, distanciaTotal, velocidad);
    // printf("Movimientos necesarios: %.2f\n", distanciaTotal/velocidad);
    // printf("Movimiento cada %d ms\n", (int)(duracion / (distanciaTotal/velocidad)));
    //Se restan 10 centesimas para dar margen a la animación y funcione correctamente
    return (duracion / (distanciaTotal/velocidad))-10;
}
void AvanceHorizontalIntro(int contador){
    // tiempoCorregido se usa como contador en esta funcion para poder determinar mas facilmente la duración de cada animacion de cada letra.
    int tiempoCorregido = 0;
    for(int i = 0; i < letraActual; i++){
        //Calcula todo el tiempo ocupado por las letras anteriores actual y lo almacena
        tiempoCorregido += letrasTitulo[i].animSalto.duracion;
    }

    //Luego le resta este tiempo calculado al contador y lo reasigna
    tiempoCorregido = contador-(tiempoCorregido/1000);
    printf("tiempoCorregido %d\n",tiempoCorregido);
    
    if (tiempoCorregido <= letrasTitulo[letraActual].animSalto.duracion / 1000 && HacerCadaX(&letrasTitulo[letraActual].animSalto.temporizador, CalcularTiempoPorAvance(letrasTitulo[letraActual].animSalto.duracion, letrasTitulo[letraActual].distanciaSalto, letrasTitulo[letraActual].animSalto.velocidad))){
        MoveCollider(&letrasTitulo[letraActual].sprite.collider,letrasTitulo[letraActual].direccion, letrasTitulo[letraActual].animSalto.velocidad);
        if(tiempoCorregido >= letrasTitulo[letraActual].animSalto.duracion/1000){
            RellocateSprite(&letrasTitulo[letraActual].sprite, letrasTitulo[letraActual].finSalto.P1);
            letrasTitulo[++letraActual].animSalto.temporizador=last_time;
        }
    }

}

void ActualizarEstadoIntro(){
    int contador = GetContadorFromTemp(animIntro.temporizador,1000);

    if(contador <= 30){
        AvanceHorizontalIntro(contador);
        
    }else{
        if(contador <= 34){

        }else{

        }
    }
}


void ActualizarEstadoJuego(){
    ActualizarEstadoRio();
    ActualizarEstadoVehiculos();
    ActualizarEstadoJugador();
}

void ActualizarEstados(){
    switch(pantallaActual){
        case INTRO:
            ActualizarEstadoIntro();
            break;
        case JUEGO:
            ActualizarEstadoJuego();
            break;
    }
}


//*** FUNCIONES DE DIBUJADO DE ELEMENTOS EN PANTALLA ***///
void DrawCollider(Collider collider){
    float coords[10] = {
        collider.P1.x,
        collider.P1.y,

        collider.P1.x,
        collider.P2.y,
        
        collider.P2.x,
        collider.P2.y,
        
        collider.P2.x,
        collider.P1.y,

        collider.P1.x,
        collider.P1.y,
    };
    esat::DrawPath(coords,5);
}

//** Dibujados de texto y sobrecargas en función de los parametros  **/

// Muestra el texto indicado por pantalla con las características proporcionadas.
//  -texto -> Texto a mostrar
//  -ubicacion -> Indica un punto de coordenadas específico desde el que empezar a dibujar
//  -color  -> Indica de que color se mostrará el texto con el struct Color en formato RGBa. Por defecto 200,200,200,255
void DrawText(char texto[], PuntoCoord ubicacion, Color color = {200,200,200,255}){
    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawText(ubicacion.x, ubicacion.y,texto);
}

// Muestra el texto indicado por pantalla con las características proporcionadas.
//  -texto -> Texto a mostrar
//  -longitud -> Longitud del texto para calcular correctamente las alineaciones
//  -ubi_y -> Indica una ubicacion específica en coordenada Y
//  -alineacionH -> Alineacion en vertical
//  -margen_X -> Desplazamiento en horizontal adicional a partir de alineacionH (indicar positivo o negativo)
//  -color  -> Indica de que color se mostrará el texto con el struct Color en formato RGBa. Por defecto 200,200,200,255
void DrawText(char texto[], int longitud, float ubi_y, Align_H alineacion, float margen_x, Color color = {200,200,200,255}){
    PuntoCoord ubicacion = {0.0,ubi_y};
    switch(alineacion){
        case LEFT:
            ubicacion.x = VENTANA_X-(FONT_SIZE*longitud);
            break;
        case CENT:
            ubicacion.x = VENTANA_X/2-(FONT_SIZE*(longitud/2-(longitud%2 == 0 ? 1 : 0)));
            break;
        case RIGHT:
            ubicacion.x = 0;
            break;
    }
    ubicacion.x += margen_x;

    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawText(ubicacion.x, ubicacion.y,texto);
}
// Muestra el texto indicado por pantalla con las características proporcionadas.
//  -texto -> Texto a mostrar
//  -longitud -> Longitud del texto para calcular correctamente las alineaciones
//  -ubi_x -> Indica una ubicacion específica en coordenada X
//  -alineacionV -> Alineacion en vertical
//  -margen_y -> Desplazamiento en vertical adicional a partir de alineacionV (indicar positivo o negativo)
//  -color  -> Indica de que color se mostrará el texto con el struct Color en formato RGBa. Por defecto 200,200,200,255
void DrawText(char texto[], int longitud, float ubi_x, Align_V alineacion, float margen_y, Color color = {200,200,200,255}){
    PuntoCoord ubicacion = {ubi_x,0.0};
    switch(alineacion){
        case TOP:
            ubicacion.y = FONT_SIZE;
            break;
        case MID:
            ubicacion.y = VENTANA_Y/2-(FONT_SIZE*((longitud/2)+1));
            break;
        case BOT:
            ubicacion.y = VENTANA_Y;
            break;
    }

    ubicacion.y += margen_y;

    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawText(ubicacion.x, ubicacion.y,texto);
}

// Muestra el texto indicado por pantalla con las características proporcionadas.
//  -texto -> Texto a mostrar
//  -longitud -> Longitud del texto para calcular correctamente las alineaciones
//  -alineacionV -> Alineacion en vertical
//  -alineacionH -> Alineacion en horizontal
//  -margen_y -> Desplazamiento en vertical adicional a partir de alineacionV (indicar positivo o negativo)
//  -margen_x -> Desplazamiento en horizontal adicional a partir de alineacionH (indicar positivo o negativo)
//  -color  -> Indica de que color se mostrará el texto con el struct Color en formato RGBa. Por defecto 200,200,200,255
void DrawText(char texto[], int longitud, Align_V alineacionV, Align_H alineacionH, float margen_y, float margen_x, Color color = {200,200,200,255}){
    PuntoCoord ubicacion = {0.0,0.0};
    switch(alineacionH){
        case LEFT:
            ubicacion.x = 0;
            break;
        case CENT:
            //Le restara 1 a la longitud a desplazarse si es par para corregir el centro segun el juego original (Operación ternaria)
            ubicacion.x = VENTANA_X/2-(FONT_SIZE*(longitud/2-(longitud%2 == 0 ? 1 : 0)));
            break;
        case RIGHT:
            ubicacion.x = VENTANA_X-(FONT_SIZE*longitud);
            break;
    }
    switch(alineacionV){
        case TOP:
            ubicacion.y = FONT_SIZE;
            break;
        case MID:
            ubicacion.y = VENTANA_Y/2;
            break;
        case BOT:
            ubicacion.y = VENTANA_Y;
            break;
    }

    ubicacion.x += margen_x;
    ubicacion.y += margen_y;

    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawText(ubicacion.x, ubicacion.y,texto);
}

void DrawRect(Collider collider, Color color){
    float coords[10] = {
        collider.P1.x,
        collider.P1.y,

        collider.P1.x,
        collider.P2.y,
        
        collider.P2.x,
        collider.P2.y,
        
        collider.P2.x,
        collider.P1.y,

        collider.P1.x,
        collider.P1.y,
    };
    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawSetStrokeColor(0,0,0,0);
    esat::DrawSolidPath(coords,5);
}

void DrawSprite(Sprite sprite, bool isPlayer = false){
    if(areCollidersVisible){
        if(sprite.isActive){
            esat::DrawSetStrokeColor(0,255,0);
        }else{
            esat::DrawSetStrokeColor(255,0,0);
        }

        if(isPlayer){
            DrawCollider({
                    {sprite.collider.P1.x+6, sprite.collider.P1.y+6},
                    {sprite.collider.P2.x-6, sprite.collider.P2.y-6}
                }
            );
        }else{
            DrawCollider(sprite.collider);
        }
    }

    if(sprite.isVisible){
        esat::DrawSprite(sprite.imagen, sprite.collider.P1.x, sprite.collider.P1.y);
    }
}

void DibujarFondoRio(){
    DrawRect({{0,0},{VENTANA_X,(VENTANA_Y)/2}},{0,0,70});
}

//-- DIBUJADO PANTALLA DE INTRO --//
void DibujarTitulo(){
    for(int i = 0; i < maxLetrasTitulo; i++){
        DrawSprite(letrasTitulo[i].sprite);
        DrawCollider(letrasTitulo[i].finSalto);
    }
}

void DibujarCopyrightKonami(){
    esat::DrawSetTextSize(FONT_SIZE);
    DrawText("KONAMI",0,MID,LEFT,FONT_SIZE*13,FONT_SIZE*6);
    esat::DrawSprite(copySprite,VENTANA_X/2,(VENTANA_Y/2)+(FONT_SIZE*12));
    DrawText("1981",4,MID,RIGHT,FONT_SIZE*13,FONT_SIZE*-7);
    esat::DrawSetTextSize(FONT_SIZE);
}


void DibujarIntro(){
    if(HacerDuranteX(&animIntro.temporizador,animIntro.duracion)){
        // La los movimientos de la animacion se ejecutan en la actualización de estados.
        // solo si está en la pantalla de INTRO
        DibujarTitulo();
    }else{
        InicializarTitulo();
        animPuntuaciones.temporizador = last_time;
        pantallaActual = PUNTUACIONES;
    }
}

//-- DIBUJADO PANTALLA DE PUNTUACIONES --//
void DibujarPuntuaciones(){
    // int limiteAparicion sirve para calcular cada cuanto tiempo deberá de aparecer cada texto comparandolo con el resultado de GetContadorFromTemp
    // Por cada limiteAparicion += 2 en este caso, tarda 2 segundos mas en aparecer que el if anterior;
    int limiteAparicion = 1, contador;
    if(HacerDuranteX(&animPuntuaciones.temporizador,animPuntuaciones.duracion)){
        DibujarTitulo();
        DrawText("-POINT TABLE-",13,MID,CENT,FONT_SIZE*-5,0);

        contador = GetContadorFromTemp(animPuntuaciones.temporizador);

        if(contador >= (int)(animPuntuaciones.velocidad)*limiteAparicion){
            //Los que están alineados a la izquierda, no importa su longitud de texto
            DrawText("10 PTS FOR EACH STEP",0,MID,LEFT,FONT_SIZE*-2,FONT_SIZE*2,{200,200,0});
        }
        limiteAparicion+=2;
        if(contador >= (int)(animPuntuaciones.velocidad)*limiteAparicion){
            DrawText("50 PTS FOR EVERY FROG",0,MID,LEFT,FONT_SIZE,FONT_SIZE*2,{200,200,0});
            DrawText("ARRIVED HOME SAFELY",0,MID,LEFT,FONT_SIZE*2,FONT_SIZE*2,{200,0,0});
        }
        limiteAparicion+=2;
        if(contador >= (int)(animPuntuaciones.velocidad)*limiteAparicion){
            DrawText("1000 PTS BY SAVING FROGS",0,MID,LEFT,FONT_SIZE*4,FONT_SIZE*2,{200,200,0});
            DrawText("INTO FIVE HOMES",0,MID,LEFT,FONT_SIZE*5,FONT_SIZE*2,{200,0,0});
        }
        limiteAparicion+=2;
        if(contador >= (int)(animPuntuaciones.velocidad)*limiteAparicion){
            DrawText("PLUS BONUS",0,MID,LEFT,FONT_SIZE*7,FONT_SIZE*2,{200,200,0});
            DrawText("10 PTS X REMAINING SECOND",0,MID,LEFT,FONT_SIZE*8,FONT_SIZE*2,{200,0,0});
        }

        DibujarCopyrightKonami();
    }else{
        InicializarTitulo();
        animRanking.temporizador = last_time;
        pantallaActual = RANKING;
    }

}

//-- DIBUJADO PANTALLA DE RANKING --//
void DibujarRanking(){
    char scoreDigits[maxScoreDigits];
    if(HacerDuranteX(&animRanking.temporizador,animRanking.duracion)){
        DibujarTitulo();

        DrawText("SCORE RANKING",13,MID,CENT,FONT_SIZE*-3,0,{200,200,0});

        for(int i = 0, k = 0; i < maxRankingScores; i++, k++){
            switch (i){
            case 0:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("1 ST",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("1 ST",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            case 1:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("2 ND",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("2 ND",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            case 2:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("3 RD",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("3 RD",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            case 3:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("4 TH",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("4 TH",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            case 4:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("5 TH",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("5 TH",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            }
            itoa(rankingScores[i]+100000,scoreDigits,10);
            if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                DrawText(scoreDigits+1,maxScoreDigits,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*2,{220,70,220});
                DrawText("PTS",3,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*7,{220,70,220});
            }else{
                DrawText(scoreDigits+1,maxScoreDigits,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*2);
                DrawText("PTS",3,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*7);
            }
            
        }

        DibujarCopyrightKonami();
    }else{
        animInsert.temporizador = last_time;
        pantallaActual = INSERT;
    }
}

//-- DIBUJADO PANTALLA DE INSERT COIN --//
void DibujarInsert(){
    if(HacerDuranteX(&animInsert.temporizador,animInsert.duracion)){
        DrawText("INSERT COIN",12,MID,CENT,FONT_SIZE*-2,0,{0,200,0});
        DrawText("3 FROGS  PER PLAYER",19,MID,CENT,FONT_SIZE*6,0,{200,200,0});
    }else{
        InicializarTitulo(true);
        animIntro.temporizador = last_time;
        pantallaActual = INTRO;
    }
}

//-- DIBUJADO PANTALLA DE START --//
void DibujarStart(){
    DrawText("PUSH",4,MID,CENT,FONT_SIZE*-7,0);
    DrawText("START BUTTON",12,MID,CENT,FONT_SIZE*-2,0,{220,70,220});
    DrawText("ONE PLAYER ONLY",15,MID,CENT,FONT_SIZE*2,0);
    DrawText("ONE EXTRA FROG 20000 PTS",24,MID,CENT,FONT_SIZE*5,0,{200,0,0});
}


//-- DIBUJADO PANTALLA DE JUEGO --//
void DibujarArbustos(){
    int posInicial_X = 0;
    int posActual_X;
    int posFila1_Y = VENTANA_Y-(esat::SpriteHeight(arbustoSprite)*2);
    int posFila2_Y = VENTANA_Y-(esat::SpriteHeight(arbustoSprite)*8);
    for(int i = 0; i < 2; i++){
        posActual_X = posInicial_X;
        for(int arbusto = 0; arbusto < tamanyoFilaArbustos; arbusto++){
            if(i == 0){
                esat::DrawSprite(arbustoSprite, posActual_X, posFila1_Y);
            }else{
                esat::DrawSprite(arbustoSprite, posActual_X, posFila2_Y);
            }
            posActual_X += esat::SpriteWidth(arbustoSprite);
        }
    }
}

// Dibujar la fila del rio pasada por parametro
void DibujarFilaRio(Tortuga array[]){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        DrawSprite(array[i].sprite);
    }
}
// Dibujar la fila del rio pasada por parametro
void DibujarFilaRio(Troncodrilo array[]){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        DrawSprite(array[i].sprite);
    }
}
// Dibujar las zonas finales donde deben acabar las ranas del jugador para puntuar
void DibujarZonasFinales(){
    for(int i = 0; i < maxZonasFinales; i++){
        DrawSprite(zonasFinales[i]);
    }
}

void DibujarRanasFinales(){
    for(int i = 0; i < maxRanasFinales; i++){
        DrawSprite(ranasFinales[i]);
    }
}

void DibujarRio(){
    for(int i = 0 ; i < ((int) FILA_RIO_T); i++){
        switch ((FilaRio) i){
            case FILA_RIO_0:
                DibujarFilaRio(tortugas_1);
            break;
            case FILA_RIO_3:
                DibujarFilaRio(tortugas_2);
            break;
            case FILA_RIO_1:
                DibujarFilaRio(troncos_1);
            break;
            case FILA_RIO_2:
                DibujarFilaRio(troncos_2);
            break;
            case FILA_RIO_4:
                DibujarFilaRio(troncos_3);
            break;
            case FILA_RIO_5:
                DibujarZonasFinales();
                DibujarRanasFinales();
            break;
        }
    }
}

void DibujarVehiculos(){
    for(int i = 0; i < maxCamiones; i++){
        if(i < maxCochesAmarillos){
            DrawSprite(cochesAmarillos[i].sprite);
        }

        if(i < maxTractores){
            DrawSprite(tractores[i].sprite);
        }

        if(i < maxCochesRosas){
            DrawSprite(cochesRosas[i].sprite);
        }
        
        if(i < maxCochesBlancos){
            DrawSprite(cochesBlancos[i].sprite);
        }

        DrawSprite(camiones[i].sprite);
    }
}

void DibujarJugadores(){
    DrawSprite(jugadores[jugadorActual].ranaJugador.sprite, true);
}

void DibujarJuego(){
    DibujarArbustos();
    DibujarRio();
    DibujarVehiculos();
    DibujarJugadores();
}

//Dibuja la barra de tiempo del cronometro en la parte inferior derecha de la pantalla
void DibujarBarraCronometro(){
    int c;
        if(HacerDuranteX(&cronometro.temporizador, cronometro.duracion)){
            c = (int)(cronometro.duracion-GetContadorFromTemp(cronometro.temporizador));

            // A partir de los 5 segundos, se empiza a pintar la barra en rojo
            if(c/1000 <= 5){
                DrawRect({
                    {(float)((VENTANA_X-(FONT_SIZE*4))-(((c/1000)+1)*12)),VENTANA_Y-FONT_SIZE},
                    {VENTANA_X-(FONT_SIZE*4),VENTANA_Y-2}
                },
                {200,0,0}
            );
            }else{
                DrawRect({
                    // El primer punto de la barra se calcula restandole lo siguiente a VENTANA_X: 
                    //  Por un lado:
                    //  Lo que ocupa el texto "TIME"
                    //
                    //  Por otro lado:
                    //  El valor del contador invertido "c"/1000 para que solo se actualice cada segundo al ser un entero, 
                    //  +1 (para evitar la barra vacia con el juego activo) 
                    //  *12 para que la barra ocupe una cantidad razonable máxima de 372px
                    {(float)((VENTANA_X-(FONT_SIZE*4))-(((c/1000)+1)*12)),VENTANA_Y-FONT_SIZE},
                    // El segundo punto es el tamaño de la ventana menos lo que ocupa el texto "TIME" en pantalla
                    {VENTANA_X-(FONT_SIZE*4),VENTANA_Y-2}
                },
                {0,200,0}
            );
            }
        }else{
            MatarJugador(&jugadores[jugadorActual]);
        }
}

// Se encarga de dibujar la cabecera de la UI por completo
void DibujarCabecera(){
    char scoreDigits[maxScoreDigits];

    //Puntuacion del jugador
    if(jugadorActual == 0){
        DrawText("1-UP",4+16,TOP,CENT,0,0);
    }else{
        DrawText("2-UP",4+16,TOP,CENT,0,0);
    }
    itoa(jugadores[jugadorActual].puntuacion+100000,scoreDigits,10);
    DrawText(scoreDigits+1,maxScoreDigits+16,TOP,CENT,FONT_SIZE,0,{200,0,0});

    //Puntuacion mas alta durante esta ejecución
    DrawText("HI-SCORE",8,TOP,CENT,0,0);
    itoa(hiScore+100000,scoreDigits,10);
    DrawText(scoreDigits+1,maxScoreDigits,TOP,CENT,FONT_SIZE,0,{200,0,0});
}

//Dibuja la cantidad de vidas del jugador actual
void DibujarVidas(){
    for(int i = 1; i < jugadores[jugadorActual].vidas; i++){
        esat::DrawSprite(vidaSprite,i*esat::SpriteWidth(vidaSprite),VENTANA_Y-SPRITE_SIZE);
    }
}

//Dibuja la cantidad de niveles superados + el actual
void DibujarNiveles(){
    for(int i = 1; i <= jugadores[jugadorActual].nivelActual; i++){
        esat::DrawSprite(indicadorNivelSprite,VENTANA_X-(i*esat::SpriteWidth(indicadorNivelSprite)),VENTANA_Y-SPRITE_SIZE);
    }
}

//Dibuja el texto y la barra de tiempo en la parte inferior derecha de la pantalla
void DibujarTiempo(){
    DibujarBarraCronometro();
    if(((int)(cronometro.duracion-GetContadorFromTemp(cronometro.temporizador))/1000 <= 5)){
        DrawText("TIME",4,BOT,RIGHT,-2,0);
    }else{
        DrawText("TIME",4,BOT,RIGHT,-2,0,{255,255,0});
    }
}

// Se encarga de dibujar el pie de la UI por completo
void DibujarPie(){
    switch (pantallaActual){
        case JUEGO:
            DibujarVidas();
            DibujarNiveles();
            DibujarTiempo();
        break;
        default:
            char creditDigits[maxCreditDigits];
            //Dibuja los creditos
            itoa(credits+100,creditDigits,10);
            DrawText(creditDigits+1,maxCreditDigits,BOT,RIGHT,-2,0,{0,200,255});

            DrawText("CREDIT",6,BOT,RIGHT,-2,FONT_SIZE*-3,{0,200,255});
        break;
    }
}

// Es la única función que se debe llamar en el main. 
// Se ocupa del dibujado de todos los elementos en pantalla mediante subrutinas
void DibujarEntorno(){
    // Se dibuja primero el fondo del rio para que siempre aparezca por debajo de todos los
    // demás dibujados
    DibujarFondoRio();

    // Dibuja todo lo que aparezca en la zona central de la ventana
    // En función de la pantalla seleccionada
    switch(pantallaActual){
        case INTRO:
            DibujarIntro();
            break;
        case PUNTUACIONES:
            DibujarPuntuaciones();
            break;
        case RANKING:
            DibujarRanking();
            break;
        case INSERT:
            DibujarInsert();
            break;
        case START:
            DibujarStart();
            break;
        case JUEGO:
            DibujarJuego();
            break;
    }

    // Se dibuja lo ultimo para asegurarnos de que se muestra por encima 
    // de todo como la UI que es
    //TO_DO
    DibujarCabecera();
    DibujarPie();
} 

//*** FUNCIONES DE LIBERADO DE MEMORIA AL TERMINAL EL PROCESO ***///

void LiberarSpriteSheets(){
    esat::SpriteRelease(animMuerteSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaBaseSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaRosaSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaRojaSpriteSheet.spriteSheet);
    esat::SpriteRelease(vehiculosSpriteSheet.spriteSheet);
    esat::SpriteRelease(tortugaSpriteSheet.spriteSheet);
    esat::SpriteRelease(troncoSpriteSheet.spriteSheet);
    esat::SpriteRelease(zonaFinalSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaFinSpriteSheet.spriteSheet);
    esat::SpriteRelease(tituloSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaIntroSpriteSheet.spriteSheet);
}

void LiberarSpritesBase(){
    esat::SpriteRelease(arbustoSprite);
    esat::SpriteRelease(indicadorNivelSprite);
    esat::SpriteRelease(vidaSprite);
    esat::SpriteRelease(copySprite);
}

void LiberarSpritesLetrasTitulo(){
    //Libera los sprites del muro final
    for(int i = 0; i < maxLetrasTitulo; i++){
        esat::SpriteRelease(letrasTitulo[i].sprite.imagen);
    }
}

void LiberarSpritesZonaFinal(){
    //Libera los sprites del muro final
    for(int i = 0; i < maxZonasFinales; i++){
        esat::SpriteRelease(zonasFinales[i].imagen);
    }

    //Libera los sprites de las ranas finales
    for(int i = 0; i < maxRanasFinales; i++){
        esat::SpriteRelease(ranasFinales[i].imagen);
    }

    //TO_DO
    //Libera los sprites de las moscas bonus

    //TO_DO
    //Libera los sprites de cocodrilo trampa
}

void LiberarSpritesRio(){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        if(tortugas_1[i].sprite.imagen != NULL){
            esat::SpriteRelease(tortugas_1[i].sprite.imagen);
        }
        if(tortugas_2[i].sprite.imagen != NULL){
            esat::SpriteRelease(tortugas_2[i].sprite.imagen);
        }
        if(troncos_1[i].sprite.imagen != NULL){
            esat::SpriteRelease(troncos_1[i].sprite.imagen);
        }
        if(troncos_2[i].sprite.imagen != NULL){
            esat::SpriteRelease(troncos_2[i].sprite.imagen);
        }
        if(troncos_3[i].sprite.imagen != NULL){
            esat::SpriteRelease(troncos_3[i].sprite.imagen);
        }
    }
}

void LiberarSpritesVehiculos(){
    for(int i = 0; i < maxCochesAmarillos; i++){
        if(cochesAmarillos[i].sprite.imagen != NULL){
            esat::SpriteRelease(cochesAmarillos[i].sprite.imagen);
        }
    }
    for(int i = 0; i < maxTractores; i++){
        if(tractores[i].sprite.imagen != NULL){
            esat::SpriteRelease(tractores[i].sprite.imagen);
        }
    }
    for(int i = 0; i < maxCochesRosas; i++){
        if(cochesRosas[i].sprite.imagen != NULL){
            esat::SpriteRelease(cochesRosas[i].sprite.imagen);
        }
    }
    for(int i = 0; i < maxCochesBlancos; i++){
        if(cochesBlancos[i].sprite.imagen != NULL){
            esat::SpriteRelease(cochesBlancos[i].sprite.imagen);
        }
    }
    for(int i = 0; i < maxCamiones; i++){
        if(camiones[i].sprite.imagen != NULL){
            esat::SpriteRelease(camiones[i].sprite.imagen);
        }
    }
}

void LiberarSpritesJugadores(){
    for(int i = 0; i < maxJugadores; i++){
        if(jugadores[i].ranaJugador.sprite.imagen != NULL){
            esat::SpriteRelease(jugadores[i].ranaJugador.sprite.imagen);
        }
    }
}

//Libera de memoria todos los spriteHandle inicializados
void LiberarSprites(){
    LiberarSpritesZonaFinal();
    LiberarSpritesRio();
    LiberarSpritesVehiculos();
    LiberarSpritesJugadores();
    LiberarSpritesBase();
    LiberarSpriteSheets();
}
/* FIN FUNCIONALIDADES*/

/* INICIO MAIN */
int esat::main(int argc, char **argv) {
    GenerarSemillaAleatoria();

    esat::WindowInit(VENTANA_X,VENTANA_Y);
    WindowSetMouseVisibility(true);

    esat::DrawSetTextFont("./Recursos/Fuentes/arcade-legacy.ttf");
    esat::DrawSetTextSize(FONT_SIZE);

    // Inicialización de todos aquellos recursos que necesitan ser precargados para
    // que otros los puedan usar
    InicializarSpriteSheets();
    InicializarSprites();
    InicializarTitulo(true);

    while(esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {
        last_time = esat::Time(); 

        //Inicio dibujado de pantalla
        esat::DrawBegin();
        esat::DrawClear(0,0,0);

        //INPUT
        DetectarControles();
        //UPDATE
        ActualizarEstados();
        //DRAW
        DibujarEntorno();

        esat::DrawEnd();      
        esat::WindowFrame();
        //Fin dibujado de pantalla
        ControlFPS();
    }

    LiberarSprites();
    esat::WindowDestroy();
    return 0;  
}
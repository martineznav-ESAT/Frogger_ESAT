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
    INICIO,
    MENU,
    JUEGO
};

enum Direccion {
    ARRIBA,
    DERECHA,
    ABAJO,
    IZQUIERDA
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
enum TipoObjeto {
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
    int puntuacion = 0, vidas = 0;
    Animacion animMuerte;
};

struct Vehiculo{
    TipoObjeto tipoVehiculo;
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
const int VENTANA_COLUMNAS = VENTANA_X/SPRITE_SIZE, VENTANA_FILAS = VENTANA_Y/SPRITE_SIZE;

//-- FPS
const unsigned char FPS=60;
double current_time,last_time;

//-- Prototipado
bool areCollidersVisible = false;

//-- UI
const unsigned char FONT_SIZE = 24;

Pantalla pantallaActual = INICIO;
int nivelActual = 0;

//-- Fin UI


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

//-- Sprites | Declaración de los handles cuyos sprites: 
//  -Siempre serán iguales (Sin animación ni acceso multiple como los vehiculos)
//  -No necesitan collider
esat::SpriteHandle arbustoSprite;
esat::SpriteHandle indicadorNivelSprite;
esat::SpriteHandle vidaSprite;


//-- Jugadores
const unsigned char maxJugadores = 2;
unsigned char jugadoresActuales = 1;
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

// Actua como "trigger" donde:
// Si el tiempo actual de ejecución menos el temporizador pasado 
// por parametro es mayor a 'x' entonces permitirá acceso 
// (Todo en milesimas de segundo)
bool HacerCadaX(float *temp, float x){
    bool isAccesible = false;
    // printf("LOG--- Valores funcion HacerCadaX()\n");
    // printf("LOG--- T.Actual    Temp    Contador    Limite\n");
    // printf("LOG--- %14.10f | %14.10f | %14.10f |%14.10f\n",last_time,(*temp), last_time - (*temp),x);
    isAccesible = last_time - (*temp) > x;
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
}

void InicializarSprites(){
    arbustoSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/ArbustoSprite.png");
    indicadorNivelSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/IndicadorNivelSprite.png");
    vidaSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/VidaSprite.png");
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

bool CabeGrupoObstaculos(int posicionGrupo, int columna, int longitud){
    return posicionGrupo == 0 && columna + longitud < VENTANA_COLUMNAS || posicionGrupo != 0 && columna + (longitud - posicionGrupo) < VENTANA_COLUMNAS;
}

// Inicializar valores de las ranas que aparecen al llegar a una zona final donde:
// filaRio      -> Indica la fila del rio en la que se debe ubicar en Y
void InicializarRanasFinales(FilaRio filaRio){
    for(int i = 0; i < maxRanasFinales; i++){
        ranasFinales[i].collider.P1 = {((float)i*zonaFinalSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*9))-(((float)filaRio)*SPRITE_SIZE))};
        ranasFinales[i].tipoAnimacion = 0;
        ranasFinales[i].indiceAnimacion = 0;
        ranasFinales[i].isVisible = true;
        ranasFinales[i].isActive = true;

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
    Vehiculo *vehiculo, TipoObjeto tipo, Direccion direccion,
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
        VENTANA_X/2,(float)(VENTANA_Y-ranaBaseSpriteSheet.spriteHeight*8)
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

// Instancia los valores por defecto de cada jugador
// Pensado para utilizarse al inicio de cada partida
void InicializarJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        //Propiedades del jugador al inicio de la partida
        jugadores[i].ranaJugador.sprite.isActive = true;
        jugadores[i].puntuacion = false;
        jugadores[i].vidas = 3;

        jugadores[i].animMuerte.duracion = 2000;
        jugadores[i].animMuerte.temporizador = 0;
        jugadores[i].animMuerte.velocidad = jugadores[i].animMuerte.duracion/animMuerteSpriteSheet.indicesAnim;

        //Spawn Rana en posición inicial del jugador
        SpawnJugador(&jugadores[i]);
    }
}

void InicializarNivel(){
    // Inicialización de cosas interactivas durante la ejecución del nivel
    InicializarRio();
    InicializarVehiculos();
    InicializarJugadores();
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
        if(!jugadores[0].ranaJugador.isJumping && jugadores[0].ranaJugador.sprite.isActive){
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
        printf("INICIO\n");
        pantallaActual = INICIO;
    }
    if(esat::IsKeyDown('2')){
        printf("MENU\n");
        pantallaActual = MENU;
    }
    if(esat::IsKeyDown('3')){
        printf("JUEGO\n");
        pantallaActual = JUEGO;
        nivelActual = 0;
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

//Actualización del estado de la animacion de muerte del jugador 
void ActualizarMuerteRanaJugador(Jugador *jugador){
    if(HacerCadaX(&(*jugador).animMuerte.temporizador, (*jugador).animMuerte.velocidad)){
        if((*jugador).ranaJugador.sprite.indiceAnimacion >= animMuerteSpriteSheet.totalCoordsAnim-2){
            SpawnJugador(&(*jugador));
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

void ActualizarEstadoJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        ComprobarColisionesJugador(&jugadores[i]);

        if(!jugadores[i].ranaJugador.sprite.isActive){
            //Si la rana del jugador está muerta...
            ActualizarMuerteRanaJugador(&jugadores[i]);
        }else{
            //En caso de que no...

            //Si la rana está saltando actualizará el estado del salto
            if(jugadores[i].ranaJugador.isJumping){
                ActualizarSaltoRana(&jugadores[i].ranaJugador);
            }
        }
    }
}

void ActualizarEstadoJuego(){
    //TO_DO
    ActualizarEstadoRio();
    ActualizarEstadoVehiculos();
    ActualizarEstadoJugadores();
}

void ActualizarEstados(){
    switch(pantallaActual){
        case INICIO:
            break;
        case MENU:
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

void DrawRect(Collider collider, unsigned char r = 0, unsigned char g = 0, unsigned char b = 0){
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
    esat::DrawSetFillColor(r,g,b);
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
    float coords[10] = {0,0,0,VENTANA_Y/2,VENTANA_X,VENTANA_Y/2,VENTANA_X,0,0,0};
    esat::DrawSetFillColor(0,0,70);
    esat::DrawSolidPath(coords,5);
}

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
    for(int i = 0; i < jugadoresActuales; i++){
        DrawSprite(jugadores[i].ranaJugador.sprite, true);
    }
}

void DibujarJuego(){
    DibujarArbustos();
    DibujarRio();
    DibujarVehiculos();
    DibujarJugadores();
}

void DibujarPie(){
    switch (pantallaActual){
        case JUEGO:
            esat::DrawSetFillColor(255,255,0);
            esat::DrawText(VENTANA_X-(FONT_SIZE*4),VENTANA_Y-2,"TIME");
            esat::DrawSetFillColor(255,255,255);
        break;
        default:
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
        case INICIO:
            break;
        case MENU:
            break;
        case JUEGO:
            DibujarJuego();
            break;
    }

    // Se dibuja lo ultimo para asegurarnos de que se muestra por encima 
    // de todo como la UI que es
    //TO_DO
    // DibujarCabecera()
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
}

void LiberarSpritesBase(){
    esat::SpriteRelease(arbustoSprite);
    esat::SpriteRelease(indicadorNivelSprite);
    esat::SpriteRelease(vidaSprite);
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
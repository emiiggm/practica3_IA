#include "AgenteEstudiante.hpp"
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

AgenteEstudiante::AgenteEstudiante(int id, int profundidadMax, double tiempoMax, int numHeuristica, ModoJuego modo) 
    : id(id), profundidadMax(profundidadMax), tiempoMaxSegundos(tiempoMax), numHeuristica(numHeuristica), modo(modo), abortarBanda(false) {
    nodosVisitados = 0;
}

bool AgenteEstudiante::tieneLimiteDeTiempo() const {
    return modo != ModoJuego::STATUS;
}

std::pair<int, int> AgenteEstudiante::think(const Tablero& tablero) {
    std::pair<int, int> mejor;
    nodosVisitados = 0;
    abortarBanda = false;
    inicioBusqueda = std::chrono::steady_clock::now();

    switch (modo)
    {
    case ModoJuego::ALEATORIO:
        return JuegaAleatorio(tablero);
        break;
    
    case ModoJuego::STATUS:
        Status(tablero, mejor);
        return mejor;
        break;    

    case ModoJuego::MINIMAX:
        minimax(tablero, 0, profundidadMax, mejor);
        return mejor;
        break; 

    case ModoJuego::INTELIGENTE:
        return JuegaInteligente(tablero);   
        break;
    }
        
    return {-1, -1};
}


/**
 * @brief Compara dos tableros para identificar cuál ha sido el movimiento realizado.
 * @param padre Estado inicial del tablero.
 * @param hijo Estado resultante tras un movimiento.
 * @return Un par (fila, columna) con la posición de la nueva pieza.
 */
std::pair<int, int> SacarMovimiento(const Tablero& padre, const Tablero &hijo){
    for(int f=0; f<padre.getFilas(); ++f)
        for(int c=0; c<padre.getColumnas(); ++c)
            if (padre.getCelda(f,c) == 0 && hijo.getCelda(f,c) != 0) 
                return {f, c};
    return {-1, -1};
}

/**
 * @brief Implementa un agente que juega de forma totalmente aleatoria.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida al azar.
 */
std::pair<int, int> AgenteEstudiante::JuegaAleatorio(const Tablero& tablero) {

    // Calculo los tableros descendientes de tablero
    auto sucesores = tablero.getSucesores();

    // Si no tiene descendientes, paso el turno
    if (sucesores.empty()) return {-1, -1};

    // Elijo aleatoriamente uno de los descendientes
    int elegido = rand() % sucesores.size();

    // Saco el movimiento realizado comparando el tablero original con el elegido.
    std::pair<int,int> Mov = SacarMovimiento(tablero, sucesores[elegido]);

    return Mov;
}


/**
 * @brief Algoritmo de resolución completa para estados de final de juego.
 * Determina si una posición está matemáticamente ganada, perdida o empatada.
 * @param tablero Estado a evaluar.
 * @param Mov [Salida] La jugada óptima encontrada.
 * @return Resultado del análisis (VICTORIA, DERROTA o EMPATE).
 */
AgenteEstudiante::Resultado AgenteEstudiante::Status(const Tablero &tablero, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    /* ============== Empieza a partir de aquí tu implementación  =============== */
    int ganador = tablero.comprobarGanador();
    if (ganador == id) return Resultado::VICTORIA;
    if (ganador == -1) return Resultado::EMPATE;
    if (ganador != 0) return Resultado::DERROTA;

    auto sucesores = tablero.getSucesoresConMovimientos();

    if (sucesores.empty()){
        int desempate = tablero.getGanadorDesempate();
        if (desempate == id){
            return Resultado::VICTORIA;
        }else if (desempate == -1){
            return Resultado::EMPATE;
        }else{
            return Resultado::DERROTA;
        }
    }

    bool esMax = (tablero.getJugadorTurno() == id);
    Mov = sucesores[0].second;

    if (esMax){
        
        bool todosDerrota = true;
        for (int i = 0; i < sucesores.size(); i++){
            std::pair<int,int> movHijo = {-1,-1};
            Resultado res = Status(sucesores[i].first, movHijo);
            if (res == Resultado::VICTORIA){
                Mov = sucesores[i].second;
                return Resultado::VICTORIA;
            }
            if(res == Resultado::EMPATE && todosDerrota){
                Mov = sucesores[i].second;
                todosDerrota = false;
            }
        }

        if (todosDerrota) return Resultado::DERROTA;
        return Resultado::EMPATE;
    }else{
        
        bool todosVictoria = true;
        for (int i = 0; i < sucesores.size(); i++){
            std::pair<int,int> movHijo = {-1,-1};
            Resultado res = Status(sucesores[i].first, movHijo);
            if (res == Resultado::DERROTA){
                Mov = sucesores[i].second;
                return Resultado::DERROTA;
            }
            if(res == Resultado::EMPATE && todosVictoria){
                Mov = sucesores[i].second;
                todosVictoria = false;
            }
        }
        if (todosVictoria) return Resultado::VICTORIA;
        return Resultado::EMPATE;
    }
}



/**
 * @brief Implementación del algoritmo Minimax clásico.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado.
 */
double AgenteEstudiante::minimax(const Tablero &tablero, int profundidad, int prof_Max, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */
    int ganador = tablero.comprobarGanador();
    if (ganador == id) return GANAR;
    if (ganador == -1) return 0.0; //valor neutro entre GANAR y PERDER
    if (ganador != 0) return PERDER;

    if (profundidad >= prof_Max || !tablero.tieneMovimientosValidos()) return heuristica(tablero);

    auto sucesores = tablero.getSucesoresConMovimientos();
    if(sucesores.empty()) return heuristica(tablero);

    bool esMaximizador = tablero.getJugadorTurno() == id;

    
    std::pair<int,int> movHijo = {-1,-1};
    double AV = minimax(sucesores[0].first, profundidad+1, prof_Max, movHijo);
    Mov = sucesores[0].second; //para k=1: AV(J) = V(J1)

    //para k=2,...,b: AV(J) = max{AV(J),V(Jk)} si MAX, min{AV(J),V(Jk)} si MIN
    for (int k = 1; k < sucesores.size(); k++){
        if (abortarBanda) break;
        movHijo = {-1,-1};
        double VJk = minimax(sucesores[k].first, profundidad +1, prof_Max, movHijo);
        
        if (esMaximizador){
            if (VJk > AV) { 
                AV= VJk; 
                Mov = sucesores[k].second;
            }
        }else {
            if (VJk < AV){
                AV= VJk; 
                Mov = sucesores[k].second;
            }
        }  
    }

    return AV; //AV(J)=V(J)
}


/**
 * @brief Punto de entrada para el juego inteligente.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida por el algoritmo de búsqueda.
 */
std::pair<int, int> AgenteEstudiante::JuegaInteligente(const Tablero& tablero) {
    std::pair<int,int> Mov;

    double valor = alfaBeta(tablero, 0, profundidadMax, MenosInfinito, MasInfinito, Mov);
    std::cout << "Valor Minimax: " << valor << "\tJugada: (" << Mov.first << ", " << Mov.second << ")\n";
    return Mov;
}




/**
 * @brief Implementación del algoritmo Minimax con Poda Alfa-Beta.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param alfa Valor mínimo garantizado para el jugador MAX.
 * @param beta Valor máximo garantizado para el jugador MIN.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado tras la poda.
 */
double AgenteEstudiante::alfaBeta(const Tablero &tablero, int profundidad, int prof_Max, double alfa, double beta, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */
    int ganador = tablero.comprobarGanador();
    if (ganador == id) return GANAR;
    if (ganador == -1) return 0.0; //valor neutro entre GANAR y PERDER
    if (ganador != 0) return PERDER;

    if (profundidad >= prof_Max || !tablero.tieneMovimientosValidos()) return heuristica(tablero);

    auto sucesores = tablero.getSucesoresConMovimientos();
    if(sucesores.empty()) return heuristica(tablero);

    bool esMaximizador = tablero.getJugadorTurno() == id;
    Mov = sucesores[0].second;

    if (esMaximizador){
        //Nodo MAX:
        //alfa <-- max(alfa, V(Jk, alfa, beta))
        for (int k = 1; k < sucesores.size(); k++){
            if (abortarBanda) break;
            std::pair<int,int> movHijo = {-1,-1};
            double val = alfaBeta(sucesores[k].first, profundidad +1, prof_Max, alfa, beta, movHijo);
            if (val > alfa) {
                alfa = val;
                Mov = sucesores[k].second;
            }
            if (alfa >= beta){
                return beta;
            }
        }
        return alfa;
        
    }else {
        //Nodo MIN:
        //beta <-- min(beta, V(Jk, alfa, beta))
        for (int k = 1; k < sucesores.size(); k++){
            if (abortarBanda) break;
            std::pair<int,int> movHijo = {-1,-1};
            double val = alfaBeta(sucesores[k].first, profundidad +1, prof_Max, alfa, beta, movHijo);
            if (val < beta) {
                beta = val;
                Mov = sucesores[k].second;
            }
            if (alfa >= beta){
                return alfa;
            }
        }
        return beta;
    }  

    return 0;
}

/**
 * @brief Función heurística para evaluar la calidad de un tablero.
 * @param tablero Estado a evaluar.
 * @return Puntuación numérica (positiva para ventaja de J1, negativa para J2).
 */
double AgenteEstudiante::heuristica(const Tablero& tablero) {
    switch(numHeuristica) {
        case 0: return heuristicaPrueba(tablero);
                break;
        case 1: return heuristica1(tablero);
                break;
        case 2: return heuristica2(tablero);
                break;
        default: return heuristica1(tablero);
    }
}

double AgenteEstudiante::heuristicaPrueba(const Tablero& tablero) {
    // n es el número de fichas en línea para ganar.
    int n = tablero.getNParaGanar();
    int oponente = (id == 1) ? 2 : 1;
    double score_positivo = 0;

    double score_negativo = 0;

    for (int f=0; f< tablero.getFilas(); f++ ){
        for (int c = 0; c< tablero.getColumnas(); c++){
            if (tablero.getCelda(f,c) != 0 ){
                int valor = tablero.getFilas()-abs(f-(tablero.getFilas()/2)) + tablero.getColumnas()-abs(c-(tablero.getColumnas()/2)); 
                if (tablero.getCelda(f,c) == id){
                  score_positivo += valor;
                 }
                else {
                  score_negativo += valor;
                }
            }
        }
    }

   
    return score_positivo - score_negativo;
}


double AgenteEstudiante::heuristica1(const Tablero& tablero) {
    //A implementar por el estudiante
    int rival = (id == 1) ? 2 : 1;
    int n = tablero.getNParaGanar();
    int filas = tablero.getFilas();
    int cols  = tablero.getColumnas();

    double score = 0.0;

    int dr[] = {0, 1, 1,  1};
    int dc[] = {1, 0, 1, -1};

    for (int d = 0; d < 4; d++) {
        for (int f = 0; f < filas; f++) {
            for (int c = 0; c < cols; c++) {
                int ef = f + dr[d] * (n - 1);
                int ec = c + dc[d] * (n - 1);
                if (ef < 0 || ef >= filas || ec < 0 || ec >= cols) continue;

                int propias = 0, rivales = 0;
                for (int k = 0; k < n; k++) {
                    int celda = tablero.getCelda(f + dr[d]*k, c + dc[d]*k);
                    if (celda == id)         propias++;
                    else if (celda == rival) rivales++;
                }

                if (rivales == 0 && propias > 0) {
                    if (propias == 2)      score += 10.0;
                    else if (propias == 3) score += 100.0;
                    else if (propias == 4) score += 10000.0;
                    else if (propias >= 5) score += 1000000.0;
                }

                if (propias == 0 && rivales > 0) {
                    if (rivales == 2)      score -= 11.0;
                    else if (rivales == 3) score -= 110.0;
                    else if (rivales == 4) score -= 11000.0;
                    else if (rivales >= 5) score -= 1100000.0;
                }
            }
        }
    }

    int cf = filas / 2;
    int cc = cols / 2;
    for (int f = 0; f < filas; f++) {
        for (int c = 0; c < cols; c++) {
            int distF = abs(f - cf);
            int distC = abs(c - cc);
            int dist  = distF > distC ? distF : distC;
            int bonus = filas / 2 - dist;
            if (bonus < 0) bonus = 0;
            int celda = tablero.getCelda(f, c);
            if (celda == id)         score += bonus * 2;
            else if (celda == rival) score -= bonus * 2;
        }
    }

    return score;
}

double AgenteEstudiante::heuristica2(const Tablero& tablero) {
    //A implementar por el estudiante
    int rival = (id == 1) ? 2 : 1;
    int n = tablero.getNParaGanar();
    double score = 0.0;
    for (int len = 2; len <= n; len++) {
        double peso = std::pow(10.0, len - 1);
        score += tablero.contarCombinaciones(len, id)    * peso;
        score -= tablero.contarCombinaciones(len, rival) * peso * 1.1;
    }
    return score;
}


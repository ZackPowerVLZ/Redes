#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PUERTO 7777
#define MAX_CLIENTES 10

const int FILAS = 6;
const int COLUMNAS = 7;

char tablero[FILAS][COLUMNAS];

void inicializarTablero() {
    for (int i = 0; i < FILAS; ++i) {
        for (int j = 0; j < COLUMNAS; ++j) {
            tablero[i][j] = '.';
        }
    }
}

void imprimirTablero() {
    for (int i = 0; i < FILAS; ++i) {
        for (int j = 0; j < COLUMNAS; ++j) {
            std::cout << tablero[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

bool verificarVictoria(char jugador) {
    // Verificar horizontal, vertical y diagonal (ambas direcciones) para una victoria
    for (int i = 0; i < FILAS; ++i) {
        for (int j = 0; j < COLUMNAS; ++j) {
            if (tablero[i][j] == jugador) {
                // Verificación horizontal
                if (j + 3 < COLUMNAS &&
                    tablero[i][j + 1] == jugador &&
                    tablero[i][j + 2] == jugador &&
                    tablero[i][j + 3] == jugador)
                    return true;
                // Verificación vertical
                if (i + 3 < FILAS &&
                    tablero[i + 1][j] == jugador &&
                    tablero[i + 2][j] == jugador &&
                    tablero[i + 3][j] == jugador)
                    return true;
                // Verificación diagonal (\ dirección)
                if (i + 3 < FILAS && j + 3 < COLUMNAS &&
                    tablero[i + 1][j + 1] == jugador &&
                    tablero[i + 2][j + 2] == jugador &&
                    tablero[i + 3][j + 3] == jugador)
                    return true;
                // Verificación diagonal (/ dirección)
                if (i - 3 >= 0 && j + 3 < COLUMNAS &&
                    tablero[i - 1][j + 1] == jugador &&
                    tablero[i - 2][j + 2] == jugador &&
                    tablero[i - 3][j + 3] == jugador)
                    return true;
            }
        }
    }
    return false;
}

bool realizarMovimiento(int columna, char jugador) {
    if (columna < 0 || columna >= COLUMNAS) return false;
    for (int i = FILAS - 1; i >= 0; --i) {
        if (tablero[i][columna] == '.') {
            tablero[i][columna] = jugador;
            return true;
        }
    }
    return false;
}

void manejarCliente(int socketCliente) {
    char jugador = 'C'; // Cliente
    char servidorJugador = 'S'; // Servidor
    int columna;
    bool enEjecucion = true;

    while (enEjecucion) {
        // Recibir movimiento del cliente
        read(socketCliente, &columna, sizeof(columna));
        if (!realizarMovimiento(columna, jugador)) {
            // Movimiento inválido
            columna = -1;
            write(socketCliente, &columna, sizeof(columna));
            continue;
        }
        write(socketCliente, &columna, sizeof(columna)); // Enviar movimiento para confirmar

        // Verificar victoria del cliente
        if (verificarVictoria(jugador)) {
            columna = -2; // Gana el cliente
            write(socketCliente, &columna, sizeof(columna));
            enEjecucion = false;
            break;
        }

        // Turno del servidor
        columna = rand() % COLUMNAS;
        while (!realizarMovimiento(columna, servidorJugador)) {
            columna = rand() % COLUMNAS;
        }
        write(socketCliente, &columna, sizeof(columna));

        // Verificar victoria del servidor
        if (verificarVictoria(servidorJugador)) {
            columna = -3; // Gana el servidor
            write(socketCliente, &columna, sizeof(columna));
            enEjecucion = false;
        }
    }

    close(socketCliente);
}

int main() {
    int socketServidor, socketCliente;
    struct sockaddr_in direccionServidor, direccionCliente;
    socklen_t longitudDireccion = sizeof(direccionCliente);

    socketServidor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketServidor == -1) {
        perror("Fallo en la creación del socket");
        exit(EXIT_FAILURE);
    }

    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = INADDR_ANY;
    direccionServidor.sin_port = htons(PUERTO);

    if (bind(socketServidor, (struct sockaddr*)&direccionServidor, sizeof(direccionServidor)) == -1) {
        perror("Fallo en el bind");
        close(socketServidor);
        exit(EXIT_FAILURE);
    }

    if (listen(socketServidor, MAX_CLIENTES) == -1) {
        perror("Fallo en el listen");
        close(socketServidor);
        exit(EXIT_FAILURE);
    }

    std::cout << "Servidor escuchando en el puerto " << PUERTO << std::endl;

    while (true) {
        socketCliente = accept(socketServidor, (struct sockaddr*)&direccionCliente, &longitudDireccion);
        if (socketCliente == -1) {
            perror("Fallo en el accept");
            continue;
        }

        std::thread hiloCliente(manejarCliente, socketCliente);
        hiloCliente.detach();
    }

    close(socketServidor);
    return 0;
}
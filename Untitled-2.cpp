#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

void actualizarTablero(int columna, char jugador) {
    for (int i = FILAS - 1; i >= 0; --i) {
        if (tablero[i][columna] == '.') {
            tablero[i][columna] = jugador;
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <ip_servidor> <puerto_servidor>" << std::endl;
        return 1;
    }

    const char* ipServidor = argv[1];
    int puertoServidor = atoi(argv[2]);
    int socketCliente;
    struct sockaddr_in direccionServidor;

    socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socketCliente == -1) {
        perror("Fallo en la creación del socket");
        return 1;
    }

    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_port = htons(puertoServidor);
    inet_pton(AF_INET, ipServidor, &direccionServidor.sin_addr);

    if (connect(socketCliente, (struct sockaddr*)&direccionServidor, sizeof(direccionServidor)) == -1) {
        perror("Fallo en la conexión");
        close(socketCliente);
        return 1;
    }

    std::cout << "Conectado al servidor" << std::endl;
    inicializarTablero();
    imprimirTablero();

    int columna;
    char jugador = 'C'; // Cliente

    while (true) {
        std::cout << "Ingrese columna (0-6): ";
        std::cin >> columna;
        write(socketCliente, &columna, sizeof(columna));
        read(socketCliente, &columna, sizeof(columna));
        if (columna == -1) {
            std::cout << "Movimiento inválido, intente de nuevo" << std::endl;
            continue;
        } else if (columna == -2) {
            std::cout << "¡Has ganado!" << std::endl;
            break;
        } else if (columna == -3) {
            std::cout << "¡El servidor ha ganado!" << std::endl;
            break;
        }
        actualizarTablero(columna, jugador);
        imprimirTablero();

        read(socketCliente, &columna, sizeof(columna));
        if (columna >= 0) {
            actualizarTablero(columna, 'S'); // Servidor
            imprimirTablero();
        }
    }

    close(socketCliente);
    return 0;
}
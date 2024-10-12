#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }

    const char* filename = argv[1];
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }

    // Создание сокета
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "Socket creation failed" << endl;
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        cerr << "Set socket options failed" << endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listening on all interfaces
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Binding failed" << endl;
        return -1;
    }

    // Ожидание подключения
    if (listen(server_fd, 3) < 0) {
        cerr << "Listening failed" << endl;
        return -1;
    }

    // Принимаем соединение
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        cerr << "Accept failed" << endl;
        return -1;
    }

    // Отправляем содержимое файла
    string line;
    while (getline(file, line)) {
        send(new_socket, line.c_str(), line.size(), 0);
        send(new_socket, "\n", 1, 0);  // Отправка новой строки
    }

    close(new_socket);
    close(server_fd);
    file.close();
    return 0;
}

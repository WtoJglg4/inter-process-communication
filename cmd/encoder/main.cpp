#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <vector>
#include <fstream>


using namespace std;

#define srv1_port 8080
#define srv2_port 8081
#define BUFFER_SIZE 1024

void receiveData(int sock, vector<char>& buffer) {
    char tempBuffer[BUFFER_SIZE];
    int bytesReceived;
    while ((bytesReceived = recv(sock, tempBuffer, BUFFER_SIZE, 0)) > 0) {
        printf("Bytes received %d\n", bytesReceived);
        buffer.insert(buffer.end(), tempBuffer, tempBuffer + bytesReceived);
    }
}

struct sockaddr_in configureServer(int port) {
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    return serv_addr;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <file1> <file2>" << endl;
        return EXIT_FAILURE;
    }
    cout << "ARGUMENTS: " << argv[1] << " " << argv[2] << endl; 

    // Запускаем два экземпляра программы 1
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execlp("./bin/readwriter", "readwriter", argv[1], to_string(srv1_port).c_str(), nullptr);
        exit(1);
    }
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execlp("./bin/readwriter", "readwriter", argv[2], to_string(srv2_port).c_str(), nullptr);
        exit(1);
    }

    // Ожидание подключения
    sleep(1); // Подождем немного для подготовки серверов

    // Подключение к серверу
    int sock1 = socket(AF_INET, SOCK_STREAM, 0);
    int sock2 = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in srv1 = configureServer(srv1_port);
    struct sockaddr_in srv2 = configureServer(srv2_port);

    // Подключаемся к первому серверу
    if (connect(sock1, (struct sockaddr*)&srv1, sizeof(srv1)) < 0) {
        perror("Error connecting to server 1");
        return EXIT_FAILURE;
    }

    // Подключаемся ко второму серверу
    if (connect(sock2, (struct sockaddr*)&srv2, sizeof(srv2)) < 0) {
        perror("Error connecting to server 2");
        return EXIT_FAILURE;
    }

    // Чтение данных с сокетов в буферы
    vector<char> buffer1, buffer2;
    receiveData(sock1, buffer1);
    receiveData(sock2, buffer2);

    close(sock1);
    close(sock2);

    // Ожидание завершения дочерних процессов
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);

    // Выполнение операции XOR между данными из двух буферов и запись в result.txt
    ofstream result("result.txt");

    // Определяем минимальный размер между двумя буферами
    size_t minSize = min(buffer1.size(), buffer2.size());

    // XOR только для общего размера двух буферов
    for (size_t i = 0; i < minSize; ++i) {
        result << static_cast<char>(buffer1[i] ^ buffer2[i]); // XOR
    }

    result.close();

    return EXIT_SUCCESS;
}

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

void receiveData(int sock, ofstream& output) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    while ((bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        output.write(buffer, bytesReceived);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <file1> <file2>" << endl;
        return 1;
    }

    // Запускаем два экземпляра программы 1
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execlp("./socket_server", "socket_server", argv[1], nullptr);
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        execlp("./socket_server", "socket_server", argv[2], nullptr);
        exit(1);
    }

    // Ожидание подключения
    sleep(1); // Подождем немного для подготовки сервера

    // Подключение к серверу
    int sock1 = socket(AF_INET, SOCK_STREAM, 0);
    int sock2 = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // Подключаемся к первому серверу
    connect(sock1, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    ofstream output1("output1.txt");
    receiveData(sock1, output1);
    output1.close();
    close(sock1);

    // Подключаемся ко второму серверу
    connect(sock2, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    ofstream output2("output2.txt");
    receiveData(sock2, output2);
    output2.close();
    close(sock2);

    // Ожидание завершения дочерних процессов
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);

    // Выполнение операции XOR
    ifstream file1("output1.txt");
    ifstream file2("output2.txt");
    ofstream result("result.txt");

    string line1, line2;
    while (getline(file1, line1) && getline(file2, line2)) {
        string result_line;
        for (size_t i = 0; i < min(line1.size(), line2.size()); ++i) {
            result_line += (line1[i] ^ line2[i]); // XOR
        }
        result << result_line << endl;
    }

    file1.close();
    file2.close();
    result.close();

    return 0;
}

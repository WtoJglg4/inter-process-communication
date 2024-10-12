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

const char * reader_path = "./bin/reader";

void receiveData(int sock, vector<char>& buffer) {
    char tempBuffer[BUFFER_SIZE];
    int bytesReceived;
    while ((bytesReceived = recv(sock, tempBuffer, BUFFER_SIZE, 0)) > 0) {
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

    // Start 2 instances of reader
    pid_t pid1 = fork();
    if (pid1 == 0) {
        execlp(reader_path, "reader", argv[1], to_string(srv1_port).c_str(), nullptr);
        exit(1);
    }
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        execlp(reader_path, "reader", argv[2], to_string(srv2_port).c_str(), nullptr);
        exit(1);
    }

    // Waiting for start servers
    sleep(1);

    // Connect to servers
    int sock1 = socket(AF_INET, SOCK_STREAM, 0);
    int sock2 = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in srv1 = configureServer(srv1_port);
    struct sockaddr_in srv2 = configureServer(srv2_port);

    if (connect(sock1, (struct sockaddr*)&srv1, sizeof(srv1)) < 0) {
        perror("Error connecting to server 1");
        return EXIT_FAILURE;
    }

    if (connect(sock2, (struct sockaddr*)&srv2, sizeof(srv2)) < 0) {
        perror("Error connecting to server 2");
        return EXIT_FAILURE;
    }

    // Read data from sockets to buffers
    vector<char> buffer1, buffer2;
    receiveData(sock1, buffer1);
    receiveData(sock2, buffer2);

    close(sock1);
    close(sock2);

    // Waiting for forks stop
    waitpid(pid1, nullptr, 0);
    waitpid(pid2, nullptr, 0);

     // XORing two buffers and write the result
    ofstream result("result.txt");
    size_t size1 = buffer1.size();
    size_t size2 = buffer2.size();

    // XORing two buffers
    for (size_t i = 0; i < size1; ++i) {
        char xorResult = static_cast<char>(buffer1[i] ^ buffer2[i % size2]);
        result << xorResult;
    }
    result.close();
    return EXIT_SUCCESS;
}

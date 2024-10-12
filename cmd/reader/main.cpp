#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <filename> <port>" << endl;
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "error opening file: " << filename << endl;
        return EXIT_FAILURE;
    }

    // Socket initialization
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "socket creation failed" << endl;
        return EXIT_FAILURE;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        cerr << "set socket options failed" << endl;
        return EXIT_FAILURE;
    }

    int port = atoi(argv[2]);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listening on all interfaces
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "binding failed" << endl;
        return EXIT_FAILURE;
    }

    // Waiting for connection
    if (listen(server_fd, 3) < 0) {
        cerr << "listening failed" << endl;
        return EXIT_FAILURE;
    }

    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
        cerr << "accept failed" << endl;
        return EXIT_FAILURE;
    }

    // Send file's content
    string line;
    while (getline(file, line)) {
        send(new_socket, line.c_str(), line.size(), 0);
        // Its important cause we want to distinguish 'dirty' and encoded messages.
        // '\n' in encoded message breaks everything :D
        // if (strcmp(filename, "result.txt") != 0){
            send(new_socket, "\n", 1, 0);
        // }
    }

    close(new_socket);
    close(server_fd);
    file.close();
    return EXIT_SUCCESS;
}

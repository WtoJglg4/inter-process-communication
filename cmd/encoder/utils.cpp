#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void format(const string& filename) {
    ifstream file(filename, ios::in | ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    if (content.empty()) {
        cerr << "File is empty." << endl;
        return;
    }
    content.pop_back();
    ofstream outfile(filename, ios::out | ios::trunc | ios::binary);
    if (!outfile.is_open()) {
        cerr << "Error opening file for writing: " << filename << endl;
        return;
    }
    outfile << content;
    outfile.close();
}
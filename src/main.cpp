// Copyright (C) 2024-2025 Manuel Schneider

#include <iostream>
using namespace std;
namespace albert {
extern int run(int, char **);
}

int main(int argc, char **argv)
{
    try {
        return albert::run(argc, argv);
    } catch (const exception &e) {
        cout << e.what() << endl;
    } catch (...) {
        cout << "Unknown exception in main!" << endl;
    }
}

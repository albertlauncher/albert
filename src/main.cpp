// Copyright (C) 2024-2025 Manuel Schneider

#include <iostream>
using namespace std;
namespace albert::detail {
extern int run(int, char **);
}

int main(int argc, char **argv)
{
    try {
        return albert::detail::run(argc, argv);
    } catch (const exception &e) {
        cout << e.what() << endl;
    } catch (...) {
        cout << "Unknown exception in main!" << endl;
    }
    return EXIT_FAILURE;
}

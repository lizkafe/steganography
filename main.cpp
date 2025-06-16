#include "steganography.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif



int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif

    std::locale::global(std::locale(""));

    std::cout << "Выберите стеганографический метод:\n";
    std::cout << " 1 - LSB (Least Significant Bit)\n";
    std::cout << " 2 - HS (Histogram Shifting)\n";
    std::cout << " 3 - QIM (Quantization Index Modulation)\n";
    std::cout << " 4 - PM1 (Plus-Minus One)\n";
    std::cout << "Ваш выбор: ";
    int method;
    std::cin >> method;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (static_cast<Method>(method)) {
        case Method::LSB: runLSB(); break;
        case Method::HS:  runHS();  break;
        case Method::QIM: runQIM(); break;
        case Method::PM1: runPM1(); break;
        default: std::cerr << "Неверный выбор метода.\n"; break;
    }
    return 0;
}
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

void generate_matrix(const std::string& filename, int rows, int cols) {
    auto start = std::chrono::high_resolution_clock::now();

    std::ofstream file(filename, std::ios::out | std::ios::trunc);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.1, 10.0);

    file << rows << " " << cols << '\n'; // Первая строка с размерами матрицы
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            file << dis(gen) << " "; // Сразу записываем случайные числа в файл
        }
        file << '\n'; // Переход на новую строку
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Matrix generated at: " << filename << std::endl;
    std::cout << "Time taken to generate " << filename << ": " << duration.count() << " seconds." << std::endl;
}

int main() {
    std::string path;
    int rows, cols;

    // Ввод пути до папки
    std::cout << "Enter the path to save the files: ";
    std::getline(std::cin, path);

    // Проверка, существует ли путь
    if (!fs::exists(path)) {
        std::cerr << "The path does not exist!" << std::endl;
        return 1;
    }

    // Ввод размеров матрицы
    std::cout << "Enter the number of rows: ";
    std::cin >> rows;
    std::cout << "Enter the number of columns: ";
    std::cin >> cols;

    auto total_start = std::chrono::high_resolution_clock::now();

    // Генерация и запись матриц
    generate_matrix(path + "/A.txt", rows, cols);
    generate_matrix(path + "/B.txt", rows, cols);
    generate_matrix(path + "/C.txt", rows, cols);

    auto total_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_duration = total_end - total_start;

    std::cout << "\nAll matrices have been successfully saved to the specified path." << std::endl;
    std::cout << "Total time taken to generate all matrices: " << total_duration.count() << " seconds." << std::endl;

    return 0;
}

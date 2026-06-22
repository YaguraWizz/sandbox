#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <omp.h>
#include <chrono> // Для замера времени

using Matix = std::vector<std::vector<double>>;

// Функция для загрузки матрицы из файла
static Matix load_matrix(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        std::cerr << "Error opening file from path: " << path << std::endl;
        throw std::runtime_error("Failed to open file");
    }

    int rows, cols;
    input >> rows >> cols;

    Matix matrix(rows, std::vector<double>(cols));
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            input >> matrix[i][j];
        }
    }

    return matrix;
}

// Умножение матриц A и B
static Matix multiply_matrices(const Matix& A, const Matix& B) {
    size_t A_rows = A.size();
    size_t A_cols = A[0].size();
    size_t B_cols = B[0].size();

    Matix result(A_rows, std::vector<double>(B_cols, 0.0));

    for (int i = 0; i < A_rows; ++i) {
        for (int j = 0; j < B_cols; ++j) {
            for (int k = 0; k < A_cols; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return result;
}

int main() {
    std::string path;

    std::cout << "Enter directory for matrix files: ";
    std::cin >> path;

    Matix A, B, C, AB, result;

    // Загружаем матрицы из файлов
    A = load_matrix(path + "\\A.txt");
    B = load_matrix(path + "\\B.txt");
    C = load_matrix(path + "\\C.txt");

    // Проверка производительности на 1 потоке
    omp_set_num_threads(1); // Устанавливаем количество потоков = 1
    auto start_1 = std::chrono::high_resolution_clock::now();

#pragma omp parallel sections
    {
#pragma omp section
        {
            // Умножаем A на B
            AB = multiply_matrices(A, B);
        }

#pragma omp section
        {
            // После получения AB, умножаем AB на C
#pragma omp barrier // Убедимся, что AB завершено
            result = multiply_matrices(AB, C);
        }
    }

    auto end_1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_1 = end_1 - start_1;
    std::cout << "Time on 1 thread: " << duration_1.count() << " seconds." << std::endl;

    // Проверка производительности на 4 потоках
    omp_set_num_threads(4); // Устанавливаем количество потоков = 4
    auto start_4 = std::chrono::high_resolution_clock::now();

#pragma omp parallel sections
    {
#pragma omp section
        {
            // Умножаем A на B
            AB = multiply_matrices(A, B);
        }

#pragma omp section
        {
            // После получения AB, умножаем AB на C
#pragma omp barrier // Убедимся, что AB завершено
            result = multiply_matrices(AB, C);
        }
    }

    auto end_4 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_4 = end_4 - start_4;
    std::cout << "Time on 4 threads: " << duration_4.count() << " seconds." << std::endl;

    // Расчёт ускорения и эффективности
    double speedup = duration_1.count() / duration_4.count();
    double efficiency = speedup / 4.0;

    std::cout << "Speedup: " << speedup << std::endl;
    std::cout << "Efficiency: " << efficiency << std::endl;

    return 0;
}

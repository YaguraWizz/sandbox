#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <iomanip> 

// Функция для генерации случайной матрицы заданного размера
std::vector<std::vector<double>> generate_matrix(int rows, int cols) {
    std::vector<std::vector<double>> matrix(rows, std::vector<double>(cols));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
        }
    }

    return matrix;
}

// Функция для умножения двух матриц
std::vector<std::vector<double>> multiply_matrices(const std::vector<std::vector<double>>& A, const std::vector<std::vector<double>>& B) {
    int rowsA = static_cast<int>(A.size());
    int colsA = static_cast<int>(A[0].size());
    int rowsB = static_cast<int>(B.size());
    int colsB = static_cast<int>(B[0].size());

    if (colsA != rowsB) {
        throw std::invalid_argument("Matrix dimensions do not match for multiplication.");
    }

    std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            for (int k = 0; k < colsA; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}

// Функция для мониторинга времени
void monitor_time(std::atomic<bool>& stop_flag, int max_time_seconds, std::condition_variable& cv, std::mutex& cv_mutex) {
    auto start_time = std::chrono::high_resolution_clock::now();
    std::unique_lock<std::mutex> lock(cv_mutex);
    while (!stop_flag.load()) {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - start_time;
        if (elapsed.count() > max_time_seconds) {
            std::cerr << "Maximum execution time exceeded. Stopping computation.\n";
            stop_flag.store(true);
            cv.notify_all(); // Уведомляем основной поток об остановке
            break;
        }
        cv.wait_for(lock, std::chrono::milliseconds(100)); // Ожидаем уведомления или тайм-аут
    }
}

int main() {
    try {
        int size = 10; // Начальный размер матриц
        const int max_time_seconds = 600; // Максимальное время выполнения 10 минут
        std::atomic<bool> stop_flag(false); // Флаг для остановки работы
        std::condition_variable cv;
        std::mutex cv_mutex;

        // Поток мониторинга времени
        std::thread timer_thread(monitor_time, std::ref(stop_flag), max_time_seconds, std::ref(cv), std::ref(cv_mutex));

        // Заголовок таблицы
        std::cout << std::setw(15) << "Matrix size" << std::setw(30) << "Execution time (seconds)\n";

        while (!stop_flag.load()) {
            auto A = generate_matrix(size, size);
            auto B = generate_matrix(size, size);
            auto C = generate_matrix(size, size);

            auto start_time = std::chrono::high_resolution_clock::now();
            try {
                // Умножаем матрицы: R = A * B * C
                auto AB = multiply_matrices(A, B);
                auto R = multiply_matrices(AB, C);
            }
            catch (const std::exception& e) {
                std::cerr << "Matrix multiplication error: " << e.what() << "\n";
                break;
            }
            auto end_time = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_time = end_time - start_time;

            if (stop_flag.load()) {
                break;
            }

            std::cout << std::setw(15) << (std::to_string(size) + "x" + std::to_string(size))
                << std::setw(20) << elapsed_time.count() << "\n";

            size *= 10; // Увеличиваем размер матрицы на порядок
        }

        // Завершаем поток мониторинга времени
        {
            std::lock_guard<std::mutex> lock(cv_mutex);
            stop_flag.store(true);
        }
        cv.notify_all(); // Уведомляем поток мониторинга о завершении
        timer_thread.join();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

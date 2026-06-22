#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <omp.h>
#include <iomanip>
#include <unordered_map>
#include <stdexcept>


static void generate_random_matrix(std::vector<double>& matrix, int size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);

    if (size != static_cast<int>(std::sqrt(matrix.size()))) {
        throw std::runtime_error("Error: size != matrix.size");
    }

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i * size + j] = dis(gen);
        }
    }
}

template <typename Tptr>
static bool comput_range(const Tptr* range1, const Tptr* range2, size_t size) {
    if (!range1 || !range2) {
        std::cerr << "RUNTIME_ERROR: Null pointer encountered in comput_range_v4\n";
        return false;
    }

    for (size_t i = 0; i < size; ++i) {
        int a_int = static_cast<int>(range1[i]);
        int b_int = static_cast<int>(range2[i]);

        if (a_int != b_int) {
            std::cout << std::string(90, '-') << std::endl;
            std::cout << "ERROR. range1: " << range1[i] << ", range2: " << range2[i] << "\n";
            std::cout << "Integer parts: range1: " << a_int << ", range2: " << b_int << "\n";
            std::cout << std::string(90, '-') << "\n\n";
            return false;
        }
    }

    return true;
}



template<typename T>
static void print_matrix(T* matrix, size_t size, size_t offset = 0) {
    if (!matrix) {
        throw std::runtime_error("error: ptr matrix nullptr");
    }

    size_t row = static_cast<size_t>(sqrt(size));
    size_t current = 0;
    std::string offset_line(offset, ' ');
    std::cout << offset_line << "Matrix size: " << size << "\n";
    for (size_t i = 0; i < size; ++i) {
        std::cout << offset_line << matrix[i] << " ";  // Печатаем элемент
        current++;

        if (current == row) {
            std::cout << "\n";  // Печатаем новую строку
            current = 0;  // Сброс текущего индекса
        }
    }
    std::cout << std::endl;
}


template<typename T>
static void matrix_mult(const T* matrixA, const T* matrixB, T* matrixR, size_t size) {
    if (!matrixA || !matrixB || !matrixR) {
        throw std::invalid_argument("Error: one or more matrix pointers are null");
    }

    for (size_t row = 0; row < size; ++row) {       // Перебор строк в C
        for (size_t col = 0; col < size; ++col) {   // Перебор столбцов в C
            for (size_t k = 0; k < size; ++k) {     // Скалярное произведение строки A и столбца B
                matrixR[row * size + col] += matrixA[row * size + k] * matrixB[k * size + col];
            }
        }
    }
}

template<typename T>
static void matrix_mult_openmp_schedule_runtime(const T* matrixA, const T* matrixB, T* matrixR, size_t size) {
    if (!matrixA || !matrixB || !matrixR) {
        throw std::invalid_argument("Error: one or more matrix pointers are null");
    }

#pragma omp parallel for schedule(runtime)
    for (size_t row = 0; row < size; ++row) {       // Перебор строк в C
        for (size_t col = 0; col < size; ++col) {   // Перебор столбцов в C
            T sum = 0;
            for (size_t k = 0; k < size; ++k) {     // Скалярное произведение строки A и столбца B
                sum += matrixA[row * size + k] * matrixB[k * size + col];
            }
            matrixR[row * size + col] = sum;
        }
    }
}

template<typename T>
static void matrix_mult_openmp_no_schedule(const T* matrixA, const T* matrixB, T* matrixR, size_t size) {
    if (!matrixA || !matrixB || !matrixR) {
        throw std::invalid_argument("Error: one or more matrix pointers are null");
    }

#pragma omp parallel for
    for (size_t row = 0; row < size; ++row) {       // Перебор строк в C
        for (size_t col = 0; col < size; ++col) {   // Перебор столбцов в C
            for (size_t k = 0; k < size; ++k) {     // Скалярное произведение строки A и столбца B
                matrixR[row * size + col] += matrixA[row * size + k] * matrixB[k * size + col];
            }
        }
    }
}





static void print_table_row(int size, const std::string& schedule, int chunk,
    double seq_time, double par_time, double speedup, double efficiency, const std::string& result) {
    
    std::cout << std::setw(8) << size
        << std::setw(15) << schedule
        << std::setw(10) << ((chunk == -1) ? "-" : std::to_string(chunk))
        << std::setw(15) << std::fixed << std::setprecision(6) << seq_time
        << std::setw(15) << std::fixed << std::setprecision(6) << par_time
        << std::setw(12) << std::fixed << std::setprecision(3) << speedup
        << std::setw(12) << std::fixed << std::setprecision(3) << efficiency
        << std::setw(21) << result
        << std::endl;
}






int main() {
    try
    {
       
        std::vector<int> sizes = { 10, 100, 1000 };

        std::unordered_map<std::string, std::vector<int>> schedules = {
            { "static", { 1, 100 } },
            { "dynamic", { 1, 100 } },
            { "guided", { 1, 100 } },
            { "auto", { 1 } } // Можно добавить auto для сравнения
        };


        int num_threads = omp_get_num_procs();
        omp_set_num_threads(num_threads);
        std::cout << "Number of threads: " << num_threads << std::endl;

        std::cout << std::left
            << std::setw(8) << "Size"
            << std::setw(15) << "Schedule"
            << std::setw(10) << "Chunk"
            << std::setw(15) << "Seq. Time (s)"
            << std::setw(15) << "Par. Time (s)"
            << std::setw(12) << "Speedup"
            << std::setw(12) << "Efficiency"
            << std::setw(12) << "Diff"
            << std::endl;

        std::cout << std::string(90, '-') << std::endl;

        for (int size : sizes) {
            int matrix_size = size * size;

            std::vector<double> A(matrix_size, 0.0), B(matrix_size, 0.0), C(matrix_size, 0.0),
                D1(matrix_size, 0.0), R1(matrix_size, 0.0),
                D2(matrix_size, 0.0), R2(matrix_size, 0.0),
                D3(matrix_size, 0.0), R3(matrix_size, 0.0);

            generate_random_matrix(A, size);
            generate_random_matrix(B, size);
            generate_random_matrix(C, size);


            // Последовательное вычисление
            auto start_seq = std::chrono::high_resolution_clock::now();
            matrix_mult(A.data(), B.data(), D1.data(), size);
            matrix_mult(D1.data(), C.data(), R1.data(), size);
            auto end_seq = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> seq_duration = end_seq - start_seq;

            // Вычисление БЕЗ явного указания schedule
            auto start_no_schedule = std::chrono::high_resolution_clock::now();
            matrix_mult_openmp_no_schedule(A.data(), B.data(), D2.data(), size);
            matrix_mult_openmp_no_schedule(D2.data(), C.data(), R2.data(), size);
            auto end_no_schedule = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> no_schedule_duration = end_no_schedule - start_no_schedule;


            std::string result = (comput_range(R1.data(), R2.data(), matrix_size) ? "\033[32mSUCCESS\033[0m" : "\033[31mFAILED\033[0m");

            double speedup_no_schedule = seq_duration.count() / no_schedule_duration.count();
            double efficiency_no_schedule = speedup_no_schedule / num_threads;


            print_table_row(size, "no schedule", -1, seq_duration.count(), no_schedule_duration.count(),
                speedup_no_schedule, efficiency_no_schedule, result);


            auto setting = [](const std::string& str_setting) -> omp_sched_t {
                if (str_setting == "static") { return omp_sched_static; }
                else if (str_setting == "dynamic") { return omp_sched_dynamic; }
                else if (str_setting == "guided") { return omp_sched_guided; }
                else { return omp_sched_auto; }
                };


            // Цикл по различным значениям schedule
            for (const auto& [schedule, vec_size_thread_work] : schedules) {
                for (const auto& size_thread_work : vec_size_thread_work) {
                    omp_set_schedule(setting(schedule), size_thread_work); // Добавили auto
                    auto start_par = std::chrono::high_resolution_clock::now();
                    matrix_mult_openmp_schedule_runtime(A.data(), B.data(), D3.data(), size);
                    matrix_mult_openmp_schedule_runtime(D3.data(), C.data(), R3.data(), size);
                    auto end_par = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> par_duration = end_par - start_par;


                    result = "\033[32mSUCCESS\033[0m";
                    if (!comput_range(R1.data(), R3.data(), matrix_size)) {
                        result = "\033[31mFAILED\033[0m";
                        print_matrix(R1.data(), matrix_size, 4);
                        std::cout << "\n\n\n";
                        print_matrix(R3.data(), matrix_size, 4);
                    }

                    double speedup = seq_duration.count() / par_duration.count();
                    double efficiency = speedup / num_threads;



                    print_table_row(size, schedule, size_thread_work, seq_duration.count(), par_duration.count(),
                        speedup, efficiency, result);
                }
            }
            std::cout << std::endl;
        }


    }
    catch (const std::exception&e)
    {
        std::cerr << "\033[32m" << e.what() << "\033[0m";
    }

    return 0;
}


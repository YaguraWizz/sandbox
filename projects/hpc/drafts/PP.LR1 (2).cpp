#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <cassert>


namespace rizz {
    static int mpi_size(MPI_Comm comm = MPI_COMM_WORLD) {
        int size = -1;
        MPI_Comm_size(comm, &size);
        return size;
    }
    static int mpi_rank(MPI_Comm comm = MPI_COMM_WORLD) {
        int rank = -1;
        MPI_Comm_rank(comm, &rank);
        return rank;
    }

    class mpi_context {
    private:
        int _size, _rank;
        bool _finalized;  
        MPI_Comm _comm;
        std::ofstream error_log; 

        // Утилитная функция для обработки ошибок и записи в файл
        void check_mpi_error(int errcode, const char* message) {
            if (errcode != MPI_SUCCESS) {
                char error_string[512];
                int length_of_error_string;
                MPI_Error_string(errcode, error_string, &length_of_error_string);

                // Запись ошибки в файл
                if (error_log.is_open()) {
                    error_log << "MPI Error: " << message << " (" << error_string << ")" << std::endl;
                }
                else {
                    std::cerr << "Failed to open error log file." << std::endl;
                }

                // Прерываем выполнение программы с ошибкой
                MPI_Abort(_comm, errcode);
            }
        }

    public:
        mpi_context(const int* argc, char*** argv, MPI_Comm comm = MPI_COMM_WORLD)
            : _size(0), _rank(0), _finalized(false), _comm(comm) {

            // Открытие файла для логирования ошибок
            error_log.open("mpi_errors.log", std::ios::out | std::ios::app);
            if (!error_log.is_open()) {
                std::cerr << "Failed to open the error log file!" << std::endl;
                MPI_Abort(comm, 1); 
            }

            // Инициализация MPI
            int errcode = MPI_Init(argc, argv);
            check_mpi_error(errcode, "MPI_Init failed");

            // Получение ранга и размера
            errcode = MPI_Comm_rank(_comm, &_rank);
            check_mpi_error(errcode, "MPI_Comm_rank failed");

            errcode = MPI_Comm_size(_comm, &_size);
            check_mpi_error(errcode, "MPI_Comm_size failed");
        }

        ~mpi_context() {
            if (!_finalized) {
                destroy();  
            }

            // Закрытие файла после завершения
            if (error_log.is_open()) {
                error_log.close();
            }
        }

        int destroy() {
            if (_finalized) {
                return MPI_SUCCESS;  
            }

            // Завершаем MPI
            int errcode = MPI_Finalize();
            check_mpi_error(errcode, "MPI_Finalize failed");

            _finalized = true; 
            return errcode;
        }

        int get_rank() const noexcept { return _rank; }
        int get_size() const noexcept { return _size; }
        MPI_Comm get_comm() const noexcept { return _comm; }

        bool is_rank(int rank) const noexcept { return _rank == rank; }
        bool is_size(int size) const noexcept { return _size == size; }
    };


    template <typename T>
    void generate(T* array, size_t size) {
        static_assert(std::is_arithmetic<T>::value, "Тип должен быть числовым.");

        std::random_device rd;
        std::mt19937 gen(rd());

        if constexpr (std::is_integral<T>::value) {
            std::uniform_int_distribution<T> dist(1, 100);  // Генерация случайных целых чисел от 1 до 100
            for (size_t i = 0; i < size; ++i) {
                array[i] = dist(gen);
            }
        }
        else if constexpr (std::is_floating_point<T>::value) {
            std::uniform_real_distribution<T> dist(1.0, 100.0);  // Генерация случайных чисел с плавающей точкой от 1.0 до 100.0
            for (size_t i = 0; i < size; ++i) {
                array[i] = dist(gen);
            }
        }
        else {
            // Это не целое число и не число с плавающей точкой
            throw std::invalid_argument("Неподдерживаемый тип.");
        }
    }

    // Функция для вывода матрицы
    template <typename T>
    void print(const T* matrix, size_t size) {
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                std::cout << matrix[i * size + j] << " ";
            }
            std::cout << std::endl;
        }
    }

    template<typename ...Argc>
    void message(Argc&&... argc) {
        (std::cout << ... << argc) << std::endl;
    }


    template<typename T>
    struct MPI_Type;

    // Специализация для int
    template<>
    struct MPI_Type<int> {
        static MPI_Datatype type() { return MPI_INT; }
    };

    // Специализация для double
    template<>
    struct MPI_Type<double> {
        static MPI_Datatype type() { return MPI_DOUBLE; }
    };





    template<typename T>
    static void Isend_ALL(T* data, int size, MPI_Comm comm) {
        int _size = ::rizz::mpi_size(comm);
        
        std::vector<MPI_Request> requests(_size - 1);

        for (int i = 1; i < _size; i++) {
            MPI_Isend(data, size * size, MPI_Type<T>().type(), i, 0, comm, &requests[i - 1]);
        }

        MPI_Waitall(_size - 1, requests.data(), MPI_STATUSES_IGNORE);
    }


    template<typename T>
    static void Irecv_ALL(T* data, int size, MPI_Comm comm) {
        int _size = ::rizz::mpi_size(comm);

        std::vector<T> buffer(size * size, T{}); // Буфер для получения данных от других процессов

       // rizz::message("Matrix data. size:", size * size);
       // rizz::print(data, size); // Вывод начальной матрицы

        // Для каждого процесса (кроме 0) будет осуществляться прием данных
        for (int rank = 1; rank < _size; ++rank) {
            MPI_Request request;

            // Инициализация асинхронного приема
            MPI_Irecv(buffer.data(), size * size, MPI_Type<T>().type(), rank, 0, comm, &request);

            // Ожидаем завершения приема данных
            MPI_Wait(&request, MPI_STATUSES_IGNORE);

            //rizz::message(std::string(100,'-'));

            //rizz::message("Matrix buffer. size:", buffer.size());
            //rizz::print(buffer.data(), size); // Печать полученных данных

            // Сложение матриц A и B в результат C (суммируем в data)
            ::sum_matrices(data, buffer.data(), data, size);
        }

        //rizz::message("\n");
    }


    class Timer {
    private:
        double start_time = 0.0;

    public:
        Timer() = default;

        // Метод для старта таймера
        void start() {
            start_time = MPI_Wtime();
        }

        // Метод для получения времени, прошедшего с последнего старта
        double end() const {
            return MPI_Wtime() - start_time;
        }
    };

}


#define debug false
#define print_debug(name, enable) if(enable) { rizz::message("Matrix "#name". size:", name.size()); rizz::print(name.data(), N); rizz::message("\n"); }




// Функция умножения двух матриц
template<typename T>
static void multiply_matrices(const T* A, const T* B, T* C, int size_matrix, int rank_proc, int size_proc) {
    int N = size_matrix;
    int chunk_size = N / size_proc;
    int start = chunk_size * rank_proc;
    int stop = chunk_size * (rank_proc + 1);
    if (rank_proc == size_proc - 1) stop = N;

    for (int i = start; i < stop; i++)
    {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i * N + j] += A[i * N + k] * B[k * N + j];
            }
        }
    }
}

template<typename T>
static void sum_matrices(const T* A, const T* B, T* C, int size) {
    // Убедитесь, что размер матриц корректный
    //std::cout << "Matrix size: " << size << std::endl;

    // Перебор всех элементов матрицы
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int index = i * size + j; // Индекс в одномерном массиве
            // Отладочное сообщение
            //std::cout << "Summing A[" << i << "][" << j << "] + B[" << i << "][" << j << "] into C[" << i << "][" << j << "]\n";
            C[index] = A[index] + B[index];
        }
    }
}




template<typename T>
static void multiply_matrices_cpu(const T* matrixA, const T* matrixB, T* matrixR, size_t size) {
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

int main(int argc, char** argv) {
    rizz::mpi_context context{ &argc , &argv };
    rizz::Timer timer{};
    size_t N = 1000;
    std::vector<double> A(N * N, 0), B(N * N, 0), C(N * N, 0), D(N * N, 0), R(N * N, 0), temp1(N * N, 0), temp2(N * N, 0);;

    if (context.is_rank(0)) {
        rizz::message("Start MPI. size: ", context.get_size(), ", matrix size: ", N);

        rizz::generate<double>(A.data(), N * N);
        rizz::generate<double>(B.data(), N * N);
        rizz::generate<double>(C.data(), N * N);

        {
            print_debug(A, debug)
            print_debug(B, debug)
            print_debug(C, debug)
            print_debug(D, debug)
            print_debug(R, debug)
            print_debug(temp1, debug)
            print_debug(temp2, debug)
        }

        timer.start();

        // Отправка данных всем процессам
        for (int rank = 1; rank < context.get_size(); ++rank) {
            MPI_Request request[3];

            // Асинхронная отправка матриц A, B, C
            MPI_Isend(A.data(), A.size(), MPI_DOUBLE, rank, 0, context.get_comm(), &request[0]);
            MPI_Isend(B.data(), B.size(), MPI_DOUBLE, rank, 1, context.get_comm(), &request[1]);
            MPI_Isend(C.data(), C.size(), MPI_DOUBLE, rank, 2, context.get_comm(), &request[2]);

            // Ожидание завершения всех отправок
            MPI_Waitall(3, request, MPI_STATUSES_IGNORE);
        }

        //rizz::message("End Isend");
        //rizz::message("Start Math\n");
    }
    else {
        // Прием данных от процесса 0
        MPI_Request request[3];

        // Асинхронный прием матриц A, B, C
        MPI_Irecv(A.data(), A.size(), MPI_DOUBLE, 0, 0, context.get_comm(), &request[0]);
        MPI_Irecv(B.data(), B.size(), MPI_DOUBLE, 0, 1, context.get_comm(), &request[1]);
        MPI_Irecv(C.data(), C.size(), MPI_DOUBLE, 0, 2, context.get_comm(), &request[2]);

        // Ожидание завершения всех приемов
        MPI_Waitall(3, request, MPI_STATUSES_IGNORE);
    }

    
    multiply_matrices(A.data(), B.data(), D.data(), N, context.get_rank(), context.get_size());


    if (context.is_rank(0)) {
        rizz::Irecv_ALL(D.data(), N, context.get_comm());
        rizz::Isend_ALL(D.data(), N, context.get_comm());
    }
    else {
        MPI_Request request{};
        MPI_Isend(D.data(), D.size(), MPI_DOUBLE, 0, 0, context.get_comm(), &request);
        MPI_Wait(&request, MPI_STATUSES_IGNORE);

        std::fill(D.begin(), D.end(), 0.0);

        MPI_Irecv(D.data(), D.size(), MPI_DOUBLE, 0, 0, context.get_comm(), &request);
        MPI_Wait(&request, MPI_STATUSES_IGNORE);
    }

    multiply_matrices(D.data(), C.data(), R.data(), N, context.get_rank(), context.get_size());


    if (context.is_rank(0)) {
        rizz::Irecv_ALL(R.data(), N, context.get_comm());
        auto mpi_time = timer.end();


        timer.start();
        multiply_matrices_cpu(A.data(), B.data(), temp1.data(), N);
        multiply_matrices_cpu(temp1.data(), C.data(), temp2.data(), N);
        auto cpu_time = timer.end();


        print_debug(temp2, debug)
        rizz::message("CPU Time = ", cpu_time, "s");

        print_debug(R, debug)
        rizz::message("MPI Time = ", mpi_time, "s");
    }
    else {
        MPI_Request request{};
        MPI_Isend(R.data(), R.size(), MPI_DOUBLE, 0, 0, context.get_comm(), &request);
        MPI_Wait(&request, MPI_STATUSES_IGNORE);
    }

    return 0;
}

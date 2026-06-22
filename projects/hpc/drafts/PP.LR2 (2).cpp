#include <mpi.h>
#include <vector>
#include <iostream>

template <typename T>
void multiply_matrices(const T* A, const T* B, T* C, int size_matrix, int rank_proc, int size_proc) {
    int N = size_matrix;
    int chunk_size = N / size_proc;
    int start = chunk_size * rank_proc;
    int stop = chunk_size * (rank_proc + 1);
    if (rank_proc == size_proc - 1) stop = N;

    for (int i = start; i < stop; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i * N + j] += A[i * N + k] * B[k * N + j];
            }
        }
    }
}

template <typename T>
void sum_matrices(const T* A, const T* B, T* C, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            int index = i * size + j;
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

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
  
    Timer timer{};
   
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Создаем группу, состоящую только из процессов с рангами 0 и 1
    MPI_Group world_group{};
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);

    std::vector<int> ranks = { 0, 1 }; // Список процессов с рангами 0 и 1
    MPI_Group new_group{};
    MPI_Group_incl(world_group, ranks.size(), ranks.data(), &new_group);

    // Создаем новый коммуникатор для группы с рангами 0 и 1
    MPI_Comm new_comm{};
    MPI_Comm_create(MPI_COMM_WORLD, new_group, &new_comm);

    // Чтение или генерация матриц A, B, C
    int N = 1000;
    std::vector<double> A(N * N, 0), B(N * N, 0), C(N * N, 0), D(N * N, 0), R(N * N, 0), temp1(N * N, 0), temp2(N * N, 0);

    if (rank == 0) {
        // Генерация данных для матриц
        for (int i = 0; i < N * N; ++i) {
            A[i] = rand() % 10;
            B[i] = rand() % 10;
            C[i] = rand() % 10;
        }
        timer.start();
    }

    // Широковещательная передача матриц A, B и C всем процессам в новом коммуникаторе
    if (rank == 0) {
        MPI_Bcast(A.data(), N * N, MPI_DOUBLE, 0, new_comm);
        MPI_Bcast(B.data(), N * N, MPI_DOUBLE, 0, new_comm);
        MPI_Bcast(C.data(), N * N, MPI_DOUBLE, 0, new_comm);
    }
    else if (rank == 1) {
        MPI_Bcast(A.data(), N * N, MPI_DOUBLE, 0, new_comm);
        MPI_Bcast(B.data(), N * N, MPI_DOUBLE, 0, new_comm);
        MPI_Bcast(C.data(), N * N, MPI_DOUBLE, 0, new_comm);
    }

    // Распределение работы по перемножению матриц
    multiply_matrices(A.data(), B.data(), D.data(), N, rank, size);


    // Сбор всех частей матрицы D с каждого процесса в процессе 0
    MPI_Gather(D.data() + rank * N * N / size, N * N / size, MPI_DOUBLE, temp1.data(), N * N / size, MPI_DOUBLE, 0, new_comm);


    if (rank == 0) {
        MPI_Bcast(temp1.data(), N * N, MPI_DOUBLE, 0, new_comm);
        std::fill(temp1.begin(), temp1.end(), 0.0);
    }
    else if (rank == 1) {
        std::fill(D.begin(), D.end(), 0.0);
        MPI_Bcast(D.data(), N * N, MPI_DOUBLE, 0, new_comm);
    }


    // Распределение работы по перемножению матриц
    multiply_matrices(D.data(), C.data(), R.data(), N, rank, size);


    // Сбор всех частей матрицы D с каждого процесса в процессе 0
    MPI_Gather(R.data() + rank * N * N / size, N * N / size, MPI_DOUBLE, temp1.data(), N * N / size, MPI_DOUBLE, 0, new_comm);



    if (rank == 0) {
        std::swap(temp1, R);
        auto mpi_time = timer.end();

        message("Matrix MPI. size: ", R.size());
       // print(R.data(), N);
        message("Time MPI = ", mpi_time);

        std::fill(temp1.begin(), temp1.end(), 0.0);
       
        timer.start();
        multiply_matrices_cpu(A.data(), B.data(), temp1.data(), N);
        multiply_matrices_cpu(temp1.data(), C.data(), temp2.data(), N);
        auto cpu_time = timer.end();

        message("Matrix CPU. size: ", temp2.size());
       // print(temp2.data(), N);
        message("Time CPU = ", cpu_time);
    }



    // Завершаем работу с новым коммуникатором
    MPI_Comm_free(&new_comm);
    MPI_Group_free(&world_group);
    MPI_Group_free(&new_group);

    MPI_Finalize();
    return 0;
}

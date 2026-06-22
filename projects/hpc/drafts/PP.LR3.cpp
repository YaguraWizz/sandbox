#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>

class Contex {
public:
    Contex(int* argc, char*** argv) { MPI_Init(argc, argv); }
    ~Contex() { MPI_Finalize(); }
};

using Matrix = std::vector<std::vector<double>>;

// Загрузка матрицы из файла
static Matrix load_matrix(const std::string& path) {
    std::ifstream input(path);

    if (!input.is_open()) {
        std::cerr << "Error opening file from path: " << path << std::endl;
        throw std::runtime_error("Failed to open file");
    }

    int rows, cols;
    input >> rows >> cols;

    Matrix matrix(rows, std::vector<double>(cols));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (!(input >> matrix[i][j])) {
                throw std::runtime_error("Invalid data format in file");
            }
        }
    }

    return matrix;
}

// Транспонирование матрицы
static Matrix transpose(const Matrix& U) {
    int n = U.size();
    Matrix UT(n, std::vector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            UT[i][j] = U[j][i];
        }
    }
    return UT;
}

// Функция умножения двух матриц
static void multiply_matrices(const Matrix& A, const Matrix& B, Matrix& buffer, int rank, int size) {
    int N = A.size();
    int chunk_size = N / size;
    int start = chunk_size * rank;
    int stop = chunk_size * (rank + 1);
    if (rank == size - 1) stop = N;

    buffer.resize(N, std::vector<double>(N, 0.0));

    for (int i = start; i < stop; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                buffer[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Функция печати матрицы
static void print_matrix(const Matrix& matrix, const std::string& msg = "") {
    int N = matrix.size();
    if (!msg.empty()) { std::cout << msg << " "; }

    std::cout << "Size: " << matrix.size() << ", " << matrix[0].size() << "\n";
    for (int j = 0; j < N; j++) {
        std::cout << "row [" << j << "]: ";
        for (int k = 0; k < N; k++) {
            std::cout << matrix[j][k] << ", ";
        }
        std::cout << "\n";
    }
}

int main(int argc, char** argv) {
    Contex contex(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Matrix A, B, C, buffer, result;
    int matrix_size;

    // Время начала инициализации
    double start_time = MPI_Wtime();
    double io_start_time = 0.0, io_end_time = 0.0;
    double compute_start_time = 0.0, compute_end_time = 0.0;
    double gather_start_time = 0.0, gather_end_time = 0.0;

    if (rank == 0) {
        io_start_time = MPI_Wtime(); // Начало загрузки данных
        std::string base_path = "D:\\Dev\\temp_project\\UNIVER_LABS\\PP.LR2\\Debug\\data\\test_4_x_4";

        std::cout << "Enter the path to the directory: ";
        std::cin >> base_path;

        A = load_matrix(base_path + "\\A.txt");
        B = load_matrix(base_path + "\\B.txt");
        C = load_matrix(base_path + "\\C.txt");
        io_end_time = MPI_Wtime(); // Конец загрузки данных

        if (A.empty() || B.empty() || C.empty() || A[0].size() != B.size() || B[0].size() != C.size()) {
            std::cerr << "Matrices are not compatible for multiplication!" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        matrix_size = A.size();
    }

    // Рассылка размера матрицы всем процессам
    MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Рассылка матриц всем процессам
    A.resize(matrix_size, std::vector<double>(matrix_size));
    B.resize(matrix_size, std::vector<double>(matrix_size));
    C.resize(matrix_size, std::vector<double>(matrix_size));
    for (int i = 0; i < matrix_size; ++i) {
        MPI_Bcast(A[i].data(), matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(B[i].data(), matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Bcast(C[i].data(), matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }

    compute_start_time = MPI_Wtime(); // Начало вычислений

    // Умножение Q = A * B
    multiply_matrices(A, B, buffer, rank, size);

    // Сборка результата умножения
    Matrix Q(matrix_size, std::vector<double>(matrix_size, 0));
    for (int i = 0; i < matrix_size; ++i) {
        MPI_Reduce(buffer[i].data(), Q[i].data(), matrix_size, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    // Умножение R = Q * C
    multiply_matrices(Q, C, buffer, rank, size);

    // Сборка финальной матрицы
    result.resize(matrix_size, std::vector<double>(matrix_size, 0));
    for (int i = 0; i < matrix_size; ++i) {
        MPI_Reduce(buffer[i].data(), result[i].data(), matrix_size, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    }

    compute_end_time = MPI_Wtime(); // Конец вычислений

    if (rank == 0) {
        // Общий вывод времени
        double total_time = MPI_Wtime() - start_time;
        double io_time = io_end_time - io_start_time;
        double compute_time = compute_end_time - compute_start_time;

        std::cout << "\n--- Time Profiling ---" << std::endl;
        std::cout << "I/O Time: " << io_time << " seconds (" << (io_time / total_time) * 100 << "%)" << std::endl;
        std::cout << "Computation Time: " << compute_time << " seconds (" << (compute_time / total_time) * 100 << "%)" << std::endl;
        std::cout << "Total Execution Time: " << total_time << " seconds" << std::endl;
    }

    return 0;
}

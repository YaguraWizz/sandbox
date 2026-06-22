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
    input >> rows >> cols;  // Чтение количества строк и столбцов

    Matrix matrix(rows, std::vector<double>(cols));

    // Чтение данных построчно
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
    int chunk_size = N / size;              // Размер подзадачи
    int start = chunk_size * rank;          // Начало итерации по строкам
    int stop = chunk_size * (rank + 1);     // Конец итерации по строкам
    if (rank == size - 1) stop = N;         // Последний процесс берет остаток

    buffer.resize(N, std::vector<double>(N, 0.0));

    // Умножение матриц A и B, результат сохраняется в buffer
    for (int i = start; i < stop; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                buffer[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Функция наложения данных (сборка матрицы)
static int overlay(Matrix& A, Matrix& B, int rank, int size) {
    int N = A.size();
    int chunk_size = N / size;              // Размер подзадачи
    int start = chunk_size * rank;          // Начало итерации по строкам
    int stop = chunk_size * (rank + 1);     // Конец итерации по строкам
    if (rank == size - 1) stop = N;         // Последний процесс берет остаток

    int index_last_value_or_not_zero = 0;

    // Копирование данных текущего процесса в итоговую матрицу
    for (int i = start; i < stop; ++i) {
        for (int j = 0; j < N; ++j) {
            if (A[i][j] != 0) {
                B[i][j] = A[i][j]; A[i][j] = 0;
            }
            else {
                index_last_value_or_not_zero = i;
            }
        }
    }
    return index_last_value_or_not_zero;
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


static Matrix add_matrices(const Matrix& A, const Matrix& B) {
    // Проверяем, что размеры матриц одинаковые
    if (A.size() != B.size() || A[0].size() != B[0].size()) {
        throw std::invalid_argument("Matrices must have the same dimensions.");
    }

    // Создаем матрицу для результата
    Matrix result(A.size(), std::vector<double>(A[0].size()));

    // Проходим по всем элементам матриц и складываем их
    for (size_t i = 0; i < A.size(); ++i) {
        for (size_t j = 0; j < A[0].size(); ++j) {
            result[i][j] = A[i][j] + B[i][j];
        }
    }

    return result;
}


int main(int argc, char** argv) {
    Contex contex(&argc, &argv);  // Инициализация MPI
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  // Получение ранга текущего процесса
    MPI_Comm_size(MPI_COMM_WORLD, &size);  // Получение количества процессов
    Matrix A, B, C, Q, buffer, result;
    int matrix_size;

    if (rank == 0) {
        // Загрузка данных в процессе 0
        std::string base_path = "D:\\Dev\\temp_project\\UNIVER_LABS\\PP.LR2\\Debug\\data\\test_4_x_4\\";
        A = load_matrix(base_path + "A.txt");
        B = load_matrix(base_path + "B.txt");
        C = load_matrix(base_path + "C.txt");

        // Проверка валидности матриц
        if (A.empty() || B.empty() || C.empty() || A[0].size() != B.size() || B[0].size() != C.size()) {
            std::cerr << "Matrices are not compatible for multiplication!" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        transpose(B);
        transpose(C);

        matrix_size = A.size();

        // Отправка размера матриц всем процессам
        for (int i = 1; i < size; ++i) {
            MPI_Send(&matrix_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        // Отправка данных матриц всем процессам
        for (int i = 1; i < size; ++i) {
            for (int j = 0; j < matrix_size; ++j) {
                MPI_Send(A[j].data(), matrix_size, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
                MPI_Send(B[j].data(), matrix_size, MPI_DOUBLE, i, 2, MPI_COMM_WORLD);
                MPI_Send(C[j].data(), matrix_size, MPI_DOUBLE, i, 3, MPI_COMM_WORLD);
            }
        }

    }
    else {
        // Получение размера матрицы
        MPI_Recv(&matrix_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Инициализация матриц
        A.resize(matrix_size, std::vector<double>(matrix_size));
        B.resize(matrix_size, std::vector<double>(matrix_size));
        C.resize(matrix_size, std::vector<double>(matrix_size));

        // Получение данных матриц
        for (int i = 0; i < matrix_size; ++i) {
            MPI_Recv(A[i].data(), matrix_size, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(B[i].data(), matrix_size, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(C[i].data(), matrix_size, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    // Q = A * B
    multiply_matrices(A, B, buffer, rank, size);



    // собираем матрицу Q
    if (rank == 0) {
        Q.resize(matrix_size, std::vector<double>(matrix_size, 0));
        auto index = overlay(buffer, Q, rank, size);

        for (int rank_ = 1; rank_ < size; ++rank_) {
            for (int i = 0; i < matrix_size; ++i) {
                auto pointer = std::next(buffer[i].data(), index);
                int code = MPI_Recv(pointer, matrix_size, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
                if (code != MPI_SUCCESS) {
                    std::cout << "Error recv, rank: " << rank;
                }
            }
            print_matrix(buffer, "Final Matrix buffer");
            print_matrix(Q, "Final Matrix Q");
            Q = add_matrices(Q, buffer);
        }
        print_matrix(Q, "\n\nFinal Matrix Q");


        for (int i = 1; i < size; ++i) {
            for (int j = 0; j < matrix_size; ++j) {
                MPI_Send(Q[j].data(), matrix_size, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
            }
        }
    }
    else {
        for (int j = 0; j < matrix_size; ++j) {
            MPI_Send(buffer[j].data(), matrix_size, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD);
        }

        Q.resize(matrix_size, std::vector<double>(matrix_size, 0));
        for (int i = 0; i < matrix_size; ++i) {
            int code = MPI_Recv(Q[i].data(), matrix_size, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
            if (code != MPI_SUCCESS) {
                std::cout << "Error recv, rank: " << rank;
            }
        }
    }


    // R = Q * C
    multiply_matrices(Q, C, buffer, rank, size);

    if (rank == 0) {
        result.resize(matrix_size, std::vector<double>(matrix_size, 0));
        auto index = overlay(buffer, result, rank, size);
        for (int rank_ = 1; rank_ < size; ++rank_) {
            for (int i = 0; i < matrix_size; ++i) {
                auto pointer = std::next(buffer[i].data(), index);
                int code = MPI_Recv(pointer, matrix_size, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
                if (code != MPI_SUCCESS) {
                    std::cout << "Error recv, rank: " << rank;
                }
            }
            result = add_matrices(result, buffer);
        }
        print_matrix(result, "\nFinal Matrix result");
    }
    else {
        for (int j = 0; j < matrix_size; ++j) {
            MPI_Send(buffer[j].data(), matrix_size, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD);
        }
    }

    return 0;
}

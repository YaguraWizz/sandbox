#pragma comment(lib, "cublas.lib")


#include "cuda.h"
#include "cuda_runtime.h"
#include "cublas_v2.h"

#include <type_traits>
#include <stdexcept>
#include <iostream>
#include <cstddef>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <random>  
#include <omp.h>
#include <cmath>

constexpr int BLOCK_SIZE = 1024;


namespace rizz {

    class timer_chrono {
    private:
        std::chrono::steady_clock::time_point start_time;
    public:
        void start() {
            start_time = std::chrono::steady_clock::now();
        }

        double stop() const {
            auto end_time = std::chrono::steady_clock::now();
            std::chrono::duration<double> duration = end_time - start_time;
            return duration.count();
        }
    };

    class timer_open_mp {
    private:
        double start_time;
    public:
        void start() {
            start_time = omp_get_wtime();
        }

        double stop() const {
            return omp_get_wtime() - start_time;
        }
    };


    template<typename T>
    class cuda_shared_ptr {
        T* _device_ptr = nullptr;
        size_t _size_range = 0;

    public:
        explicit cuda_shared_ptr(const std::vector<T>& vec)
            : cuda_shared_ptr(vec.data(), vec.size()) {
        }

        explicit cuda_shared_ptr(const T* raw_array, size_t size) {
            if (!raw_array) {
                std::cerr << "RUNTIME_ERROR: Attempt to allocate from a null pointer\n";
                return;
            }
            if (size == 0) {
                std::cerr << "RUNTIME_ERROR: Attempt to allocate with size 0\n";
                return;
            }
            _size_range = size;
            cudaError_t err = cudaMalloc(&_device_ptr, _size_range * sizeof(T));
            if (!error_handler(err, "cudaMalloc failed")) {
                _device_ptr = nullptr;
                _size_range = 0;
            }
            else {
                copy_to_device(raw_array, _size_range);
            }
        }

        explicit cuda_shared_ptr(size_t size, T value) {
            if (size == 0) {
                std::cerr << "RUNTIME_ERROR: Attempt to allocate with size 0\n";
                return;
            }
            _size_range = size;
            cudaError_t err = cudaMalloc(&_device_ptr, _size_range * sizeof(T));
            if (!error_handler(err, "cudaMalloc failed")) {
                _device_ptr = nullptr;
                _size_range = 0;
                return;
            }
            std::vector<T> temp(size, value);
            copy_to_device(temp.data(), size);
        }

        ~cuda_shared_ptr() {
            if (_device_ptr) {
                cudaFree(_device_ptr);
            }
        }

        const T* data() const noexcept { return _device_ptr; }
        T* data() noexcept { return _device_ptr; }
        size_t size() const noexcept { return _size_range; }

        bool copy_to_device(const T* host_ptr, size_t size) {
            if (!validate_ptrs(host_ptr, "Host pointer is null") || !validate_size(size) || !_device_ptr) {
                std::cerr << "RUNTIME_ERROR: Device pointer is null\n";
                return false;
            }
            cudaError_t err = cudaMemcpy(_device_ptr, host_ptr, size * sizeof(T), cudaMemcpyHostToDevice);
            return error_handler(err, "cudaMemcpy to device failed");
        }

        bool copy_from_device(T* host_ptr, size_t size) {
            if (!validate_ptrs(host_ptr, "Host pointer is null") || !validate_size(size) || !_device_ptr) {
                std::cerr << "RUNTIME_ERROR: Device pointer is null\n";
                return false;
            }
            cudaError_t err = cudaMemcpy(host_ptr, _device_ptr, size * sizeof(T), cudaMemcpyDeviceToHost);
            return error_handler(err, "cudaMemcpy from device failed");
        }

    private:
        bool validate_ptrs(const T* ptr, const std::string& msg) const {
            if (!ptr) {
                std::cerr << "RUNTIME_ERROR: " << msg << "\n";
                return false;
            }
            return true;
        }

        bool validate_size(size_t size) const {
            if (size > _size_range) {
                std::cerr << "RUNTIME_ERROR: Requested size " << size
                    << " exceeds allocated size " << _size_range << "\n";
                return false;
            }
            return true;
        }

        bool error_handler(cudaError_t error, const std::string& msg) const {
            if (error != cudaSuccess) {
                std::cerr << "RUNTIME_ERROR: " << msg << ": " << cudaGetErrorString(error) << "\n";
                return false;
            }
            return true;
        }
    };



    template <typename Tptr>
    static bool comput_range_v4(const Tptr* range1, const Tptr* range2, const Tptr* range3, const Tptr* range4, size_t size) {
        if (!range1 || !range2 || !range3 || !range4) {
            std::cerr << "RUNTIME_ERROR: Null pointer encountered in comput_range_v4\n";
            return false;
        }

        for (size_t i = 0; i < size; ++i) {
            int a_int = static_cast<int>(range1[i]);
            int b_int = static_cast<int>(range2[i]);
            int c_int = static_cast<int>(range3[i]);
            int d_int = static_cast<int>(range4[i]);

            if (a_int != b_int || a_int != c_int) {
                std::cout << "ERROR. range1: " << range1[i] << ", range2: " << range2[i]
                    << ", range3: " << range3[i] << ", range4: " << range4[i] << "\n";
                std::cout << "Integer parts: range1: " << a_int << ", range2: " << b_int
                    << ", range3: " << c_int << ", range4: " << d_int << "\n\n";
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
    static void generate_matrix(T* matrix, size_t size) {
        if (!matrix) {
            throw std::runtime_error("Error: matrix pointer is null");
        }

        std::random_device rd;
        std::mt19937 gen(rd());

        if constexpr (std::is_integral_v<T>) {
            std::uniform_int_distribution<T> distrib(0, 99);
            for (size_t i = 0; i < size; ++i) {
                matrix[i] = distrib(gen);
            }
        }
        else if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> distrib(0.0, 99.0);
            for (size_t i = 0; i < size; ++i) {
                matrix[i] = distrib(gen);
            }
        }
        else {
            throw std::runtime_error("Error: Unsupported type for generate_matrix");
        }
    }

    static void get_gpu_info() {
        int deviceCount = 0;
        cudaGetDeviceCount(&deviceCount);

        if (deviceCount == 0) {
            std::cerr << "No CUDA devices available." << std::endl;
            return;
        }

        for (int device = 0; device < deviceCount; ++device) {
            cudaDeviceProp prop;
            cudaGetDeviceProperties(&prop, device);

            std::cout << "Device " << device << ": " << prop.name << std::endl;
            std::cout << "  Total memory: " << prop.totalGlobalMem / (1024 * 1024) << " MB" << std::endl;
            std::cout << "  Multiprocessor count: " << prop.multiProcessorCount << std::endl;
            std::cout << "  Max threads per block: " << prop.maxThreadsPerBlock << std::endl;
            std::cout << "  Max threads per multiprocessor: " << prop.maxThreadsPerMultiProcessor << std::endl;
            std::cout << "  Max block dimensions: ("
                << prop.maxThreadsDim[0] << ", "
                << prop.maxThreadsDim[1] << ", "
                << prop.maxThreadsDim[2] << ")" << std::endl;
            std::cout << "  Max grid size: ("
                << prop.maxGridSize[0] << ", "
                << prop.maxGridSize[1] << ", "
                << prop.maxGridSize[2] << ")\n" << std::endl;
        }
    }
}


// c = a * b
// blockIdx - номер блока, blockDim - размер блока(количество потоков), 
// threadIdx - номер потока внутри блока
// gridDim - размер сетки(количество блоков)
__global__ void matrix_mult_gpu(float* A, float* B, float* C, size_t N)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    // for(int i=0; i<N; i++)
    while (i < N)
    {
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
        i += gridDim.x * blockDim.x;
    }
}

#if 0
template<typename T>
__global__ void matrix_mult_gpu_shared(T* A, T* B, T* C, size_t N)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    __shared__ float temp;

    while (i < N)
    {
        for (int j = 0; j < N; j++)
        {
            temp = 0;
            for (int k = 0; k < N; k++) {
                temp += A[i * N + k] * B[k * N + j];
            }

            C[i * N + j] = temp;
        }
        i += gridDim.x * blockDim.x;
    }
}
#endif // 0


__global__ void matrix_mult_gpu_shared(float* A, float* B, float* C, size_t N)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    // Предполагаем, что размер блока не превышает 1024.
    __shared__ float temp[BLOCK_SIZE];

    while (i < N)
    {
        for (int j = 0; j < N; j++)
        {
            // Каждый поток инициализирует свой элемент в shared memory.
            temp[threadIdx.x] = 0.0f;
            __syncthreads();  // Синхронизация: убеждаемся, что все потоки проинициализировали свои ячейки.

            // Вычисляем скалярное произведение i-й строки A и j-го столбца B.
            for (int k = 0; k < N; k++) {
                temp[threadIdx.x] += A[i * N + k] * B[k * N + j];
            }
            __syncthreads();  // Синхронизация: убеждаемся, что вычисления завершены во всех потоках.

            // Каждый поток записывает свой результат в глобальную память.
            C[i * N + j] = temp[threadIdx.x];
            __syncthreads();  // Синхронизация перед переходом к следующей итерации j.
        }
        i += gridDim.x * blockDim.x;
    }
}

// cuBLAS matrix multiplication
void matrix_mult_gpu_cuBLAS(float* d_A, float* d_B, float* d_C, int N) {
    cublasHandle_t handle;
    cublasCreate(&handle);
    float alpha = 1.0f, beta = 0.0f;
    cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N, N, N, N, &alpha, d_A, N, d_B, N, &beta, d_C, N);
    cublasDestroy(handle);
}



static void matrix_mult_cpu(const float* matrixA, const float* matrixB, float* matrixR, size_t size) {
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


std::vector<float> transpose(const float* matrixData, size_t n) {
    std::vector<float> result(n * n);
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            // Элемент (i, j) исходной матрицы становится (j, i) в транспонированной матрице.
            result[j * n + i] = matrixData[i * n + j];
        }
    }
    return result;
}


static void benchmark(const float* matrixA, const float* matrixB, const float* matrixC, float* matrixR, size_t matrix_size) {
    if (!matrixA || !matrixB || !matrixC || !matrixR) {
        std::cerr << "Error nullpointer matrix benchmark. MATRIX SIZE: "<< matrix_size << "\n";
        return;
    }
    rizz::timer_open_mp timer{};
    double cpu_time = 0.0;
    double gpu_time = 0.0;
    double gpu_time_shared = 0.0;
    double gpu_time_cuBLAS = 0.0;

    size_t count_thead = 0;
    size_t count_block = 0;

    std::vector<float> temp_matrixR_cpu{};
    std::vector<float> temp_matrixR_gpu{};
    std::vector<float> temp_matrixR_gpu_shared{};
    std::vector<float> temp_matrixR_gpu_cuBLAS{};


    temp_matrixR_cpu.resize(matrix_size * matrix_size, 0);
    temp_matrixR_gpu.resize(matrix_size * matrix_size, 0);
    temp_matrixR_gpu_shared.resize(matrix_size * matrix_size, 0);
    temp_matrixR_gpu_cuBLAS.resize(matrix_size * matrix_size, 0);

#pragma region [BENCHMARK GPU1]
    {
        timer.start();
        rizz::cuda_shared_ptr<float> d_A{ matrixA, matrix_size * matrix_size };
        rizz::cuda_shared_ptr<float> d_B{ matrixB, matrix_size * matrix_size };
        rizz::cuda_shared_ptr<float> d_C{ matrixC, matrix_size * matrix_size };

        rizz::cuda_shared_ptr<float> d_M{ temp_matrixR_gpu.data(), matrix_size * matrix_size};
        rizz::cuda_shared_ptr<float> d_D{ temp_matrixR_gpu.data(), matrix_size * matrix_size };

     
        dim3 block(BLOCK_SIZE);
        dim3 grid((static_cast<int>(matrix_size) + BLOCK_SIZE - 1) / BLOCK_SIZE);

        // асинхронный вызов ядра cuda
        count_thead = block.x;  // Количество потоков в одном блоке
        count_block = grid.x;   // Количество блоков в сетке

        matrix_mult_gpu <<<grid, block >>> (d_A.data(), d_B.data(), d_M.data(), matrix_size);
        matrix_mult_gpu <<<grid, block >>> (d_M.data(), d_C.data(), d_D.data(), matrix_size);

        // Проверка успешности копирования данных с устройства
        bool copy_success = d_D.copy_from_device(temp_matrixR_gpu.data(), matrix_size * matrix_size);
        if (!copy_success) {
            printf("CUDA Error: Failed to copy data from device\n");
            std::abort();
        }
        gpu_time = timer.stop();
    }
#pragma endregion


#pragma region [BENCHMARK GPU1]
    {
        timer.start();
        rizz::cuda_shared_ptr<float> d_A{ matrixA, matrix_size * matrix_size };
        rizz::cuda_shared_ptr<float> d_B{ matrixB, matrix_size * matrix_size };
        rizz::cuda_shared_ptr<float> d_C{ matrixC, matrix_size * matrix_size };

        rizz::cuda_shared_ptr<float> d_M{ temp_matrixR_gpu_shared.data(), matrix_size * matrix_size };
        rizz::cuda_shared_ptr<float> d_D{ temp_matrixR_gpu_shared.data(), matrix_size * matrix_size };


        dim3 block(BLOCK_SIZE);
        dim3 grid((static_cast<int>(matrix_size) + BLOCK_SIZE - 1) / BLOCK_SIZE);

        // асинхронный вызов ядра cuda
        count_thead = block.x;  // Количество потоков в одном блоке
        count_block = grid.x;   // Количество блоков в сетке

        matrix_mult_gpu_shared <<<grid, block >>> (d_A.data(), d_B.data(), d_M.data(), matrix_size);
        matrix_mult_gpu_shared <<<grid, block >>> (d_M.data(), d_C.data(), d_D.data(), matrix_size);

        // Проверка успешности копирования данных с устройства
        bool copy_success = d_D.copy_from_device(temp_matrixR_gpu_shared.data(), matrix_size * matrix_size);
        if (!copy_success) {
            printf("CUDA Error: Failed to copy data from device\n");
            std::abort();
        }
        gpu_time_shared = timer.stop();
    }
#pragma endregion


#pragma region [BENCHMARK cuBLAC]
    {
        timer.start();

        auto matrixA_T = transpose(matrixA, matrix_size);
        auto matrixB_T = transpose(matrixB, matrix_size);
        auto matrixC_T = transpose(matrixC, matrix_size);


        rizz::cuda_shared_ptr<float> d_A{ matrixA_T.data(), matrix_size * matrix_size};
        rizz::cuda_shared_ptr<float> d_B{ matrixB_T.data(), matrix_size * matrix_size };
        rizz::cuda_shared_ptr<float> d_C{ matrixC_T.data(), matrix_size * matrix_size };

        rizz::cuda_shared_ptr<float> d_M{ temp_matrixR_gpu_cuBLAS.data(), matrix_size * matrix_size };
        rizz::cuda_shared_ptr<float> d_D{ temp_matrixR_gpu_cuBLAS.data(), matrix_size * matrix_size };


        matrix_mult_gpu_cuBLAS(d_A.data(), d_B.data(), d_M.data(), static_cast<int>(matrix_size));
        matrix_mult_gpu_cuBLAS(d_M.data(), d_C.data(), d_D.data(), static_cast<int>(matrix_size));


        // Проверка успешности копирования данных с устройства
        bool copy_success = d_D.copy_from_device(temp_matrixR_gpu_cuBLAS.data(), matrix_size * matrix_size);

        auto rezult = transpose(temp_matrixR_gpu_cuBLAS.data(), matrix_size);
        std::swap(rezult, temp_matrixR_gpu_cuBLAS);

        if (!copy_success) {
            printf("CUDA Error: Failed to copy data from device\n");
            std::abort();
        }
        gpu_time_cuBLAS = timer.stop();
    }
#pragma endregion


#pragma region [BENCHMARK CPU]
    {
        timer.start();
        std::vector<float> temp_matrix{};
        temp_matrix.resize(matrix_size * matrix_size, 0);
        matrix_mult_cpu(matrixA, matrixB, temp_matrix.data(), matrix_size);
        matrix_mult_cpu(temp_matrix.data(), matrixC, temp_matrixR_cpu.data(), matrix_size);
        cpu_time = timer.stop();
    }
#pragma endregion



    if (!rizz::comput_range_v4(temp_matrixR_cpu.data(), temp_matrixR_gpu.data(), temp_matrixR_gpu_shared.data(), temp_matrixR_gpu_cuBLAS.data(), matrix_size * matrix_size)) {
        std::cerr << "Error math mylt matrix\n";
        std::cerr << " CPU matrix\n";
        rizz::print_matrix(temp_matrixR_cpu.data(), matrix_size * matrix_size, 2);
        
        std::cerr << "\n";  
        std::cerr << " GPU matrix\n";
        rizz::print_matrix(temp_matrixR_gpu.data(), matrix_size* matrix_size, 2);

        std::cerr << "\n";
        std::cerr << " GPU shared matrix\n";
        rizz::print_matrix(temp_matrixR_gpu_shared.data(), matrix_size* matrix_size, 2);

        std::cerr << "\n";
        std::cerr << " GPU cuBLAS matrix\n";
        rizz::print_matrix(temp_matrixR_gpu_cuBLAS.data(), matrix_size* matrix_size, 2);
    }
    else {
        std::cout << "Size: " << matrix_size << " | CPU: " << cpu_time << "s | GPU: " << gpu_time << "s | GPU Shared: " << gpu_time_shared << "s | GPU cuBLAS: " << gpu_time_cuBLAS << "s\n";
        std::cout << "Threads per block (count_thead): " << count_thead << "\n";
        std::cout << "Number of blocks (count_block): " << count_block << "\n";
    }
}

#define ENABLE(enabled) if(enabled) 

int main(int argc, char** argv) {
    rizz::get_gpu_info();
    rizz::timer_open_mp total_timer{};
    rizz::timer_open_mp temp_timer{};
    total_timer.start();

    rizz::timer_open_mp generate_timer{};
    std::vector<size_t> _size{ 10, 100 };

    for (auto size : _size)
    {
        temp_timer.start();
        size_t total_size = size * size;
        std::vector<float> A(total_size, 1), B(total_size, 10), C(total_size, 1), R(total_size, 0);
        std::cout << "Start generated size: " << total_size<<". ";


        {
            generate_timer.start();
            rizz::generate_matrix(A.data(), total_size);
            double time = generate_timer.stop();
            std::cout << "MatrixA: " << time << " | ";
            //rizz::print_matrix(A.data(), total_size, 2);
        }
       
        {
            generate_timer.start();
            rizz::generate_matrix(B.data(), total_size);
            double time = generate_timer.stop();
            std::cout << "MatrixB: " << time << " | ";
            //rizz::print_matrix(B.data(), total_size, 2);
        }

        {
            generate_timer.start();
            rizz::generate_matrix(C.data(), total_size);
            double time = generate_timer.stop();
            std::cout << "MatrixC: " << time << "\n";
            //rizz::print_matrix(C.data(), total_size, 2);
        }

        {
            std::cout << "MatrixR: " << "\n";
            //rizz::print_matrix(R.data(), total_size, 2);
        }


        benchmark(A.data(), B.data(), C.data(), R.data(), size);
        std::cout << "total benchmark timer: " << temp_timer.stop() << "\n\n";
    }

    std::cout << "TOTAL TIMER: " << total_timer.stop() << "\n";
    return 0;
}



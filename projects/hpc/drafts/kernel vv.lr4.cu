#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <omp.h>

#include <cuda.h>
#include <cuda_runtime.h>

using std::cout;
using std::cin;
using std::vector;
using std::endl;

// CUDA ядро для сортировки вставками
__global__ void insertionSortKernel(int* arr, int length) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < length) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// Функция для сортировки вставками на CPU с OpenMP
void insertionSortCPU(int* arr, int length) {
#pragma omp parallel for
    for (int i = 1; i < length; ++i) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}


// Функция для генерации массива
int* generateArray(int length) {
    int* arr = new int[length];
    for (int i = 0; i < length; ++i) {
        arr[i] = rand() % 1000;
    }
    return arr;
}

int main() {
    int max_volume = 200000; // Максимальный объем данных


    std::ofstream dataFile; // Файл для записи результатов
    dataFile.open("sort_data_cuda_cpu.csv", std::ios::app); // Открываем файл в режиме добавления
    if (!dataFile.is_open()) {
        std::cerr << "Ошибка открытия файла для записи!" << std::endl;
        return 1;
    }

    // Записываем заголовок в файл, если он пустой
    if (dataFile.tellp() == 0) {
        dataFile << "Volume,TimeCPU,TimeGPU,Speedup\n";
    }

    // Вывод заголовка таблицы в консоль
    cout << std::setw(14) << " Объем данных " << " | "
        << std::setw(15) << " Время CPU (мс) " << " | "
        << std::setw(15) << " Время GPU (мс) " << " | "
        << std::setw(10) << " Ускорение " << endl;

    // Цикл по разным объемам данных
    for (int volume = 10; volume <= max_volume; volume += 10000) {
        int* arr_cpu = generateArray(volume);
        int* arr_gpu = generateArray(volume);


        //CPU sorting with OpenMP
        auto start_time_cpu = std::chrono::high_resolution_clock::now();
        insertionSortCPU(arr_cpu, volume);
        auto end_time_cpu = std::chrono::high_resolution_clock::now();
        double elapsed_time_cpu = std::chrono::duration<double, std::milli>(end_time_cpu - start_time_cpu).count();



        int* d_arr; // Указатель на массив на GPU
        cudaMalloc(&d_arr, volume * sizeof(int)); // Выделяем память на GPU
        cudaMemcpy(d_arr, arr_gpu, volume * sizeof(int), cudaMemcpyHostToDevice); // Копируем данные с CPU на GPU

        auto start_time_gpu = std::chrono::high_resolution_clock::now(); // Засекаем время начала сортировки

        int threadsPerBlock = 256; // Количество потоков в блоке (можно подбирать)
        int blocksPerGrid = (volume + threadsPerBlock - 1) / threadsPerBlock; // Количество блоков в гриде

        // Запускаем ядро CUDA
        insertionSortKernel << <blocksPerGrid, threadsPerBlock >> > (d_arr, volume);

        cudaDeviceSynchronize(); // Ждем завершения работы ядра

        auto end_time_gpu = std::chrono::high_resolution_clock::now(); // Засекаем время окончания сортировки
        double elapsed_time_gpu = std::chrono::duration<double, std::milli>(end_time_gpu - start_time_gpu).count(); // Вычисляем время сортировки

        int* sorted_arr_gpu = new int[volume]; // Массив для хранения отсортированных данных на CPU
        cudaMemcpy(sorted_arr_gpu, d_arr, volume * sizeof(int), cudaMemcpyDeviceToHost); // Копируем данные с GPU на CPU


        double speedup = elapsed_time_cpu / elapsed_time_gpu;

        // Записываем результаты в файл
        dataFile << volume << "," << elapsed_time_cpu << "," << elapsed_time_gpu << "," << speedup << "\n";
            // Выводим результаты в консоль
            cout << std::setw(14) << volume << " | "
            << std::setw(15) << std::fixed << std::setprecision(6) << elapsed_time_cpu << " | "
            << std::setw(15) << std::fixed << std::setprecision(6) << elapsed_time_gpu << " | "
            << std::setw(10) << std::fixed << std::setprecision(2) << speedup << endl;

        delete[] arr_cpu;
        delete[] arr_gpu;
        delete[] sorted_arr_gpu; // Освобождаем память на CPU
        cudaFree(d_arr); // Освобождаем память на GPU
    }

    dataFile.close(); // Закрываем файл
    cout << "Данные сохранены в файл sort_data_cuda_cpu.csv" << endl;

    return 0;
}
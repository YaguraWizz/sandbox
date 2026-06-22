#include <cstdio>
#include <cmath>
#include <list>
#include <array>
#include <chrono>
#include <vector>
#include <mpi.h>
#include <fstream> 
#include <iostream>
#include <string>


// command start mpiexec -n 1 .\VV.LR3.exe  
namespace LR3 {

    // Генерация списка значений от 0 до size - 1
    std::vector<int> generateList(size_t size) {
        std::vector<int> lis(size);
        for (size_t i = 0; i < size; ++i) {
            lis[i] = i; 
        }
        return lis;
    }

    // Параллельный линейный поиск в подсписке
    static int parallelLinearSearch(const std::vector<int>& sublist, int target) {
        for (size_t i = 0; i < sublist.size(); ++i) {
            if (sublist[i] == target) {
                return i; // Возвращаем индекс в подсписке
            }
        }
        return -1; // Если не найдено
    }

    // Класс для замера времени выполнения
    class Timer {
    public:
        Timer(std::vector<double>& duration)
            : start(std::chrono::high_resolution_clock::now()), duration_(duration) {}

        ~Timer() {
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            duration_.push_back(duration.count());
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        std::vector<double>& duration_;
    };

    // Запуск тестирования
    static void start_test() {
        int numtasks, rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

        std::array<size_t, 8> degrees{ 0, 1, 2, 3, 4, 5, 6, 7 };
        std::vector<double> duration_;
        int target = 0;
        for (size_t i = 0; i < degrees.size(); ++i) {
            size_t dataSize = static_cast<size_t>(6 * pow(10, degrees[i]));
            std::vector<int> fullList;

            // Генерация полного списка на процессе 0
            if (rank == 0) {
                fullList = generateList(dataSize);
                target = fullList.back();
            }

            // Определяем размер подсписка для каждого процесса
            size_t localSize = dataSize / numtasks;
            std::vector<int> sublist(localSize);

            // Распределение данных между процессами
            MPI_Scatter(fullList.data(), localSize, MPI_INT, sublist.data(), localSize, MPI_INT, 0, MPI_COMM_WORLD);

        
            {
                Timer timer(duration_);
                int localIndex = parallelLinearSearch(sublist, target);
            }

            // Синхронизация всех процессов после завершения тестирования
            MPI_Barrier(MPI_COMM_WORLD);
        }

        // Вывод результатов на процессе 0 в файл
        if (rank == 0) {
            std::string filename = "test_" + std::to_string(numtasks) + "_results.txt";
            std::ofstream outFile(filename, std::ios::app);
            if (outFile.is_open()) {
                outFile << "Durations for each test:\n";
                for (const auto& duration : duration_) {
                    outFile << duration << " ms\n"; 
                }
                outFile.close(); 
                std::cout << "Completed: " << numtasks << "\n";
            }
            else {
                std::cerr << "Unable to open file for writing." << std::endl; 
            }
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    LR3::start_test(); 

    MPI_Finalize();
    return 0;
}

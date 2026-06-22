#include "matplotlibcpp.h"

#include <list>
#include <iostream>
#include <chrono>
#include <iomanip>

namespace LR1 {
    namespace plt = matplotlibcpp;
    // Функция линейного поиска
    static int linearSearch(const std::list<int>& arr, int target) {
        int index = 0;
        for (const auto& element : arr) {
            if (element == target) {
                return index;
            }
            ++index;
        }
        return -1;
    }

    // Функция для генерации списка
    static std::list<int> generateList(size_t size) {
        std::list<int> lst;
        for (int i = 0; i < size; ++i) {
            lst.push_back(i);
        }
        return lst;
    }

    // Функция для выполнения тестов и вывода результатов
    static void start_test() {
        int V = 1;  // Базовое значение для увеличения
        int N = 7;  // Увеличим количество шагов для более детализированного графика

        std::vector<double> log_sizes, durations;
        for (int i = 1; i <= N; ++i) {
            size_t size = static_cast<size_t>(V * pow(10, i));
            std::list<int> data = generateList(size);
            int target = data.back();

            auto start = std::chrono::high_resolution_clock::now();
            linearSearch(data, target);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;  // Используем миллисекунды

            log_sizes.push_back(log10(size));  // Логарифмируем размер списка
            durations.push_back(duration.count());
        }

        // Выводим заголовки для колонок
        std::cout << std::fixed;  // Используем фиксированную точность
        std::cout << "Logarithm of list size (log10)\tExecution time (ms)\n";
        std::cout << "----------------------------\t-------------------------\n";

        for(size_t index = 0; index < log_sizes.size(); ++index){
            // Форматируем вывод с точностью до 5 знаков после запятой
            std::cout << std::setprecision(5) <<"\t"<< log_sizes[index] << "\t\t\t" << durations[index] << "\n";
        }

        plt::plot(log_sizes, durations, "r-o");  // Красная линия с маркерами
        plt::xlabel("Логарифм размера списка (log10)");
        plt::ylabel("Время выполнения (мс)");
        plt::title("Зависимость времени линейного поиска от логарифма размера списка");
        plt::grid(true);
        plt::show();  
    }
}


int main() {
    LR1::start_test();
    system("pause");
    return 0;
}

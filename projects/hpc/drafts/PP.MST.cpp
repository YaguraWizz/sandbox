#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include "Benchmark.h"



int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");

    try {
        std::cout << "Добро пожаловать в программу по анализу расчета минимального остовного дерева. "
            << "Программа поддерживает линейный и параллельный расчеты с использованием OpenMP. "
            << "Для начала работы введите путь до каталога с графами. Файлы графов должны быть формата *.txt "
            << "и иметь следующее именование: graph_<size>.txt" << std::endl;

        std::string path;
        std::cin >> path;

        // Проверяем, существует ли указанный каталог
        if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
            std::cerr << "Ошибка: Указанный каталог не существует или не является директорией." << std::endl;
            return 1;
        }

        // Список для хранения найденных файлов
        std::vector<std::string> graph_files;

        // Поиск всех файлов формата graph_*.txt
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt" &&
                entry.path().filename().string().find("graph_") == 0) {
                graph_files.push_back(entry.path().string());
            }
        }

        // Проверяем, найдены ли файлы
        if (graph_files.empty()) {
            std::cerr << "Ошибка: В указанном каталоге не найдены файлы графов." << std::endl;
            return 1;
        }

        std::cout << "Найдено файлов графов: " << graph_files.size() << std::endl;

        // Создаем и запускаем бенчмарк
        mst::Benchmark benchmark{ graph_files, {1, 2, 3, 4} };

        std::cout << "Запуск Benchmark" << std::endl;
        benchmark.start();

    }
    catch (const std::exception& ex) {
        std::cerr << "Ошибка: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

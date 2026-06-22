#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "Timer.h"
#include "Graph.h"

namespace mst {

    struct Rapotr
    {
        int vertices;
        int edges;
        std::vector<double> timepoints{};

        void Print() const {
            std::cout << "Vertices: " << vertices << "\n";
            std::cout << "Edges: " << edges << "\n";
            std::cout << "Time points for MST (1, 2, 3, 4 threads): ";
            for (const auto& time : timepoints) {
                std::cout << time << " ";
            }
            std::cout << "\n\n";
        }
    };


    class Benchmark {
        std::vector<std::string> path_graph_file;
        std::vector<int> count_threads; // Количество потоков
    public:
        Benchmark(std::vector<std::string> path_current_file, const std::vector<int>& threads)
            : path_graph_file(std::move(path_current_file)), count_threads(threads) {}

        void start() const {
            if (path_graph_file.empty()) {
                std::cerr << "Ошибка: не найдено файлов графов для обработки.\n";
                return;
            }

            Timer timer;
            double time_point = 0.0;
            int vertices = 0;
            for (const auto& graph_file : path_graph_file) {
                Rapotr raport{};
                bool isParalel = false;
                auto [_vertices, _edges] = mst::Load(graph_file);
                raport.edges = static_cast<int>(_edges.size());
                raport.vertices = _vertices;

                for (int threads : count_threads) {
                    mst::Graph graph{ _vertices, _edges };
                    timer.reset();
                    graph.SearchMST(threads);
                    raport.timepoints.push_back(timer.elapsed());
                }
                raport.Print();
            }
        }
    };




    static void test_sort() {

        std::vector<int> rang{ 1, 2, 3, 4 };
        for (const int index : rang) {
            std::vector<int> arr(static_cast<int>(std::pow(10, index)));
            for (size_t i = 0; i < arr.size(); ++i) {
                arr[i] = rand();
            }

            for (int count : rang)
            {
                std::vector<int> copu = arr;
                omp_set_num_threads(count);
                // Запуск параллельной быстрой сортировки
                double start_time = omp_get_wtime();

                mst::bubbleSortParallel(copu);

                double end_time = omp_get_wtime();

                std::cout << "Размер: "<< copu.size() << ", время сортировки : " << (end_time - start_time) << " секунд\n";
            }
        }
    }

}

#include <list>
#include <iostream>
#include <sstream>
#include <chrono>
#include <omp.h>
#include <vector>
#include <cmath>
#include <fstream>


namespace LR2 {

    static int parallelLinearSearch(const std::list<int>& arr, int target) {
        int index = -1;  
        std::size_t arr_size = arr.size();
        std::vector<int> arr_vec(arr.begin(), arr.end()); 

#pragma omp parallel for shared(index)
        for (int i = 0; i < arr_size; ++i) {
            if (index == -1 && arr_vec[i] == target) {  
#pragma omp critical  
                {
                    if (index == -1) {
                        index = i;  
                    }
                }
            }
        }
        return index;
    }

    static std::list<int> generateList(size_t size) {
        std::list<int> lst;
        for (int i = 0; i < size; ++i) {
            lst.push_back(i);
        }
        return lst;
    }

    static std::ofstream create_ostream_file(const std::string& filename = "execution_time.csv") {
        std::ofstream file;
        file.open(filename, std::ios::out | std::ios::app);

        if (!file.is_open()) {
            std::cerr << "The file could not be opened for writing." << std::endl;
            return std::ofstream(); 
        }

        return file; 
    }

    static void start_test(int num_threads, std::ostream& output) {
        int V = 6;
        int N = 7;

        std::vector<double> log_sizes, durations;
        std::cout << "Number of running threads: " << num_threads << "\n";
        for (int i = 0; i <= N; ++i) {
            size_t size = static_cast<size_t>(V * pow(10, i));
            std::list<int> data = generateList(size);
            int target = data.back();

            // Установка количества потоков
            omp_set_num_threads(num_threads);

            auto start = std::chrono::high_resolution_clock::now();
            int found_index = parallelLinearSearch(data, target);
            auto end = std::chrono::high_resolution_clock::now();

            
            std::chrono::duration<double, std::milli> duration = end - start;

            log_sizes.push_back(log10(size));
            durations.push_back(duration.count());

            output << duration.count() << ", ";
            std::cout << "\tNumber index value search: "<< i << "\t" << duration.count() << "\n";
        }
        output << "\n";
        std::cout << std::string(50,'-') << "\n";
    }
}

int main() {
    std::vector<int> num_threads = { 1, 2, 3, 4 };
    std::ofstream output = LR2::create_ostream_file();

    for (auto num_thread : num_threads) {
        LR2::start_test(num_thread, output);
    }
    system("pause");
    return 0;
}

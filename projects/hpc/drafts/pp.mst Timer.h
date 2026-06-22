#pragma once
#include <chrono>


namespace mst {

    class Timer {
    public:
        Timer() { start_time = std::chrono::high_resolution_clock::now(); }

        void reset() { start_time = std::chrono::high_resolution_clock::now(); }

        double elapsed() const {
            auto end_time = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<double>(end_time - start_time).count();
        }

    private:
        std::chrono::high_resolution_clock::time_point start_time;
    };

}
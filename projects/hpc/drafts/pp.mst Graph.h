#include <omp.h>
#include <vector>
#include <string>
#include <fstream>
#include <numeric>
#include <algorithm>


namespace mst {
    class Edge {
    public:
        int u, v, weight;
        Edge() = default;

        Edge(int u, int v, int weight) : u(u), v(v), weight(weight) {}

        friend bool operator<(const Edge& lhs, const Edge& rhs) noexcept {
            return lhs.weight < rhs.weight;
        }

        friend bool operator>(const Edge& lhs, const Edge& rhs) noexcept {
            return lhs.weight > rhs.weight;
        }
    };

    template <typename T>
    void bubbleSortParallel(std::vector<T>& arr) {
        bool sorted = false;
        int n = static_cast<int>(arr.size());
        T tmp;
        while (!sorted) {
            sorted = true;

#pragma omp parallel private(tmp)
            {
                // Even phase
#pragma omp for reduction(&&:sorted)
                for (int i = 0; i < n - 1; i += 2) {
                    if (arr[i] > arr[i + 1]) {
                        tmp = arr[i];
                        arr[i] = arr[i + 1];
                        arr[i + 1] = tmp;
                        sorted = false;
                    }
                }

                // Odd phase
#pragma omp for reduction(&&:sorted)
                for (int i = 1; i < n - 1; i += 2) {
                    if (arr[i] > arr[i + 1]) {
                        tmp = arr[i];
                        arr[i] = arr[i + 1];
                        arr[i + 1] = tmp;
                        sorted = false;
                    }
                }
            }
        }
    }
    class Graph {
    public:
        Graph(int vertices, std::vector<Edge> new_edges)
            : vertices(vertices), edges(std::move(new_edges)), parent(vertices), rank(vertices, 0) {
            std::iota(parent.begin(), parent.end(), 0);  
        }

        std::vector<Edge> SearchMST(int count_threads) {
            resetDisjointSet();
            std::vector<Edge> mst;
            mst.reserve(static_cast<size_t>(vertices - 1)); 

            omp_set_num_threads(count_threads);
            bubbleSortParallel(edges);

            for (const auto& edge : edges) {
                if (find(edge.u) != find(edge.v)) {
                    unionSets(edge.u, edge.v);
                    mst.push_back(edge);
                }
            }

            return mst;
        }

        int Vertices() const noexcept { return vertices; }
        int Edges() const noexcept { return static_cast<int>(edges.size()); }

    private:
        int vertices;
        std::vector<int> parent;
        std::vector<int> rank;
        std::vector<Edge> edges;

        void resetDisjointSet() {
            std::iota(parent.begin(), parent.end(), 0);
            std::fill(rank.begin(), rank.end(), 0);
        }

        int find(int v) {
            if (parent[v] != v) {
                parent[v] = find(parent[v]); // Path compression
            }
            return parent[v];
        }

        void unionSets(int u, int v) {
            int rootU = find(u);
            int rootV = find(v);

            if (rootU != rootV) {
                if (rank[rootU] < rank[rootV]) {
                    parent[rootU] = rootV;
                }
                else if (rank[rootU] > rank[rootV]) {
                    parent[rootV] = rootU;
                }
                else {
                    parent[rootV] = rootU;
                    rank[rootU]++;
                }
            }
        }
    };
    static std::pair<int, std::vector<Edge>> Load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Ошибка: не удалось открыть файл " + filename);
        }

        int vertices, edgeCount;
        file >> vertices >> edgeCount;

        std::vector<Edge> edges;
        edges.reserve(edgeCount);

        for (int i = 0; i < edgeCount; ++i) {
            int u, v, weight;
            file >> u >> v >> weight;
            edges.emplace_back(u, v, weight);
        }
        return { vertices, std::move(edges) };
    }
}


#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <chrono>
#include <tuple>

using namespace std;

struct Node {
    int x, y, t; // Coordenadas (x, y) y tiempo t
    double g, h; // Coste acumulado y heurístico
    string moves; // Movimientos realizados

    Node(int x, int y, int t, double g, double h, string moves)
        : x(x), y(y), t(t), g(g), h(h), moves(moves) {}

    bool operator>(const Node& other) const {
        return (g + h) > (other.g + other.h);
    }
};

struct Map {
    vector<vector<char>> grid;
    vector<pair<int, int>> init_positions;
    vector<pair<int, int>> goal_positions;
    int rows, cols;
};

// Lee el mapa del fichero CSV
Map read_map(const string& path) {
    ifstream file(path);
    Map map;
    string line;

    int n;
    file >> n;
    map.init_positions.resize(n);
    map.goal_positions.resize(n);

    for (int i = 0; i < n; i++) {
        file >> map.init_positions[i].first >> map.init_positions[i].second;
        file >> map.goal_positions[i].first >> map.goal_positions[i].second;
    }

    while (file >> line) {
        vector<char> row;
        for (char c : line) {
            if (c != ';') {
                row.push_back(c);
            }
        }
        map.grid.push_back(row);
    }

    map.rows = map.grid.size();
    map.cols = map.grid[0].size();
    return map;
}

// Calcula la distancia Manhattan como heurística
double manhattan_heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

// Expande los nodos para el A*
vector<Node> expand_node(const Node& current, const Map& map, const pair<int, int>& goal) {
    vector<Node> successors;
    vector<pair<int, int>> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    vector<string> dir_symbols = {"→", "↓", "←", "↑"};

    for (size_t i = 0; i < directions.size(); i++) {
        int nx = current.x + directions[i].first;
        int ny = current.y + directions[i].second;

        if (nx >= 0 && ny >= 0 && nx < map.rows && ny < map.cols && map.grid[nx][ny] != 'G') {
            Node successor(nx, ny, current.t + 1, current.g + 1, manhattan_heuristic(nx, ny, goal.first, goal.second), current.moves + "(" + to_string(nx) + "," + to_string(ny) + ")" + dir_symbols[i] + " ");
            successors.push_back(successor);
        }
    }

    // Agregar opción de espera
    Node wait_node(current.x, current.y, current.t + 1, current.g, current.h, current.moves + "w ");
    successors.push_back(wait_node);

    return successors;
}

// Implementación del algoritmo A*
vector<Node> astar(const Map& map, int num_h) {
    vector<Node> solutions;
    for (size_t i = 0; i < map.init_positions.size(); i++) {
        auto compare = [](const Node& a, const Node& b) {
            return (a.g + a.h) > (b.g + b.h);
        };
        priority_queue<Node, vector<Node>, decltype(compare)> open_set(compare);
        unordered_set<string> closed_set;

        auto [init_x, init_y] = map.init_positions[i];
        auto [goal_x, goal_y] = map.goal_positions[i];

        Node start(init_x, init_y, 0, 0, manhattan_heuristic(init_x, init_y, goal_x, goal_y), "(" + to_string(init_x) + "," + to_string(init_y) + ") ");
        open_set.push(start);

        while (!open_set.empty()) {
            Node current = open_set.top();
            open_set.pop();

            if (current.x == goal_x && current.y == goal_y) {
                solutions.push_back(current);
                break;
            }

            string current_state = to_string(current.x) + "," + to_string(current.y) + "," + to_string(current.t);
            if (closed_set.count(current_state)) continue;
            closed_set.insert(current_state);

            for (const Node& successor : expand_node(current, map, {goal_x, goal_y})) {
                string successor_state = to_string(successor.x) + "," + to_string(successor.y) + "," + to_string(successor.t);
                if (!closed_set.count(successor_state)) {
                    open_set.push(successor);
                }
            }
        }
    }
    return solutions;
}

// Escritura de soluciones
void write_solution(const vector<Node>& solutions, const string& output_path) {
    ofstream file(output_path);
    for (const Node& sol : solutions) {
        file << sol.moves << endl;
    }
}

// Escritura de estadísticas
void write_stats(const vector<Node>& solutions, const string& stats_path, chrono::duration<double> elapsed) {
    ofstream file(stats_path);
    file << "Tiempo total: " << elapsed.count() << "s\n";
    file << "Makespan: " << solutions.back().t << "\n";
    file << "h inicial: " << solutions.front().h << "\n";
    file << "Nodos expandidos: " << solutions.size() << "\n";
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " <path mapa.csv> <num-h>\n";
        return 1;
    }

    string map_path = argv[1];
    int num_h = stoi(argv[2]);

    Map map = read_map(map_path);

    auto start_time = chrono::high_resolution_clock::now();
    vector<Node> solutions = astar(map, num_h);
    auto end_time = chrono::high_resolution_clock::now();

    chrono::duration<double> elapsed = end_time - start_time;

    string output_path = map_path + "-" + to_string(num_h) + ".output";
    string stats_path = map_path + "-" + to_string(num_h) + ".stat";

    write_solution(solutions, output_path);
    write_stats(solutions, stats_path, elapsed);

    return 0;
}

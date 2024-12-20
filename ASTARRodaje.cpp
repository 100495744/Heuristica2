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

    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << path << endl;
        exit(EXIT_FAILURE);
    }

    if (file.peek() == EOF) {
        cerr << "Error: El archivo " << path << " está vacío." << endl;
        exit(EXIT_FAILURE);
    }

    int n;
    file >> n;
    map.init_positions.resize(n);
    map.goal_positions.resize(n);

    // Leer posiciones iniciales y metas
    for (int i = 0; i < n; i++) {
        string init_pos, goal_pos;
        file >> init_pos >> goal_pos;

        // Extraer coordenadas de las posiciones iniciales
        int init_x = stoi(init_pos.substr(1, init_pos.find(',') - 1));
        int init_y = stoi(init_pos.substr(init_pos.find(',') + 1, init_pos.find(')') - init_pos.find(',') - 1));
        map.init_positions[i] = {init_x, init_y};

        // Extraer coordenadas de las posiciones finales
        int goal_x = stoi(goal_pos.substr(1, goal_pos.find(',') - 1));
        int goal_y = stoi(goal_pos.substr(goal_pos.find(',') + 1, goal_pos.find(')') - goal_pos.find(',') - 1));
        map.goal_positions[i] = {goal_x, goal_y};
    }

    // Leer el mapa
    getline(file, line); // Limpiar cualquier salto de línea restante
    while (getline(file, line)) {
        vector<char> row;
        for (char c : line) {
            if (c != ';' && c != '\n' && c != '\r') {
                row.push_back(c);
            }
        }
        if (!row.empty()) {
            map.grid.push_back(row);
        }
    }

    if (map.grid.empty() || map.grid[0].empty()) {
        cerr << "Error: El mapa no tiene dimensiones válidas." << endl;
        exit(EXIT_FAILURE);
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
vector<Node> expand_node(const Node& current, const Map& map, const pair<int, int>& goal, int& nodes_generated) {
    vector<Node> successors;
    vector<pair<int, int>> directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    vector<string> dir_symbols = {"→", "↓", "←", "↑"};

    for (size_t i = 0; i < directions.size(); i++) {
        int nx = current.x + directions[i].first;
        int ny = current.y + directions[i].second;

        if (nx >= 0 && ny >= 0 && nx < map.rows && ny < map.cols && map.grid[nx][ny] != 'G') {
            Node successor(nx, ny, current.t + 1, current.g + 1, manhattan_heuristic(nx, ny, goal.first, goal.second), current.moves + "(" + to_string(nx) + "," + to_string(ny) + ")" + dir_symbols[i] + " ");
            successors.push_back(successor);
            nodes_generated++;
        }
    }

    // Agregar opción de espera
    Node wait_node(current.x, current.y, current.t + 1, current.g, current.h, current.moves + "w ");
    successors.push_back(wait_node);
    nodes_generated++;

    return successors;
}

// Implementación del algoritmo A*
vector<Node> astar(const Map& map, int num_h, double& initial_heuristic, int& nodes_generated, int& nodes_expanded) {
    vector<Node> solutions;
    unordered_map<string, double> best_cost; // Mejor coste por nodo

    for (size_t i = 0; i < map.init_positions.size(); i++) {
        auto compare = [](const Node& a, const Node& b) {
            return (a.g + a.h) > (b.g + b.h);
        };
        priority_queue<Node, vector<Node>, decltype(compare)> open_set(compare);
        unordered_set<string> closed_set;

        auto [init_x, init_y] = map.init_positions[i];
        auto [goal_x, goal_y] = map.goal_positions[i];

        initial_heuristic = manhattan_heuristic(init_x, init_y, goal_x, goal_y);
        Node start(init_x, init_y, 0, 0, initial_heuristic, "(" + to_string(init_x) + "," + to_string(init_y) + ") ");
        open_set.push(start);
        nodes_generated++;

        while (!open_set.empty()) {
            Node current = open_set.top();
            open_set.pop();
            nodes_expanded++;

            if (current.x == goal_x && current.y == goal_y) {
                solutions.push_back(current);
                break;
            }

            string current_state = to_string(current.x) + "," + to_string(current.y);
            if (best_cost.find(current_state) != best_cost.end() && best_cost[current_state] <= current.g) {
                continue; // Nodo ya procesado con un mejor coste
            }
            best_cost[current_state] = current.g;

            for (const Node& successor : expand_node(current, map, {goal_x, goal_y}, nodes_generated)) {
                open_set.push(successor);
            }
        }
    }

    return solutions;
}

// Escritura de soluciones
void write_solution(const vector<Node>& solutions, const string& output_path) {
    ofstream file(output_path);
    for (const Node& sol : solutions) {
        string formatted_moves = sol.moves;
        if (formatted_moves.find(" ") == 0) {
            formatted_moves.erase(0, 1); // Elimina espacio inicial si existe
        }
        while (formatted_moves.find(") ") != string::npos) {
            formatted_moves.replace(formatted_moves.find(") "), 2, ")");
        }
        file << formatted_moves << endl;
    }
}

// Escritura de estadísticas
void write_stats(const vector<Node>& solutions, const string& stats_path, chrono::duration<double> elapsed, double initial_heuristic, int nodes_generated, int nodes_expanded) {
    ofstream file(stats_path);
    file << "Tiempo total: " << elapsed.count() << "s\n";
    file << "Makespan: " << solutions.back().t << "\n";
    file << "h inicial: " << initial_heuristic << "\n";
    file << "Nodos expandidos: " << nodes_expanded << "\n";
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Uso: " << argv[0] << " <path mapa.csv> <num-h>\n";
        return 1;
    }

    string map_path = argv[1];
    int num_h = stoi(argv[2]);

    Map map = read_map(map_path);

    double initial_heuristic = 0.0;
    int nodes_generated = 0;
    int nodes_expanded = 0;

    auto start_time = chrono::high_resolution_clock::now();
    vector<Node> solutions = astar(map, num_h, initial_heuristic, nodes_generated, nodes_expanded);
    auto end_time = chrono::high_resolution_clock::now();

    chrono::duration<double> elapsed = end_time - start_time;

    // Remover extensión .csv del nombre del archivo
    size_t dot_pos = map_path.find_last_of('.');
    if (dot_pos != string::npos) {
        map_path = map_path.substr(0, dot_pos);
    }

    // Generar archivos .output y .stat
    string output_path = map_path + "-" + to_string(num_h) + ".output";
    string stats_path = map_path + "-" + to_string(num_h) + ".stat";

    write_solution(solutions, output_path);
    write_stats(solutions, stats_path, elapsed, initial_heuristic, nodes_generated, nodes_expanded);

    return 0;
}



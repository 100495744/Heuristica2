// ASTARRodaje.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <chrono>
#include <sstream>
#include <string>

using namespace std;

// Constantes
const int INF = 1e9;

// Estructura para una posición en el mapa
struct Position {
    int x, y;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

// Estructura de un nodo en el algoritmo A*
struct Node {
    Position pos;
    int g, h, t;
    vector<string> path;

    bool operator>(const Node& other) const {
        return (g + h) > (other.g + other.h);
    }
};

// Estructuras para almacenar el mapa y datos del problema
vector<vector<char>> mapGrid;
vector<pair<Position, Position>> planes;

// Dirección de movimientos y sus representaciones
vector<pair<int, int>> directions = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}, {0, 0}};
vector<string> dirStr = {"←", "→", "↑", "↓", "w"};

// Funciones auxiliares
bool isValid(Position pos, int rows, int cols) {
    return pos.x >= 0 && pos.x < rows && pos.y >= 0 && pos.y < cols && mapGrid[pos.x][pos.y] != 'G';
}

int manhattan(Position a, Position b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

// Implementación de Floyd-Warshall para calcular distancias mínimas entre todas las posiciones
vector<vector<int>> floydWarshall(int rows, int cols) {
    vector<vector<int>> dist(rows * cols, vector<int>(rows * cols, INF));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (mapGrid[i][j] != 'G') {
                int u = i * cols + j;
                dist[u][u] = 0;

                for (int k = 0; k < directions.size() - 1; ++k) {
                    int nx = i + directions[k].first;
                    int ny = j + directions[k].second;

                    if (isValid({nx, ny}, rows, cols)) {
                        int v = nx * cols + ny;
                        dist[u][v] = 1;
                    }
                }
            }
        }
    }

    for (int k = 0; k < rows * cols; ++k) {
        for (int i = 0; i < rows * cols; ++i) {
            for (int j = 0; j < rows * cols; ++j) {
                dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
            }
        }
    }

    return dist;
}

// A* algorithm
void aStar(Position start, Position goal, int rows, int cols, int heuristicType, vector<string>& path) {
    auto startTime = chrono::steady_clock::now();
    priority_queue<Node, vector<Node>, greater<Node>> openList;
    unordered_map<int, unordered_map<int, bool>> visited;

    vector<vector<int>> floyd = (heuristicType == 2) ? floydWarshall(rows, cols) : vector<vector<int>>();

    auto heuristic = [&](Position current) {
        if (heuristicType == 1) {
            return manhattan(current, goal);
        } else {
            int u = current.x * cols + current.y;
            int v = goal.x * cols + goal.y;
            return floyd[u][v];
        }
    };

    openList.push({start, 0, heuristic(start), 0, {}});

    while (!openList.empty()) {
        Node current = openList.top();
        openList.pop();

        if (visited[current.pos.x][current.pos.y]) continue;
        visited[current.pos.x][current.pos.y] = true;

        if (current.pos == goal) {
            path = current.path;
            auto endTime = chrono::steady_clock::now();
            cout << "Tiempo total: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() / 1000.0 << "s\n";
            cout << "Nodos expandidos: " << visited.size() << "\n";
            return;
        }

        for (int i = 0; i < directions.size(); ++i) {
            Position next = {current.pos.x + directions[i].first, current.pos.y + directions[i].second};
            if (isValid(next, rows, cols) && !visited[next.x][next.y]) {
                vector<string> newPath = current.path;
                newPath.push_back(dirStr[i]);
                openList.push({next, current.g + 1, heuristic(next), current.t + 1, newPath});
            }
        }
    }
}

// Función principal
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <path mapa.csv> <num-h>\n";
        return 1;
    }

    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "No se puede abrir el archivo " << argv[1] << "\n";
        return 1;
    }

    int numPlanes, heuristicType = stoi(argv[2]);
    file >> numPlanes;

    planes.resize(numPlanes);
    for (int i = 0; i < numPlanes; ++i) {
        file >> planes[i].first.x >> planes[i].first.y >> planes[i].second.x >> planes[i].second.y;
    }

    string line;
    while (getline(file, line)) {
        if (!line.empty()) {
            vector<char> row;
            for (char c : line) {
                if (c != ';') row.push_back(c);
            }
            mapGrid.push_back(row);
        }
    }

    file.close();

    int rows = mapGrid.size();
    int cols = mapGrid[0].size();

    for (int i = 0; i < numPlanes; ++i) {
        vector<string> path;
        aStar(planes[i].first, planes[i].second, rows, cols, heuristicType, path);
        for (const string& step : path) {
            cout << step << " ";
        }
        cout << endl;
    }

    return 0;
}
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <cmath>
#include <algorithm>

using namespace std;

struct Position {
    int x, y;
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct Node {
    Position pos;
    int g; // Coste acumulado
    int h; // Heurística
    Node* parent;
    string action; // Acción para llegar a este nodo

    int f() const { return g + h; }

    bool operator<(const Node& other) const {
        return f() > other.f(); // Min-heap
    }
};

// Hash function for Position
namespace std {
    template <>
    struct hash<Position> {
        size_t operator()(const Position& pos) const {
            return hash<int>()(pos.x) ^ hash<int>()(pos.y);
        }
    };
}

vector<vector<char>> map;
vector<pair<Position, Position>> planes;
int heuristic_option;

// Direction vectors for movements and corresponding actions
vector<pair<int, int>> directions = {
    {0, 1},  // Right
    {0, -1}, // Left
    {1, 0},  // Down
    {-1, 0}, // Up
};
vector<string> actions = {"→", "←", "↓", "↑", "w"};

unordered_map<int, unordered_map<Position, int>> occupied_positions; // time -> position -> plane index

bool is_valid(Position pos, int t, int plane_idx, Position current_pos, bool is_waiting, unordered_set<Position>& closed_list, const vector<unordered_map<int, Position>>& predicted_positions) {
    // Check if the position is within bounds and not a blocked cell
    if (!is_waiting && (pos.x < 0 || pos.x >= map.size() || pos.y < 0 || pos.y >= map[0].size() || map[pos.x][pos.y] == 'G')) {
        return false;
    }

    // Check if the node has already been expanded
    if (closed_list.count(pos)) {
        return false;
    }

    // Restriction 1: Avoid swapping positions
    if (occupied_positions.count(t)) {
        const auto& current_occupied = occupied_positions[t];
        if (current_occupied.count(pos) && current_occupied.at(pos) != plane_idx) {
            const int other_plane_idx = current_occupied.at(pos);
            if (predicted_positions[other_plane_idx].count(t + 1) && predicted_positions[other_plane_idx].at(t + 1) == current_pos) {
                return false; // Prevent position swap
            }
        }
    }

    // Restriction 2: Avoid collision at the same position
    if (occupied_positions.count(t) && occupied_positions[t].count(pos) && occupied_positions[t][pos] != plane_idx) {
        return false; // Another plane is occupying this position
    }

    return true;
}

int heuristic(const Position& current, const Position& goal) {
    if (heuristic_option == 1) {
        // Manhattan distance
        return abs(current.x - goal.x) + abs(current.y - goal.y);
    } else {
        // Euclidean distance (rounded)
        return static_cast<int>(sqrt(pow(current.x - goal.x, 2) + pow(current.y - goal.y, 2)));
    }
}

void a_star(int plane_idx, vector<string>& movements, int& makespan, int& expanded_nodes, vector<unordered_map<int, Position>>& predicted_positions) {
    auto start_pos = planes[plane_idx].first;
    auto goal_pos = planes[plane_idx].second;

    priority_queue<Node> open_list;
    unordered_set<Position> closed_list;

    Node start_node{start_pos, 0, heuristic(start_pos, goal_pos), nullptr, ""};
    open_list.push(start_node);
    occupied_positions[0][start_pos] = plane_idx; // Mark start position as occupied

    while (!open_list.empty()) {
        Node current = open_list.top();
        open_list.pop();

        if (current.pos == goal_pos) {
            // Construct the path
            Node* n = &current;
            vector<string> path;

            while (n->parent) {
                stringstream move;
                move << n->action << " (" << n->pos.x << "," << n->pos.y << ")";
                path.push_back(move.str());
                n = n->parent;
            }

            path.push_back("(" + to_string(start_pos.x) + "," + to_string(start_pos.y) + ")");
            reverse(path.begin(), path.end());

            movements.insert(movements.end(), path.begin(), path.end());
            makespan = max(makespan, current.g);
            return;
        }

        // Mark node as closed
        closed_list.insert(current.pos);
        expanded_nodes++;

        vector<Node> successors;
        for (size_t i = 0; i <= directions.size(); ++i) { // Include "w" as an option
            Position new_pos;
            bool is_waiting = (i == directions.size());
            if (is_waiting) {
                new_pos = current.pos; // Waiting in place
            } else {
                new_pos = {current.pos.x + directions[i].first, current.pos.y + directions[i].second};
            }

            int next_time = current.g + 1;

            if (is_valid(new_pos, next_time, plane_idx, current.pos, is_waiting, closed_list, predicted_positions)) {
                Node successor{new_pos, next_time, heuristic(new_pos, goal_pos), new Node(current), is_waiting ? "w" : actions[i]};
                successors.push_back(successor);
                occupied_positions[next_time][new_pos] = plane_idx; // Mark as occupied
                predicted_positions[plane_idx][next_time] = new_pos; // Predict future position
            }
        }

        // Sort successors to prioritize non-waiting actions
        sort(successors.begin(), successors.end(), [](const Node& a, const Node& b) {
            return a.f() < b.f(); // Prioritize nodes with lower f()
        });

        for (const auto& successor : successors) {
            open_list.push(successor);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: ./ASTARRodaje <path mapa.csv> <num-h>\n";
        return 1;
    }

    string file_path = argv[1];
    heuristic_option = stoi(argv[2]);

    ifstream file(file_path);
    if (!file) {
        cerr << "Error: Could not open file " << file_path << "\n";
        return 1;
    }

    int num_planes;
    file >> num_planes;
    planes.resize(num_planes);

    for (int i = 0; i < num_planes; ++i) {
        char discard;
        file >> discard >> planes[i].first.x >> discard >> planes[i].first.y >> discard;
        file >> discard >> planes[i].second.x >> discard >> planes[i].second.y >> discard;
    }

    string line;
    while (getline(file, line)) {
        if (!line.empty()) {
            stringstream ss(line);
            vector<char> row;
            char cell;
            while (ss >> cell) {
                row.push_back(cell);
                if (ss.peek() == ';') ss.ignore();
            }
            map.push_back(row);
        }
    }
    file.close();

    auto start_time = chrono::steady_clock::now();

    vector<vector<string>> all_movements(num_planes);
    int makespan = 0;
    int expanded_nodes = 0;

    vector<unordered_map<int, Position>> predicted_positions(num_planes);

    for (int i = 0; i < num_planes; ++i) {
        a_star(i, all_movements[i], makespan, expanded_nodes, predicted_positions);
    }

    auto end_time = chrono::steady_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;

    string output_base = file_path.substr(0, file_path.find_last_of("."));
    ofstream solution_file(output_base + "-" + to_string(heuristic_option) + ".output");
    ofstream stats_file(output_base + "-" + to_string(heuristic_option) + ".stat");

    for (const auto& moves : all_movements) {
        for (const auto& move : moves) {
            solution_file << move << " ";
        }
        solution_file << "\n";
    }

    stats_file << "Tiempo total: " << elapsed_time.count() << "s\n";
    stats_file << "Makespan: " << makespan << "\n";
    stats_file << "h inicial: " << heuristic(planes[0].first, planes[0].second) << "\n";
    stats_file << "Nodos expandidos: " << expanded_nodes << "\n";

    solution_file.close();
    stats_file.close();

    return 0;
}










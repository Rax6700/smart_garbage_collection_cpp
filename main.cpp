#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <queue>
#include <string>
#include <vector>

using namespace std;

struct Road {
    int to;
    int baseWeight;
    int trafficFactor; // 1 = Low, 2 = Medium, 4 = High
};

struct Bin {
    string location;
    int wasteLevel;
};

class SmartGarbageSystem {
private:
    vector<string> names = {"A", "B", "C", "D", "E"};
    vector<vector<Road>> graph;
    vector<Bin> bins;

    int indexOf(const string& name) const {
        for (size_t i = 0; i < names.size(); ++i) {
            if (names[i] == name) return static_cast<int>(i);
        }
        return -1;
    }

    int effectiveCost(const Road& road) const {
        return road.baseWeight + road.trafficFactor;
    }

    string trafficName(int factor) const {
        if (factor == 1) return "Low";
        if (factor == 2) return "Medium";
        return "High";
    }

    void addRoad(int a, int b, int base, int traffic = 1) {
        graph[a].push_back({b, base, traffic});
        graph[b].push_back({a, base, traffic});
    }

    vector<int> dijkstra(int source, vector<int>& parent, vector<int>& dist) const {
        const int INF = numeric_limits<int>::max() / 4;
        int n = static_cast<int>(graph.size());
        dist.assign(n, INF);
        parent.assign(n, -1);
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

        dist[source] = 0;
        pq.push({0, source});

        while (!pq.empty()) {
            // Unpack pair manually to ensure compatibility with pre-C++17 compilers
            int d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            if (d != dist[u]) continue;

            for (const Road& road : graph[u]) {
                int v = road.to;
                int candidate = d + effectiveCost(road);
                if (candidate < dist[v]) {
                    dist[v] = candidate;
                    parent[v] = u;
                    pq.push({candidate, v});
                }
            }
        }
        return dist;
    }

    vector<string> makePath(int source, int destination, const vector<int>& parent) const {
        vector<string> path;
        int cur = destination;
        while (cur != -1) {
            path.push_back(names[cur]);
            if (cur == source) break;
            cur = parent[cur];
        }
        if (path.empty() || path.back() != names[source]) return {};
        reverse(path.begin(), path.end());
        return path;
    }

    void printPath(const vector<string>& path) const {
        for (size_t i = 0; i < path.size(); ++i) {
            if (i) cout << " -> ";
            cout << path[i];
        }
    }

    void addBin() {
        string location;
        int level;
        cout << "Enter location (A-E): ";
        cin >> location;
        int idx = indexOf(location);
        if (idx < 0) {
            cout << "Unknown location. Use A, B, C, D, or E.\n";
            return;
        }
        cout << "Enter waste level (0-100): ";
        if (!(cin >> level) || level < 0 || level > 100) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid level. Enter a number from 0 to 100.\n";
            return;
        }
        for (Bin& bin : bins) {
            if (bin.location == location) {
                bin.wasteLevel = level;
                cout << "Existing bin at " << location << " updated to " << level << "%.\n";
                return;
            }
        }
        bins.push_back({location, level});
        cout << "Garbage bin added at " << location << " with " << level << "% waste.\n";
    }

    void updateTraffic() {
        string from, to;
        cout << "Enter road endpoints (e.g., A B): ";
        cin >> from >> to;
        int u = indexOf(from), v = indexOf(to);
        if (u < 0 || v < 0) {
            cout << "Unknown location. Use A-E.\n";
            return;
        }
        cout << "Traffic level: 1=Low, 2=Medium, 3=High: ";
        int level;
        if (!(cin >> level) || level < 1 || level > 3) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid traffic level.\n";
            return;
        }
        int factor = (level == 1 ? 1 : (level == 2 ? 2 : 4));
        bool found = false;
        for (Road& road : graph[u]) {
            if (road.to == v) { road.trafficFactor = factor; found = true; }
        }
        for (Road& road : graph[v]) {
            if (road.to == u) road.trafficFactor = factor;
        }
        if (found) {
            cout << "Traffic on " << from << " <-> " << to
                 << " updated to " << trafficName(factor) << ".\n";
        } else {
            cout << "That road does not exist in the network.\n";
        }
    }

    string priorityType(int level) const {
        if (level >= 75) return "High";
        if (level >= 40) return "Medium";
        return "Low";
    }

    void showPriority() const {
        if (bins.empty()) {
            cout << "No bins added yet.\n";
            return;
        }
        vector<Bin> sorted = bins;
        sort(sorted.begin(), sorted.end(), [](const Bin& a, const Bin& b) {
            return a.wasteLevel > b.wasteLevel;
        });
        cout << "\nGarbage Collection Priority Order:\n";
        for (const Bin& bin : sorted)
            cout << "  " << bin.location << " (Garbage Level: " << bin.wasteLevel
                 << "%, Priority: " << priorityType(bin.wasteLevel) << ")\n";
    }

    void shortestCollectionRoute() const {
        if (bins.empty()) {
            cout << "No bins added yet. Choose option 1 first.\n";
            return;
        }
        string startName;
        cout << "Enter truck start location / depot (A-E): ";
        cin >> startName;
        int startNode = indexOf(startName);
        if (startNode < 0) {
            cout << "Unknown location.\n";
            return;
        }

        vector<Bin> pending = bins;
        int current = startNode;
        int total = 0;
        cout << "\nShortest Collection Route (Nearest-Bin Heuristic):\n";

        while (!pending.empty()) {
            vector<int> parent, dist;
            dijkstra(current, parent, dist);

            int chosen = -1;
            int bestCost = numeric_limits<int>::max() / 4;
            for (size_t i = 0; i < pending.size(); ++i) {
                if (pending[i].wasteLevel <= 0) continue;
                int dest = indexOf(pending[i].location);
                if (dist[dest] < bestCost) {
                    bestCost = dist[dest];
                    chosen = static_cast<int>(i);
                }
            }
            if (chosen < 0) break;

            int destination = indexOf(pending[chosen].location);
            vector<string> path = makePath(current, destination, parent);
            if (path.empty() || bestCost >= numeric_limits<int>::max() / 8) {
                cout << "No route found to " << pending[chosen].location << ".\n";
                pending.erase(pending.begin() + chosen);
                continue;
            }
            cout << "Path to " << names[destination] << " (Cost: " << bestCost << "): ";
            printPath(path);
            cout << " | Bin: " << pending[chosen].wasteLevel << "%\n";

            total += bestCost;
            current = destination;
            pending.erase(pending.begin() + chosen);
        }

        if (current != startNode) {
            vector<int> parent, dist;
            dijkstra(current, parent, dist);
            vector<string> returnPath = makePath(current, startNode, parent);
            cout << "Return to depot " << names[startNode] << " (Cost: " << dist[startNode] << "): ";
            printPath(returnPath);
            cout << "\n";
            total += dist[startNode];
        }

        cout << "Total Cost of Complete Tour: " << total << " units\n";
    }

    void showMap() const {
        cout << "\n================ CITY GARBAGE ROUTE MAP ================\n";
        cout << "Nodes: A, B, C, D, E  |  base distance + traffic factor = cost\n";
        for (size_t u = 0; u < graph.size(); ++u) {
            for (const Road& road : graph[u]) {
                if (static_cast<int>(u) < road.to) {
                    cout << "  " << names[u] << " --" << road.baseWeight
                         << "(+" << road.trafficFactor << ", " << trafficName(road.trafficFactor)
                         << ")--> " << names[road.to]
                         << "   effective cost=" << effectiveCost(road) << "\n";
                }
            }
        }
        cout << "=========================================================\n";
    }

    void showBinLevels() const {
        if (bins.empty()) {
            cout << "No bins added yet.\n";
            return;
        }
        cout << "\nBin Levels at Locations:\n";
        for (const Bin& bin : bins)
            cout << "  [Bin] " << bin.location << " -> " << bin.wasteLevel
                 << "% (" << priorityType(bin.wasteLevel) << " priority)\n";
    }

public:
    SmartGarbageSystem() : graph(5) {
        addRoad(0, 1, 4, 1); // A-B
        addRoad(0, 2, 2, 1); // A-C
        addRoad(1, 2, 5, 2); // B-C
        addRoad(1, 3, 5, 1); // B-D
        addRoad(2, 3, 3, 1); // C-D
        addRoad(2, 4, 7, 2); // C-E
        addRoad(3, 4, 4, 1); // D-E
        bins = {{"A", 25}, {"B", 63}, {"C", 36}};
    }

    void run() {
        int choice;
        do {
            cout << "\n🚛 Smart Garbage Collection System 🚛\n"
                 << "1. Add / Update a garbage bin\n"
                 << "2. Update traffic congestion\n"
                 << "3. Display garbage collection priority\n"
                 << "4. Find shortest collection route\n"
                 << "5. Show map\n"
                 << "6. Show bin levels at locations\n"
                 << "7. Exit\n"
                 << "Enter your choice: ";
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Please enter a number from 1 to 7.\n";
                continue;
            }
            switch (choice) {
                case 1: addBin(); break;
                case 2: updateTraffic(); break;
                case 3: showPriority(); break;
                case 4: shortestCollectionRoute(); break;
                case 5: showMap(); break;
                case 6: showBinLevels(); break;
                case 7: cout << "Exiting. Thank you!\n"; break;
                default: cout << "Invalid choice. Choose 1-7.\n";
            }
        } while (choice != 7);
    }
};

int main() {
    SmartGarbageSystem app;
    app.run();
    return 0;
}
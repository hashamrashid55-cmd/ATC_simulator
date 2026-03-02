#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <iostream>
#include <string>
#include <fstream>
#include <climits> 
#include <cmath>
#include <iomanip> 

using namespace std;

// ==========================================
// CENTRAL DATA OBJECT: THE AIRCRAFT
// ==========================================
struct Plane {
    string id;
    string model;
    int fuel;
    int priority;
    string status;

    // position data
    int x, y;           // current coordinates grid pe
    int currentID;      // abhi kis node pe hain
    int nextHopID;      // agla step kahan lena hai
    int finalDestID;    // aakhri manzil

    char icon;          // direction dekhne ke liye >, <, ^, v

    Plane() : id(""), priority(4), x(0), y(0), icon('P') {}
    Plane(string _id, string _mod, int _f, int _p, int _x, int _y, int _start, int _end)
        : id(_id), model(_mod), fuel(_f), priority(_p), status("In Air"),
        x(_x), y(_y), currentID(_start), nextHopID(_start), finalDestID(_end), icon('P') {}
};

// ==========================================
// MODULE C: HASH TABLE
// ==========================================
struct HashNode {
    Plane data;
    HashNode* next;
    HashNode(Plane p) : data(p), next(nullptr) {}
};

class HashTable {
    static const int TABLE_SIZE = 20;
    HashNode* table[TABLE_SIZE];

    // simple hash function ascii sum krke
    int hashFunction(string key) {
        int sum = 0;
        for (char c : key) sum += c;
        return sum % TABLE_SIZE;
    }

public:
    HashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) table[i] = nullptr;
    }

    // naya plane add krne ke liye
    void insert(Plane p) {
        int idx = hashFunction(p.id);
        HashNode* newNode = new HashNode(p);
        newNode->next = table[idx]; // chaining kr rhe hain collision ke liye
        table[idx] = newNode;
    }

    // id se plane dhoondne ke liye
    Plane* search(string id) {
        int idx = hashFunction(id);
        HashNode* curr = table[idx];
        while (curr) {
            if (curr->data.id == id) return &curr->data;
            curr = curr->next;
        }
        return nullptr;
    }

    // radar pe dikhane ke liye sare planes chahiye
    void getAllPlanes(Plane* list[], int& count) {
        count = 0;
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashNode* curr = table[i];
            while (curr) {
                list[count++] = &curr->data;
                curr = curr->next;
            }
        }
    }

    // file me data save krne ke liye
    void saveToFile(ofstream& out) {
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashNode* curr = table[i];
            while (curr) {
                out << curr->data.id << " " << curr->data.model << " " << curr->data.fuel << " "
                    << curr->data.priority << " " << curr->data.x << " " << curr->data.y << " "
                    << curr->data.currentID << " " << curr->data.finalDestID << endl;
                curr = curr->next;
            }
        }
    }
};

// ==========================================
// MODULE B: MIN-HEAP
// ==========================================
class MinHeap {
    Plane* heapArray[100];
    int currentSize;
    int parent(int i) { return (i - 1) / 2; }
    int left(int i) { return (2 * i + 1); }
    int right(int i) { return (2 * i + 2); }

    // agar structure kharab ho jaye to ye fix krega
    void heapify(int i) {
        int l = left(i);
        int r = right(i);
        int smallest = i;
        if (l < currentSize && heapArray[l]->priority < heapArray[smallest]->priority) smallest = l;
        if (r < currentSize && heapArray[r]->priority < heapArray[smallest]->priority) smallest = r;
        if (smallest != i) { swap(heapArray[i], heapArray[smallest]); heapify(smallest); }
    }

public:
    MinHeap() : currentSize(0) {}

    // nayi request queue me dalne ke liye
    void insert(Plane* p) {
        if (currentSize >= 100) return;
        int index = currentSize;
        heapArray[index] = p;
        currentSize++;
        // bubble up krna parega agar priority zyada hai
        while (index > 0 && heapArray[parent(index)]->priority > heapArray[index]->priority) {
            swap(heapArray[index], heapArray[parent(index)]);
            index = parent(index);
        }
    }

    // sab se urgent plane nikalne ke liye
    Plane* extractMin() {
        if (currentSize <= 0) return nullptr;
        if (currentSize == 1) { currentSize--; return heapArray[0]; }
        Plane* root = heapArray[0];
        heapArray[0] = heapArray[currentSize - 1];
        currentSize--;
        heapify(0); // root remove hone ke bad fix kro
        return root;
    }

    // agar emergency ho jaye to priority change kro
    void decreaseKey(string id, int newPriority) {
        for (int i = 0; i < currentSize; i++) {
            if (heapArray[i]->id == id) {
                heapArray[i]->priority = newPriority;
                int index = i;
                // priority barh gayi hai to upar le jao
                while (index > 0 && heapArray[parent(index)]->priority > heapArray[index]->priority) {
                    swap(heapArray[index], heapArray[parent(index)]);
                    index = parent(index);
                }
                return;
            }
        }
    }
    Plane* peek() { return (currentSize > 0) ? heapArray[0] : nullptr; }
    bool isEmpty() { return currentSize == 0; }

    // option 9 ke liye queue print kro
    void printQueue() {
        if (currentSize == 0) { cout << "Queue is Empty." << endl; return; }
        MinHeap temp;
        for (int i = 0; i < currentSize; i++) temp.insert(heapArray[i]);

        cout << "============= LANDING QUEUE =============" << endl;
        int rank = 1;
        while (!temp.isEmpty()) {
            Plane* p = temp.extractMin();
            cout << rank++ << ". " << p->id << " [Prio: " << p->priority << "] "
                << (p->priority == 1 ? "CRITICAL" : "Normal") << endl;
        }
        cout << "=========================================" << endl;
    }
};

// ==========================================
// MODULE D: AVL TREE
// ==========================================
struct AVLNode {
    string logEntry;
    int height;
    AVLNode* left, * right;
    AVLNode(string log) : logEntry(log), height(1), left(nullptr), right(nullptr) {}
};

class AVLTree {
    AVLNode* root;
    int height(AVLNode* N) { return (N == nullptr) ? 0 : N->height; }
    int maxVal(int a, int b) { return (a > b) ? a : b; }

    // right rotation balancing ke liye
    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left; AVLNode* T2 = x->right;
        x->right = y; y->left = T2;
        y->height = maxVal(height(y->left), height(y->right)) + 1;
        x->height = maxVal(height(x->left), height(x->right)) + 1;
        return x;
    }

    // left rotation balancing ke liye
    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right; AVLNode* T2 = y->left;
        y->left = x; x->right = T2;
        x->height = maxVal(height(x->left), height(x->right)) + 1;
        y->height = maxVal(height(y->left), height(y->right)) + 1;
        return y;
    }
    int getBalance(AVLNode* N) { return (N == nullptr) ? 0 : height(N->left) - height(N->right); }

    // node insert kro aur balance check kro
    AVLNode* insertNode(AVLNode* node, string log) {
        if (node == nullptr) return new AVLNode(log);
        if (log < node->logEntry) node->left = insertNode(node->left, log);
        else if (log > node->logEntry) node->right = insertNode(node->right, log);
        else return node;
        node->height = 1 + maxVal(height(node->left), height(node->right));
        int balance = getBalance(node);
        // 4 cases rotation ke
        if (balance > 1 && log < node->left->logEntry) return rightRotate(node);
        if (balance < -1 && log > node->right->logEntry) return leftRotate(node);
        if (balance > 1 && log > node->left->logEntry) { node->left = leftRotate(node->left); return rightRotate(node); }
        if (balance < -1 && log < node->right->logEntry) { node->right = rightRotate(node->right); return leftRotate(node); }
        return node;
    }

    // logs ko order me print krne ke liye (in-order)
    void inOrder(AVLNode* root, ofstream* file = nullptr) {
        if (root != nullptr) {
            inOrder(root->left, file);
            cout << root->logEntry << endl;
            if (file) *file << root->logEntry << endl;
            inOrder(root->right, file);
        }
    }
public:
    AVLTree() : root(nullptr) {}
    void insert(string log) { root = insertNode(root, log); }
    void displayLogs() { inOrder(root); }
    void saveLogs(ofstream& out) { inOrder(root, &out); }
};

// ==========================================
// MODULE A: GRAPH
// ==========================================
struct AdjListNode {
    int dest;
    int weight;
    AdjListNode* next;
    AdjListNode(int d, int w) : dest(d), weight(w), next(nullptr) {}
};

struct GraphNode {
    string name;
    int x, y;
    char type;
};

class Graph {
public:
    static const int MAX_NODES = 20;
    GraphNode nodes[MAX_NODES];
    AdjListNode* adjList[MAX_NODES];
    int nodeCount;

    Graph() : nodeCount(0) {
        for (int i = 0; i < MAX_NODES; i++) adjList[i] = nullptr;
    }

    void addNode(string name, int x, int y, char type) {
        nodes[nodeCount] = { name, x, y, type };
        nodeCount++;
    }

    void addEdge(int src, int dest, int weight) {
        AdjListNode* newNode = new AdjListNode(dest, weight);
        newNode->next = adjList[src];
        adjList[src] = newNode;
    }

    // coordinate ke qareeb wala node dhoondo
    int getNearestNode(int px, int py) {
        int bestNode = 0;
        double minDist = 99999.0;
        for (int i = 0; i < nodeCount; i++) {
            double dist = sqrt(pow(nodes[i].x - px, 2) + pow(nodes[i].y - py, 2));
            if (dist < minDist) {
                minDist = dist;
                bestNode = i;
            }
        }
        return bestNode;
    }

    // dijkstra se agla qadam (next hop) nikalne ke liye
    int getNextHop(int current, int target) {
        if (current == target) return target;
        int dist[MAX_NODES]; int parent[MAX_NODES]; bool visited[MAX_NODES];
        for (int i = 0; i < MAX_NODES; i++) { dist[i] = INT_MAX; visited[i] = false; parent[i] = -1; }
        dist[current] = 0;

        for (int count = 0; count < nodeCount - 1; count++) {
            int min = INT_MAX, u = -1;
            for (int v = 0; v < nodeCount; v++) { if (!visited[v] && dist[v] <= min) { min = dist[v]; u = v; } }
            if (u == -1) break; visited[u] = true;
            AdjListNode* curr = adjList[u];
            while (curr) {
                int v = curr->dest;
                if (!visited[v] && dist[u] != INT_MAX && dist[u] + curr->weight < dist[v]) { dist[v] = dist[u] + curr->weight; parent[v] = u; }
                curr = curr->next;
            }
        }
        if (dist[target] == INT_MAX) return -1;
        // backtrack krke pehla node nikalo
        int currNode = target;
        while (parent[currNode] != current && parent[currNode] != -1) {
            currNode = parent[currNode];
        }
        return currNode;
    }

    // safe route print krne ke liye
    void printSafeRoute(int start, int end) {
        int dist[MAX_NODES]; int parent[MAX_NODES]; bool visited[MAX_NODES];
        for (int i = 0; i < MAX_NODES; i++) { dist[i] = INT_MAX; visited[i] = false; parent[i] = -1; }
        dist[start] = 0;

        for (int count = 0; count < nodeCount - 1; count++) {
            int min = INT_MAX, u = -1;
            for (int v = 0; v < nodeCount; v++) { if (!visited[v] && dist[v] <= min) { min = dist[v]; u = v; } }
            if (u == -1) break; visited[u] = true;
            AdjListNode* curr = adjList[u];
            while (curr) {
                int v = curr->dest;
                if (!visited[v] && dist[u] != INT_MAX && dist[u] + curr->weight < dist[v]) { dist[v] = dist[u] + curr->weight; parent[v] = u; }
                curr = curr->next;
            }
        }

        if (dist[end] == INT_MAX) {
            cout << "No safe path found!" << endl;
        }
        else {
            cout << "[System] Calculating Path..." << endl;
            cout << "Start: " << nodes[start].name << endl;
            printPathRecursive(parent, end);
            cout << "\nTotal Distance: " << dist[end] << "km" << endl;
        }
    }

    void printPathRecursive(int parent[], int j) {
        if (parent[j] == -1) return;
        printPathRecursive(parent, parent[j]);
        cout << "   |\n   V\n" << nodes[j].name;
    }
};

#endif
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <iostream>
#include <windows.h>
#include <string>
#include <ctime>
#include <cmath> 
#include <iomanip>
#include "DataStructures.h"

using namespace std;

// global objects
Graph airspace;
HashTable registry;
MinHeap landingQueue;
AVLTree flightLog;

// file se data load krne ke liye
void loadData() {
    ifstream inFile("flight_data.txt");
    if (!inFile) return;
    string id, model; int fuel, priority, x, y, curr, dest;
    while (inFile >> id >> model >> fuel >> priority >> x >> y >> curr >> dest) {
        Plane p(id, model, fuel, priority, x, y, curr, dest);
        p.nextHopID = curr;
        if (x == airspace.nodes[dest].x && y == airspace.nodes[dest].y) p.status = "Landed";
        else p.status = "In Air";
        registry.insert(p);
        if (p.status != "Landed") landingQueue.insert(registry.search(id));
    }
    inFile.close();
}

// map aur nodes setup krne ke liye
void initializeSystem() {
    // 1. Airports
    airspace.addNode("Lahore", 18, 5, 'A');   // 0
    airspace.addNode("Islamabad", 2, 2, 'A');  // 1
    airspace.addNode("Karachi", 5, 18, 'A');   // 2

    // 2. Waypoints
    airspace.addNode("Bholay ki Chuggi", 10, 5, 'W'); // 3
    airspace.addNode("Shehar Ghanta Ghar", 10, 10, 'W'); // 4
    airspace.addNode("Allied Mor", 5, 10, 'W'); // 5

    // 3. Edges (raaste)
    airspace.addEdge(1, 3, 50); airspace.addEdge(3, 1, 50);
    airspace.addEdge(3, 0, 40); airspace.addEdge(0, 3, 40);
    airspace.addEdge(1, 5, 60); airspace.addEdge(5, 1, 60);
    airspace.addEdge(5, 4, 30); airspace.addEdge(4, 5, 30);
    airspace.addEdge(4, 2, 80); airspace.addEdge(2, 4, 80);
    airspace.addEdge(4, 0, 50); airspace.addEdge(0, 4, 50);
    airspace.addEdge(2, 5, 70); airspace.addEdge(5, 2, 70);

    loadData();
}

// radar grid draw krne ka function
void displayRadar() {
    system("cls");
    char grid[20][20];
    for (int i = 0; i < 20; i++) for (int j = 0; j < 20; j++) grid[i][j] = '.';

    // nodes place kro
    for (int i = 0; i < airspace.nodeCount; i++) {
        int x = airspace.nodes[i].x;
        int y = airspace.nodes[i].y;
        if (x < 20 && y < 20) grid[y][x] = airspace.nodes[i].type;
    }

    // planes place kro
    Plane* allPlanes[100]; int count = 0;
    registry.getAllPlanes(allPlanes, count);
    for (int i = 0; i < count; i++) {
        if (allPlanes[i]->status != "Landed") {
            int px = allPlanes[i]->x;
            int py = allPlanes[i]->y;
            if (px < 20 && py < 20) {
                if (grid[py][px] == '.') grid[py][px] = allPlanes[i]->icon;
            }
        }
    }

    cout << "=================== SKYNET RADAR ===================" << endl;
    cout << "   ";
    cout << setfill('0');
    for (int k = 0; k < 20; k++) cout << setw(2) << k << " ";
    cout << endl;

    for (int i = 0; i < 20; i++) {
        cout << setw(2) << i << " ";
        for (int j = 0; j < 20; j++) {
            char c = grid[i][j];
            // custom icons airport aur waypoints ke liye
            if (c == 'A') {
                int nodeIdx = airspace.getNearestNode(j, i);
                char initial = airspace.nodes[nodeIdx].name[0];
                cout << "[" << initial << "]";
            }
            else if (c == 'W') {
                int nodeIdx = airspace.getNearestNode(j, i);
                char initial = tolower(airspace.nodes[nodeIdx].name[0]);
                cout << " " << initial << " ";
            }
            else if (c == '.') cout << " . ";
            else cout << " " << c << " ";
        }
        cout << endl;
    }
    // format reset krdo warna har jaga 000 ayega
    cout << setfill(' ');
    cout << "====================================================" << endl;
    cout << "NEXT LANDING: ";
    Plane* next = landingQueue.peek();
    if (next) cout << next->id << " (Prio: " << next->priority << ") -> Dest: " << airspace.nodes[next->finalDestID].name << endl;
    else cout << "Runway Clear." << endl;
}

// option 7 logic: har plane ek step aage jayega
void stepSimulation() {
    Plane* allPlanes[100]; int count = 0;
    registry.getAllPlanes(allPlanes, count);
    bool moved = false;

    for (int i = 0; i < count; i++) {
        Plane* p = allPlanes[i];
        if (p->status == "Landed") continue;
        if (p->x == airspace.nodes[p->finalDestID].x && p->y == airspace.nodes[p->finalDestID].y) continue;

        // agar waypoint pe pohanch gaye to agla raasta dhoondo
        if (p->x == airspace.nodes[p->nextHopID].x && p->y == airspace.nodes[p->nextHopID].y) {
            p->currentID = p->nextHopID;
            p->nextHopID = airspace.getNextHop(p->currentID, p->finalDestID);
        }

        int tx = airspace.nodes[p->nextHopID].x;
        int ty = airspace.nodes[p->nextHopID].y;
        int dx = 0, dy = 0;

        // direction logic
        if (p->x < tx) { dx = 1; p->icon = '>'; }
        else if (p->x > tx) { dx = -1; p->icon = '<'; }
        if (p->y < ty) { dy = 1; p->icon = 'v'; }
        else if (p->y > ty) { dy = -1; p->icon = '^'; }

        // move one step
        if (p->x != tx) p->x += dx;
        else if (p->y != ty) p->y += dy;

        p->fuel -= 1;
        moved = true;
    }
    if (moved) cout << "\n>>> Traffic updated." << endl;
    else cout << "\n>>> Static." << endl;
    system("pause");
}

void printLocations() {
    cout << "\n--- LOCATIONS LIST ---" << endl;
    for (int i = 0; i < airspace.nodeCount; i++) {
        cout << "ID " << i << ": " << airspace.nodes[i].name
            << " (" << airspace.nodes[i].x << "," << airspace.nodes[i].y << ")" << endl;
    }
    cout << "----------------------" << endl;
}

// naya flight add krne ka menu
void addFlightMenu() {
    string id, model; int fuel, dest;
    cout << "Enter Flight ID: "; cin >> id;
    cout << "Enter Model: "; cin >> model;
    cout << "Enter Fuel %: "; cin >> fuel;

    cout << "\n--- Spawn Location? ---" << endl;
    cout << "1. At a Specific Node" << endl;
    cout << "2. At Custom Coordinates (x, y)" << endl;
    int type; cin >> type;

    int startX, startY, startNodeID;
    if (type == 1) {
        printLocations(); // pehle list dikhao
        cout << "Enter Node ID: "; cin >> startNodeID;
        if (startNodeID < 0 || startNodeID >= airspace.nodeCount) startNodeID = 0;
        startX = airspace.nodes[startNodeID].x;
        startY = airspace.nodes[startNodeID].y;
    }
    else {
        cout << "Enter X (0-19): "; cin >> startX;
        cout << "Enter Y (0-19): "; cin >> startY;
        startNodeID = airspace.getNearestNode(startX, startY);
        cout << "[ATC] Nearest Navigation Beacon is " << airspace.nodes[startNodeID].name << endl;
    }

    printLocations();
    cout << "Enter Destination Node ID: "; cin >> dest;
    if (dest < 0 || dest >= airspace.nodeCount) dest = 0;

    int prio = (fuel < 10) ? 2 : 3;
    Plane p(id, model, fuel, prio, startX, startY, startNodeID, dest);
    p.nextHopID = airspace.getNextHop(startNodeID, dest);
    if (type == 2) p.nextHopID = startNodeID;

    registry.insert(p);
    landingQueue.insert(registry.search(id));
    cout << "Flight Filed!" << endl; system("pause");
}

// plane land krne ka function
void landFlight() {
    if (landingQueue.isEmpty()) { cout << "Queue Empty." << endl; system("pause"); return; }
    Plane* p = landingQueue.extractMin();

    // check kro agar destination pe hai ya emergency hai
    if ((p->x != airspace.nodes[p->finalDestID].x || p->y != airspace.nodes[p->finalDestID].y) && p->priority > 1) {
        cout << "Flight " << p->id << " is NOT at destination! Cannot land." << endl;
        landingQueue.insert(p);
    }
    else {
        p->status = "Landed";
        time_t now = time(0); string dt = ctime(&now); dt.pop_back();
        flightLog.insert("[" + dt + "] " + p->id + " Landed.");
        cout << "Touchdown confirmed for " << p->id << endl;
    }
    system("pause");
}

// flight search logic (clean box output)
void searchFlight() {
    string id; cout << "Enter Flight ID: "; cin >> id;
    Plane* p = registry.search(id);

    cout << "\nSEARCHING DATABASE... ";
    if (p) {
        cout << "FOUND." << endl;
        cout << setfill(' '); // 00000 wala bug fix
        cout << "------------------------------------------" << endl;
        cout << "| Flight ID : " << setw(25) << left << p->id << "|" << endl;
        cout << "| Model     : " << setw(25) << left << p->model << "|" << endl;
        cout << "| Origin    : " << setw(25) << left << airspace.nodes[p->currentID].name << "|" << endl;
        cout << "| Fuel      : " << p->fuel << "% " << setw(20) << left << (p->fuel < 10 ? "(CRITICAL)" : "") << "|" << endl;
        cout << "| Status    : " << setw(25) << left << p->status << "|" << endl;
        cout << "------------------------------------------" << endl;
    }
    else {
        cout << "NOT FOUND." << endl;
    }
    system("pause");
}

// roman urdu emergency menu
void emergencyMenu() {
    string id; cout << "Enter ID: "; cin >> id;
    Plane* p = registry.search(id);
    if (p) {
        cout << "Select Emergency:" << endl;
        cout << "1. Medical (Banda marr rha hai)" << endl;
        cout << "2. Engine (Dhuan nikal raha hai)" << endl;
        cout << "3. Hijack (Bhai ne pistol nikal li)" << endl;
        int t; cin >> t;
        p->priority = 1;
        landingQueue.decreaseKey(p->id, 1);
        cout << "Emergency Declared!" << endl;
    }
    else cout << "Not Found." << endl;
    system("pause");
}

// dijkstra route calculator
void calculateRouteMenu() {
    cout << "\n--- ROUTE CALCULATOR ---" << endl;
    cout << "1. For an Active Plane" << endl;
    cout << "2. Manual Start/End Points" << endl;
    int c; cin >> c;

    if (c == 1) {
        string id; cout << "Enter Flight ID: "; cin >> id;
        Plane* p = registry.search(id);
        if (p) {
            cout << "Current Loc: (" << p->x << "," << p->y << ")" << endl;
            airspace.printSafeRoute(p->currentID, p->finalDestID);
        }
        else cout << "Flight not found." << endl;
    }
    else {
        int s, e;
        printLocations();
        cout << "Start Node ID: "; cin >> s;
        cout << "End Node ID: "; cin >> e;
        airspace.printSafeRoute(s, e);
    }
    system("pause");
}

// save and exit
void saveData() {
    ofstream outFile("flight_data.txt"); registry.saveToFile(outFile); outFile.close();
    ofstream logFile("flight_logs.txt"); flightLog.saveLogs(logFile); logFile.close();
    cout << "Saved." << endl; system("pause");
}

int main() {
    initializeSystem();
    while (true) {
        displayRadar();
        cout << "\n--- SKYNET ATC CONSOLE ---" << endl;
        cout << "1. Add Flight (Node or Coords)" << endl;
        cout << "2. Declare Emergency" << endl;
        cout << "3. Land Plane" << endl;
        cout << "4. Search Flight Details" << endl;
        cout << "5. View Flight Logs" << endl;
        cout << "6. Save & Exit" << endl;
        cout << "7. [STEP SIMULATION]" << endl;
        cout << "8. Calculate Safe Route (Dijkstra)" << endl;
        cout << "9. Print Landing Queue (Heap)" << endl;
        cout << "Choice: ";
        int c; cin >> c;
        switch (c) {
        case 1: addFlightMenu(); break;
        case 2: emergencyMenu(); break;
        case 3: landFlight(); break;
        case 4: searchFlight(); break;
        case 5: flightLog.displayLogs(); system("pause"); break;
        case 6: saveData(); return 0;
        case 7: stepSimulation(); break;
        case 8: calculateRouteMenu(); break;
        case 9: landingQueue.printQueue(); system("pause"); break;
        }
    }
}
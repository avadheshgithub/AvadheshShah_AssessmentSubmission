#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <limits>
#include <iomanip>
#include <random>

using namespace std;

// --- DATA STRUCTURES ---

struct Room {
    int floor;
    int number;
    int index;      // Horizontal distance from lift 
    bool isBooked;
    string id;      // "Floor-Index" for easy lookup

    // Constructor
    Room(int f, int i) {
        floor = f;
        index = i;
        isBooked = false;
        // Floor 1-9: 101, 201... Floor 10: 1001
        number = (f * 100) + (i + 1);
        id = to_string(f) + "-" + to_string(i);
    }
};

// Global State
vector<Room> hotel;
vector<string> lastBookedIds;

// --- HELPER FUNCTIONS ---

// Initialize the Hotel (97 Rooms)
void initHotel() {
    hotel.clear();
    for (int f = 1; f <= 10; ++f) {
        [cite_start]// Floor 10 has 7 rooms, others have 10 [cite: 19-22]
        int roomCount = (f == 10) ? 7 : 10;
        for (int i = 0; i < roomCount; ++i) {
            hotel.push_back(Room(f, i));
        }
    }
}

// Visualization in Console
void printHotel() {
    cout << "\n--- HOTEL VISUALIZATION ---\n";
    cout << "[ ]=Available  [X]=Booked  [*]=Just Booked\n\n";
    
    // Print Floor 10 down to 1
    for (int f = 10; f >= 1; --f) {
        cout << "Floor " << setw(2) << f << " | ";
        
        // Get rooms for this floor
        vector<Room*> floorRooms;
        for (auto &r : hotel) {
            if (r.floor == f) floorRooms.push_back(&r);
        }
        
        // Sort by index (Left to Right)
        sort(floorRooms.begin(), floorRooms.end(), [](Room* a, Room* b) {
            return a->index < b->index;
        });

        for (auto r : floorRooms) {
            bool isLastBooked = false;
            for(const string& id : lastBookedIds) {
                if(id == r->id) isLastBooked = true;
            }

            if (isLastBooked) cout << "[*] ";
            else if (r->isBooked) cout << "[X] ";
            else cout << "[ ] ";
        }
        cout << "\n";
    }
    cout << "          ^ LIFT ^\n\n";
}

// Reset System
void resetSystem() {
    initHotel();
    lastBookedIds.clear();
    cout << "System Reset. All rooms available.\n";
}

// Generate Random Occupancy
void generateRandom() {
    random_device rd;
    mt19937 gen(rd());
    bernoulli_distribution d(0.3); // 30% chance

    for (auto &r : hotel) {
        r.isBooked = d(gen);
    }
    lastBookedIds.clear();
    cout << "Random occupancy generated.\n";
    printHotel();
}

[cite_start]// --- CORE LOGIC ENGINE [cite: 31-33] ---

void bookRooms(int n) {
    if (n < 1 || n > 5) {
        cout << "Error: You can only book 1 to 5 rooms.\n";
        return;
    }

    // Filter available rooms
    vector<Room*> available;
    for (auto &r : hotel) {
        if (!r.isBooked) available.push_back(&r);
    }

    if (available.size() < n) {
        cout << "Error: Not enough rooms available.\n";
        return;
    }

    vector<Room*> bestSet;
    int minCost = numeric_limits<int>::max();
    bool foundSameFloor = false;

    // --- PRIORITY 1: SAME FLOOR OPTIMIZATION ---
    // Group by floor
    map<int, vector<Room*>> floors;
    for (auto r : available) {
        floors[r->floor].push_back(r);
    }

    for (auto const& [floorNum, roomsOnFloor] : floors) {
        if (roomsOnFloor.size() >= n) {
            // Create a local copy to sort
            vector<Room*> sortedRooms = roomsOnFloor;
            sort(sortedRooms.begin(), sortedRooms.end(), [](Room* a, Room* b) {
                return a->index < b->index;
            });

            // Sliding Window
            for (size_t i = 0; i <= sortedRooms.size() - n; ++i) {
                // Cost = Horizontal Distance (Last Index - First Index)
                int cost = sortedRooms[i + n - 1]->index - sortedRooms[i]->index;
                
                if (cost < minCost) {
                    minCost = cost;
                    bestSet.clear();
                    for (int k = 0; k < n; ++k) bestSet.push_back(sortedRooms[i + k]);
                    foundSameFloor = true;
                }
            }
        }
    }

    // --- PRIORITY 2: CROSS-FLOOR OPTIMIZATION ---
    // Only if Same Floor failed
    if (!foundSameFloor) {
        cout << "Info: Checking across floors (Minimizing Travel Time)...\n";
        
        
        vector<Room*> sortedAvailable = available;
        sort(sortedAvailable.begin(), sortedAvailable.end(), [](Room* a, Room* b) {
            int scoreA = (a->floor * 2) + a->index;
            int scoreB = (b->floor * 2) + b->index;
            return scoreA < scoreB;
        });

        minCost = numeric_limits<int>::max(); // Reset for global search

        // Sliding Window over sorted list
        for (size_t i = 0; i <= sortedAvailable.size() - n; ++i) {
            int maxF = 0, minF = 11, maxI = 0, minI = 11;
            
            // Analyze the subset window
            for (int k = 0; k < n; ++k) {
                Room* r = sortedAvailable[i + k];
                if (r->floor > maxF) maxF = r->floor;
                if (r->floor < minF) minF = r->floor;
                if (r->index > maxI) maxI = r->index;
                if (r->index < minI) minI = r.index;
            }

            // Calculate Total Travel Time
            int vCost = (maxF - minF) * 2; // 2 mins per floor
            int hCost = (maxI - minI) * 1; // 1 min per room
            int totalCost = vCost + hCost;

            if (totalCost < minCost) {
                minCost = totalCost;
                bestSet.clear();
                for (int k = 0; k < n; ++k) bestSet.push_back(sortedAvailable[i + k]);
            }
        }
    }

    // --- APPLY BOOKING ---
    if (!bestSet.empty()) {
        lastBookedIds.clear();
        cout << "Success! Booked Rooms: ";
        for (auto r : bestSet) {
            r->isBooked = true;
            lastBookedIds.push_back(r->id);
            cout << r->number << " ";
        }
        cout << "(Travel Cost: " << minCost << ")\n";
        printHotel();
    } else {
        cout << "Error: Could not find suitable rooms.\n";
    }
}

// --- MAIN MENU ---

int main() {
    initHotel();
    
    int choice;
    while (true) {
        cout << "\n--- HOTEL MENU ---\n";
        cout << "1. Book Rooms\n";
        cout << "2. Generate Random Occupancy\n";
        cout << "3. Reset System\n";
        cout << "4. Show Grid\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 0) break;
        
        switch (choice) {
            case 1: {
                int n;
                cout << "Enter number of rooms (1-5): ";
                cin >> n;
                bookRooms(n);
                break;
            }
            case 2:
                generateRandom();
                break;
            case 3:
                resetSystem();
                printHotel();
                break;
            case 4:
                printHotel();
                break;
            default:
                cout << "Invalid choice.\n";
        }
    }
    return 0;
}

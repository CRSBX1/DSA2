#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>
#include <iostream>
using namespace std;

/*
Data amount:
1. Amount of orders depends on entered user input
2. Items may be deleted or added by users
3. Amount of robots depends on entered user input
4. number of navigation modules per robot amount

Data structures:
1. Orders: Queue
2. Robots: Circular Queue
3. Robot Navigation: Stacks
4. Items: Binary search tree
5. Warehouse: Binary search tree

All data structure except robot navigation needs to be sorted:
1. Order by arrival time
2. Robots by ID
3. Not needed
4. Item by ID
5. Hierarchical structure
*/

//section 1: Linear Queue
struct Order {
    string orderID;
    string itemID;
    int arrivalTime;     // stored as total minutes, ex. 1:30 = 90
    string processState; // "Pending", "In Progress", or "Completed"

    // constructor with parameters
    Order(string id, int hour, int minute, string item1) {
        orderID = id;
        itemID = item1;
        arrivalTime = hour * 60 + minute;
        processState = "Pending";
    }

    // default constructor (needed when dequeue returns an empty order)
    Order() {
        orderID = "";
        itemID = "";
        arrivalTime = 0;
        processState = "";
    }
};

// node for the order queue linked list
struct OrderNode {
    Order order;
    OrderNode* next;

    OrderNode(Order o) : order(o), next(nullptr) {}
};

//function declarations (section 1)
// max orders the queue can hold before its full
const int MAX_QUEUE_SIZE = 200;
// basic queue operations
void enqueue(Order newOrder);
Order dequeue();
bool isEmpty();
bool isFull();
int getQueueSize();
// sorted insert (by arrival time)
void enqueueSorted(Order newOrder);
// order management features
void addNewOrder();
void processNextOrder();
void processAllOrders();
void displayPendingOrders();
void displayNextOrder();
void displayCompletedOrders();
void displaySystemStatus();
// extra features
void searchOrder();
void cancelOrder();
// for robot assignment module to grab the next order
Order getNextOrderForRobot();
bool hasOrdersWaiting();
void markOrderCompleted(Order completedOrder);
void processAllPendingOrders();
// menu
void orderManagementMenu();
// helper
string convertTimeToString(int minutes);

//section 2: Circular Queue
struct Robot{ //Robot object
    string robotID;
    string assigneditemID;
    string currentTask;
    bool idle;
    bool inMaintenance;
};

struct RobotNode{ //Robot Node
    Robot robot;
    RobotNode* next;
};

void robotMenu();

//section 3: stack (1 stack per robot)

struct RobotMovement{
    int id;
    string direction;
    RobotMovement* next;
    RobotMovement* prev;
};

// Fixed-size log that records every navigation event during a journey.
// Uses a plain array instead of an STL container (containers are not allowed).
struct NavigationLog{
    static const int LOG_CAPACITY = 100; // maximum number of log entries
    string entries[LOG_CAPACITY];        // the stored log messages
    int count;                           // how many entries are currently stored

    NavigationLog(){
        count = 0;
    }

    // adds one message to the log if there is still room left
    void add(string entry){
        if (count < LOG_CAPACITY){
            entries[count] = entry;
            count++;
        }
    }

    // prints every stored entry in the order it happened
    void display(){
        cout << endl;
        cout << "  ---------- Complete Navigation Log ----------" << endl;
        for (int i = 0; i < count; i++){
            cout << "  " << entries[i] << endl;
        }
        cout << "  ---------------------------------------------" << endl;
    }
};

class movementStack{ //Navigation module stack
    int id;
    int maxSize;

    public:
        movementStack(int size);
        void push(RobotMovement* &top, string drc);
        string pop(RobotMovement* &top);
        bool isStackEmpty();
        // reads the top move, manually reverses it, removes it, returns the reversed direction
        string backtrack(RobotMovement* &top);
        // runs a full forward + return journey along the given movement path string
        void executeNavigation(RobotMovement* &top, string path, bool useObstacles);
};

// largest number of movement steps a single navigation stack can hold
const int MAX_PATH_STEPS = 100;

// returns true when a random obstacle blocks the current step (probability is a 0-100 percent chance)
bool obstacleDetected(int probability);
// returns a detour direction for a blocked one: Forward->Left, Left->Right, Right->Forward
string getAlternative(string blocked);
// standalone menu for the robot navigation & path tracking module
void navigationMenu();

//Section 4: Binary search tree
struct Item{
    string itemID;
    string itemName;
    int zoneLocation;
    int aisleLocation;
    int shelfLocation;
    Item* left;
    Item* right;
};

//Shared globals
extern Item* itemManagement;
extern int totalItems;
//Public functions
void itemMenu();
Item* searchIDNode(Item* node, string id, int &comparisons);
void loadItemsFromFile(string filename);
void saveItemsToFile(string filename);

//Section 5: Binary tree representing the warehouse hierarchy
//Tree shape: Warehouse -> 2 Zones -> 4 Aisles -> 8 Shelves (15 nodes total)
//At every level the left child has a smaller location number than the right child,
//which lets us treat the tree as a BST for navigation.
struct Warehouse{
    string locationName; //"Warehouse", "Zone", "Aisle", or "Shelf"
    int locationNumber;  //actual number of this section (0 for root, 1-2 for zones, 1-4 for aisles, 1-8 for shelves)
    Warehouse* left;     //child holding the smaller-numbered sub-location
    Warehouse* right;    //child holding the larger-numbered sub-location
};

//Shared globals
extern Warehouse* warehouseRoot;

//Public functions
void warehouseMenu();
void buildWarehouse();
void cleanupWarehouse();
string getPathToLocation(int zone, int aisle, int shelf);
bool isValidWarehouseLocation(int zone, int aisle, int shelf);

#endif

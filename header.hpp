#include <string>
#include <iostream>
using namespace std;

/*
Data amount:
1. 200 orders
2. 200 items
3. 50 robots
4. 50 navigation modules

Data structures:
1. Orders: Queue
2. Robots: Circular Queue
3. Robot Navigation: Stacks
4. Items: Binary search tree
5. Warehouse: Binary search tree

All data structure except robot navigation needs to be sorted:
1. Order by arrival time
2. Robots by ID
3. Not needed, since 
4. 

*/


//section 1: Linear Queue
struct Order{
    string orderID;
    string itemID;
    int arrivalTime; // arrival time as minutes; i.e. 10:30 to 630 minutes
    string processState; //Pending, In Progress, Completed

    Order(string id, int hour, int minute, string item1){
        orderID = id;
        itemID = item1;
        arrivalTime = hour*60+minute;
        processState = "Pending";
    }
};

struct OrderNode{
    Order order;
    OrderNode* next;
};

//section 2: Circular Queue

struct Robot{
    string robotID;
    string assigneditemID;
    bool active;
};

struct RobotNode{
    Robot robot;
    RobotNode* next;
};

//section 3: stack (1 stack per robot)

struct RobotMovement{
    string direction;
    RobotMovement* next;
    RobotMovement* previous;
};

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

//Section 5: Binary search tree
struct Warehouse{
    string locationName; //Zone, Aisle, Shelf
    int locationNumber; //1 or 2
    Warehouse* left;
    Warehouse* right;
};
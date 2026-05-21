#ifndef HEADER_HPP
#define HEADER_HPP

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

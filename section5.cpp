/*
Section 5: Warehouse Layout & Navigation Module

This module builds a binary tree representing the warehouse structure.
The tree has 4 levels: Warehouse -> Zones -> Aisles -> Shelves

We use BST property (left child = smaller number, right child = bigger number)
so we can navigate by comparing target values at each level.

The main output is a path string like "LRL" which tells the robot
which direction to turn at each level. Section 3 pushes these onto
a stack so the robot can reverse the path later.
*/

#include "header.hpp"
#include <limits>
using namespace std;

Warehouse* warehouseRoot = nullptr;

int readWarehouseInt(string prompt);


// ---- TREE CONSTRUCTION ----

// just creates a node with the given name and number
Warehouse* createWarehouseNode(string name, int number) {
    Warehouse* node = new Warehouse();
    node->locationName = name;
    node->locationNumber = number;
    node->left = nullptr;
    node->right = nullptr;
    return node;
}

// builds the whole warehouse tree manually
// structure is fixed: 2 zones, 4 aisles, 8 shelves
void buildWarehouse() {
    cleanupWarehouse(); // clear old tree if any

    warehouseRoot = createWarehouseNode("Warehouse", 0);

    // zones
    warehouseRoot->left  = createWarehouseNode("Zone", 1);
    warehouseRoot->right = createWarehouseNode("Zone", 2);

    // aisles - zone 1 gets aisle 1&2, zone 2 gets aisle 3&4
    warehouseRoot->left->left   = createWarehouseNode("Aisle", 1);
    warehouseRoot->left->right  = createWarehouseNode("Aisle", 2);
    warehouseRoot->right->left  = createWarehouseNode("Aisle", 3);
    warehouseRoot->right->right = createWarehouseNode("Aisle", 4);

    // shelves - 2 per aisle, numbered 1-8 across the whole warehouse
    warehouseRoot->left->left->left    = createWarehouseNode("Shelf", 1);
    warehouseRoot->left->left->right   = createWarehouseNode("Shelf", 2);
    warehouseRoot->left->right->left   = createWarehouseNode("Shelf", 3);
    warehouseRoot->left->right->right  = createWarehouseNode("Shelf", 4);
    warehouseRoot->right->left->left   = createWarehouseNode("Shelf", 5);
    warehouseRoot->right->left->right  = createWarehouseNode("Shelf", 6);
    warehouseRoot->right->right->left  = createWarehouseNode("Shelf", 7);
    warehouseRoot->right->right->right = createWarehouseNode("Shelf", 8);
}

// delete tree using post-order (children first then parent)
void deleteSubtree(Warehouse* node) {
    if (node == nullptr) return;
    deleteSubtree(node->left);
    deleteSubtree(node->right);
    delete node;
}

void cleanupWarehouse() {
    deleteSubtree(warehouseRoot);
    warehouseRoot = nullptr;
}


// ---- VALIDATION ----

// checks if a zone/aisle/shelf combo actually exists in our layout
// zone 1 -> aisle 1,2 -> shelf 1-4
// zone 2 -> aisle 3,4 -> shelf 5-8
bool isValidWarehouseLocation(int zone, int aisle, int shelf) {
    if (zone == 1) {
        if (aisle < 1 || aisle > 2) return false;
        if (shelf < 1 || shelf > 4) return false;
    } else if (zone == 2) {
        if (aisle < 3 || aisle > 4) return false;
        if (shelf < 5 || shelf > 8) return false;
    } else {
        return false;
    }
    // each aisle only has 2 shelves so we check that too
    int shelfMin = (aisle - 1) * 2 + 1;
    int shelfMax = shelfMin + 1;
    return shelf >= shelfMin && shelf <= shelfMax;
}


// ---- PATH GENERATION ----

// walks down the tree level by level to find the target shelf
// at each level: if target <= left child number, go left. otherwise go right.
// builds a string like "LRL" that section 3 can push onto the stack
string getPathToLocation(int zone, int aisle, int shelf) {
    if (warehouseRoot == nullptr) return "";
    if (!isValidWarehouseLocation(zone, aisle, shelf)) return "";

    string path = "";
    Warehouse* current = warehouseRoot;

    // level 1 - pick zone
    if (zone <= current->left->locationNumber) {
        path += 'L';
        current = current->left;
    } else {
        path += 'R';
        current = current->right;
    }

    // level 2 - pick aisle
    if (aisle <= current->left->locationNumber) {
        path += 'L';
        current = current->left;
    } else {
        path += 'R';
        current = current->right;
    }

    // level 3 - pick shelf
    if (shelf <= current->left->locationNumber) {
        path += 'L';
    } else {
        path += 'R';
    }

    return path;
}

// finds path between two shelves using lowest common ancestor
// basically: find where the two paths diverge, go UP from start to that point,
// then go DOWN to the destination
// e.g. shelf 1 ("LLL") to shelf 8 ("RRR") -> they diverge at index 0
//      so result is "UUURRR" (3 ups + 3 downs)
string getPathBetween(int zoneA, int aisleA, int shelfA,
                      int zoneB, int aisleB, int shelfB) {
    string from = getPathToLocation(zoneA, aisleA, shelfA);
    string to   = getPathToLocation(zoneB, aisleB, shelfB);
    if (from.empty() || to.empty()) return "";

    // find shared prefix
    int common = 0;
    int minLen = (int)from.length() < (int)to.length() ? (int)from.length() : (int)to.length();
    while (common < minLen && from[common] == to[common]) common++;

    // go up for remaining steps in 'from', then down for remaining steps in 'to'
    string result = "";
    for (int i = common; i < (int)from.length(); i++) result += 'U';
    for (int i = common; i < (int)to.length();   i++) result += to[i];
    return result;
}


// ---- TRAVERSALS ----

// pre-order: root first then children
void preOrder(Warehouse* node) {
    if (node == nullptr) return;
    cout << "  " << node->locationName << " " << node->locationNumber << endl;
    preOrder(node->left);
    preOrder(node->right);
}

// in-order: left, root, right (gives sorted order because BST)
void inOrder(Warehouse* node) {
    if (node == nullptr) return;
    inOrder(node->left);
    cout << "  " << node->locationName << " " << node->locationNumber << endl;
    inOrder(node->right);
}

// post-order: children first then root
void postOrder(Warehouse* node) {
    if (node == nullptr) return;
    postOrder(node->left);
    postOrder(node->right);
    cout << "  " << node->locationName << " " << node->locationNumber << endl;
}


// ---- DISPLAY (table layout) ----

// cell widths - each parent spans 2 children + 1 divider
const int SHELF_W = 8;
const int AISLE_W = SHELF_W * 2 + 1;   // 17
const int ZONE_W  = AISLE_W * 2 + 1;   // 35
const int WH_W    = ZONE_W  * 2 + 1;   // 71
const int TABLE_W = WH_W + 2;          // 73

// manual int to string (cant use to_string cuz STL restriction)
string intToStr(int n) {
    if (n == 0) return "0";
    string s = "";
    int x = n < 0 ? -n : n;
    while (x > 0) {
        char c = '0' + (x % 10);
        s = string(1, c) + s;
        x /= 10;
    }
    if (n < 0) s = "-" + s;
    return s;
}

// prints "|" then centers the text inside the given width
void printCell(string text, int width) {
    int len = (int)text.length();
    if (len > width) len = width;
    int leftPad  = (width - len) / 2;
    int rightPad = width - len - leftPad;
    cout << "|";
    for (int i = 0; i < leftPad; i++)  cout << ' ';
    cout << text;
    for (int i = 0; i < rightPad; i++) cout << ' ';
}

// checks if theres a vertical bar at this column for a given level
bool hasBarAt(int level, int col) {
    int step = (level == 0) ? WH_W + 1
             : (level == 1) ? ZONE_W + 1
             : (level == 2) ? AISLE_W + 1
             :                SHELF_W + 1;
    return (col % step) == 0;
}

// prints the horizontal line between rows
// puts '+' where vertical bars from above or below meet the line
void printSeparator(int aboveLevel, int belowLevel) {
    cout << "  ";
    for (int col = 0; col < TABLE_W; col++) {
        bool plus = false;
        if (aboveLevel >= 0 && hasBarAt(aboveLevel, col)) plus = true;
        if (belowLevel >= 0 && hasBarAt(belowLevel, col)) plus = true;
        cout << (plus ? '+' : '-');
    }
    cout << endl;
}

// recursively goes to the target depth and prints a cell for each node there
void emitLevelCells(Warehouse* node, int depth, int targetDepth, int cellWidth) {
    if (node == nullptr) return;
    if (depth == targetDepth) {
        string label;
        if (node->locationName == "Warehouse")
            label = "Warehouse";
        else if (node->locationName == "Shelf")
            label = "S" + intToStr(node->locationNumber);
        else
            label = node->locationName + " " + intToStr(node->locationNumber);
        printCell(label, cellWidth);
        return;
    }
    emitLevelCells(node->left,  depth + 1, targetDepth, cellWidth);
    emitLevelCells(node->right, depth + 1, targetDepth, cellWidth);
}

// prints one row of the table
void printLevelRow(int level) {
    int width = (level == 0) ? WH_W
              : (level == 1) ? ZONE_W
              : (level == 2) ? AISLE_W
              :                SHELF_W;
    cout << "  ";
    emitLevelCells(warehouseRoot, 0, level, width);
    cout << "|" << endl;
}

// converts L/R/U to readable words
string describeStep(char c) {
    if (c == 'L') return "Left";
    if (c == 'R') return "Right";
    if (c == 'U') return "Up";
    return "?";
}

// prints path as "Left -> Right -> Left"
void printPath(string path) {
    if (path.empty()) {
        cout << "  (no steps)" << endl;
        return;
    }
    cout << "  ";
    for (int i = 0; i < (int)path.length(); i++) {
        cout << describeStep(path[i]);
        if (i < (int)path.length() - 1) cout << " -> ";
    }
    cout << endl;
}

// shows the full path details including which nodes are visited
void printPathWithNodes(int zone, int aisle, int shelf) {
    string path = getPathToLocation(zone, aisle, shelf);
    if (path.empty()) {
        cout << "  Invalid destination." << endl;
        return;
    }

    cout << "  Path string: " << path << endl;
    cout << "  Steps:" << endl;
    printPath(path);

    cout << "  Nodes visited:" << endl;
    Warehouse* current = warehouseRoot;
    cout << "    " << current->locationName << endl;
    for (int i = 0; i < (int)path.length(); i++) {
        current = (path[i] == 'L') ? current->left : current->right;
        cout << "    -> " << current->locationName << " " << current->locationNumber << endl;
    }
}


// ---- INPUT HELPER ----

// reads an int safely, keeps asking if user types garbage
int readWarehouseInt(string prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "  Invalid input. Please enter a number." << endl;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}


// ---- MENU ----

void displayLayout() {
    if (warehouseRoot == nullptr) {
        cout << "  Warehouse layout has not been built yet." << endl;
        return;
    }
    cout << "  Warehouse Layout:" << endl;
    cout << endl;
    printSeparator(-1, 0);
    printLevelRow(0);       // warehouse
    printSeparator(0, 1);
    printLevelRow(1);       // zones
    printSeparator(1, 2);
    printLevelRow(2);       // aisles
    printSeparator(2, 3);
    printLevelRow(3);       // shelves
    printSeparator(3, -1);
}

void runTraversals() {
    if (warehouseRoot == nullptr) {
        cout << "  Warehouse layout has not been built yet." << endl;
        return;
    }
    cout << "  Pre-order:" << endl;
    preOrder(warehouseRoot);
    cout << endl;
    cout << "  In-order:" << endl;
    inOrder(warehouseRoot);
    cout << endl;
    cout << "  Post-order:" << endl;
    postOrder(warehouseRoot);
}

void findPathToShelf() {
    int zone  = readWarehouseInt("  Enter Zone (1 or 2): ");
    int aisle = readWarehouseInt("  Enter Aisle: ");
    int shelf = readWarehouseInt("  Enter Shelf: ");

    if (!isValidWarehouseLocation(zone, aisle, shelf)) {
        cout << "  That combination does not exist in the layout." << endl;
        return;
    }

    cout << endl;
    cout << "  Route from Warehouse to Zone " << zone
         << " / Aisle " << aisle << " / Shelf " << shelf << ":" << endl;
    printPathWithNodes(zone, aisle, shelf);
}

void findPathBetweenShelves() {
    cout << "  Starting location" << endl;
    int zoneA  = readWarehouseInt("    Enter Zone: ");
    int aisleA = readWarehouseInt("    Enter Aisle: ");
    int shelfA = readWarehouseInt("    Enter Shelf: ");

    cout << "  Destination" << endl;
    int zoneB  = readWarehouseInt("    Enter Zone: ");
    int aisleB = readWarehouseInt("    Enter Aisle: ");
    int shelfB = readWarehouseInt("    Enter Shelf: ");

    if (!isValidWarehouseLocation(zoneA, aisleA, shelfA) ||
        !isValidWarehouseLocation(zoneB, aisleB, shelfB)) {
        cout << "  One of the locations does not exist in the layout." << endl;
        return;
    }

    string path = getPathBetween(zoneA, aisleA, shelfA, zoneB, aisleB, shelfB);
    cout << endl;
    cout << "  Steps from start to destination:" << endl;
    printPath(path);
    cout << "  ('U' means step back up to the parent node.)" << endl;
}

void warehouseMenu() {
    if (warehouseRoot == nullptr) buildWarehouse();

    int choice;
    do {
        cout << endl;
        cout << "  ========================================================" << endl;
        cout << "          Warehouse Layout & Navigation Module" << endl;
        cout << "  ========================================================" << endl;
        cout << "  1. Display Warehouse Layout" << endl;
        cout << "  2. Run Tree Traversals (pre/in/post-order)" << endl;
        cout << "  3. Find Path to a Shelf" << endl;
        cout << "  4. Find Path Between Two Shelves" << endl;
        cout << "  5. Back to Main Menu" << endl;
        cout << "  ========================================================" << endl;
        choice = readWarehouseInt("  Enter choice: ");
        cout << endl;

        switch (choice) {
            case 1: displayLayout(); break;
            case 2: runTraversals(); break;
            case 3: findPathToShelf(); break;
            case 4: findPathBetweenShelves(); break;
            case 5: cout << "  Returning to main menu..." << endl; break;
            default: cout << "  Invalid choice." << endl;
        }
    } while (choice != 5);
}

int main() {
    warehouseMenu();
    cleanupWarehouse();
    return 0;
}

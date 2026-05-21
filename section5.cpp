#include "header.hpp"
#include <limits>
using namespace std;

//Definition of the shared global declared in header.hpp
Warehouse* warehouseRoot = nullptr;

//Forward declarations for internal helpers
int readWarehouseInt(string prompt);

//TREE CONSTRUCTION

//Allocate a single warehouse node
Warehouse* createWarehouseNode(string name, int number) {
    Warehouse* node = new Warehouse();
    node->locationName = name;
    node->locationNumber = number;
    node->left = nullptr;
    node->right = nullptr;
    return node;
}

//Build the fixed warehouse hierarchy described in the layout diagram.
//Re-running this safely tears down the previous tree first so the test
//menu can be opened multiple times during a single program run.
void buildWarehouse() {
    cleanupWarehouse();

    //Root
    warehouseRoot = createWarehouseNode("Warehouse", 0);

    //Zone level
    warehouseRoot->left  = createWarehouseNode("Zone", 1);
    warehouseRoot->right = createWarehouseNode("Zone", 2);

    //Aisle level: zone 1 owns aisles 1-2, zone 2 owns aisles 3-4
    warehouseRoot->left->left   = createWarehouseNode("Aisle", 1);
    warehouseRoot->left->right  = createWarehouseNode("Aisle", 2);
    warehouseRoot->right->left  = createWarehouseNode("Aisle", 3);
    warehouseRoot->right->right = createWarehouseNode("Aisle", 4);

    //Shelf level: each aisle holds 2 shelves, numbered consecutively across the warehouse
    warehouseRoot->left->left->left    = createWarehouseNode("Shelf", 1);
    warehouseRoot->left->left->right   = createWarehouseNode("Shelf", 2);
    warehouseRoot->left->right->left   = createWarehouseNode("Shelf", 3);
    warehouseRoot->left->right->right  = createWarehouseNode("Shelf", 4);
    warehouseRoot->right->left->left   = createWarehouseNode("Shelf", 5);
    warehouseRoot->right->left->right  = createWarehouseNode("Shelf", 6);
    warehouseRoot->right->right->left  = createWarehouseNode("Shelf", 7);
    warehouseRoot->right->right->right = createWarehouseNode("Shelf", 8);
}

//Recursive post-order delete to release every node
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

//VALIDATION
//validates a (zone, aisle, shelf) triple against the fixed layout
//Mirrors the rule used by Section 4 so the two modules can never disagree
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
    //Each aisle owns exactly 2 shelves, so the shelf must fall in that aisle's range
    int shelfMin = (aisle - 1) * 2 + 1;
    int shelfMax = shelfMin + 1;
    return shelf >= shelfMin && shelf <= shelfMax;
}

//PATH GENERATION (BST traversal)
//descends the tree level by level, choosing left or right by comparing the
//target value against the current node's children. Each decision is appended
//to the path string as 'L' or 'R'. Section 3 will push these characters onto
//the robot navigation stack so the return trip can pop them in reverse
string getPathToLocation(int zone, int aisle, int shelf) {
    if (warehouseRoot == nullptr) return "";
    if (!isValidWarehouseLocation(zone, aisle, shelf)) return "";

    string path = "";
    Warehouse* current = warehouseRoot;

    //lLevel 1: pick the zone (left = zone 1, right = zone 2)
    if (zone <= current->left->locationNumber) {
        path += 'L';
        current = current->left;
    } else {
        path += 'R';
        current = current->right;
    }

    //level 2: pick the aisle inside the chosen zone
    if (aisle <= current->left->locationNumber) {
        path += 'L';
        current = current->left;
    } else {
        path += 'R';
        current = current->right;
    }

    //level 3: pick the shelf inside the chosen aisle
    if (shelf <= current->left->locationNumber) {
        path += 'L';
    } else {
        path += 'R';
    }

    return path;
}

//Computes the path from one shelf to another by walking up to the lowest
//common ancestor and then down to the destination. Going up is recorded as
//'U', going down as 'L' or 'R'. Useful when robots move between tasks
//without returning to the warehouse root in between.
string getPathBetween(int zoneA, int aisleA, int shelfA,
                      int zoneB, int aisleB, int shelfB) {
    string from = getPathToLocation(zoneA, aisleA, shelfA);
    string to   = getPathToLocation(zoneB, aisleB, shelfB);
    if (from.empty() || to.empty()) return "";

    //find the shared prefix; what remains in 'from' must be undone, what
    //remains in 'to' must be walked.
    int common = 0;
    int minLen = (int)from.length() < (int)to.length() ? (int)from.length() : (int)to.length();
    while (common < minLen && from[common] == to[common]) common++;

    string result = "";
    for (int i = common; i < (int)from.length(); i++) result += 'U';
    for (int i = common; i < (int)to.length();   i++) result += to[i];
    return result;
}

//TRAVERSALS
//pre-order: root, left, right — useful for printing the structure top down.
void preOrder(Warehouse* node) {
    if (node == nullptr) return;
    cout << "  " << node->locationName << " " << node->locationNumber << endl;
    preOrder(node->left);
    preOrder(node->right);
}

//in-order: left, root, right 
void inOrder(Warehouse* node) {
    if (node == nullptr) return;
    inOrder(node->left);
    cout << "  " << node->locationName << " " << node->locationNumber << endl;
    inOrder(node->right);
}

//post-order: left, right, root 
void postOrder(Warehouse* node) {
    if (node == nullptr) return;
    postOrder(node->left);
    postOrder(node->right);
    cout << "  " << node->locationName << " " << node->locationNumber << endl;
}

//DISPLAY
//Cell widths derived from the fixed perfect-binary-tree shape:
//1 warehouse -> 2 zones -> 4 aisles -> 8 shelves, each shelf gets 8 chars
//Each parent spans (2 * child width + 1) so dividers from above flow
//straight down through every row, matching the merged-cell diagram
const int SHELF_W = 8;
const int AISLE_W = SHELF_W * 2 + 1;   //17
const int ZONE_W  = AISLE_W * 2 + 1;   //35
const int WH_W    = ZONE_W  * 2 + 1;   //71
const int TABLE_W = WH_W + 2;          //73 including outer borders

//Manual int-to-string so we don't pull in std::to_string or any helper that
//relies on built-in containers, matching the rules used in section 4
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

//Print "|" followed by `text` centered inside `width` characters.
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

//True when the row at level (0=Warehouse, 1=Zone, 2=Aisle, 3=Shelf) has a
//vertical bar at column `col`. Bar spacing equals each level's cell width
//plus its trailing border.
bool hasBarAt(int level, int col) {
    int step = (level == 0) ? WH_W + 1
             : (level == 1) ? ZONE_W + 1
             : (level == 2) ? AISLE_W + 1
             :                SHELF_W + 1;
    return (col % step) == 0;
}

//Separator line. `aboveLevel` / `belowLevel` are the row levels immediately
//above and below this line (use -1 for the table's top/bottom edge). A '+'
//appears wherever either neighbour has a bar so vertical dividers from
//upper rows pass through cleanly.
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

//Recursively walk down to the requested depth, then emit one cell per node
//at that depth. The cells naturally line up under their parent's cell
//because the warehouse tree is a perfect binary tree
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

//Print one full table row for the given level
void printLevelRow(int level) {
    int width = (level == 0) ? WH_W
              : (level == 1) ? ZONE_W
              : (level == 2) ? AISLE_W
              :                SHELF_W;
    cout << "  ";
    emitLevelCells(warehouseRoot, 0, level, width);
    cout << "|" << endl;
}

//translate a single direction character into a human readable label
string describeStep(char c) {
    if (c == 'L') return "Left";
    if (c == 'R') return "Right";
    if (c == 'U') return "Up";
    return "?";
}

//print a path with arrows between steps, e.g. "Left -> Right -> Right"
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

//print the path along with the locations it visits so the user can sanity
//check the result against the warehouse diagram.
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

//INPUT HELPER
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

//MENU
void displayLayout() {
    if (warehouseRoot == nullptr) {
        cout << "  Warehouse layout has not been built yet." << endl;
        return;
    }
    cout << "  Warehouse Layout:" << endl;
    cout << endl;
    printSeparator(-1, 0);//top edge
    printLevelRow(0);//Warehouse
    printSeparator(0, 1);
    printLevelRow(1);//Zones
    printSeparator(1, 2);
    printLevelRow(2);//Aisles
    printSeparator(2, 3);
    printLevelRow(3);//Shelves
    printSeparator(3, -1);//bottom edge
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

//Testing
int main() {
    warehouseMenu();
    cleanupWarehouse();
    return 0;
}

#include "header.hpp"
#include <fstream>
#include <limits>

//Definition of shared globals declared in header.hpp
Item* itemManagement = nullptr;
int totalItems = 0;

//Forward declarations for internal helpers
void displayItem(Item* item);
Item* createNode(string id, string name, int zone, int aisle, int shelf);
Item* insertNode(Item* node, Item* newItem);
void rebalanceTree();

//FILE HANDLING FUNCTIONS

//Load items from file into BST
void loadItemsFromFile(string filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "  Error: Could not open " << filename << endl;
        return;
    }

    string header;
    getline(file, header);

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        //Parse comma separated values
        string id = "", name = "", zoneStr = "", aisleStr = "", shelfStr = "";
        int field = 0;
        for (int i = 0; i <= (int)line.length(); i++) {
            char c = (i < (int)line.length()) ? line[i] : ',';
            if (c == ',') {
                field++;
            } else {
                if (field == 0) id += c;
                else if (field == 1) name += c;
                else if (field == 2) zoneStr += c;
                else if (field == 3) aisleStr += c;
                else if (field == 4) shelfStr += c;
            }
        }

        int zone = 0, aisle = 0, shelf = 0;
        for (int i = 0; i < (int)zoneStr.length(); i++)
            zone = zone * 10 + (zoneStr[i] - '0');
        for (int i = 0; i < (int)aisleStr.length(); i++)
            aisle = aisle * 10 + (aisleStr[i] - '0');
        for (int i = 0; i < (int)shelfStr.length(); i++)
            shelf = shelf * 10 + (shelfStr[i] - '0');

        Item* newItem = createNode(id, name, zone, aisle, shelf);
        itemManagement = insertNode(itemManagement, newItem);
    }
    file.close();

    //Rebalance after bulk load, because sequential IDs in the CSV would produce a right-skewed tree
    rebalanceTree();

    cout << "  Loaded " << totalItems << " items from file." << endl;
}

//Recursive helper to write nodes in order
void writeNodes(Item* node, ofstream &file) {
    if (node == nullptr) return;
    writeNodes(node->left, file);
    file << node->itemID << ","
         << node->itemName << ","
         << node->zoneLocation << ","
         << node->aisleLocation << ","
         << node->shelfLocation << endl;
    writeNodes(node->right, file);
}

//Save all items back to file using inorder traversal
void saveItemsToFile(string filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "  Error: Could not save to " << filename << endl;
        return;
    }
    file << "ItemID,ItemName,Zone,Aisle,Shelf" << endl;
    writeNodes(itemManagement, file);
    file.close();
}

//BST CORE OPERATIONS

//Create a new item node
Item* createNode(string id, string name, int zone, int aisle, int shelf) {
    Item* newNode = new Item();
    newNode->itemID = id;
    newNode->itemName = name;
    newNode->zoneLocation = zone;
    newNode->aisleLocation = aisle;
    newNode->shelfLocation = shelf;
    newNode->left = nullptr;
    newNode->right = nullptr;
    return newNode;
}

//Recursive insert into BST sorted by itemID
Item* insertNode(Item* node, Item* newItem) {
    if (node == nullptr) {
        totalItems++;
        return newItem;
    }
    if (newItem->itemID < node->itemID)
        node->left = insertNode(node->left, newItem);
    else if (newItem->itemID > node->itemID)
        node->right = insertNode(node->right, newItem);
    else
        cout << "  Item ID " << newItem->itemID << " already exists." << endl;
    return node;
}

//Recursive search by ID with comparison counter
Item* searchIDNode(Item* node, string id, int &comparisons) {
    if (node == nullptr) return nullptr;
    comparisons++;
    if (id == node->itemID) return node;
    if (id < node->itemID) return searchIDNode(node->left, id, comparisons);
    return searchIDNode(node->right, id, comparisons);
}

//Find the minimum node in a subtree
Item* findMin(Item* node) {
    while (node->left != nullptr)
        node = node->left;
    return node;
}

//Recursive delete from BST by ID
Item* deleteNode(Item* node, string id, bool &deleted) {
    if (node == nullptr) return nullptr;

    if (id < node->itemID)
        node->left = deleteNode(node->left, id, deleted);
    else if (id > node->itemID)
        node->right = deleteNode(node->right, id, deleted);
    else {
        //Case 1: Leaf node
        if (node->left == nullptr && node->right == nullptr) {
            delete node;
            totalItems--;
            deleted = true;
            return nullptr;
        }
        //Case 2: One child
        if (node->left == nullptr) {
            Item* temp = node->right;
            delete node;
            totalItems--;
            deleted = true;
            return temp;
        }
        if (node->right == nullptr) {
            Item* temp = node->left;
            delete node;
            totalItems--;
            deleted = true;
            return temp;
        }
        //Case 3: Two children, copy successor data, then delete the successor
        //The recursive delete handles totalItems decrement exactly once
        Item* successor = findMin(node->right);
        node->itemID = successor->itemID;
        node->itemName = successor->itemName;
        node->zoneLocation = successor->zoneLocation;
        node->aisleLocation = successor->aisleLocation;
        node->shelfLocation = successor->shelfLocation;
        node->right = deleteNode(node->right, successor->itemID, deleted);
    }
    return node;
}

//BST BALANCING (FLATTEN AND REBUILD)

//Recursive in-order traversal: flattens the BST into a sorted array of node pointers
void flattenBST(Item* node, Item** arr, int &index) {
    if (node == nullptr) return;
    flattenBST(node->left, arr, index);
    arr[index++] = node;
    flattenBST(node->right, arr, index);
}

//Rebuild a balanced BST: always picks the middle element as the subtree root
Item* buildBalanced(Item** arr, int start, int end) {
    if (start > end) return nullptr;
    int mid = (start + end) / 2;
    Item* root = arr[mid];
    root->left  = buildBalanced(arr, start, mid - 1);
    root->right = buildBalanced(arr, mid + 1, end);
    return root;
}

//Rebalance the BST
void rebalanceTree() {
    if (itemManagement == nullptr || totalItems <= 2) return;

    Item** arr = new Item*[totalItems];
    int index = 0;
    flattenBST(itemManagement, arr, index);

    itemManagement = buildBalanced(arr, 0, totalItems - 1);

    delete[] arr;
}

//HELPER FUNCTIONS

//Convert string to lowercase
string toLower(string s) {
    string result = s;
    for (int i = 0; i < (int)result.length(); i++) {
        if (result[i] >= 'A' && result[i] <= 'Z')
            result[i] = result[i] + 32;
    }
    return result;
}

//Check if needle is contained in haystack (case insensitive)
bool containsSubstring(string haystack, string needle) {
    string h = toLower(haystack);
    string n = toLower(needle);
    if (n.length() > h.length()) return false;
    for (int i = 0; i <= (int)(h.length() - n.length()); i++) {
        bool match = true;
        for (int j = 0; j < (int)n.length(); j++) {
            if (h[i + j] != n[j]) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}

//Recursive search by name with partial case insensitive matching
int searchNameNode(Item* node, string name, int &matches) {
    if (node == nullptr) return matches;
    if (containsSubstring(node->itemName, name)) {
        displayItem(node);
        matches++;
    }
    searchNameNode(node->left, name, matches);
    searchNameNode(node->right, name, matches);
    return matches;
}

//Validate location based on warehouse layout
//Zone 1, Aisle 1-2, Shelf 1-4
//Zone 2, Aisle 3-4, Shelf 5-8
bool isValidLocation(int zone, int aisle, int shelf) {
    if (zone < 1 || zone > 2) {
        cout << "  Invalid zone. Must be 1 or 2." << endl;
        return false;
    }
    if (zone == 1) {
        if (aisle < 1 || aisle > 2) {
            cout << "  Invalid aisle for Zone 1. Must be 1 or 2." << endl;
            return false;
        }
        if (shelf < 1 || shelf > 4) {
            cout << "  Invalid shelf for Zone 1. Must be 1 to 4." << endl;
            return false;
        }
    }
    if (zone == 2) {
        if (aisle < 3 || aisle > 4) {
            cout << "  Invalid aisle for Zone 2. Must be 3 or 4." << endl;
            return false;
        }
        if (shelf < 5 || shelf > 8) {
            cout << "  Invalid shelf for Zone 2. Must be 5 to 8." << endl;
            return false;
        }
    }
    return true;
}

//Find the highest existing ID in the BST
string findMaxID(Item* node) {
    if (node == nullptr) return "1000";
    while (node->right != nullptr)
        node = node->right;
    return node->itemID;
}

//Generate next incremental 4 digit ID
string generateNextID() {
    string maxID = findMaxID(itemManagement);
    int num = 0;
    for (int i = 0; i < (int)maxID.length(); i++)
        num = num * 10 + (maxID[i] - '0');
    num++;

    char digits[5];
    int temp = num;
    for (int i = 3; i >= 0; i--) {
        digits[i] = '0' + (temp % 10);
        temp /= 10;
    }
    digits[4] = '\0';
    return string(digits);
}

//Safely read an integer from input
int readInt(string prompt) {
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

//DISPLAY FUNCTIONS

//Display single item details
void displayItem(Item* item) {
    cout << "  ID: " << item->itemID
         << " | Name: " << item->itemName
         << " | Zone: " << item->zoneLocation
         << " | Aisle: " << item->aisleLocation
         << " | Shelf: " << item->shelfLocation << endl;
}

//Inorder traversal to display all items sorted by ID
void displayAllNodes(Item* node) {
    if (node == nullptr) return;
    displayAllNodes(node->left);
    displayItem(node);
    displayAllNodes(node->right);
}

//Display items filtered by zone
void displayZoneNodes(Item* node, int zone) {
    if (node == nullptr) return;
    displayZoneNodes(node->left, zone);
    if (node->zoneLocation == zone)
        displayItem(node);
    displayZoneNodes(node->right, zone);
}

//MENU 

void addItem() {
    string id, name;
    int zone, aisle, shelf;

    //Auto generate next incremental ID
    id = generateNextID();
    cout << "  Generated Item ID: " << id << endl;

    cout << "  Enter Item Name: ";
    getline(cin, name);
    zone = readInt("  Enter Zone (1 or 2): ");
    aisle = readInt("  Enter Aisle: ");
    shelf = readInt("  Enter Shelf: ");

    //Validate location based on warehouse layout
    if (!isValidLocation(zone, aisle, shelf)) return;

    Item* newItem = createNode(id, name, zone, aisle, shelf);
    itemManagement = insertNode(itemManagement, newItem);
    saveItemsToFile("items.txt");
    cout << "  Item added successfully." << endl;
}

void searchByID() {
    string id;
    cout << "  Enter Item ID to search: ";
    cin >> id;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    int comparisons = 0;
    Item* result = searchIDNode(itemManagement, id, comparisons);

    if (result != nullptr) {
        cout << endl;
        cout << "  Item Found:" << endl;
        displayItem(result);
        cout << "  Comparisons: " << comparisons << " / " << totalItems << " items" << endl;
    } else {
        cout << "  Item not found." << endl;
        cout << "  Comparisons: " << comparisons << " / " << totalItems << " items" << endl;
    }
}

void searchByName() {
    string name;
    cout << "  Enter Item Name (or part of name) to search: ";
    getline(cin, name);

    if (name.empty()) {
        cout << "  Search term cannot be empty." << endl;
        return;
    }

    int matches = 0;
    cout << endl;
    cout << "  Search Results:" << endl;
    searchNameNode(itemManagement, name, matches);

    if (matches == 0) {
        cout << "  No items match \"" << name << "\"." << endl;
    } else {
        cout << endl;
        cout << "  Found " << matches << " match(es)." << endl;
    }
}

void editItem() {
    string id;
    cout << "  Enter Item ID to edit: ";
    cin >> id;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    int comp = 0;
    Item* item = searchIDNode(itemManagement, id, comp);

    if (item == nullptr) {
        cout << "  Item not found." << endl;
        return;
    }

    cout << "  Current details:" << endl;
    displayItem(item);
    cout << endl;

    string name;
    int zone, aisle, shelf;

    cout << "  Enter new Item Name (or press Enter to keep): ";
    getline(cin, name);

    zone = readInt("  Enter new Zone (0 to keep): ");
    aisle = readInt("  Enter new Aisle (0 to keep): ");
    shelf = readInt("  Enter new Shelf (0 to keep): ");

    //Use existing values if user entered 0
    int newZone = (zone != 0) ? zone : item->zoneLocation;
    int newAisle = (aisle != 0) ? aisle : item->aisleLocation;
    int newShelf = (shelf != 0) ? shelf : item->shelfLocation;

    //Validate combined location
    if (!isValidLocation(newZone, newAisle, newShelf)) return;

    if (!name.empty()) item->itemName = name;
    item->zoneLocation = newZone;
    item->aisleLocation = newAisle;
    item->shelfLocation = newShelf;

    saveItemsToFile("items.txt");
    cout << "  Item updated successfully." << endl;
}

void deleteItem() {
    string id;
    cout << "  Enter Item ID to delete: ";
    cin >> id;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    int comp = 0;
    Item* item = searchIDNode(itemManagement, id, comp);
    if (item == nullptr) {
        cout << "  Item not found." << endl;
        return;
    }

    cout << "  Deleting:" << endl;
    displayItem(item);

    char confirm;
    cout << "  Confirm delete? (y/n): ";
    cin >> confirm;

    if (confirm == 'y' || confirm == 'Y') {
        bool deleted = false;
        itemManagement = deleteNode(itemManagement, id, deleted);
        saveItemsToFile("items.txt");
        cout << "  Item deleted successfully." << endl;
    } else {
        cout << "  Delete cancelled." << endl;
    }
}

void displayAll() {
    if (itemManagement == nullptr) {
        cout << "  No items in the system." << endl;
        return;
    }
    cout << "  All Items (sorted by ID):" << endl;
    cout << endl;
    displayAllNodes(itemManagement);
    cout << endl;
    cout << "  Total: " << totalItems << " items" << endl;
}

void displayByZone() {
    int zone = readInt("  Enter Zone number (1 or 2): ");

    if (zone < 1 || zone > 2) {
        cout << "  Invalid zone." << endl;
        return;
    }

    cout << "  Items in Zone " << zone << ":" << endl;
    cout << endl;
    displayZoneNodes(itemManagement, zone);
}

//MAIN MENU

void itemMenu() {
    int choice;
    do {
        cout << endl;
        cout << "  ========================================================" << endl;
        cout << "           Item Search & Management Module" << endl;
        cout << "  ========================================================" << endl;
        cout << "  1. Add Item" << endl;
        cout << "  2. Search Item by ID" << endl;
        cout << "  3. Search Item by Name" << endl;
        cout << "  4. Edit Item" << endl;
        cout << "  5. Delete Item" << endl;
        cout << "  6. Display All Items" << endl;
        cout << "  7. Display Items by Zone" << endl;
        cout << "  8. Back to Main Menu" << endl;
        cout << "  ========================================================" << endl;
        choice = readInt("  Enter choice: ");
        cout << endl;

        switch (choice) {
            case 1: addItem(); break;
            case 2: searchByID(); break;
            case 3: searchByName(); break;
            case 4: editItem(); break;
            case 5: deleteItem(); break;
            case 6: displayAll(); break;
            case 7: displayByZone(); break;
            case 8: cout << "  Returning to main menu..." << endl; break;
            default: cout << "  Invalid choice." << endl;
        }
    } while (choice != 8);
}

/*
//Testing
int main() {
    itemMenu();
    return 0;
}
*/
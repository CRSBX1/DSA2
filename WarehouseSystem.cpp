#include "header.hpp"
using namespace std;

int main(){
    loadItemsFromFile("items.txt");
    int userChoice;
    bool loop = true;
    while(loop){
        cout << endl <<"========================================================" << endl;
        cout << "                        Main Menu                       " << endl;
        cout << "========================================================" << endl;
        cout << "1. Order Management" << endl;
        cout << "2. Robot Assignment" << endl;
        cout << "3. Robot Navigation Simulation" << endl;
        cout << "4. Item Management" << endl;
        cout << "5. Warehouse Layout" << endl;
        cout << "6. Exit" << endl;
        cout << "========================================================" << endl;
        cout << "Enter your choice: ";
        cin >> userChoice;

        switch (userChoice)
        {
        case 1:
            orderManagementMenu();
            break;
        case 2:
            robotMenu();
            break;
        case 3:
            navigationMenu();
            break;
        case 4:
            itemMenu();
            break;
        case 5:
            warehouseMenu();
            cleanupWarehouse();
            break;
        case 6:
            loop = false;
            break;
        }
    }
}

/*
System flow:
1. A new order is received
2. The order is stored and prepared for processing
3. A robot is assigned to the task
4. The system identifies the item location 
5. A route is generated for the robot 
6. The robot moves step-by-step to the item 
7. After completing the task, the robot returns using the reverse path 

Proposed menu structure:
--------------------------------------------------------
Main Menu
--------------------------------------------------------
1. Create new orders
2. Manage items
3. Execute operation (system flow that I wrote above)
--------------------------------------------------------

--------------------------------------------------------
Item Sub-Menu
--------------------------------------------------------
1. Create Item
2. Edit Item
3. Delete Item
--------------------------------------------------------
*/
#include "header.hpp"
using namespace std;

int main(){
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
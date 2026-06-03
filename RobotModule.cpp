#include "header.hpp"
using namespace std;

RobotNode* frontRobot = nullptr;
RobotNode* nextRobotTurn = nullptr;
int robotCount = 0;

void createRobots(int amount){
    frontRobot = nullptr;
    nextRobotTurn = nullptr;
    robotCount = amount;
    RobotNode* tail = nullptr;

    if(amount <= 0){
        robotCount = 0;
        return;
    }

    for(int i=0; i<amount;i++){
        Robot robotNew;
        robotNew.robotID = to_string(i+1);
        robotNew.assigneditemID = "None";
        robotNew.currentTask = "none";
        robotNew.inMaintenance = false;
        robotNew.idle = true;

        RobotNode* node = new RobotNode{robotNew, nullptr};
        if(i==0){ //first item
            frontRobot = node;
        }
        else{
            tail->next = node;
        }
        tail = node;
    }

    tail->next = frontRobot;
    nextRobotTurn = frontRobot;
}

Robot assignTask(OrderNode* front){
    Robot emptyRobot;
    emptyRobot.robotID = "";
    emptyRobot.assigneditemID = "None";
    emptyRobot.currentTask = "None";
    emptyRobot.idle = true;
    emptyRobot.inMaintenance = false;

    if(frontRobot == nullptr || front == nullptr){
        return emptyRobot;
    }

    if(nextRobotTurn == nullptr){
        nextRobotTurn = frontRobot;
    }

    RobotNode* temp = nextRobotTurn;
    do{
        if(temp->robot.idle && !temp->robot.inMaintenance){
            temp->robot.assigneditemID = front->order.itemID;
            temp->robot.currentTask = "fetch";
            temp->robot.idle = false;
            nextRobotTurn = temp->next;
            return temp->robot;
        }
        temp = temp->next;
    }while(temp != nextRobotTurn);

    return emptyRobot;
}

void printAssignedRobots(Robot robotList[], int count){
    if(count == 0){
        cout << "None" << endl;
        return;
    }

    for(int i=0; i<count; i++){
        cout << "Robot number " << robotList[i].robotID << endl;
    }
}

void displayRobotsBasedOnAssignment(int amount){
    RobotNode* temp = frontRobot;
    int fCount=0, pCount=0, iCount=0;
    Robot fetch[100];
    Robot package[100];
    Robot idle[100];

    if(temp == nullptr || amount <= 0){
        cout << "[INFO] No robots available." << endl;
        return;
    }

    cout << "========================================================" << endl;
    cout << "                  Robot Assignment List                 " << endl;
    cout << "========================================================" << endl;
    for(int i=0;i<amount;i++){
        if(temp->robot.currentTask=="fetch"){
            fetch[fCount] = temp->robot;
            fCount++;
        }
        else if(temp->robot.currentTask=="pack"){
            package[pCount] = temp->robot;
            pCount++;
        }
        else{
            idle[iCount] = temp->robot;
            iCount++;
        }
        temp = temp->next;
    }
    cout << "Fetching items:" << endl;
    printAssignedRobots(fetch, fCount);
    cout << "Packaging items:" << endl;
    printAssignedRobots(package, pCount);
    cout << "Currently Idle:" << endl;
    printAssignedRobots(idle, iCount);
}

void displayRobots(int amount){
    //robot status overview
    RobotNode* temp = frontRobot;
    int available = 0, occupied = 0;
    string availabity;

    if(temp == nullptr || amount <= 0){
        cout << "[INFO] No robots available." << endl;
        return;
    }

    cout << "========================================================" << endl;
    cout << "                 Robot Status Overview                  " << endl;
    cout << "========================================================" << endl;
    for(int i=0;i<amount;i++){
        if(temp->robot.idle == true && !temp->robot.inMaintenance){
            available++;
            availabity = "Yes";
        }
        else{
            occupied++;
            availabity = "No";
        }

        cout << endl << "Robot number: " << temp->robot.robotID << endl;
        cout << "Availability status: " << availabity << endl;
        cout << "Maintenance status : " << (temp->robot.inMaintenance ? "Yes" : "No") << endl;
        cout << "Current task       : " << temp->robot.currentTask << endl;
        cout << "Assigned item ID   : " << temp->robot.assigneditemID << endl;
        temp = temp->next;
    }
    cout << "========================================================" << endl;
    cout << "Status Summary" << endl;
    cout << "Available Robots        : " << available << endl;
    cout << "Unavailable Robots      : " << occupied << endl;
}

void runNavigation(Robot robot, Item* item){
    if(warehouseRoot == nullptr){
        buildWarehouse();
    }

    movementStack navModule(6); //6 for the number of max movements in the warehouse;
    RobotMovement* top = nullptr; //the top of the stack
    string path = getPathToLocation(item->zoneLocation, item->aisleLocation, item->shelfLocation);
    string movement = "";
    
    cout << "Navigation module is being prepared for Robot " << robot.robotID << endl;
    cout << "Item location: Zone " << item->zoneLocation
         << ", Aisle " << item->aisleLocation
         << ", Shelf " << item->shelfLocation << endl;
    if(path.empty()){
        cout << "[WARNING] No valid route generated for this item location." << endl;
        return;
    }

    for(char c:path){
        if(c == 'F'){
            movement = "Forward";
        }
        else if(c == 'L'){
            movement = "Left";
        }
        else if(c == 'R'){
            movement = "Right";
        }
        navModule.push(top, movement);
    }

    cout << "Path to item: " << path << endl;
    cout << "Robot moving to retrieve item:" << endl;

    RobotMovement* traversal = top;
    while(traversal != nullptr && traversal->next != nullptr){
        traversal = traversal->next; //traverse to the first entered node
    }
    while(traversal != nullptr){
        cout << "Robot performing movement maneuver: " << traversal->direction << endl;
        traversal = traversal->prev;
    }

    cout << "Robot returning to initial location: " << endl;
    while(!navModule.isStackEmpty()){
        cout << "Robot performing movement maneuver: " << navModule.pop(top) << endl;
    }
}

void releaseRobot(string robotID){
    RobotNode* temp = frontRobot;

    if(temp == nullptr){
        return;
    }

    do{
        if(temp->robot.robotID == robotID){
            temp->robot.assigneditemID = "None";
            temp->robot.currentTask = "none";
            temp->robot.idle = true;
            return;
        }
        temp = temp->next;
    }while(temp != frontRobot);
}

void processOnePendingOrder(){
    if(frontRobot == nullptr){
        cout << "[INFO] No robots have been created yet. Opening assignment with 5 default robots." << endl;
        createRobots(5);
    }

    int processedCount = 0;

    Order order = getNextOrderForRobot();
    if(order.orderID == ""){
        return;
    }

    OrderNode currentOrder(order);
    Robot robot = assignTask(&currentOrder);

    if(robot.robotID == ""){
        cout << "[INFO] No available robot for " << order.orderID << ". Re-queueing order." << endl;
        enqueueSorted(order);
        return;
    }

    cout << "--------------------------------------------------------" << endl;
    cout << "Order " << order.orderID << " status: In Progress" << endl;
    cout << "Current robot handling task: Robot " << robot.robotID << endl;
    cout << "Assigned item: " << order.itemID << endl;
    cout << "Robot " << robot.robotID << "'s current task: " << robot.currentTask << endl;

    int comparisons = 0;
    Item* item = searchIDNode(itemManagement, order.itemID, comparisons);
    if(item == nullptr){
        cout << "[ERROR] Item " << order.itemID << " was not found. Robot released." << endl;
        releaseRobot(robot.robotID);
        enqueueSorted(order);
        return;
    }

    runNavigation(robot, item);
    cout << "Robot "<< robot.robotID << " has reached its initial position" << endl;
    robot.currentTask = "pack";
    cout << "Robot " << robot.robotID << "'s current task: " << robot.currentTask << endl;
    cout << "Packing in progress..." << endl;   
    releaseRobot(robot.robotID);
    markOrderCompleted(order);
    processedCount++;

    cout << "Order " << order.orderID << " status: Completed" << endl;
    cout << "Robot " << robot.robotID << " released." << endl;
    

    cout << "========================================================" << endl;
    cout << "Processed orders: " << processedCount << endl;
    cout << "Remaining pending orders: " << getQueueSize() << endl;
    displayRobotsBasedOnAssignment(robotCount);
    displayRobots(robotCount);
}

void processAllPendingOrders(){
    if(frontRobot == nullptr){
        cout << "[INFO] No robots have been created yet. Opening assignment with 5 default robots." << endl;
        createRobots(5);
    }

    int processedCount = 0;

    while(hasOrdersWaiting()){
        Order order = getNextOrderForRobot();
        if(order.orderID == ""){
            break;
        }

        OrderNode currentOrder(order);
        Robot robot = assignTask(&currentOrder);

        if(robot.robotID == ""){
            cout << "[INFO] No available robot for " << order.orderID << ". Re-queueing order." << endl;
            enqueueSorted(order);
            break;
        }

        cout << "--------------------------------------------------------" << endl;
        cout << "Order " << order.orderID << " status: In Progress" << endl;
        cout << "Current robot handling task: Robot " << robot.robotID << endl;
        cout << "Assigned item: " << order.itemID << endl;
        cout << "Robot " << robot.robotID << "'s current task: " << robot.currentTask << endl;

        int comparisons = 0;
        Item* item = searchIDNode(itemManagement, order.itemID, comparisons);
        if(item == nullptr){
            cout << "[ERROR] Item " << order.itemID << " was not found. Robot released." << endl;
            releaseRobot(robot.robotID);
            enqueueSorted(order);
            break;
        }

        runNavigation(robot, item);
        cout << "Robot "<< robot.robotID << " has reached its initial position" << endl;
        robot.currentTask = "pack";
        cout << "Robot " << robot.robotID << "'s current task: " << robot.currentTask << endl;
        cout << "Packing in progress..." << endl;   
        releaseRobot(robot.robotID);
        markOrderCompleted(order);
        processedCount++;

        cout << "Order " << order.orderID << " status: Completed" << endl;
        cout << "Robot " << robot.robotID << " released." << endl;
    }

    cout << "========================================================" << endl;
    cout << "Processed orders: " << processedCount << endl;
    cout << "Remaining pending orders: " << getQueueSize() << endl;
    displayRobotsBasedOnAssignment(robotCount);
    displayRobots(robotCount);
}

void robotSearch(int amount){
    string id, maintenance, idle;
    cout << "========================================================" << endl;
    cout << "Enter robot ID: ";
    cin >> id;
    RobotNode* temp = frontRobot;
    cout << "========================================================" << endl;
    cout << "                   Robot Search Result                  " << endl;
    cout << "========================================================" << endl;
    for(int i=0; i<amount; i++){
        if(temp->robot.robotID == id){
            idle = temp->robot.idle==true ? "True":"False";
            maintenance = temp->robot.inMaintenance==true ? "True":"False";
            cout << "Robot found:" << endl;
            cout << "ID: " << temp->robot.robotID << endl;
            cout << "Assigned Item ID: " << temp->robot.assigneditemID << endl;
            cout << "Current Task: " << temp->robot.currentTask << endl;
            cout << "Idle Status: " << idle << endl;
            cout << "Maintenance Status: " << maintenance << endl;
            return;
        }
        temp = temp->next;
    }
    cout << "Robot with the ID number " << id << " isn't found." << endl;
}

void assignMaintainStatus(int amount){
    RobotNode* temp = frontRobot;
    string id;
    cout << "========================================================" << endl;
    cout << "Enter robot ID: ";
    cin >> id;
    for(int i=0; i<amount; i++){
        if(temp->robot.robotID == id){
            temp->robot.inMaintenance = true;
            cout << "Robot ID " << id << " is being maintained." << endl;
            return;
        }
        temp = temp->next;
    }
    cout << "Robot ID " << id << " was not found." << endl;
}

void robotMenu(){
    bool loop = true;
    int menuChoice, robotAmount;

    cout << "Enter the amount of robots: ";
    cin >> robotAmount;
    createRobots(robotAmount);

    while(loop){
        cout << "========================================================"<< endl;
        cout << "                Robot Assignment Module                 "<< endl;
        cout << "========================================================"<< endl;
        cout << "1. Display current robot task assignments" << endl;
        cout << "2. Display robot status overview" << endl;
        cout << "3. Track a robot" << endl;
        cout << "4. Maintain robot" << endl;
        cout << "5. Exit" << endl;
        cout << "======================================================== "<< endl;
        cout << "Enter your choice: ";
        cin >> menuChoice;

        switch (menuChoice)
        {
        case 1:
            displayRobotsBasedOnAssignment(robotAmount);
            break;
        case 2:
            displayRobots(robotAmount);
            break;
        case 3:
            robotSearch(robotAmount);
            break;
        case 4:
            assignMaintainStatus(robotAmount);
            break;
        case 5:
            loop = false;
            break;
        default:
            cout << "Invalid choice, try again." << endl;
            break;
        }
    }

}

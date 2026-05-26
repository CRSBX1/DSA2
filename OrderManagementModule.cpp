#include "header.hpp"

//queue pointers
OrderNode* front = nullptr; //first order (next to be processed)
OrderNode* rear = nullptr; //last order (most recently added)
int queueSize = 0;

//completed orders stored as a separate linked list
OrderNode* completedHead = nullptr; 
int completedCount = 0;

//counter for generating order IDs
int orderCounter = 1;

//converts int to string
string intToString(int num){
    if(num == 0){
        return "0";
    }
    string result = "";
    int temp = num < 0 ? -num : num;
    while(temp > 0){
        char c = '0' + (temp % 10);
        result = string(1, c) + result;
        temp /= 10;
    }
    if(num < 0){
        result = "-" + result;
    }
    return result;
}

//converts total minutes back to HH:MM format
string convertTimeToString(int minutes){
    int hours = minutes / 60;
    int mins = minutes % 60;

    string timeStr = "";
    if(hours < 10){
        timeStr += "0";
    }
    timeStr += intToString(hours);
    timeStr += ":";
    if(mins < 10){
        timeStr += "0";
    }
    timeStr += intToString(mins);
    return timeStr;
}

//enqueu - adds order to the back of the queue
void enqueue(Order newOrder){
    if(isFull()){
        cout<<"\n[ERROR] System overload! Queue is full ("<<MAX_QUEUE_SIZE<< "orders)"<<endl;
        cout<<"Please process existing orders first"<<endl;
        return;
    }
        OrderNode* newNode = new OrderNode(newOrder);

        if(front == nullptr){
            //queue was empty, new node is both front and rear
            front = newNode;
            rear = newNode;
        }else{
            //link at the back
            rear->next = newNode;
            rear = newNode;
        }
        queueSize++;
}

//inserts an order in sorted position based on arrival time
//so earlier orders always get processed first even if they were added to the system later than others
void enqueueSorted(Order newOrder){
    if(isFull()){
        cout<<"\n[ERROR] System overload! Queue is full"<<endl;
        return;
    }
    
    OrderNode* newNode = new OrderNode(newOrder);

    //if queue is empty just put it in
    if(front == nullptr){
        front = newNode;
        rear = newNode;
    }
    //if this order is earlier than everything, it goes to the front
    else if(newOrder.arrivalTime < front->order.arrivalTime){
        newNode->next = front;
        front = newNode;
    }
    //otherwise walk through and find where it fits
    else{
        OrderNode* current = front;
        while(current->next != nullptr && current->next->order.arrivalTime < newOrder.arrivalTime){
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
        //if we put it at the very end, update rear pointer
        if(newNode->next == nullptr){
            rear = newNode;
        }
    }
    queueSize++;
}

//removes the front order and returns it
Order dequeue(){
    Order emptyOrder;

    if(isEmpty()){
        cout<<"\n[ERROR] Queue is empty! No orders to process"<<endl;
        return emptyOrder;
    }

    OrderNode* temp = front;
    Order removedOrder = front->order;
    front = front->next;

    //if that was the last order, rear needs to be null too
    if(front == nullptr){
        rear = nullptr;
    }
    delete temp;
    queueSize--;
    return removedOrder;
}

//simple checks for queue state
bool isEmpty(){
    return (front == nullptr);
}

bool isFull(){
    return (queueSize >= MAX_QUEUE_SIZE);
}

int getQueueSize(){
    return queueSize;
}

//lets the user type in order details and adds it to the queue
//also checks if the item actually exist in the warehouse first
void addNewOrder(){
    cout<<"\n====================================="<<endl;
    cout<<"       ADD NEW CUSTOMER ORDER"<<endl;
    cout<<"====================================="<<endl;

    string orderID, itemID;
    int hour, minute;

    orderID =  "ORD" + intToString(orderCounter);

    cout<<"Order ID: "<<orderID<<endl;
    cout<<"Enter Item ID to order: ";
    cin.ignore();
    getline(cin, itemID);

    if(itemID.empty()){
        cout<<"[ERROR] Item ID cannot be empty!"<<endl;
        return;
    }

    //make sure the item is in the BST before we accept the order
    int comp = 0;
    Item* foundItem = searchIDNode(itemManagement, itemID, comp);
    if(foundItem == nullptr){
        cout<<"[ERROR] Item "<<itemID<<" does not exist in warehouse"<<endl;
        return;
    }
    cout<<"Item found: "<<foundItem->itemName
        <<" (Zone "<<foundItem->zoneLocation
        <<", Aisle "<<foundItem->aisleLocation
        <<", Shelf "<<foundItem->shelfLocation<<")"<<endl;

    cout<<"Enter arrival hour (0-23): ";
    cin>>hour;
    cout<<"Enter arrival minute (0-59): ";
    cin>>minute;

    if(hour < 0||hour > 23||minute < 0||minute > 59){
        cout<<"[ERROR] Invalid time!"<<endl;
        return;
    }

    Order newOrder(orderID, hour, minute, itemID);
    enqueueSorted(newOrder);
    orderCounter++;

    cout<<"\n[SUCCESS] Order "<<orderID<<" added to queue"<<endl;
    cout<<"Item     : "<<itemID<<endl;
    cout<<"Time     : "<<convertTimeToString(newOrder.arrivalTime)<<endl;
    cout<<"Status   : "<<newOrder.processState<<endl;
    cout<<"Position : Queue has "<<queueSize<<" order(s)"<<endl;
}

void markOrderCompleted(Order completedOrder){
    completedOrder.processState = "Completed";

    OrderNode* completedNode = new OrderNode(completedOrder);
    completedNode->next = completedHead;
    completedHead = completedNode;
    completedCount++;
}

//takes the front order out, marks it done, and moves it to completed list
void processNextOrder(){
    cout<<"\n====================================="<<endl;
    cout<<"         PROCESS NEXT ORDER"<<endl;
    cout<<"====================================="<<endl;

    if(isEmpty()){
        cout<<"[INFO] No pending orders to process"<<endl;
        cout<<"Waiting for new orders..."<<endl;
        return;
    }
    
    processAllPendingOrders();
}

void processAllOrders(){
    cout<<"\n====================================="<<endl;
    cout<<"          PROCESS ALL ORDERS"<<endl;
    cout<<"====================================="<<endl;

    if(isEmpty()){
        cout<<"[INFO] No pending orders to process"<<endl;
        return;
    }

    processAllPendingOrders();
}

//prints all order currently waiting in queue
void displayPendingOrders(){
    cout<<"\n====================================="<<endl;
    cout<<"         PENDING ORDERS LIST"<<endl;
    cout<<"====================================="<<endl;

    if(isEmpty()){
        cout<<"[INFO] No pending orders"<<endl;
        return;
    }

    OrderNode* current = front;
    int position = 1;

    cout<<"No.  Order ID  Item ID    Time      Status"<<endl;
    cout<<"---  --------  -------   -------   ---------"<<endl;

    while(current != nullptr){
        cout<< " "<<position<<".   "<<current->order.orderID;

        for(int i=current->order.orderID.length(); i<10; i++){
            cout<< " ";
        }
        cout<<current->order.itemID;

        for(int i=current->order.itemID.length(); i<10; i++){
            cout<<" ";
        }
        cout<<convertTimeToString(current->order.arrivalTime)<<"     "<<current->order.processState<<endl;
        current = current->next;
        position++;
    }
    cout<<"\nTotal pending: "<<queueSize<<endl;
}

//shows the front order without removing it
void displayNextOrder(){
    cout<<"\n====================================="<<endl;
    cout<<"        NEXT ORDER TO PROCESS"<<endl;
    cout<<"====================================="<<endl;

    if(isEmpty()){
        cout<<"[INFO] No orders in queue"<<endl;
        return;
    }

    cout<<"Order ID  : "<<front->order.orderID<<endl;
    cout<<"Item ID   : "<<front->order.itemID<<endl;
    cout<<"Time      : "<<convertTimeToString(front->order.arrivalTime)<<endl;
    cout<<"Status    : "<<front->order.processState<<endl;
}

//shows all orders that have already been processed
void displayCompletedOrders(){
    cout<<"\n====================================="<<endl;
    cout<<"       COMPLETED ORDER HISTORY"<<endl;
    cout<<"====================================="<<endl;

    if(completedHead == nullptr){
        cout<<"[INFO] No completed orders yet"<<endl;
        return;
    }

    OrderNode* current = completedHead;
    int count = 1;

    cout<<"No.  Order ID  Item ID    Time      Status"<<endl;
    cout<<"---  --------  -------   -------  ----------"<<endl;

    while(current != nullptr){
        cout<<" "<<count<<".   "<<current->order.orderID;

        for(int i=current->order.orderID.length(); i<10; i++){
            cout<< " ";
        }
        cout<<current->order.itemID;

        for(int i=current->order.itemID.length(); i<10; i++){
            cout<< " ";
        }
        cout<<convertTimeToString(current->order.arrivalTime)<<"    "<<current->order.processState<<endl;
        current = current->next;
        count++;
    }
    cout<<"\nTotal completed: "<<completedCount<<endl;
}

//quick summary of how many orders are pending, done, etc
void displaySystemStatus(){
    cout<<"\n====================================="<<endl;
    cout<<"            SYSTEM STATUS"<<endl;
    cout<<"====================================="<<endl;
    cout<<"Pending orders   : "<<queueSize<<endl;
    cout<<"Completed orders : "<<completedCount<<endl;
    cout<<"Total received   : "<<(orderCounter-1)<<endl;
    cout<<"Queue capacity   : "<<queueSize<<"/"<<MAX_QUEUE_SIZE<<endl;

    if(isFull()){
        cout<<"\n[WARNING] System overload! Queue is full"<<endl;
    }else if(isEmpty()){
        cout<<"\nSystem is idle. Waiting for orders..."<<endl;
    }
}

//searches for an order by ID, just walks through the queue
//checking each node one by one until we find it or reach the end
void searchOrder(){
    cout<<"\n====================================="<<endl;
    cout<<"         SEARCH ORDER BY ID"<<endl;
    cout<<"====================================="<<endl;

    if(isEmpty()){
        cout<<"[INFO] Queue is empty. Nothing to search"<<endl;
        return;
    }

    string searchID;
    cout<<"Enter Order ID to search: ";
    cin.ignore();
    getline(cin, searchID);

    if(searchID.empty()){
        cout<<"[ERROR] Order ID cannot be empty!"<<endl;
        return;
    }

    OrderNode* current = front;
    int position = 1;
    bool found = false;

    while(current != nullptr){
        if(current->order.orderID == searchID){
            found = true;
            cout<<"\n[FOUND] Order located in queue"<<endl;
            cout<<"Position : "<<position<<" of "<<queueSize<<endl;
            cout<<"Order ID : "<<current->order.orderID<<endl;
            cout<<"Item ID  : "<<current->order.itemID<<endl;
            cout<<"Time     : "<<convertTimeToString(current->order.arrivalTime)<<endl;
            cout<<"Status   : "<<current->order.processState<<endl;
            break;
        }
        current = current->next;
        position++;
    }

    if(!found){
        cout<<"[NOT FOUND] Order "<<searchID<<" is not in the pending queue"<<endl;
    }
}

//removes a specific order from the queue by ID
void cancelOrder(){
    cout<<"\n====================================="<<endl;
    cout<<"            CANCEL ORDER"<<endl;
    cout<<"====================================="<<endl;

    if(isEmpty()){
        cout<<"[INFO] Queue is empty. Nothing to cancel"<<endl;
        return;
    }

    string cancelID;
    cout<<"Enter Order ID to cancel: ";
    cin.ignore();
    getline(cin, cancelID);

    if(cancelID.empty()){
        cout<<"[ERROR] Order ID cannot be empty!"<<endl;
        return;
    }

    //case 1: its the front order
    if(front->order.orderID == cancelID){
        OrderNode* temp = front;
        front = front->next;

        if(front == nullptr){
            rear = nullptr;
        }
        
        delete temp;
        queueSize--;

        cout<<"\n[SUCCESS] Order "<<cancelID<<" has been cancelled"<<endl;
        cout<<"Remaining in queue: "<<queueSize<<endl;
        return;
    }

    //case 2 & 3: somewhere in the middle or at the end
    OrderNode* current = front;

    while(current->next != nullptr){
        if(current->next->order.orderID == cancelID){
            OrderNode* temp = current->next;
            current->next = temp->next;
            //if we removed the last node, update rear
            if(temp == rear){
                rear = current;
            }
            delete temp;
            queueSize--;

            cout<<"[SUCCESS] Order "<<cancelID<<" has been cancelled"<<endl;
            cout<<"Remaining in queue: "<<queueSize<<endl;
            return;
        }
        current = current->next;
    }
    cout<<"[NOT FOUND] Order "<<cancelID<<" is not in the pending queue"<<endl;
}

//this is for the robot assignment module to call
//it grabs the next order and sets it to in progress
Order getNextOrderForRobot(){
    if(isEmpty()){
        Order empty;
        return empty;
    }

    Order nextOrder = dequeue();
    nextOrder.processState = "In Progress";
    return nextOrder;
}

//a quick check if there are orders waiting
bool hasOrdersWaiting(){
    return !isEmpty();
}

//menu loop
void orderManagementMenu(){
    int choice;

    do{
        cout<<"\n====================================="<<endl;
        cout<<"        ORDER MANAGEMENT MENU"<<endl;
        cout<<"====================================="<<endl;
        cout<<"1. Add New Order"<<endl;
        cout<<"2. Process Next Order"<<endl;
        cout<<"3. Process All Orders"<<endl;
        cout<<"4. View Pending Orders"<<endl;
        cout<<"5. View Next Order to Process"<<endl;
        cout<<"6. View Completed Orders"<<endl;
        cout<<"7. View System Status"<<endl;
        cout<<"8. Search Order by ID"<<endl;
        cout<<"9. Cancel Order"<<endl;
        cout<<"0. Back to Main Menu"<<endl;
        cout<<"======================================"<<endl;
        cout<<"Enter choice: ";
        cin>>choice;

        if(cin.fail()){
            cin.clear();
            cin.ignore(10000, '\n');
            cout<<"\n[ERROR] Please enter a number"<<endl;
            continue;
        }

        switch(choice){
            case 1:
                addNewOrder(); break;
            case 2:
                processNextOrder(); break;
            case 3:
                processAllOrders(); break;
            case 4:
                displayPendingOrders(); break;
            case 5: 
                displayNextOrder(); break;
            case 6:
                displayCompletedOrders(); break;
            case 7:
                displaySystemStatus(); break;
            case 8:
                searchOrder(); break;
            case 9:
                cancelOrder(); break;
            case 0:
                cout<<"\nReturning to main menu..."<<endl; break;
            default:
                cout<<"[ERROR] Invalid choice"<<endl;
        }
    }while(choice!=0);
}

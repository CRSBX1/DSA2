#include "header.hpp"
#include <iostream>
using namespace std;

int main(){
    Order order1("AE1",10,30,"bus");
    cout << order1.orderID << order1.arrivalTime;
}

/*
The Order Management Module is responsible for handling all incoming customer orders 
within the warehouse. It ensures that every order is recorded, organized, and processed in a 
structured and fair manner based on its arrival time. 
Functional Requirements 
• Accept and record new customer orders into the system 
• Maintain an ordered list of all incoming requests 
• Process orders sequentially according to their arrival 
• Remove orders from the list once they are assigned to robots 
• Display current pending and completed orders 
• Handle exceptional cases such as empty order lists or system overload 
Key Features: 
• Ensures fair processing of orders  
• Maintains real-time order status updates  
• Supports continuous inflow of new orders 
Expected Core Output: 
• List of pending orders  
• Current order being processed  
• Completed order history
*/
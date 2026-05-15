#include "header.hpp"
using namespace std;

int main(){

}

/*
This module controls the movement of robots within the warehouse and ensures accurate 
navigation. It also enables robots to return safely by retracing their movement path in reverse 
order. 
Functional Requirements 
• Record each movement step taken by the robot (e.g., forward, left, right) 
• Store the full path from starting point to destination 
• Allow the robot to reverse its path step-by-step after completing a task 
• Handle navigation issues such as obstacles or incorrect paths 
• Simulate robot movement through logs or step-by-step visualization 
Key Features: 
• Accurate path tracking and movement recording 
• Reverse navigation capability for return journeys 
• Basic obstacle handling through backtracking 
Expected Core Output: 
• Forward movement path 
• Reverse path for returning 
• Complete navigation log 
*/
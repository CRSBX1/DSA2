#include "header.hpp"
using namespace std;

int main(){

}

/*
The Robot Assignment Module manages how tasks are distributed among available robots. It 
ensures that all robots are utilized efficiently and fairly by assigning tasks in a rotating and 
balanced manner. 
Functional Requirements 
• Maintain a list of all robots and their current status (available/busy)  
• Assign tasks to robots in a continuous rotation  
• Skip robots that are currently unavailable or under maintenance  
• Track task assignments for each robot 
• Ensure uninterrupted task allocation without restarting the assignment cycle 
Key Features: 
• Balanced workload distribution among robots  
• Continuous and automated assignment process  
• Real-time robot availability tracking 
Expected Core Output: 
• Robot assignment list 
• Current robot handling each task 
• Robot status overview
*/
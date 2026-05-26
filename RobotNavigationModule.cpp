#include "header.hpp"
#include <string>
#include <iostream>
using namespace std;

movementStack::movementStack(int size){
    id = 0;
    maxSize = size;
}

void movementStack::push(RobotMovement* &top, string drc){
    if(id == maxSize){
        cout << "Navigation module construction completed.";
        return;
    }

    RobotMovement* newStack = new RobotMovement;
    newStack->id = id+1;
    newStack->direction = drc;
    newStack->prev = nullptr;
    id++;
    if(top == nullptr){
        newStack->next = nullptr;
        top = newStack;
        return;
    }
    //if stack not empty
    newStack->next = top;
    top->prev = newStack;
    top = newStack;
}

string movementStack::pop(RobotMovement* &top){
    if(id==0){
        cout << "Stack is empty";
        return "";
    }
    //Change the direction for backtracking
    if(top->direction == "Forward"){
        top->direction = "Backwards";
    }
    else if(top->direction == "Left"){
        top->direction = "Right";
    }
    else{
        top->direction="Left"; //If direction is right
    }
    RobotMovement* temp = top;
    top = top->next;
    string poppedDirection = temp->direction;
    delete temp;
    id--;
    return poppedDirection;
}

bool movementStack::isStackEmpty(){
    return id==0;
}

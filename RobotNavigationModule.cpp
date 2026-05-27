#include "header.hpp"
#include <string>
#include <iostream>
#include <limits> // for clearing bad input in cin
#include <ctime>  // for seeding our own random generator (NOT rand())
using namespace std;

movementStack::movementStack(int size)
{
    id = 0;
    maxSize = size;
}

void movementStack::push(RobotMovement *&top, string drc)
{
    if (id == maxSize)
    {
        cout << "Navigation module construction completed.";
        return;
    }

    RobotMovement *newStack = new RobotMovement;
    newStack->id = id + 1;
    newStack->direction = drc;
    newStack->prev = nullptr;
    id++;
    if (top == nullptr)
    {
        newStack->next = nullptr;
        top = newStack;
        return;
    }
    // if stack not empty
    newStack->next = top;
    top->prev = newStack;
    top = newStack;
}

string movementStack::pop(RobotMovement *&top)
{
    if (id == 0)
    {
        cout << "Stack is empty";
        return "";
    }
    // Change the direction for backtracking
    if (top->direction == "Forward")
    {
        top->direction = "Backward";
    }
    else if (top->direction == "Left")
    {
        top->direction = "Right";
    }
    else
    {
        top->direction = "Left"; // If direction is right
    }
    RobotMovement *temp = top;
    top = top->next;
    string poppedDirection = temp->direction;
    delete temp;
    id--;
    return poppedDirection;
}

bool movementStack::isStackEmpty()
{
    return id == 0;
}

//  backtracking, obstacle handling, navigation
static string numToStr(int n)
{
    if (n == 0)
        return "0";

    bool negative = n < 0;
    int value = negative ? -n : n;
    string result = "";

    // peel off the least significant digit each loop, building right to left
    while (value > 0)
    {
        char digit = '0' + (value % 10);
        result = string(1, digit) + result;
        value /= 10;
    }

    if (negative)
        result = "-" + result;
    return result;
}

// Converts a single path character into a readable movement direction.
// The warehouse module produces paths made of F (forward) and L/R (turns).
static string charToDirection(char c)
{
    if (c == 'F')
        return "Forward";
    if (c == 'L')
        return "Left";
    if (c == 'R')
        return "Right";
    if (c == 'B')
        return "Backward";
    return ""; // unknown characters are ignored by the caller
}

// SEUDO-RANDOM NUMBER GENERATION (Linear Congruential Generator)
// cannot use <cstdlib> or rand(), so we implement our own simple LCG.

static unsigned long lcgSeed = 0; // current generator state
static bool lcgSeeded = false;    // whether the seed has been initialised yet

// Produces the next pseudo-random value in the range 0..32767.
static int lcgNext()
{
    if (!lcgSeeded)
    {
        // seed once from the system clock so each run differs
        lcgSeed = (unsigned long)time(NULL);
        lcgSeeded = true;
    }

    // classic LCG recurrence: next = (a * seed + c) mod 2^32 (the mod is the
    // natural overflow of unsigned long). The constants come from the ANSI C
    // standard generator family.
    lcgSeed = lcgSeed * 1103515245UL + 12345UL;

    // the higher bits are better distributed, so take bits 16..30 (15 bits)
    return (int)((lcgSeed >> 16) & 0x7FFF);
}

// OBSTACLE HANDLING

// Returns true when a random obstacle should block the current step.
// 'probability' is a percentage (0-100): higher means more obstacles.
bool obstacleDetected(int probability)
{
    if (probability <= 0)
        return false; // obstacles disabled
    if (probability >= 100)
        return true; // always blocked

    // roll a number 0..99 and compare against the percentage chance
    return (lcgNext() % 100) < probability;
}

// Returns an alternative direction to take when the planned one is blocked.
// Forward -> Left, Left -> Right, Right -> Forward (a fixed rotation).
string getAlternative(string blocked)
{
    if (blocked == "Forward")
        return "Left";
    if (blocked == "Left")
        return "Right";
    if (blocked == "Right")
        return "Forward";
    return "Forward"; // sensible fallback for any other value
}

// ---- BACKTRACKING ----

// Reads the move on top of the stack, reverses its direction by hand, removes
// the node from the stack, and returns the reversed direction. This is done
// independently of pop() (it does not call pop()) so it can be used while the
// robot is still navigating forward and needs to undo a single step.
string movementStack::backtrack(RobotMovement *&top)
{
    if (id == 0)
    {
        cout << "  Nothing to backtrack - the stack is empty." << endl;
        return "";
    }

    // the most recent move sits on top of the stack
    string original = top->direction;

    // manually reverse the direction (mirror of what pop() does internally)
    string reversed;
    if (original == "Forward")
        reversed = "Backward";
    else if (original == "Backward")
        reversed = "Forward";
    else if (original == "Left")
        reversed = "Right";
    else
        reversed = "Left"; // reverse of "Right"

    // detach and delete the top node so the stack shrinks by one
    RobotMovement *temp = top;
    top = top->next;
    if (top != nullptr)
        top->prev = nullptr; // new top no longer has a node above it
    delete temp;
    id--;

    return reversed;
}

// FULL NAVIGATION (forward journey + reverse return journey)
// Walks through the path string one character at a time, recording each move
// onto the stack. If obstacles are enabled, each step may be blocked: when
// that happens the robot backtracks the last move, takes a detour using an
// alternative direction, then resumes the originally planned move. Once the
// destination is reached, every move is popped (which auto-reverses it) so the
// robot retraces the exact reverse route back to its starting point.
void movementStack::executeNavigation(RobotMovement *&top, string path, bool useObstacles)
{
    NavigationLog log;              // records every event of this journey
    const int OBSTACLE_CHANCE = 30; // percent chance of an obstacle per step

    // ---------- FORWARD JOURNEY ----------
    log.add("=== FORWARD JOURNEY START ===");
    for (int i = 0; i < (int)path.length(); i++)
    {
        string dir = charToDirection(path[i]);
        if (dir == "")
            continue; // skip any unexpected character in the path

        if (useObstacles && obstacleDetected(OBSTACLE_CHANCE))
        {
            // an obstacle is blocking the intended direction
            log.add("Obstacle detected while moving " + dir + " at step " + numToStr(i + 1));

            // step the robot back one move to begin rerouting
            if (!isStackEmpty())
            {
                string undone = backtrack(top);
                log.add("  Backtracked last move (reversed to " + undone + ")");
            }

            // go around the obstacle using an alternative direction
            string alternative = getAlternative(dir);
            push(top, alternative);
            log.add("  Detour: moved " + alternative + " to avoid the obstacle");

            // resume the originally planned move
            push(top, dir);
            log.add("  Resumed planned move: " + dir);
        }
        else
        {
            // clear path - simply perform the planned move
            push(top, dir);
            log.add("Step " + numToStr(i + 1) + ": moved " + dir);
        }
    }
    log.add("=== DESTINATION REACHED ===");

    // RETURN JOURNEY
    // Popping each move returns its reversed direction automatically, so the
    // robot retraces its exact route back to where it started.
    log.add("=== RETURN JOURNEY START (reverse path) ===");
    int returnStep = 1;
    while (!isStackEmpty())
    {
        string back = pop(top);
        log.add("Return step " + numToStr(returnStep) + ": moved " + back);
        returnStep++;
    }
    log.add("=== ROBOT BACK AT STARTING POINT ===");

    // ---------- SHOW THE COMPLETE LOG ----------
    log.display();
}

// MENU INPUT HELPERS

// Reads an integer from the user, re-prompting until the input is valid.
static int readNavInt(string prompt)
{
    int value;
    while (true)
    {
        cout << prompt;
        if (cin >> value)
        {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        // recover from non-numeric input
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  Invalid input, please enter a number." << endl;
    }
}

// Reads a full line of text from the user (used for manual path entry).
static string readNavLine(string prompt)
{
    cout << prompt;
    string line;
    getline(cin, line);
    return line;
}

// SIMULATION DRIVERS

// Asks for a destination shelf, gets the route from the warehouse module, and
// runs one full navigation simulation (with or without obstacles).
static void runShelfSimulation(bool useObstacles)
{
    buildWarehouse(); // make sure the warehouse layout tree exists

    int zone = readNavInt("  Enter Zone (1 or 2): ");
    int aisle = readNavInt("  Enter Aisle: ");
    int shelf = readNavInt("  Enter Shelf: ");

    if (!isValidWarehouseLocation(zone, aisle, shelf))
    {
        cout << "  That location does not exist in the warehouse layout." << endl;
        return;
    }

    // ask the warehouse module for the movement path to the shelf
    string path = getPathToLocation(zone, aisle, shelf);
    if (path == "")
    {
        cout << "  Could not generate a path to that location." << endl;
        return;
    }
    cout << "  Planned route string: " << path << endl;

    // each robot navigates using its own private movement stack
    movementStack navStack(MAX_PATH_STEPS);
    RobotMovement *top = nullptr;
    navStack.executeNavigation(top, path, useObstacles);
}

// Lets the user type a path by hand and runs it without random obstacles.
static void runManualEntry()
{
    cout << "  Enter a path using F (Forward), L (Left), R (Right)." << endl;
    cout << "  Example: FLFRFL" << endl;
    string path = readNavLine("  Enter path: ");

    if (path == "")
    {
        cout << "  No path entered." << endl;
        return;
    }

    // make sure the user only typed valid movement characters
    for (int i = 0; i < (int)path.length(); i++)
    {
        char c = path[i];
        if (c != 'F' && c != 'L' && c != 'R' && c != 'B')
        {
            cout << "  Invalid character '" << c << "'. Please use only F, L, R." << endl;
            return;
        }
    }

    movementStack navStack(MAX_PATH_STEPS);
    RobotMovement *top = nullptr;
    navStack.executeNavigation(top, path, false);
}

// MENU
// Standalone menu for the Robot Navigation & Path Tracking module.
void navigationMenu()
{
    int choice;
    do
    {
        cout << endl;
        cout << "  ========================================================" << endl;
        cout << "        Robot Navigation & Path Tracking Module" << endl;
        cout << "  ========================================================" << endl;
        cout << "  1. Simulate navigation WITH obstacles" << endl;
        cout << "  2. Simulate navigation WITHOUT obstacles" << endl;
        cout << "  3. Manual path entry" << endl;
        cout << "  4. Back to Main Menu" << endl;
        cout << "  ========================================================" << endl;
        choice = readNavInt("  Enter choice: ");
        cout << endl;

        switch (choice)
        {
        case 1:
            runShelfSimulation(true);
            break;
        case 2:
            runShelfSimulation(false);
            break;
        case 3:
            runManualEntry();
            break;
        case 4:
            cout << "  Returning to main menu..." << endl;
            break;
        default:
            cout << "  Invalid choice." << endl;
        }
    } while (choice != 4);
}

/*
// Standalone test driver for Module 3 only.
// Uncomment to run this module on its own, then compile with:
//   g++ -std=c++11 RobotNavigationModule.cpp WarehouseLayoutModule.cpp -o nav && ./nav
// (WarehouseLayoutModule.cpp is needed for buildWarehouse / getPathToLocation.)
// IMPORTANT: re-comment this main() before building the full system
// (WarehouseSystem.cpp), otherwise there will be two main() functions.
int main()
{
    navigationMenu();
    cleanupWarehouse();
    return 0;
}
*/

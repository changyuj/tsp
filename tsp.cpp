#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <limits>
#include <set>
#include <time.h>

using namespace std;

struct Node
{
	int cityID;
	int xCoordinate;
	int yCoordinate;
	Node* next;
    Node* prev;
    map<int, int> adjacencyList;
};

int calculateDistance(int x1, int x2, int y1, int y2) {

	int xCoordCalc = x1 - x2;
	int yCoordCalc = y1 - y2;

	double distance = std::sqrt(std::pow(xCoordCalc, 2) + std::pow(yCoordCalc, 2));
    // cout << "(" << x1 << ", " << y1 << ") -> (" << x2 << ", " << y2 <<"): " << distance << endl;

	return (int) round(distance);
}

Node* createNode(int id, int x, int y, map<int, int>* adjList)
{
    Node* newNode = new Node;

	newNode->cityID = id;
	newNode->xCoordinate = x;
	newNode->yCoordinate = y;
	newNode->next = NULL;
    newNode->prev = NULL;
    newNode->adjacencyList = *adjList;

    return newNode;
}

// establishes route between two city nodes
// returns the distance between them
int connectCity(struct Node* fromNode, struct Node* toNode)
{
    fromNode->next = toNode;
    toNode->prev = fromNode;
    int distance = fromNode->adjacencyList.find(toNode->cityID)->second;

    return distance;
}

int getNearestNeighbor(struct Node* current, set<int>& visited)
{
    int minDistance = numeric_limits<int>::max();
    int minId = -1;
    set<int>::iterator it;

    while (minId == -1)
    {
        for (map<int, int>::iterator itr = current->adjacencyList.begin(); itr != current->adjacencyList.end(); itr++)
        {
            it = visited.find(itr->first);
            if (itr->second < minDistance && it == visited.end())
            {
                minDistance = itr->second;
                minId = itr->first;
                // cout << "Min city " << itr->first << ", Distance " << itr->second << endl;
            }
        }
    }
    return minId;
}

void createCityMap(map<int, Node*>& cityMap, map<int, pair<int, int>>& cities)
{
    map<int, map<int, int>> adjMatrix;  // adjacency matrix
    map<int, int> adjList;  // adjacency list for a single city

    // each city
    for (map<int, pair<int, int>>::iterator itr_thisCity = cities.begin(); itr_thisCity != cities.end(); itr_thisCity++)
    {
        adjList.clear();
        // calulate distances to all other cities for adjacency list
        for (map<int, pair<int, int>>::iterator itr_otherCity = cities.begin(); itr_otherCity != cities.end(); itr_otherCity++)
        {
            if (itr_otherCity->first != itr_thisCity->first) {
                adjList[itr_otherCity->first] = calculateDistance(itr_thisCity->second.first, itr_otherCity->second.first, itr_thisCity->second.second, itr_otherCity->second.second);
            }
        }
        // add city adjaceny list to adjacency matrix
        adjMatrix[itr_thisCity->first] = adjList;

        // create city node
        cityMap[itr_thisCity->first] = createNode(itr_thisCity->first, itr_thisCity->second.first, itr_thisCity->second.second, &adjMatrix[itr_thisCity->first]);
    }
}

// performs a swap so that routes of the two nodes do not cross over each other
vector<int> twoOptSwap(struct Node* startNode, struct Node* i, struct Node* k)
{
    vector<int> newRoute;
    Node* current = startNode;
    while (current != i)
    {
        newRoute.push_back(current->cityID);
        current = current->next;
    }
    current = k;
    while (current->next != i)
    {
        newRoute.push_back(current->cityID);
        current = current->prev;
    }
    current = k->next;
    while (current != startNode)
    {
        newRoute.push_back(current->cityID);
        current = current->next;
    }
    return newRoute;
}

// calculate the total route distance
int calculateTotalDistance(vector<int> route, map<int, Node*>& cityMap)
{
    int totalDistance = 0;
    int distance;
    int nextCity;

    for (int i = 0; i < route.size(); i++)
    {
        if (i == route.size() - 1)
            distance = cityMap[route[i]]->adjacencyList.find(route[0])->second;
        else
            distance = cityMap[route[i]]->adjacencyList.find(route[i + 1])->second;
        totalDistance += distance;
    }
    return totalDistance;
}

void reconnectNodes(vector<int> route, map<int, Node*>& cityMap)
{
    Node* startNode = cityMap[route[0]];
    Node* current = startNode;
    for (int i = 0; i < route.size(); i++)
    {
        if (i == route.size() - 1)
            connectCity(cityMap[route[i]], cityMap[route[0]]);
        else
            connectCity(cityMap[route[i]], cityMap[route[i + 1]]);
    }
}

int main(int argc, char *argv[])
{
    clock_t clock_start = clock();
    time_t start = time(NULL);
    time_t seconds = 175;
    time_t endwait = start + seconds;

	//read in file from argument
    string infile = "";
    infile = argv[1];
    ifstream readFile;
    readFile.open(infile);

    if (!readFile) {
        cout << "Cannot open file: " << argv[1] << endl;
        // exit(1);
    }

    string line;
	int id, xCoor, yCoor;
    int numCities = 0;
    map< int, pair<int, int> > cities;
    while(readFile >> id >> xCoor >> yCoor)
    {
        cities[id] = pair<int, int> (xCoor, yCoor);
        numCities++;
    }
    readFile.close();

    // Create city nodes
    map<int, Node*> cityMap;  // map of cities
    createCityMap(cityMap, cities);
    int total = 0;
    vector<int> currentRoute;

    /*
    Option 1 - connect all the nodes in the order they were read in
    */
    // start comment here if choosing Option 2
    // map<int, Node*>::iterator next = cityMap.begin();
    // for (map<int, Node*>::iterator itr = cityMap.begin(); itr != cityMap.end(); itr++)
    // {
    //     currentRoute.push_back(itr->first);
    //     next++;
    //     if (next == cityMap.end())  // connect last city to first city for cycle
    //         next = cityMap.begin();
    //     total += connectCity(itr->second, next->second);
    //     // cout << "Add city " << itr->first << " -> " << next->first << ": "<< total << endl;
    // }
    // end comment here

    Node* startNode = cityMap[0];

    /*
    Option 2 - nearest neighbor algo
    */
    // start comment here if choosing Option 1
    Node* current = startNode;
    set<int> visited;
    visited.insert(startNode->cityID);
    currentRoute.push_back(current->cityID);
    // loop until all cities are visited
    do
    {
        int minId = getNearestNeighbor(current, visited);
        visited.insert(minId);
        total += connectCity(current, cityMap[minId]);
        current = current->next;
        currentRoute.push_back(current->cityID);
    } while (visited.size() < numCities);
    // connect the last city to the first one to complete the tour
    total += connectCity(current, startNode);
    // cout << total << endl;
    // end comment here


    /*
    Optimization - 2-Opt Swap
    */
    bool improve = true;
    int bestDistance = total;
    int newDistance;
    vector<int> newRoute;

    srand(325);
    int i, k;

    cout << bestDistance << endl;
    do
    {
        improve = false;

        do {
            i = rand() % numCities;
        } while (i == 0);
        do {
            k = rand() % numCities;
        } while (k == i || k == 0);

        newRoute = twoOptSwap(startNode, cityMap[currentRoute[i]], cityMap[currentRoute[k]]);
        newDistance = calculateTotalDistance(newRoute, cityMap);
        if (newDistance < bestDistance)
        {
            cout << newDistance << endl;
            currentRoute = newRoute;
            reconnectNodes(currentRoute, cityMap);
        }

        // for (int i = 1; i < currentRoute.size() - 1; i++)
        // {
        //     for (int k = i + 1; k < currentRoute.size(); k++)
        //     {
        //         newRoute = twoOptSwap(startNode, cityMap[currentRoute[i]], cityMap[currentRoute[k]]);
        //         newDistance = calculateTotalDistance(newRoute, cityMap);
        //         if (newDistance < bestDistance)
        //         {
        //             currentRoute = newRoute;
        //             reconnectNodes(currentRoute, cityMap);
        //             improve = true;
        //         }
        //         start = time(NULL);
        //         if (start >= endwait)
        //         {
        //             improve = false;
        //             break;
        //         }
        //     }
        //     start = time(NULL);
        //     if (start >= endwait)
        //     {
        //         improve = false;
        //         break;
        //     }
        // }
        bestDistance = calculateTotalDistance(currentRoute, cityMap);
        start = time(NULL);
    } while (start <= endwait);

    // output tour to file
    string outfile = infile + ".tour";
    ofstream writeFile;
    writeFile.open(outfile);
    writeFile << bestDistance << endl;

    Node* traverse = startNode;
    do
    {
        writeFile << traverse->cityID << endl;
        traverse = traverse->next;
    } while(traverse != startNode);

    writeFile.close();

    // output time to run
    clock_t clock_end = clock();
    double elapsed = (double)(clock_end - clock_start) / CLOCKS_PER_SEC;
    cout << "Time elapsed: " << elapsed << endl;

    return 0;
}

#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <cmath>
#include <unordered_set>

// enum already assignes int values, however, i am doing it explicitly for better readability
enum TransportMode { Foot = 0, Bike = 1, Car = 2, Bus = 3, Rail = 4, Ship = 5 };

class Node;

class Arc {
public:
    Node* endNode;
    double distance;
    TransportMode mode;

    Arc() : endNode(nullptr), distance(0), mode(Foot) {}

    Arc(Node* endNode, double distance, TransportMode mode)
        : endNode(endNode), distance(distance), mode(mode) {}
};

class Node {
public:
    int reference;
    std::string name;
    double x;
    double y;
    std::unordered_map<Node*, Arc> neighbours;

    Node(int ref, const std::string& name, double x, double y)
        : reference(ref), name(name), x(x), y(y) {}

    inline void Addneighbour(Node* neighbour, double distance, TransportMode mode) {
        neighbours[neighbour] = Arc(neighbour, distance, mode);
    }
};

class Navigation {
    std::ofstream _outFile;
    std::unordered_map<int, Node*> nodes;

public:
	// Constructor and Destructor
    Navigation();
    ~Navigation();

	// Member functions
    bool BuildNetwork(const std::string& fileNamePlaces, const std::string& fileNameLinks);
    bool ProcessCommand(const std::string& commandString);

private:
    inline TransportMode StringToTransportMode(const std::string& modeStr) {
        if (modeStr == "Foot")
            return Foot;
        else if (modeStr == "Bike")
            return Bike;
        else if (modeStr == "Car")
            return Car;
        else if (modeStr == "Bus")
            return Bus;
        else if (modeStr == "Rail")
            return Rail;
        else if (modeStr == "Ship")
            return Ship;
        else
            return Foot; // Default mode
    }

	// THIS CALCULATES SQUARED DISTANCE, WHEN DISPLAYING DISTANCE, USE SQRT
    inline double CalculateDistance(const Node* startNode, const Node* endNode) {
        double dx = endNode->x - startNode->x;
        double dy = endNode->y - startNode->y;
        return dx * dx + dy * dy;
    }

    inline bool IsValidMode(TransportMode mode, TransportMode neighbourMode) {
        if (mode == Rail || mode == Ship) {
            return mode == neighbourMode;
        }
        else if (mode == Bus) {
            return neighbourMode == Bus || neighbourMode == Rail || neighbourMode == Ship;
        }
        else if (mode == Car) {
            return neighbourMode == Car || neighbourMode == Bus || neighbourMode == Ship;
        }
        else if (mode == Bike) {
            return neighbourMode == Bike || neighbourMode == Foot;
        }
        else {
            return true; // Foot can use any mode
        }
    }

    void FindMaxDist();
    void FindMaxLink();
    void FindDist(int startRef, int endRef);
    void FindNeighbour(int nodeRef);
    void CheckRoute(const std::string& modeStr, const std::vector<int>& nodeRefs);
    void FindRoute(const std::string& modeStr, int startRef, int endRef);
    void FindShortestRoute(const std::string& modeStr, int startRef, int endRef);
    bool FindRouteHelper(Node* currentNode, Node* endNode, TransportMode mode,
        std::unordered_set<Node*>& visited, std::vector<int>& route);
};
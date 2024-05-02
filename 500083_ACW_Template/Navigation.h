#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <cmath>
#include <unordered_set>

// PARASOFT WILL GIVE WARNINGS WITH THIS FILE, IGNORE IT
// "Member function '[function]' returns handles to member data: [data]"
// parasoft wants to pass by value for better encapsulation
// but we want to pass by reference as its more performant

enum class TransportMode { Foot = 0, Bike = 1, Car = 2, Bus = 3, Rail = 4, Ship = 5 };

class Node;

// arc struct
// it is a struct as it does not need any member functions
struct Arc {
    const Node* endNode;
    double distance;
    TransportMode mode;

    Arc() = default;

    Arc(const Node* p_endNode, double p_distance, TransportMode p_mode)
        : endNode(p_endNode), distance(p_distance), mode(p_mode) {}

    // Copy constructor
    Arc(const Arc& other) = default;

    // Assignment operator
    Arc& operator=(const Arc& other) {
        if (this == &other) {
            return *this;
        }

        endNode = other.endNode;
        distance = other.distance;
        mode = other.mode;
        return *this;
    }

    // Destructor
    ~Arc() = default;
};

// node class
class Node final {
private:
    std::unordered_map<const Node*, Arc> m_neighbours;
    std::string m_name;
    const double m_x;
    const double m_y;
    const int m_reference;

public:
	// constructor
    Node(int ref, const std::string& name, double x, double y)
        : m_neighbours(),
        m_name(name),
        m_x(x),
        m_y(y),
        m_reference(ref) {}

	// destructor
    Node(const Node&) = delete;
    Node& operator=(const Node&) = delete;

	// method to add neighbour to node with distance and transport mode
    void AddNeighbour(const Node* neighbour, double distance, TransportMode mode) {
        m_neighbours[neighbour] = Arc(neighbour, distance, mode);
    }

    // getters
    int GetReference() const { return m_reference; }
    double GetX() const { return m_x; }
    double GetY() const { return m_y; }

	// ignore parasoft warnings
    const std::string& GetName() const { return m_name; }
    const std::unordered_map<const Node*, Arc>& GetNeighbours() const { return m_neighbours; }
	// ignore parasoft warnings
};

// navigation class
// this class will be used to build the network and process commands
class Navigation final {
private:
	// member variables
    std::ofstream m_outFile;
    std::unordered_map<int, Node*> m_nodes;

public:
	// constructor and destructor
    Navigation();
    ~Navigation();

    Navigation(const Navigation&) = delete;
    Navigation& operator=(const Navigation&) = delete;

	// member functions
    bool BuildNetwork(const std::string& fileNamePlaces, const std::string& fileNameLinks);
    bool ProcessCommand(const std::string& commandString);

	// getters
    // ignore parasoft warnings
    const std::unordered_map<int, Node*>& GetNodes() const { return m_nodes; }
    const std::ofstream& GetOutFile() const { return m_outFile; }
	// ignore parasoft warnings

private:
	// member functions
	// method to convert string from links csv to transport mode enum value
    inline TransportMode StringToTransportMode(const std::string& modeStr) const {
        if (modeStr == "Foot")
            return TransportMode::Foot;
        else if (modeStr == "Bike")
            return TransportMode::Bike;
        else if (modeStr == "Car")
            return TransportMode::Car;
        else if (modeStr == "Bus")
            return TransportMode::Bus;
        else if (modeStr == "Rail")
            return TransportMode::Rail;
        else if (modeStr == "Ship")
            return TransportMode::Ship;
        else
            return TransportMode::Foot;
    }

	// method to calculate squared distance between two nodes
	// REMEMBER TO SQUARE ROOT THE RETURN VALUE
    inline double CalculateDistance(const Node* startNode, const Node* endNode) const {
        const double dx = endNode->GetX() - startNode->GetX();
        const double dy = endNode->GetY() - startNode->GetY();
        return dx * dx + dy * dy;
    }

	// method to check if the mode is valid for the check methods
    inline bool IsValidMode(TransportMode mode, TransportMode neighbourMode) const {
        if (mode == TransportMode::Rail || mode == TransportMode::Ship) {
            return mode == neighbourMode;
        }
        else if (mode == TransportMode::Bus) {
            return neighbourMode == TransportMode::Bus || neighbourMode == TransportMode::Rail || neighbourMode == TransportMode::Ship;
        }
        else if (mode == TransportMode::Car) {
            return neighbourMode == TransportMode::Car || neighbourMode == TransportMode::Bus || neighbourMode == TransportMode::Ship;
        }
        else if (mode == TransportMode::Bike) {
            return neighbourMode == TransportMode::Bike || neighbourMode == TransportMode::Foot;
        }
        else {
            return true;
        }
    }

	// member functions
    void FindMaxDist();
    void FindMaxLink();
    void FindDist(int startRef, int endRef);
    void FindNeighbour(int nodeRef);
    void CheckRoute(const std::string& modeStr, const std::vector<int>& nodeRefs);
    void FindRoute(const std::string& modeStr, int startRef, int endRef);
    void FindShortestRoute(const std::string& modeStr, int startRef, int endRef);
    bool FindRouteHelper(const Node* currentNode, const Node* endNode, TransportMode mode,
        std::unordered_set<const Node*>& visited, std::vector<int>& route);
};
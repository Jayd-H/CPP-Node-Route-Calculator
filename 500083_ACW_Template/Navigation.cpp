#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

#include "Navigation.h"
#include "Utility.h"

// constructor to initialise the output file
Navigation::Navigation() : m_outFile("Output.txt") {
}

// destructor to delete all nodes in the map
Navigation::~Navigation() {
    for (const auto& pair : m_nodes) {
        delete pair.second;
    }
    m_nodes.clear();
}

// method to process the command string
bool Navigation::ProcessCommand(const std::string& commandString) {
    static std::istringstream inString;
    inString.clear();
    inString.str(commandString);

    std::string command;
    inString >> command;

    if (command.compare("MaxDist") == 0) {
        FindMaxDist();
    }
    else if (command.compare("MaxLink") == 0) {
        FindMaxLink();
    }
    else if (command.compare("FindDist") == 0) {
        int startRef, endRef;
        inString >> startRef >> endRef;
        FindDist(startRef, endRef);
    }
    else if (command.compare("FindNeighbour") == 0) {
        int nodeRef;
        inString >> nodeRef;
        FindNeighbour(nodeRef);
    }
    else if (command.compare("Check") == 0) {
        std::string modeStr;
        std::vector<int> nodeRefs;
        inString >> modeStr;
        int ref;
        while (inString >> ref) {
            nodeRefs.push_back(ref);
        }
        CheckRoute(modeStr, nodeRefs);
    }
    else if (command.compare("FindRoute") == 0) {
        std::string modeStr;
        int startRef, endRef;
        inString >> modeStr >> startRef >> endRef;
        FindRoute(modeStr, startRef, endRef);
    }
    else if (command.compare("FindShortestRoute") == 0) {
        std::string modeStr;
        int startRef, endRef;
        inString >> modeStr >> startRef >> endRef;
        FindShortestRoute(modeStr, startRef, endRef);
    }
    else {
        return false;
    }

    return true;
}

// method to build the network of nodes and links
// it first reads the places file and then the links file
// it then calculates the distance between each pair of nodes
// and adds the nodes as neighbours to each other
bool Navigation::BuildNetwork(const std::string& fileNamePlaces, const std::string& fileNameLinks) {
    std::ifstream finPlaces(fileNamePlaces);
    std::ifstream finLinks(fileNameLinks);

    if (finPlaces.fail() || finLinks.fail()) {
        return false;
    }

    std::string line;

	// places file
    while (std::getline(finPlaces, line)) {
        std::string name;
        int reference;
        double latitude, longitude;
        std::istringstream iss(line);
        std::getline(iss, name, ',');
        iss >> reference;
        iss.ignore();
        iss >> latitude;
        iss.ignore();
        iss >> longitude;

		// convert latitude and longitude to UTM coordinates
        double x, y;
        Utility::LLtoUTM(latitude, longitude, x, y);
        m_nodes[reference] = new Node(reference, name, x, y);
    }

	// links file
    while (std::getline(finLinks, line)) {
        int startRef, endRef;
        std::string modeStr;
        std::istringstream iss(line);
        iss >> startRef;
        iss.ignore();
        iss >> endRef;
        iss.ignore();
        std::getline(iss, modeStr);

		// find the start and end nodes based on their references
        Node* const startNode = m_nodes[startRef];
        Node* const endNode = m_nodes[endRef];

		// get the transport mode
        const TransportMode mode = StringToTransportMode(modeStr);

		// calculate the distance between the nodes
        const double distance = CalculateDistance(startNode, endNode);

        startNode->AddNeighbour(endNode, distance, mode);
        endNode->AddNeighbour(startNode, distance, mode);
    }

    return true;
}

// COME BACK TO THIS
// 
// method to calculate the maximum distance between two nodes
// it iterates through every pair of nodes and calculates the distance between them
// it stores the maximum distance and the nodes that have this distance
// then outputs the result
void Navigation::FindMaxDist() {
    double maxSquaredDistance = 0.0;
    const Node* maxStartNode = nullptr;
    const Node* maxEndNode = nullptr;

    // iterate through all nodes
    auto current = m_nodes.begin();
    while (current != m_nodes.end()) {
        const Node* const startNode = current->second;
        auto next = std::next(current);

        // calculate distance to all other nodes that come after the current node
        while (next != m_nodes.end()) {
            const Node* const endNode = next->second;

            // calculate the squared distance between the nodes
            const double squaredDistance = CalculateDistance(startNode, endNode);

            // update the maximum squared distance and nodes
            if (squaredDistance > maxSquaredDistance) {
                maxSquaredDistance = squaredDistance;
                maxStartNode = startNode;
                maxEndNode = endNode;
            }

            ++next;
        }

        ++current;
    }

    // output
    if (maxStartNode != nullptr && maxEndNode != nullptr) {
        m_outFile << "MaxDist" << std::endl;
        m_outFile << maxStartNode->GetName() << "," << maxEndNode->GetName() << "," << std::fixed << std::setprecision(3) << sqrt(maxSquaredDistance) << std::endl;
        m_outFile << std::endl;
    }
}

// method to calculate the maximum link between two nodes
// it iterates through every pair of neighboured nodes to calculate distance between
// WOULD IT BE FASTER TO FIND THE ARC WITH THE LONGEST DISTANCE AND USE THAT INSTEAD?
// COME BACK TO THIS
void Navigation::FindMaxLink() {
    double maxSquaredDistance = 0.0;
    int maxStartRef = 0;
    int maxEndRef = 0;

    // iterate through all pairs of nodes
    for (const auto& startPair : m_nodes) {
        const Node* const startNode = startPair.second;
        for (const auto& endPair : startNode->GetNeighbours()) {

            const double distance = endPair.second.distance;

            if (distance > maxSquaredDistance) {
                const Node* const endNode = endPair.first;
                maxSquaredDistance = distance;
                maxStartRef = startNode->GetReference();
                maxEndRef = endNode->GetReference();
            }
        }
    }

    // output
    m_outFile << "MaxLink" << std::endl;
    m_outFile << maxStartRef << "," << maxEndRef << "," << std::fixed << std::setprecision(3) << sqrt(maxSquaredDistance) << std::endl;
    m_outFile << std::endl;
}

// method to find the distance between two nodes
void Navigation::FindDist(int startRef, int endRef) {
    const Node* startNode = nullptr;
    const Node* endNode = nullptr;

    // iterates through the nodes to find the start node
    const auto startIter = m_nodes.find(startRef);
    if (startIter != m_nodes.end()) {
        startNode = startIter->second;
    }

    // iterates through the nodes to find the end node
    const auto endIter = m_nodes.find(endRef);
    if (endIter != m_nodes.end()) {
        endNode = endIter->second;
    }

    // Output the result
    m_outFile << "FindDist " << startRef << " " << endRef << std::endl;


	// would it be faster to find the arc between the two nodes and use that distance instead of calculating it again?
    // COME BACK TO THIS

    if (startNode != nullptr && endNode != nullptr) {
        const double distance = CalculateDistance(startNode, endNode);
        m_outFile << startNode->GetName() << "," << endNode->GetName() << "," << std::fixed << std::setprecision(3) << sqrt(distance) << std::endl;
    }
    else {
        m_outFile << "ERROR: Invalid node reference(s)" << std::endl;
    }

    m_outFile << std::endl;
}

// method to find all the neighbours of a node
// it takes the reference to find the node in the map
// then outputs the references of all neighbouring nodes
void Navigation::FindNeighbour(int nodeRef) {
    m_outFile << "FindNeighbour " << nodeRef << std::endl;

    // find the node based on its reference
    const auto iter = m_nodes.find(nodeRef);
    if (iter != m_nodes.end()) {
        const Node* const node = iter->second;

        // output the references of all neighboring nodes
        for (const auto& pair : node->GetNeighbours()) {
            const Node* const neighbour = pair.first;
            m_outFile << neighbour->GetReference() << std::endl;
        }
    }
    else {
        m_outFile << "ERROR: Invalid node reference" << std::endl;
    }

    m_outFile << std::endl;
}

// method to check if a route is valid between multiple nodes with a given transport mode
// it takes the mode and the references of the nodes
// it then checks if there is a valid connection between the nodes 
// if the node has the correct neighbours and the correct transport mode it succeeds
// otherwise it fails
void Navigation::CheckRoute(const std::string& modeStr, const std::vector<int>& nodeRefs) {
    m_outFile << "Check " << modeStr;
    for (const int ref : nodeRefs) {
        m_outFile << " " << ref;
    }
    m_outFile << std::endl;

    const TransportMode mode = StringToTransportMode(modeStr);
    bool routeValid = true;

    for (size_t i = 1; i < nodeRefs.size(); ++i) {
        const int startRef = nodeRefs[i - 1];
        const int endRef = nodeRefs[i];

        // find the start and end nodes based on their references
        const auto startIter = m_nodes.find(startRef);
        const auto endIter = m_nodes.find(endRef);

        if (startIter != m_nodes.end() && endIter != m_nodes.end()) {
            const Node* const startNode = startIter->second;
            const Node* const endNode = endIter->second;

            // check if there is a valid connection between the nodes
            const auto& neighbours = startNode->GetNeighbours();
            const auto arcIter = neighbours.find(endNode);

            if (arcIter != neighbours.end()) {
                const TransportMode arcMode = arcIter->second.mode;
                if (IsValidMode(mode, arcMode)) {
                    m_outFile << startRef << "," << endRef << ",PASS" << std::endl;
                    continue;
                }
            }
        }

        // if the connection is invalid or nodes are not found
        m_outFile << startRef << "," << endRef << ",FAIL" << std::endl;
        routeValid = false;
        break;
    }

    m_outFile << std::endl;
}

void Navigation::FindRoute(const std::string& modeStr, int startRef, int endRef) {
    m_outFile << "FindRoute " << modeStr << " " << startRef << " " << endRef << std::endl;

    const TransportMode mode = StringToTransportMode(modeStr);

    // find the start and end nodes based on their references
    const auto startIter = m_nodes.find(startRef);
    const auto endIter = m_nodes.find(endRef);

    if (startIter != m_nodes.end() && endIter != m_nodes.end()) {
        const Node* const startNode = startIter->second;
        const Node* const endNode = endIter->second;

        std::unordered_set<const Node*> visited;
        std::vector<int> route;

        const bool found = FindRouteHelper(startNode, endNode, mode, visited, route);

        if (found) {
            for (const int ref : route) {
                m_outFile << ref << std::endl;
            }
            m_outFile << std::endl;
        }
        else {
            m_outFile << "FAIL" << std::endl;
            m_outFile << std::endl;
        }
    }
    else {
        m_outFile << "FAIL" << std::endl;
        m_outFile << std::endl;
    }
}

bool Navigation::FindRouteHelper(const Node* currentNode, const Node* endNode, TransportMode mode,
    std::unordered_set<const Node*>& visited, std::vector<int>& route) {
    visited.insert(currentNode);
    route.push_back(currentNode->GetReference());

    if (currentNode == endNode) {
        return true;
    }

    for (const auto& pair : currentNode->GetNeighbours()) {
        const Node* const neighbour = pair.first;
        const TransportMode arcMode = pair.second.mode;

        if (visited.count(neighbour) == 0 && IsValidMode(mode, arcMode)) {
            if (FindRouteHelper(neighbour, endNode, mode, visited, route)) {
                return true;
            }
        }
    }

    route.pop_back();
    return false;
}

// method to find the shortest route between two nodes
// utilises a priority queue to store nodes with their distances
// it then iterates through the nodes to find the shortest route
// uses a altered version of djikstra's algorithm with weights of 1
void Navigation::FindShortestRoute(const std::string& modeStr, int startRef, int endRef) {
    m_outFile << "FindShortestRoute " << modeStr << " " << startRef << " " << endRef << std::endl;
    const TransportMode mode = StringToTransportMode(modeStr);

    // find the start and end nodes based on their references
    const auto startIter = m_nodes.find(startRef);
    const auto endIter = m_nodes.find(endRef);

    if (startIter != m_nodes.end() && endIter != m_nodes.end()) {
        const Node* const startNode = startIter->second;
        const Node* const endNode = endIter->second;

        std::unordered_map<const Node*, int> distances;
        std::unordered_map<const Node*, const Node*> previous;
        std::unordered_set<const Node*> visited;

        // initialize distances and previous nodes
        for (const auto& pair : m_nodes) {
            const Node* const node = pair.second;
            distances[node] = std::numeric_limits<int>::max();
            previous[node] = nullptr;
        }

        distances[startNode] = 0;

        // priority queue to store nodes with their distances
        std::priority_queue<std::pair<int, const Node*>, std::vector<std::pair<int, const Node*>>, std::greater<std::pair<int, const Node*>>> pq;
        pq.push(std::make_pair(0, startNode));

        while (!pq.empty()) {
            const Node* const currentNode = pq.top().second;
            pq.pop();

            if (currentNode == endNode) {
                // reached the end node, construct the shortest route
                std::vector<int> route;
                const Node* node = endNode;
                while (node != nullptr) {
                    route.push_back(node->GetReference());
                    node = previous.at(node);
                }
                std::reverse(route.begin(), route.end());

                // output the shortest route
                for (const int ref : route) {
                    m_outFile << ref << std::endl;
                }
                m_outFile << std::endl;
                return;
            }

            if (visited.count(currentNode) > 0) {
                continue;
            }

            visited.insert(currentNode);

            for (const auto& pair : currentNode->GetNeighbours()) {
                const TransportMode arcMode = pair.second.mode;
                if (IsValidMode(mode, arcMode)) {
                    const Node* const neighbour = pair.first;
                    const int newDistance = distances.at(currentNode) + 1;
                    if (newDistance < distances.at(neighbour)) {
                        distances[neighbour] = newDistance;
                        previous[neighbour] = currentNode;
                        pq.push(std::make_pair(newDistance, neighbour));
                    }
                }
            }
        }

        // no valid route found
        m_outFile << "FAIL" << std::endl;
        m_outFile << std::endl;
    }
    else {
        // start or end node not found
        m_outFile << "FAIL" << std::endl;
        m_outFile << std::endl;
    }
}
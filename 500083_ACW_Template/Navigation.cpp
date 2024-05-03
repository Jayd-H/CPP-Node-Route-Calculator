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
Navigation::Navigation()
    : m_outFile("Output.txt")
{
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

    // calculate maximum distance between all pairs of nodes
    double maxDistance = 0.0;
    const Node* maxDistStartNode = nullptr;
    const Node* maxDistEndNode = nullptr;

    for (const auto& startPair : m_nodes) {
        const Node* const startNode = startPair.second;
        for (const auto& endPair : m_nodes) {
            if (startPair.first < endPair.first) {
                const Node* const endNode = endPair.second;
                const double distance = CalculateDistance(startNode, endNode);
                if (distance > maxDistance) {
                    maxDistance = distance;
                    maxDistStartNode = startNode;
                    maxDistEndNode = endNode;
                }
            }
        }
    }

    // prepare output stream for max distance
    if (maxDistStartNode != nullptr && maxDistEndNode != nullptr) {
        m_maxDistStream << "MaxDist" << "\n";
        m_maxDistStream << maxDistStartNode->GetName() << "," << maxDistEndNode->GetName() << "," << std::fixed << std::setprecision(3) << sqrt(maxDistance) << '\n';
        m_maxLinkStream << "\n";
    }

    // calculate maximum link distance
    double maxLinkDistance = 0.0;
    int maxLinkStartRef = 0;
    int maxLinkEndRef = 0;

    for (const auto& startPair : m_nodes) {
        const Node* const startNode = startPair.second;
        for (const auto& endPair : startNode->GetNeighbours()) {
            const double distance = endPair.second.distance;
            if (distance > maxLinkDistance) {
                const Node* const endNode = endPair.first;
                maxLinkDistance = distance;
                maxLinkStartRef = startNode->GetReference();
                maxLinkEndRef = endNode->GetReference();
            }
        }
    }

    m_maxLinkStream << "MaxLink" << "\n";
    m_maxLinkStream << maxLinkStartRef << "," << maxLinkEndRef << "," << std::fixed << std::setprecision(3) << sqrt(maxLinkDistance) << "\n";
    m_maxLinkStream << "\n";

    return true;
}

// method to output the stored output stream of maxdist
void Navigation::FindMaxDist() {
    m_outFile << m_maxDistStream.str();
}

// method to output the stored output stream of maxlink
void Navigation::FindMaxLink() {
    m_outFile << m_maxLinkStream.str();
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

    // output the result
    m_outFile << "FindDist " << startRef << " " << endRef << "\n";

    if (startNode != nullptr && endNode != nullptr) {
        const double distance = CalculateDistance(startNode, endNode);
        m_outFile << startNode->GetName() << "," << endNode->GetName() << "," << std::fixed << std::setprecision(3) << sqrt(distance) << "\n";
    }
    else {
        m_outFile << "ERROR: Invalid node reference(s)" << "\n";
    }

    m_outFile << "\n";
}

// method to find all the neighbours of a node
// it takes the reference to find the node in the map
// then outputs the references of all neighbouring nodes
void Navigation::FindNeighbour(int nodeRef) {
    m_outFile << "FindNeighbour " << nodeRef << "\n";

    // find the node based on its reference
    const auto iter = m_nodes.find(nodeRef);
    if (iter != m_nodes.end()) {
        const Node* const node = iter->second;

        // output the references of all neighboring nodes
        for (const auto& pair : node->GetNeighbours()) {
            const Node* const neighbour = pair.first;
            m_outFile << neighbour->GetReference() << "\n";
        }
    }
    else {
        m_outFile << "ERROR: Invalid node reference" << "\n";
    }

    m_outFile << "\n";
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
    m_outFile << "\n";

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
                    m_outFile << startRef << "," << endRef << ",PASS" << "\n";
                    continue;
                }
            }
        }

        // if the connection is invalid or nodes are not found
        m_outFile << startRef << "," << endRef << ",FAIL" << "\n";
        routeValid = false;
        break;
    }

    m_outFile << "\n";
}

// method to find a route between two nodes using DFS
// it takes the mode and the references of the start and end nodes
// it then uses a queue to store nodes and iterates through them to find a valid route
// if a valid route is found it outputs the references of the nodes
// otherwise it outputs FAIL
void Navigation::FindRoute(const std::string& modeStr, int startRef, int endRef) {
    m_outFile << "FindRoute " << modeStr << " " << startRef << " " << endRef << "\n";

    const TransportMode mode = StringToTransportMode(modeStr);

    // find the start and end nodes based on their references
    const auto startIter = m_nodes.find(startRef);
    const auto endIter = m_nodes.find(endRef);

    if (startIter != m_nodes.end() && endIter != m_nodes.end()) {
        const Node* const startNode = startIter->second;
        const Node* const endNode = endIter->second;

        std::unordered_set<const Node*> visited;
        std::vector<int> route;
        std::queue<const Node*> queue;

        queue.push(startNode);
        visited.insert(startNode);

        while (!queue.empty()) {
            const Node* const currentNode = queue.front();
            queue.pop();

            route.push_back(currentNode->GetReference());

            if (currentNode == endNode) {
                // Found a valid route
                for (const int ref : route) {
                    m_outFile << ref << "\n";
                }
                m_outFile << "\n";
                return;
            }

            for (const auto& pair : currentNode->GetNeighbours()) {
                const Node* const neighbour = pair.first;
                const TransportMode arcMode = pair.second.mode;

                if (visited.count(neighbour) == 0 && IsValidMode(mode, arcMode)) {
                    queue.push(neighbour);
                    visited.insert(neighbour);
                }
            }
        }

        // No valid route found
        m_outFile << "FAIL" << "\n";
        m_outFile << "\n";
    }
    else {
        m_outFile << "FAIL" << "\n";
        m_outFile << "\n";
    }
}

// method to find the shortest route between two nodes using BFS
// utilises a queue to store nodes 
// it then iterates through the nodes to find the shortest route
// if a valid route is found it outputs the references of the nodes
// otherwise it outputs FAIL
void Navigation::FindShortestRoute(const std::string& modeStr, int startRef, int endRef) {
    m_outFile << "FindShortestRoute " << modeStr << " " << startRef << " " << endRef << "\n";

    const TransportMode mode = StringToTransportMode(modeStr);

    // find the start and end nodes based on their references
    const auto startIter = m_nodes.find(startRef);
    const auto endIter = m_nodes.find(endRef);

    if (startIter != m_nodes.end() && endIter != m_nodes.end()) {
        const Node* const startNode = startIter->second;
        const Node* const endNode = endIter->second;

        std::unordered_map<const Node*, const Node*> previous;
        std::unordered_set<const Node*> visited;
        std::queue<const Node*> queue;

        visited.insert(startNode);
        queue.push(startNode);

        while (!queue.empty()) {
            const Node* const currentNode = queue.front();
            queue.pop();

            if (currentNode == endNode) {
                // reached the end node, construct the shortest route
                std::vector<int> route;
                const Node* node = endNode;
                while (node != nullptr) {
                    route.push_back(node->GetReference());
                    node = previous[node];
                }
                std::reverse(route.begin(), route.end());

                // output the shortest route
                for (const int ref : route) {
                    m_outFile << ref << "\n";
                }
                m_outFile << "\n";
                return;
            }

            for (const auto& pair : currentNode->GetNeighbours()) {
                const TransportMode arcMode = pair.second.mode;
                if (IsValidMode(mode, arcMode)) {
                    const Node* const neighbour = pair.first;
                    if (visited.count(neighbour) == 0) {
                        visited.insert(neighbour);
                        previous[neighbour] = currentNode;
                        queue.push(neighbour);
                    }
                }
            }
        }

        // no valid route found
        m_outFile << "FAIL" << "\n";
        m_outFile << "\n";
    }
    else {
        // start or end node not found
        m_outFile << "FAIL" << "\n";
        m_outFile << "\n";
    }
}
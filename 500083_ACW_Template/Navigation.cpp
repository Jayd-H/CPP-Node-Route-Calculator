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


Navigation::Navigation() : _outFile("Output.txt") {
}

Navigation::~Navigation() {
    // Iterate through the nodes map and delete each Node object
    for (auto& pair : nodes) {
        delete pair.second;
    }
    nodes.clear();
}

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

bool Navigation::BuildNetwork(const std::string& fileNamePlaces, const std::string& fileNameLinks) {
    std::ifstream finPlaces(fileNamePlaces);
    std::ifstream finLinks(fileNameLinks);

    if (finPlaces.fail() || finLinks.fail()) {
        return false;
    }

    std::string line;
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

        double x, y;
        Utility::LLtoUTM(latitude, longitude, x, y);
        nodes[reference] = new Node(reference, name, x, y);
    }

    while (std::getline(finLinks, line)) {
        int startRef, endRef;
        std::string modeStr;
        std::istringstream iss(line);
        iss >> startRef;
        iss.ignore();
        iss >> endRef;
        iss.ignore();
        std::getline(iss, modeStr);

        Node* startNode = nodes[startRef];
        Node* endNode = nodes[endRef];
        TransportMode mode = StringToTransportMode(modeStr);
        double distance = CalculateDistance(startNode, endNode);
        startNode->Addneighbour(endNode, distance, mode);
        endNode->Addneighbour(startNode, distance, mode);
    }

    return true;
}

void Navigation::FindMaxDist() {
    double maxSquaredDistance = 0.0;
    Node* maxStartNode = nullptr;
    Node* maxEndNode = nullptr;

    // iterate through all nodes
    auto current = nodes.begin();
    while (current != nodes.end()) {
        Node* startNode = current->second;
        auto next = std::next(current);

        // calculate distance to all other nodes that come after the current node
        while (next != nodes.end()) {
            Node* endNode = next->second;

            // calculate the squared distance between the nodes
            double squaredDistance = CalculateDistance(startNode, endNode);

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
        _outFile << "MaxDist" << std::endl;
        _outFile << maxStartNode->name << "," << maxEndNode->name << "," << std::fixed << std::setprecision(3) << sqrt(maxSquaredDistance) << std::endl;
        _outFile << std::endl;
    }
}

void Navigation::FindMaxLink() {
    double maxDistance = 0.0;
    int maxStartRef = 0;
    int maxEndRef = 0;

	// iterate through all pairs of nodes
    for (const auto& startPair : nodes) {
        Node* startNode = startPair.second;

        for (const auto& endPair : startNode->neighbours) {
            Node* endNode = endPair.first;
            double distance = endPair.second.distance;

            if (distance > maxDistance) {
                maxDistance = distance;
                maxStartRef = startNode->reference;
                maxEndRef = endNode->reference;
            }
        }
    }

    // output
    _outFile << "MaxLink" << std::endl;
    _outFile << maxStartRef << "," << maxEndRef << "," << std::fixed << std::setprecision(3) << sqrt(maxDistance) << std::endl;
    _outFile << std::endl;
}

void Navigation::FindDist(int startRef, int endRef) {
    Node* startNode = nullptr;
    Node* endNode = nullptr;

    auto startIter = nodes.find(startRef);
    if (startIter != nodes.end()) {
        startNode = startIter->second;
    }

    auto endIter = nodes.find(endRef);
    if (endIter != nodes.end()) {
        endNode = endIter->second;
    }

    // Output the result
    _outFile << "FindDist " << startRef << " " << endRef << std::endl;

    if (startNode != nullptr && endNode != nullptr) {
        double distance = CalculateDistance(startNode, endNode);
        _outFile << startNode->name << "," << endNode->name << "," << std::fixed << std::setprecision(3) << sqrt(distance) << std::endl;
    }
    else {
        _outFile << "ERROR: Invalid node reference(s)" << std::endl;
    }

    _outFile << std::endl;
}

void Navigation::FindNeighbour(int nodeRef) {
    _outFile << "FindNeighbour " << nodeRef << std::endl;

    // Find the node based on its reference
    auto iter = nodes.find(nodeRef);
    if (iter != nodes.end()) {
        Node* node = iter->second;

        // Output the references of all neighboring nodes
        for (const auto& pair : node->neighbours) {
            Node* neighbour = pair.first;
            _outFile << neighbour->reference << std::endl;
        }
    }
    else {
        _outFile << "ERROR: Invalid node reference" << std::endl;
    }

    _outFile << std::endl;
}

void Navigation::CheckRoute(const std::string& modeStr, const std::vector<int>& nodeRefs) {
    _outFile << "Check " << modeStr;
    for (int ref : nodeRefs) {
        _outFile << " " << ref;
    }
    _outFile << std::endl;

    TransportMode mode = StringToTransportMode(modeStr);

    bool routeValid = true;
    for (size_t i = 1; i < nodeRefs.size(); ++i) {
        int startRef = nodeRefs[i - 1];
        int endRef = nodeRefs[i];

        // Find the start and end nodes based on their references
        auto startIter = nodes.find(startRef);
        auto endIter = nodes.find(endRef);

        if (startIter != nodes.end() && endIter != nodes.end()) {
            Node* startNode = startIter->second;
            Node* endNode = endIter->second;

            // Check if there is a valid connection between the nodes
            auto arcIter = startNode->neighbours.find(endNode);
            if (arcIter != startNode->neighbours.end()) {
                TransportMode arcMode = arcIter->second.mode;
                if (IsValidMode(mode, arcMode)) {
                    _outFile << startRef << "," << endRef << ",PASS" << std::endl;
                    continue;
                }
            }
        }

        // If the connection is invalid or nodes are not found
        _outFile << startRef << "," << endRef << ",FAIL" << std::endl;
        routeValid = false;
        break;
    }

     _outFile << std::endl;

}

void Navigation::FindRoute(const std::string& modeStr, int startRef, int endRef) {
    _outFile << "FindRoute " << modeStr << " " << startRef << " " << endRef << std::endl;

    TransportMode mode = StringToTransportMode(modeStr);

    // Find the start and end nodes based on their references
    auto startIter = nodes.find(startRef);
    auto endIter = nodes.find(endRef);

    if (startIter != nodes.end() && endIter != nodes.end()) {
        Node* startNode = startIter->second;
        Node* endNode = endIter->second;

        std::unordered_set<Node*> visited;
        std::vector<int> route;

        bool found = FindRouteHelper(startNode, endNode, mode, visited, route);

        if (found) {
            for (int ref : route) {
                _outFile << ref << std::endl;
            }
            _outFile << std::endl;
        }
        else {
            _outFile << "FAIL" << std::endl;
            _outFile << std::endl;
        }
    }
    else {
        _outFile << "FAIL" << std::endl;
        _outFile << std::endl;
    }
}

bool Navigation::FindRouteHelper(Node* currentNode, Node* endNode, TransportMode mode,
    std::unordered_set<Node*>& visited, std::vector<int>& route) {
    visited.insert(currentNode);
    route.push_back(currentNode->reference);

    if (currentNode == endNode) {
        return true;
    }

    for (const auto& pair : currentNode->neighbours) {
        Node* neighbour = pair.first;
        TransportMode arcMode = pair.second.mode;

        if (visited.count(neighbour) == 0 && IsValidMode(mode, arcMode)) {
            if (FindRouteHelper(neighbour, endNode, mode, visited, route)) {
                return true;
            }
        }
    }

    route.pop_back();
    return false;
}

void Navigation::FindShortestRoute(const std::string& modeStr, int startRef, int endRef) {
    _outFile << "FindShortestRoute " << modeStr << " " << startRef << " " << endRef << std::endl;

    TransportMode mode = StringToTransportMode(modeStr);

    // Find the start and end nodes based on their references
    auto startIter = nodes.find(startRef);
    auto endIter = nodes.find(endRef);

    if (startIter != nodes.end() && endIter != nodes.end()) {
        Node* startNode = startIter->second;
        Node* endNode = endIter->second;

        std::unordered_map<Node*, int> distances;
        std::unordered_map<Node*, Node*> previous;
        std::unordered_set<Node*> visited;

        // Initialize distances and previous nodes
        for (const auto& pair : nodes) {
            Node* node = pair.second;
            distances[node] = std::numeric_limits<int>::max();
            previous[node] = nullptr;
        }
        distances[startNode] = 0;

        // Priority queue to store nodes with their distances
        std::priority_queue<std::pair<int, Node*>, std::vector<std::pair<int, Node*>>, std::greater<std::pair<int, Node*>>> pq;
        pq.push(std::make_pair(0, startNode));

        while (!pq.empty()) {
            Node* currentNode = pq.top().second;
            pq.pop();

            if (currentNode == endNode) {
                // Reached the end node, construct the shortest route
                std::vector<int> route;
                Node* node = endNode;
                while (node != nullptr) {
                    route.push_back(node->reference);
                    node = previous[node];
                }
                std::reverse(route.begin(), route.end());

                // Output the shortest route
                for (int ref : route) {
                    _outFile << ref << std::endl;
                }
                _outFile << std::endl;
                return;
            }

            if (visited.count(currentNode) > 0) {
                continue;
            }
            visited.insert(currentNode);

            for (const auto& pair : currentNode->neighbours) {
                Node* neighbour = pair.first;
                TransportMode arcMode = pair.second.mode;

                if (IsValidMode(mode, arcMode)) {
                    int newDistance = distances[currentNode] + 1;
                    if (newDistance < distances[neighbour]) {
                        distances[neighbour] = newDistance;
                        previous[neighbour] = currentNode;
                        pq.push(std::make_pair(newDistance, neighbour));
                    }
                }
            }
        }

        // No valid route found
        _outFile << "FAIL" << std::endl;
        _outFile << std::endl;
    }
    else {
        // Start or end node not found
        _outFile << "FAIL" << std::endl;
        _outFile << std::endl;
    }
}

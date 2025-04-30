#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

bool channelBusy = false; // Flag to indicate if the channel is busy

struct Node {
    int ID;
    int t; // t is the number of clock ticks since the last transmission, 0<=t<T
    int R; // R is the backoff window size
    int K; // K = (ID + t) % R
    int collisionCount; // Number of collisions for this node
    int transmissionTime; // Time of transmission for this node
};

int main(int argc, char* argv[]) {
    if(argc != 2) {
        cout << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    ifstream input(argv[1]);
    if (!input) {
        cout << "Error opening file: " << argv[1] << endl;
        return 1;
    }
    int N, delta_t, T, L;
    vector<int> R;

    string temp;
    input >> temp >> N; // Read the number of nodes
    input >> temp >> delta_t; // Read the delta_t value
    input >> temp >> T; // Read the T value
    input >> temp >> L; // Read the L value

    R.resize(L); // Resize the R vector to hold the backoff window sizes
    input >> temp; // Read the R values
    for (int i = 0; i < L; ++i) {
        input >> R[i]; // Read each backoff window size
    }
    input.close(); // Close the input file

    vector<Node> nodes(N); // Create a vector of nodes
    for (int i = 0; i < N; ++i) {
        nodes[i].ID = i; // Assign ID to each node
        nodes[i].t = 0; // Initialize t to 0
        nodes[i].R = R[0]; // Assign backoff window size based on ID
        nodes[i].K = (i + 0) % R[0]; // Calculate K
        nodes[i].collisionCount = 0; // Initialize collision count to 0
        nodes[i].transmissionTime = 0; // Initialize transmission time to 0
    }

    int successfulTicks = 0; // Count of successful transmissions

    for(int tick = 0; tick < T; ++tick) {
        vector<int> readyNodes; // Vector to hold nodes ready to transmit

        for(auto& node : nodes) node.t = tick;

        for(auto& node : nodes) {
            if(node.transsmissionTime > 0) { // Check if the node is ready to transmit
                node.transmissionTime--;
            }
        }

        channelBusy = any_of(nodes.begin(), nodes.end(), [](Node& node) { return node.transmissionTime > 0; });

        for (auto& node : nodes) {
            if(node.K == 0 && node.transmissionTime == 0) { // Check if the node is ready to transmit
                readyNodes.push_back(node.ID); // Add node ID to ready nodes
            }
        }

        if (!channelBusy && !readyNodes.empty()) { // If the channel is not busy and there are ready nodes
            if (readyNodes.size() == 1) { // If only one node is ready to transmit
                Node& node = nodes[readyNodes[0]];
                node.transmissionTime = delta_t - 1; // Set transmission time
                successfulTicks += delta_t; // Increment successful transmission count
                node.R = R[0]; // Reset backoff window size
                node.collisionCount = 0; // Reset collision count
                node.K = (ticks + node.ID) % node.R; // Recalculate K
            }
            else { // If multiple nodes are ready to transmit
                for (int id : readyNodes) {
                    Node& node = nodes[id];
                    node.collisionCount++; // Increment collision count
                    
                    if(node.collisionCount >= L) { // If collision count exceeds 1
                        node.collisionCount = 0; // Reset collision count
                        node.R = R[0]; // Reset backoff window size
                    }
                    else {
                        int newIndex = min(node.collisionCount, L - 1); // Get the new index for backoff window size
                        node.R = R[newIndex]; // Update backoff window size
                    }

                    node.K = (tick + node.ID) % node.R; // Recalculate K
                }
            }
        }
        else if (!channelBusy && readyNodes.empty()) { // If the channel is idle and no nodes are ready to transmit
            for (auto& node : nodes) {
                if (node.K > 0) { // If K is greater than 0, decrement it
                    node.K--;
                }
            }
        }
    }

    cout << "Link utilization: ";
    cout.precision(2);
    cout << fixed << (double)successfulTicks / T << endl; // Calculate and print link utilization

    return 0; // Return success
}
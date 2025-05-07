#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

bool channelBusy = false;

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
    input >> temp >> N;
    input >> temp >> delta_t;
    input >> temp >> T;
    input >> temp >> L;

    R.resize(L); // Resize the R vector to hold the backoff window sizes
    input >> temp; // Read the R values
    for (int i = 0; i < L; ++i) {
        input >> R[i];
    }
    input.close();
    vector<Node> nodes(N); // Create a vector of nodes
    for (int i = 0; i < N; ++i) {
        nodes[i].ID = i;
        nodes[i].t = 0;
        nodes[i].R = R[0];
        nodes[i].K = (i + 0) % R[0];
        nodes[i].collisionCount = 0;
        nodes[i].transmissionTime = 0;
    }

    int successfulTicks = 0;

    for(int tick = 0; tick < T; ++tick) {
        vector<int> readyNodes; // Vector to hold nodes ready to transmit

        for(auto& node : nodes) node.t = tick;

        for(auto& node : nodes) {
            if(node.transmissionTime > 0) {
                node.transmissionTime--;
            }
        }

        channelBusy = any_of(nodes.begin(), nodes.end(), [](Node& node) { return node.transmissionTime > 0; });

        for (auto& node : nodes) {
            if(node.K == 0 && node.transmissionTime == 0) {
                readyNodes.push_back(node.ID);
            }
        }

        if (!channelBusy && !readyNodes.empty()) { // If the channel is not busy and there are ready nodes
            if (readyNodes.size() == 1) { // If only one node is ready to transmit
                Node& node = nodes[readyNodes[0]];
                node.transmissionTime = delta_t - 1;
                successfulTicks += delta_t;
                node.R = R[0];
                node.collisionCount = 0;
                node.K = (tick + node.ID) % node.R;
            }
            else { // If multiple nodes are ready to transmit
                for (int id : readyNodes) {
                    Node& node = nodes[id];
                    node.collisionCount++;
                    
                    if(node.collisionCount >= L) {
                        node.collisionCount = 0;
                        node.R = R[0];
                    }
                    else {
                        int newIndex = min(node.collisionCount, L - 1);
                        node.R = R[newIndex];
                    }

                    node.K = (tick + node.ID) % node.R;
                }
            }
        }
        else if (!channelBusy && readyNodes.empty()) { // If the channel is idle and no nodes are ready to transmit
            for (auto& node : nodes) {
                if (node.K > 0) {
                    node.K--;
                }
            }
        }
    }

    cout << "Link utilization: ";
    cout.precision(2);
    cout << fixed << (double)successfulTicks / T << endl; // Calculate and print link utilization

    return 0;
}

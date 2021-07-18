// clang++ -std=c++17 SuffixTree.cpp

#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <map>
#include <numeric>
#include <optional>
#include <ostream>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using namespace std;

struct SuffixNode {
    SuffixNode(): linkNode(0), edgeIds(vector<int>(27, -1)) {}
    int getEdgeId(char c) const {
        return c == '$' ? edgeIds[26] : edgeIds[c - 'a'];
    }
    void setEdgeId(char c, int edgeId) {
        if (c == '$') {
            edgeIds[26] = edgeId;
        } else {
            edgeIds[c - 'a'] = edgeId;
        }
    }
    int linkNode;
    vector<int> edgeIds;
};

struct SuffixEdge {
    SuffixEdge(int i): SuffixEdge(i, -1, -1) {}
    SuffixEdge(int i, int j, int nodeId): i(i), j(j), toNodeId(nodeId) {}
    int i;
    int j;
    int toNodeId;
};

class SuffixTree {
public:
    SuffixTree(string str)
        : s(str + '$'),
          n(s.size()),
          end(0),
          activeNodeId(0),
          activeIndex(-1),
          activeSize(0),
          remaining(0) {
        nodes.emplace_back(SuffixNode());
        processPhases();
        report();
    }
    
    void report() const {
        cout << "The string is: " << s << endl;
        cout << "The string size is: " << s.size() << endl;
        cout << "Total number of nodes (root and internal nodes): " << nodes.size() << endl;
        cout << "Total number of edges: " << edges.size() << endl;
        vector<pair<int, int>> links;
        int leaves = 0;
        for (int i = 0; i < nodes.size(); ++i) {
            const SuffixNode& node = nodes[i];
            if (i > 0 && node.linkNode > 0) {
                links.emplace_back(i, node.linkNode);
            }
            for (int j = 0; j < 27; ++j) {
                if (int edgeId = node.edgeIds[j]; edgeId != -1) {
                    const SuffixEdge& edge = edges[edgeId];
                    cout << "NodeId\t" << i << "\tTo\t" << edge.toNodeId << ",\tedgeId\t" << edgeId <<
                        ",\t" << s.substr(edge.i, getEdgeSize(edge)) << endl;
                    if (edge.toNodeId == -1) {
                        ++leaves;
                    }
                }
            }
            cout << endl;
        }
        cout << "Total number of leaves: " << leaves << endl;
        cout << "Total number of suffix links: " << links.size() << endl;
        for (int i = 0; i < links.size(); ++i) {
            cout << "Link\t" << links[i].first << "\tTo\t" << links[i].second << endl;
        }
        cout << endl;
    }
    
private:
    void followLink(int i, int j) {
        if (activeNodeId == 0) {
            ++i;
        }
        
        activeNodeId = nodes[activeNodeId].linkNode;
        activeIndex = -1;
        activeSize = 0;
        
        for (int len = j - i + 1, skip = 0; i > 0 && len; len -= skip) {
            int edgeId = nodes[activeNodeId].getEdgeId(s[i]);
            assert(edgeId != -1);
            SuffixEdge& edge = edges[edgeId];
            if (len < getEdgeSize(edge)) {
                activeIndex = edge.i;
                activeSize = len;
            } else if (len == getEdgeSize(edge)) {
                activeNodeId = edge.toNodeId;
                activeIndex = -1;
                activeSize = 0;
            } else {
                activeNodeId = edge.toNodeId;
                i += getEdgeSize(edge);
            }
            skip = min(len, getEdgeSize(edge));
        }
    }
    
    int rule2(int i) {
        int edgeId = nodes[activeNodeId].getEdgeId(s[activeIndex]);
        int newEdgeId1 = createNewEdge(edges[edgeId].i + activeSize, edges[edgeId].j, edges[edgeId].toNodeId);
        int newEdgeId2 = createNewEdge(i);
        int newNodeId = createNewNode();
        nodes[newNodeId].setEdgeId(s[edges[newEdgeId1].i], newEdgeId1);
        nodes[newNodeId].setEdgeId(s[edges[newEdgeId2].i], newEdgeId2);
        edges[edgeId].j = edges[edgeId].i + activeSize - 1;
        edges[edgeId].toNodeId = newNodeId;
        followLink(edges[edgeId].i, edges[edgeId].j);
        return newNodeId;
    }

    void rule3(int edgeId, int i) {
        ++activeSize;
        if (activeIndex == -1) {
            activeIndex = i;
        }
        
        if (edges[edgeId].toNodeId != -1 && activeSize == getEdgeSize(edgeId)) {
            activeNodeId = edges[edgeId].toNodeId;
            assert(activeNodeId != -1);
            activeIndex = -1;
            activeSize = 0;
        }
    }
    
    void processPhase(int i) {
        ++remaining;
        int prevInternalNodeId = -1;
        for (; remaining; --remaining) {
            if (activeIndex == -1) {
                if (int edgeId = nodes[activeNodeId].getEdgeId(s[i]); edgeId != -1) {
                    rule3(edgeId, i);
                    return;
                } else {
                    int newEdgeId = createNewEdge(i);
                    nodes[activeNodeId].setEdgeId(s[i], newEdgeId);
                    if (activeNodeId != 0) {
                        followLink(-1, -1);
                    }
                }
            } else {
                int edgeId = nodes[activeNodeId].getEdgeId(s[activeIndex]);
                if (s[i] == s[edges[edgeId].i + activeSize]) {
                    rule3(edgeId, i);
                    return;
                } else {
                    int currInternalNodeId = rule2(i);
                    if (prevInternalNodeId != -1) {
                        nodes[prevInternalNodeId].linkNode = currInternalNodeId;
                    }
                    prevInternalNodeId = currInternalNodeId;
                }
            }
        }
    }
    
    void processPhases() {
        for (int i = 0; i < n; ++i, ++end) {
            processPhase(i);
        }
    }
    
    int createNewNode() {
        int newNodeId = nodes.size();
        nodes.emplace_back();
        return newNodeId;
    }
    
    int createNewEdge(int i, int j = -1, int toNodeId = -1) {
        int newEdgeId = edges.size();
        edges.emplace_back(i, j, toNodeId);
        return newEdgeId;
    }
    
    int getEdgeSize(int edgeId) const {
        return getEdgeSize(edges[edgeId]);
    }
    
    int getEdgeSize(const SuffixEdge& edge) const {
        int e = (edge.j == -1 ? end : edge.j);
        return e - edge.i + 1;
    }
    
    string s;
    int n;
    int end;
    vector<SuffixNode> nodes;
    vector<SuffixEdge> edges;
    int activeNodeId;
    int activeIndex;
    int activeSize;
    int remaining;
};

int main(int argc, const char * argv[]) {
    SuffixTree t1("banana");
    SuffixTree t2("adeacdade");
    SuffixTree t3("abcabxabcd");
    SuffixTree t4("abcdefabxybcdmnabcdex");
    SuffixTree t5("abcadak");
    SuffixTree t6("abcabxabcd");
    SuffixTree t7("mississippi");
    SuffixTree t8("ooooooooo");
    SuffixTree t9("aa");
    SuffixTree t10("dodo");
    SuffixTree t11("dedododeodo");
    return 0;
}

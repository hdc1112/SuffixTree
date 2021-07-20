// clang++ -std=c++17 SuffixTree.cpp

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

class Node {
public:
    Node(): linkNodeId(0), childEdgeIds() {}
    
    int getEdgeId(char c) const {
        auto iterator = childEdgeIds.find(c);
        return iterator == childEdgeIds.end() ? -1 : iterator->second;
    }
    
    int getLinkNodeId() const {
        return linkNodeId;
    }
    
    void setEdgeId(char c, int edgeId) {
        childEdgeIds[c] = edgeId;
    }
    
    void setLinkNodeId(int nodeId) {
        linkNodeId = nodeId;
    }

private:
    int linkNodeId;
    unordered_map<char, int> childEdgeIds;
};

class Edge {
public:
    Edge(int index1): Edge(index1, -1, -1) {}
    
    Edge(int index1, int index2, int toNodeId): index1(index1),
                                                index2(index2),
                                                toNodeId(toNodeId) {}
    
    int getIndex1() const {
        return index1;
    }
    
    int getIndex2() const {
        return index2;
    }
    
    int getToNodeId() const {
        return toNodeId;
    }
    
    void setIndex2(int index) {
        index2 = index;
    }
    
    void setToNodeId(int nodeId) {
        toNodeId = nodeId;
    }
    
private:
    int index1;
    int index2;
    int toNodeId;
};

class Tree {
public:
    Tree(string str): s(str + '$'),
                      n(s.size()),
                      end(0),
                      activeNodeId(0),
                      activeIndex(-1),
                      activeSize(0),
                      remaining(0) {
        createNewNode();
        processPhases();
        checkCorrectness();
    }
    
    void checkCorrectness() const {
        cout << "The string is: " << s << endl;
        cout << "The string size is: " << s.size() << endl;
        cout << "Total number of nodes (root and internal nodes): ";
        cout << nodes.size() << endl;
        cout << "Total number of edges: " << edges.size() << endl;
        vector<pair<int, int>> links;
        int leaves = 0;
        for (int i = 0; i < nodes.size(); ++i) {
            const Node& node = nodes[i];
            if (i > 0 && node.getLinkNodeId() > 0) {
                links.emplace_back(i, node.getLinkNodeId());
            }
            for (int j = 0; j < 27; ++j) {
                char c = (j < 26 ? char('a' + j) : '$');
                if (int edgeId = node.getEdgeId(c); edgeId != -1) {
                    const Edge& edge = edges[edgeId];
                    cout << "NodeId\t" << i << "\tTo\t" << edge.getToNodeId();
                    cout << ",\tedgeId\t" << edgeId << ",\t";
                    cout << s.substr(edge.getIndex1(), getEdgeSize(edge));
                    cout << endl;
                    if (edge.getToNodeId() == -1) {
                        ++leaves;
                    }
                }
            }
            cout << endl;
        }
        cout << "Total number of leaves: " << leaves << endl;
        cout << "Total number of suffix links: " << links.size() << endl;
        for (int i = 0; i < links.size(); ++i) {
            cout << "Link\t" << links[i].first << "\tTo\t";
            cout << links[i].second << endl;
        }
        cout << endl;
    }
    
private:
    void followLink(int i, int j) {
        if (activeNodeId == 0) {
            ++i;
        }
        
        activeNodeId = nodes[activeNodeId].getLinkNodeId();
        activeIndex = -1;
        activeSize = 0;
        
        for (int len = j - i + 1, skip = 0; i > 0 && len; len -= skip) {
            int edgeId = nodes[activeNodeId].getEdgeId(s[i]);
            Edge& edge = edges[edgeId];
            if (len < getEdgeSize(edge)) {
                activeIndex = edge.getIndex1();
                activeSize = len;
            } else if (len == getEdgeSize(edge)) {
                activeNodeId = edge.getToNodeId();
                activeIndex = -1;
                activeSize = 0;
            } else {
                activeNodeId = edge.getToNodeId();
                i += getEdgeSize(edge);
            }
            skip = min(len, getEdgeSize(edge));
        }
    }
    
    int rule2(int index) {
        int edgeId = nodes[activeNodeId].getEdgeId(s[activeIndex]);
        int newEdgeId1 = createNewEdge(edges[edgeId].getIndex1() + activeSize,
                                       edges[edgeId].getIndex2(),
                                       edges[edgeId].getToNodeId());
        int newEdgeId2 = createNewEdge(index);
        int newNodeId = createNewNode();
        nodes[newNodeId].setEdgeId(s[edges[newEdgeId1].getIndex1()],
                                   newEdgeId1);
        nodes[newNodeId].setEdgeId(s[edges[newEdgeId2].getIndex1()],
                                   newEdgeId2);
        edges[edgeId].setIndex2(edges[edgeId].getIndex1() + activeSize - 1);
        edges[edgeId].setToNodeId(newNodeId);
        followLink(edges[edgeId].getIndex1(), edges[edgeId].getIndex2());
        return newNodeId;
    }

    void rule3(int edgeId, int i) {
        ++activeSize;
        if (activeIndex == -1) {
            activeIndex = i;
        }
        
        Edge& edge = edges[edgeId];
        int toNodeId = edge.getToNodeId();
        int edgeSize = getEdgeSize(edge);
        if (toNodeId != -1 && activeSize == edgeSize) {
            activeNodeId = toNodeId;
            activeIndex = -1;
            activeSize = 0;
        }
    }
    
    void processPhase(int i) {
        ++remaining;
        
        int prevNodeId = -1;
        
        while (remaining) {
            Node& node = nodes[activeNodeId];
            if (activeIndex == -1) {
                char c = s[i];
                int edgeId = node.getEdgeId(c);
                if (edgeId != -1) {
                    rule3(edgeId, i);
                    break;
                } else {
                    int newEdgeId = createNewEdge(i);
                    node.setEdgeId(c, newEdgeId);
                    if (activeNodeId != 0) {
                        followLink(-1, -1);
                    }
                }
            } else {
                char dir = s[activeIndex];
                int edgeId = node.getEdgeId(dir);
                Edge& edge = edges[edgeId];
                if (s[i] == s[edge.getIndex1() + activeSize]) {
                    rule3(edgeId, i);
                    break;
                } else {
                    int currNodeId = rule2(i);
                    if (prevNodeId != -1) {
                        Node& prevNode = nodes[prevNodeId];
                        prevNode.setLinkNodeId(currNodeId);
                    }
                    prevNodeId = currNodeId;
                    if (activeIndex == -1) {
                        Node& currNode = nodes[currNodeId];
                        currNode.setLinkNodeId(activeNodeId);
                    }
                }
            }
            
            --remaining;
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
    
    int createNewEdge(int index1, int index2 = -1, int toNodeId = -1) {
        int newEdgeId = edges.size();
        edges.emplace_back(index1, index2, toNodeId);
        return newEdgeId;
    }
    
    int getEdgeSize(int edgeId) const {
        const Edge& edge = edges[edgeId];
        return getEdgeSize(edge);
    }
    
    int getEdgeSize(const Edge& edge) const {
        int l = edge.getIndex1();
        int r = edge.getIndex2();
        if (r == -1) {
            r = end;
        }
        return r - l + 1;
    }
    
    string s;
    int n;
    int end;
    vector<Node> nodes;
    vector<Edge> edges;
    int activeNodeId;
    int activeIndex;
    int activeSize;
    int remaining;
};

int main(int argc, const char * argv[]) {
    Tree t1("banana");
    Tree t2("adeacdade");
    Tree t3("abcabxabcd");
    Tree t4("abcdefabxybcdmnabcdex");
    Tree t5("abcadak");
    Tree t6("abcabxabcd");
    Tree t7("mississippi");
    Tree t8("ooooooooo");
    Tree t9("aa");
    Tree t10("dodo");
    Tree t11("dedododeodo");
    return 0;
}


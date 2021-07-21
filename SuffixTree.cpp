#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

struct Node {
public:
    Node(): linkId(0), edges() {}
    
    int getEdgeId(char c) const {
        auto iterator = edges.find(c);
        return iterator == edges.end() ? -1 : iterator->second;
    }
    
    int getLinkId() const {
        return linkId;
    }

    void setEdgeId(char c, int e) {
        edges[c] = e;
    }
    
    void setLinkId(int v) {
        linkId = v;
    }
    
private:
    int linkId;
    unordered_map<char, int> edges;
};

class Edge {
public:
    Edge(int i1): Edge(i1, -1, -1) {}
    
    Edge(int i1, int i2, int v): i1(i1), i2(i2), v(v) {}
    
    int getI1() const {
        return i1;
    }
    
    int getI2() const {
        return i2;
    }
    
    int getToNodeId() const {
        return v;
    }
    
    void setI2(int i2) {
        this->i2 = i2;
    }
    
    void setNode(int v) {
        this->v = v;
    }
    
private:
    int i1;
    int i2;
    int v;
};

class Point {
public:
    Point(): v(0), i(-1), size(0) {}
    
    int getNode() const {
        return v;
    }
    
    int getI() const {
        return i;
    }
    
    int getSize() const {
        return size;
    }
    
    void setNode(int v) {
        this->v = v;
    }
    
    void setI(int i) {
        this->i = i;
    }
    
    void setSize(int size) {
        this->size = size;
    }
    
    void incrementSize() {
        ++size;
    }
    
private:
    int v;
    int i;
    int size;
};

class Tree {
public:
    Tree(string str): s(str + '$'),
                      n(s.size()),
                      end(0),
                      ap(),
                      rem(0) {
        createNewNode();
        processPhases();
        print();
    }
    
    void print() const {
        int leaves = 0;
        vector<pair<int, int>> links;
        // v1, e, v2, string
        vector<tuple<int, int, int, string>> edgeStrs;
        // v => str
        unordered_map<int, string> pathLabels;
        vector<string> leafStrs;
        
        for (int i = 0; i < nodes.size(); ++i) {
            if (i > 0 && nodes[i].getLinkId() > 0) {
                links.emplace_back(i, nodes[i].getLinkId());
            }
            for (int j = 0; j < 27; ++j) {
                char c = (j < 26 ? char('a' + j) : '$');
                if (int e = nodes[i].getEdgeId(c); e != -1) {
                    const Edge& edge = edges[e];
                    int toNode = edge.getToNodeId();
                    string str = s.substr(edge.getI1(), getEdgeSize(e));
                    edgeStrs.emplace_back(i, e, toNode, str);
                    if (toNode == -1) {
                        ++leaves;
                    }
                }
            }
        }
        vector<char> str;
        collectPathLabels(0, str, pathLabels, leafStrs);
        
        cout << "The string is: " << s << endl;
        cout << "The string size is: " << s.size() << endl;
        cout << "Total number of nodes (no leaves): " << nodes.size() << endl;
        cout << "Total number of edges: " << edges.size() << endl;
        cout << "Total number of leaves: " << leaves << endl;
        cout << "Total number of suffix links: " << links.size() << endl;
        cout << endl;
        cout << "Each edge's information:" << endl;
        for (auto [v1, e, v2, str]: edgeStrs) {
            cout << "Edge id " << e << " from node " << v1 << " to ";
            cout << "node " << v2 << " string is " << str << endl;
        }
        cout << endl;
        cout << "Each link's information:" << endl;
        for (int i = 0; i < links.size(); ++i) {
            cout << "Link\t" << links[i].first << "\tTo\t" << links[i].second;
            cout << ", path label from " << pathLabels[links[i].first];
            cout << " to " << pathLabels[links[i].second] << endl;
        }
        cout << endl;
        sort(leafStrs.begin(), leafStrs.end(), [&](auto& l, auto& r) {
            return l.size() < r.size();
        });
        cout << "All suffixes: " << endl;
        for (int i = 0; i < leafStrs.size(); ++i) {
            cout << (i + 1) << ": " << leafStrs[i] << endl;
        }
        cout << endl;
        cout << "Each node's path label: " << endl;
        for (auto [v, str]: pathLabels) {
            cout << "NodeId " << v << " path label is " << str << endl;
        }
        cout << endl;
        cout << endl;
    }
    
    void collectPathLabels(int v,
                           vector<char>& str,
                           unordered_map<int, string>& pathLabels,
                           vector<string>& leafStrs) const {
        if (v == -1) {
            leafStrs.emplace_back(str.begin(), str.end());
            return;
        }
        string pathLabel(str.begin(), str.end());
        pathLabels[v] = pathLabel;
        for (int i = 0; i < 27; ++i) {
            char c = (i < 26 ? char('a' + i) : '$');
            int e = nodes[v].getEdgeId(c);
            if (e != -1) {
                for (int diff = 0; diff < getEdgeSize(e); ++diff) {
                    str.emplace_back(s[edges[e].getI1() + diff]);
                }
                collectPathLabels(edges[e].getToNodeId(), str, pathLabels,
                                  leafStrs);
                for (int diff = 0; diff < getEdgeSize(e); ++diff) {
                    str.pop_back();
                }
            }
        }
    }
    
private:
    void followLink(int i, int j) {
        if (ap.getNode() == 0) {
            ++i;
        }
        
        ap.setNode(nodes[ap.getNode()].getLinkId());
        ap.setI(-1);
        ap.setSize(0);
        
        for (int len = j - i + 1, skip = 0; i > 0 && len; len -= skip) {
            int e = nodes[ap.getNode()].getEdgeId(s[i]);
            Edge& edge = edges[e];
            if (len < getEdgeSize(edge)) {
                ap.setI(edge.getI1());
                ap.setSize(len);
            } else if (len == getEdgeSize(edge)) {
                ap.setNode(edge.getToNodeId());
                ap.setI(-1);
                ap.setSize(0);
            } else {
                ap.setNode(edge.getToNodeId());
                i += getEdgeSize(edge);
            }
            skip = min(len, getEdgeSize(edge));
        }
    }
    
    int rule2(int i) {
        int e = nodes[ap.getNode()].getEdgeId(s[ap.getI()]);
        int newE1 = createNewEdge(edges[e].getI1() + ap.getSize(),
                                  edges[e].getI2(),
                                  edges[e].getToNodeId());
        int newE2 = createNewEdge(i);
        int newV = createNewNode();
        nodes[newV].setEdgeId(s[edges[newE1].getI1()], newE1);
        nodes[newV].setEdgeId(s[edges[newE2].getI1()], newE2);
        edges[e].setI2(edges[e].getI1() + ap.getSize() - 1);
        edges[e].setNode(newV);
        followLink(edges[e].getI1(), edges[e].getI2());
        return newV;
    }

    void rule3(int e, int i) {
        ap.incrementSize();
        if (ap.getI() == -1) {
            ap.setI(i);
        }
        
        Edge& edge = edges[e];
        int v = edge.getToNodeId();
        int edgeSize = getEdgeSize(edge);
        if (v != -1 && ap.getSize() == edgeSize) {
            ap.setNode(v);
            ap.setI(-1);
            ap.setSize(0);
        }
    }
    
    void processPhase(int i) {
        ++rem;
        
        int prevNode = -1;
        
        while (rem) {
            Node& node = nodes[ap.getNode()];
            if (ap.getI() == -1) {
                char c = s[i];
                int e = node.getEdgeId(c);
                if (e != -1) {
                    rule3(e, i);
                    break;
                } else {
                    int newE = createNewEdge(i);
                    node.setEdgeId(c, newE);
                    if (ap.getNode() != 0) {
                        followLink(-1, -1);
                    }
                }
            } else {
                char dir = s[ap.getI()];
                int e = node.getEdgeId(dir);
                Edge& edge = edges[e];
                if (s[i] == s[edge.getI1() + ap.getSize()]) {
                    rule3(e, i);
                    break;
                } else {
                    int currNodeId = rule2(i);
                    if (prevNode != -1) {
                        nodes[prevNode].setLinkId(currNodeId);
                    }
                    prevNode = currNodeId;
                    if (ap.getI() == -1) {
                        nodes[currNodeId].setLinkId(ap.getNode());
                    }
                }
            }
            
            --rem;
        }
    }
    
    void processPhases() {
        for (int i = 0; i < n; ++i, ++end) {
            processPhase(i);
        }
    }
    
    int createNewNode() {
        int v = nodes.size();
        nodes.emplace_back();
        return v;
    }
    
    int createNewEdge(int i1, int i2 = -1, int v = -1) {
        int e = edges.size();
        edges.emplace_back(i1, i2, v);
        return e;
    }
    
    int getEdgeSize(int e) const {
        return getEdgeSize(edges[e]);
    }
    
    int getEdgeSize(const Edge& edge) const {
        int l = edge.getI1();
        int r = edge.getI2();
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
    Point ap;
    int rem;
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
    Tree t12("xabxac");
    
    return 0;
}

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

template<typename T>
struct Node {
public:
    Node(): linkId(0), edges() {}
    
    int getEdgeId(T c) const {
        auto iterator = edges.find(c);
        return iterator == edges.end() ? -1 : iterator->second;
    }
    
    int getLinkId() const {
        return linkId;
    }

    void setEdgeId(T c, int e) {
        edges[c] = e;
    }
    
    void setLinkId(int v) {
        linkId = v;
    }
    
    const vector<T> keySet() const {
        vector<T> keys;
        for (auto [key, _]: edges) {
            keys.emplace_back(key);
        }
        sort(begin(keys), end(keys));
        return keys;
    }
    
private:
    int linkId;
    unordered_map<T, int> edges;
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

template<typename T>
string substr(const vector<T>& vec, int i, int len) {
    ostringstream oss;
    int n = vec.size();
    assert(i >= 0);
    assert(i + len <= n);
    for (int j = 0; j < len; ++j) {
        if (j) {
            oss << ", ";
        }
        oss << vec[i + j];
    }
    return oss.str();
}

template<>
string substr(const vector<char>& vec, int i, int len) {
    return {begin(vec) + i, begin(vec) + i + len};
}

template<typename T>
class Tree {
public:
    Tree(vector<T> input, optional<T> terminator = nullopt)
            : vec(input), n(vec.size()), end(-1), ap(), rem(0) {
        if (terminator) {
            vec.emplace_back(*terminator);
            ++n;
        }
    
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
            const vector<T>& keys = nodes[i].keySet();
            for (auto c: keys) {
                if (int e = nodes[i].getEdgeId(c); e != -1) {
                    const Edge& edge = edges[e];
                    int toNode = edge.getToNodeId();
                    string str = substr(vec, edge.getI1(), getEdgeSize(e));
                    edgeStrs.emplace_back(i, e, toNode, str);
                    if (toNode == -1) {
                        ++leaves;
                    }
                }
            }
        }
        // temporary stack to hold all items in preorder walk
        vector<T> buffer;
        // longest common prefix for suffix arrays
        vector<int> lcp;
        collectPathLabels(0, buffer, pathLabels, leafStrs, 0, 0, lcp);
        vector<string> suffixArrays = leafStrs;
        sort(leafStrs.begin(), leafStrs.end(), [&](auto& l, auto& r) {
            return l.size() < r.size();
        });
        
        cout << "The string is: " << substr(vec, 0, n) << endl;
        cout << "The string size is: " << vec.size() << endl;
        cout << "Total number of nodes (no leaves): " << nodes.size() << endl;
        cout << "Total number of edges: " << edges.size() << endl;
        cout << "Total number of leaves: " << leaves << endl;
        cout << "Total number of suffix links: " << links.size() << endl;
        cout << endl;
        cout << "Each edge's information:" << endl;
        for (auto [v1, e, v2, str]: edgeStrs) {
            cout << "Edge id " << e << "\tfrom node " << v1 << "\tto ";
            cout << "node " << v2 << "\tstring is\t" << str << endl;
        }
        cout << endl;
        cout << "Each link's information:" << endl;
        for (int i = 0; i < links.size(); ++i) {
            cout << "Link\t" << links[i].first << "\tTo\t" << links[i].second;
            cout << ", path label from " << pathLabels[links[i].first];
            cout << " to " << pathLabels[links[i].second] << endl;
        }
        cout << endl;
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
        cout << "All suffix arrays and LCP array: " << endl;
        for (int i = 0; i < suffixArrays.size(); ++i) {
            cout << "Index " << i << ": LCP " << lcp[i];
            cout << ": " << suffixArrays[i] << endl;
        }
        cout << endl;
    }
    
    void collectPathLabels(int v,
                           vector<T>& buffer,
                           unordered_map<int, string>& pathLabels,
                           vector<string>& leafStrs,
                           int common,
                           int size,
                           vector<int>& lcp) const {
        if (v == -1) {
            leafStrs.emplace_back(substr(buffer, 0, buffer.size()));
            lcp.emplace_back(common);
            return;
        }
        pathLabels[v] = substr(buffer, 0, buffer.size());
        const vector<T> keys = nodes[v].keySet();
        for (int i = 0; i < keys.size(); ++i) {
            if (int e = nodes[v].getEdgeId(keys[i]); e != -1) {
                int edgeSize = getEdgeSize(e);
                for (int diff = 0; diff < edgeSize; ++diff) {
                    buffer.emplace_back(vec[edges[e].getI1() + diff]);
                }
                collectPathLabels(edges[e].getToNodeId(), buffer, pathLabels,
                                  leafStrs, i ? size : common, size + edgeSize,
                                  lcp);
                for (int diff = 0; diff < edgeSize; ++diff) {
                    buffer.pop_back();
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
            int e = nodes[ap.getNode()].getEdgeId(vec[i]);
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
        int e = nodes[ap.getNode()].getEdgeId(vec[ap.getI()]);
        int newE1 = createNewEdge(edges[e].getI1() + ap.getSize(),
                                  edges[e].getI2(),
                                  edges[e].getToNodeId());
        int newE2 = createNewEdge(i);
        int newV = createNewNode();
        nodes[newV].setEdgeId(vec[edges[newE1].getI1()], newE1);
        nodes[newV].setEdgeId(vec[edges[newE2].getI1()], newE2);
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
            auto& node = nodes[ap.getNode()];
            if (ap.getI() == -1) {
                T c = vec[i];
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
                T dir = vec[ap.getI()];
                int e = node.getEdgeId(dir);
                Edge& edge = edges[e];
                if (vec[i] == vec[edge.getI1() + ap.getSize()]) {
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
        for (int i = 0; i < n; ++i) {
            ++end;
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
    
    vector<T> vec;
    int n;
    int end;
    vector<Node<T>> nodes;
    vector<Edge> edges;
    Point ap;
    int rem;
};

vector<char> vectorize(const string& s) {
    return {begin(s), end(s)};
}

int main(int argc, const char * argv[]) {
    Tree<char> t1(vectorize("banana"), '$');
    Tree<char> t2(vectorize("adeacdade"), '$');
    Tree<char> t3(vectorize("abcabxabcd"), '$');
    Tree<char> t4(vectorize("abcdefabxybcdmnabcdex"), '$');
    Tree<char> t5(vectorize("abcadak"), '$');
    Tree<char> t6(vectorize("abcabxabcd"), '$');
    Tree<char> t7(vectorize("mississippi"), '$');
    Tree<char> t8(vectorize("ooooooooo"), '$');
    Tree<char> t9(vectorize("aa"), '$');
    Tree<char> t10(vectorize("dodo"), '$');
    Tree<char> t11(vectorize("dedododeodo"), '$');
    Tree<char> t12(vectorize("xabxac"), '$');
    Tree<int> t13(vector<int>{1}, -1);
    Tree<int> t14(vector<int>{1, 2, 3, 2, 3, 2}, -1);
    Tree<int> t15(vector<int>{0,1,2,3,4,-1,2,3,4,-2,4,0,1,2,3,-3});
    Tree<char> t16(vectorize("abca#bcad$daca%"));
    
    return 0;
}

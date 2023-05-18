#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <math.h>

using namespace std;

struct Node {
    int depth;
    int nameId;
    map<string, Node> children;
    string val;
};

struct ID3 {
    Node root;
};

struct Result {
    vector<vector<int>> matrix;
};

struct NodeDesc {
    Node node;
    string cond;
};

vector<string> traits;
map<int, set<string>> poss_val;
vector<vector<string>> dataset;
vector<vector<string>> test_dataset;
map<string, int> labels;

void printTree(Node n, vector<NodeDesc> parents) {
    if (n.children.empty()) {
        string l;
        for (int i = 0; i < parents.size(); i++) {
            NodeDesc p = parents[i];
            char *s = static_cast<char *>(::malloc(1024 * sizeof(char)));
            ::snprintf(s, 1024, "%d:%s=%s ", p.node.depth+1, traits[p.node.nameId].c_str(), p.cond.c_str());
            l += std::string(s);
        }
        ::printf("%s%s\n", l.c_str(), n.val.c_str());
    }

    for (auto const &c : n.children) {
        parents.push_back(NodeDesc{n, c.first});
        printTree(c.second, parents);
        parents.pop_back();
    }
}

double entropyf(const map<string, int> &g) {
    if (g.size() == 1) {
        return 0.;
    }
    double en = 0.;
    int sum = 0;
    for (auto const& x : g) {
        sum += x.second;
    }

    for (auto const& x : g) {
        double fr = (double)x.second/(double)sum;
        en -= fr * ::log2(fr);
    }
    return en;
}

double entropyfilter(const map<int, string> &f) {
    map<string, int> res;
    for (auto const& d : dataset) {
        bool found = true;
        for (auto const &e : f) {
            if (d[e.first] != e.second) {
                found = false;
                break;
            }
        }
        if (found) {
            res[d[d.size()-1]]++;
        }
    }

    return entropyf(res);
}

map<int, double> IG(map<int, string> filter, double entropy) {
    map<int, double> res;
    for (int i = 0; i < traits.size(); i++) {
        if (filter.find(i) != filter.end()) {
            continue; // processed already
        }
        double ent = entropy;
        map<string, map<string, int>> goals;
        for (int j = 0; j < dataset.size(); j++) {
            bool cont = true;
            for (auto const& f : filter) {
                if (dataset[j][f.first] != f.second) {
                    cont = false;
                    break;
                }
            }
            if (!cont) {
                continue;
            }
            string tv = dataset[j][i];
            goals[tv][dataset[j][traits.size()]]++;
        }

        int total = 0;
        for (auto const& x : goals) {
            for (auto const& in : x.second) {
                total += in.second;
            }
        }
        for (auto const& x : goals) {
            int in_t = 0;
            for (auto const& in : x.second) {
                in_t += in.second;
            }
            ent -= entropyf(x.second) * (double)in_t/(double)total;
        }

        res[i] = ent;
    }

    return res;
}

Node recurse(double start_entropy, map<int, string> branch, int depth, int maxdepth) {
    if (depth == maxdepth) {
        map<string, int> counts;
        for (auto const& d : dataset) {
            bool found = true;
            for (auto const &e : branch) {
                if (d[e.first] != e.second) {
                    found = false;
                    break;
                }
            }
            if (found) {
                counts[d[d.size()-1]]++;
            }
        }
        if (counts.empty()) {
            for (auto const& d : dataset) {
                counts[d[d.size()-1]]++;
            }
        }
        int max = 0;
        string maxv;
        for (auto const &e : counts) {
            if (e.second > max || (e.second == max && maxv > e.first)) {
                max = e.second;
                maxv = e.first;
            }
        }
        map<string,Node> em;
        return Node{depth, depth, em, maxv};
    }
    if (depth == traits.size() || start_entropy == 0.) {
        string l;
        for (auto const& d : dataset) {
            bool found = true;
            for (auto const &e : branch) {
                if (d[e.first] != e.second) {
                    found = false;
                    break;
                }
            }
            if (found) {
                l = d[d.size()-1];
                break;
            }
        }
        map<string,Node> em;
        return Node{depth, depth, em, l};
    }

    map<int,double> ig = IG(branch, start_entropy);
    double max = -INFINITY;
    int max_idx = -1;
    for (auto const &e : ig) {
        if (e.second > max) {
            max = e.second;
            max_idx = e.first;
        }
        ::printf("%s, %.2f\n", traits[e.first].c_str(), e.second);
    }
    map<string, Node> children;
    for (auto const& s : poss_val[max_idx]) {
        branch[max_idx] = s;
        children[s] = recurse(entropyfilter(branch), branch, depth+1, maxdepth);
    }

    return Node{depth, max_idx, children};
}

ID3 fit(int depth) {
    map<string, double> entropy;
    map<string, int> g;
    for (int i = 0; i < dataset.size(); i++) {
        g[dataset[i][traits.size()]]++;
    }
    double start_entropy = entropyf(g);
    map<int,string> f;
    return ID3{recurse(start_entropy, f, 0, depth)};
}

string most_common() {
    map<string, int> counts;
    for (auto const& d : dataset) {
        counts[d[d.size()-1]]++;
    }
    int max = 0;
    string maxv;
    for (auto const &e : counts) {
        if (e.second > max || (e.second == max && maxv > e.first)) {
            max = e.second;
            maxv = e.first;
        }
    }

    return maxv;
}

string traverse(Node n, vector<string> traits) {
    if (n.children.empty()) {
        return n.val;
    }
    if (n.children.find(traits[n.nameId]) == n.children.end()) {
        return most_common();
    }
    return traverse(n.children[traits[n.nameId]], traits);
}

Result calculate(const ID3& id3) {
    vector<vector<int>> res;
    string ress;
    for (auto const &_ : labels) {
        vector<int> rr;
        for (auto const &_ : labels) {
            rr.push_back(0);
        }
        res.push_back(rr);
    }
    for (auto const &d : test_dataset) {
        string l = traverse(id3.root, d);
        ress += l + " ";
        int exp = labels[d[d.size()-1]];
        int act = labels[l];
        res[exp][act]++;
    }
    cout << ress + "\n";
    return Result{res};
}

int main(int argc, char *argv[]) {
    int depth = -1;
    string learn_path;
    string test_path;

    if(argc < 2){
        cout << "Not enough arguments";
        return 0;
    }
    learn_path = argv[1];
    test_path = argv[2];
    if (argc > 3) {
        depth = ::stoi(argv[3]);
    }

    ifstream infile(learn_path);

    string headers;
    getline(infile,headers);

    if (headers.empty()) {
        cout << "Broken file - no header";
    }

    for (unsigned long idx = headers.find(','); idx > 0 && idx < headers.length(); idx = headers.find(',')) {
        string s = headers.substr(0, idx);
        traits.push_back(s);

        headers = headers.substr(idx + 1);
    }

    string input;
    set<string> lbls;
    while (getline(infile, input)) {
        if (input == "") {
            break;
        }
        vector<string> data;
        int i = 0;
        for (unsigned long idx = input.find(','); idx > 0 && idx < input.length(); idx = input.find(',')) {
            string s = input.substr(0, idx);
            data.push_back(s);
            poss_val[i++].insert(s);
            input = input.substr(idx + 1);
        }
        data.push_back(input);
        lbls.insert(input);
        dataset.push_back(data);
    }

    int i = 0;
    for (const auto & lbl : lbls) {
        labels[lbl] = i++;
    }

    ifstream testfile(test_path);
    getline(testfile, input);

    while (getline(testfile, input)) {
        if (input == "") {
            break;
        }
        vector<string> data;
        for (unsigned long idx = input.find(','); idx > 0 && idx < input.length(); idx = input.find(',')) {
            string s = input.substr(0, idx);
            data.push_back(s);

            input = input.substr(idx + 1);
        }
        data.push_back(input);
        test_dataset.push_back(data);
    }

    ID3 model = fit(depth);
    cout << "[BRANCHES]: \n";
    vector<NodeDesc> v;
    printTree(model.root, v);
    cout << "[PREDICTIONS]: ";
    Result res = calculate(model);
    //acc
    int corr = 0;
    int tot = 0;
    for (int i = 0; i < labels.size(); i++) {
        for (int j = 0; j < labels.size(); j++) {
            if (i == j) {
                corr += res.matrix[i][j];
            }

            tot += res.matrix[i][j];
        }
    }
    ::printf("[ACCURACY]: %.5f\n", (double)corr/tot);
    cout << "[CONFUSION_MATRIX]:\n";
    for (int i = 0; i < labels.size(); i++) {
        for (int j = 0; j < labels.size(); j++) {
            ::printf("%d ", res.matrix[i][j]);
        }
        ::printf("\n");
    }
}

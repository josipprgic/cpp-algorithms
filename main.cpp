#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <numeric>

using namespace std;

struct transition {
    string to;
    float cost;
};

struct state{
    state* parent;
    const string* name;
};

struct solution {
    state* goal;
};

int comparePairs(transition const& left, transition const& right){
    if (left.to == right.to)
        return 0;
    else if (left.to < right.to)
        return -1;
    else
        return 1;
}

tuple<vector<string>, float> reconstruct_path(state* goal);

string initial;
vector<state*> opened;
set<string> closed; // TODO check for smaller
set<string> goals;
map<string, vector<transition>> transitions;


tuple<string, vector<transition>> extract_transition(string line) {
    unsigned long idx = line.find(':');
    string state = line.substr(0, idx);
    vector<transition> ts;
    if (idx == line.length() - 1) {
        return {state, ts};
    }
    line = line.substr(idx + 2);

    for (idx = line.find(' '); idx > 0; idx = line.find(' ')) {
        string trans = line.substr(0, idx);
        unsigned long i = trans.find(',');
        string to = trans.substr(0, i);
        string s = trans.substr(i + 1);
        float cost = std::stof(s);

        transition t = transition{ to, cost };
        ts.push_back(t);
        line = line.substr(idx + 1);
        if (idx > line.length()) {
            break;
        }
    }
    std::sort(ts.begin(), ts.end(), &comparePairs);

    return {state, ts};
}

solution* bfs() {
    if (opened.empty()) {
        return nullptr;
    }

    state* s = opened[0];
    if (goals.contains(*s->name)) {
        auto* p = static_cast<solution *>(::malloc(sizeof(solution)));
        p->goal = s;
        return p;
    }
    opened.erase(opened.begin());
    if (closed.contains(*s->name)) {
        solution* ss = bfs();
        return ss;
    }
    closed.insert(*s->name);
    vector<transition> trans = transitions[*s->name];
    for ([[maybe_unused]] auto & tran : trans) {
        auto* p = static_cast<state *>(::malloc(sizeof(state)));
        p->parent = s;
        auto* sp = static_cast<string *>(::malloc(sizeof(string)));
        *sp = tran.to;
        p->name = sp;
        opened.push_back(p);
    }

    solution* ss = bfs();
    return ss;
}

solution* searchBfs() {
    auto* p = static_cast<state *>(malloc(sizeof(state)));
    p->name = &initial;
    opened.push_back(p);
    solution* s = bfs();
    return s;
}

int main() {
    std::ifstream infile("../resources/test_case_1.txt");

    std::getline(infile,initial);
    while (initial.starts_with('#')) {
        std::getline(infile,initial);
    }
    if (initial.empty()) {
        cout << "Broken file";
    }

    string input;
    while (std::getline(infile, input)) {
        if (input.starts_with('#')) {
            continue;
        }
        for (unsigned long idx = input.find(' '); idx > 0 && idx < input.length();) {
            string state = input.substr(0, idx);

            goals.insert(state);
            input = input.substr(idx + 1);
            idx = input.find(' ');
        }
        goals.insert(input);
        break;
    }

    while (std::getline(infile, input)) {
        if (input.starts_with('#')) {
            continue;
        }
        if (input.empty()) {
            break;
        }
        tuple<string, vector<transition>> t = extract_transition(input);
        transitions[std::get<string>(t)] = std::get<vector<transition>>(t);
    }

    solution* s = searchBfs();

    string found = !s->goal->name->empty() ? "yes" : "no";
    cout<< "[FOUND_SOLUTION]: " + found + "\n";
    if (!s->goal->name->empty()) {
        printf("[STATES_VISITED]: %zu\n", closed.size());
        tuple<vector<string>, float> path = reconstruct_path(s->goal);
        vector<string> p = get<vector<string>>(path);
        string full_path;
        for (int i = 0; i < p.size(); i++) {
            full_path += p[i] + " => ";
            if (i != 0 && i == p.size() - 2) {
                full_path += p[i+1];
                break;
            }
        }
        printf("[PATH_LENGTH]: %zu\n", p.size());
        printf("[TOTAL_COST]: %f\n", get<float>(path));
        cout<<"[PATH]: " + full_path + "\n";
    }

    return 0;
}

tuple<vector<string>, float> reconstruct_path(state* goal) {
    vector<string> res;
    float cost = 0;
    state* cur = goal;
    while (cur != nullptr) {
        if (cur->parent != nullptr) {
            vector<transition> trans = transitions[*cur->parent->name];
            for (auto &t: trans) {
                if (t.to == *cur->name) {
                    cost += t.cost;
                    break;
                }
            }
        }

        res.push_back(*cur->name);
        cur = cur->parent;
    }
    std::reverse(res.begin(), res.end());

    return {res, cost};
}

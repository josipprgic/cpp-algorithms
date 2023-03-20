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
    float cost;
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

vector<string> reconstruct_path(state* goal);

string initial;
vector<state*> opened;
map<string, float> closed; // TODO check for smaller
set<string> goals;
map<string, vector<transition>> transitions;
map<string, float> heuristics;

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
        closed[*s->name] = s->cost;
        return p;
    }
    opened.erase(opened.begin());
    if (closed.contains(*s->name) && closed[*s->name] < s->cost) {
        solution* ss = bfs();
        return ss;
    }
    closed[*s->name] = s->cost;
    vector<transition> trans = transitions[*s->name];
    for ([[maybe_unused]] auto & tran : trans) {
        float acc_cost = s->cost + tran.cost;
        if (closed.contains(tran.to) && closed[tran.to] < acc_cost) {
            continue;
        }

        auto* p = static_cast<state *>(::malloc(sizeof(state)));
        p->parent = s;
        auto* sp = static_cast<string *>(::malloc(sizeof(string)));
        *sp = tran.to;
        p->name = sp;
        p->cost = acc_cost;
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

bool sort_opened(const state * left, const state * right) {
    return right->cost >= left->cost;
}

solution* ucs() {
    if (opened.empty()) {
        return nullptr;
    }

    state* s = opened[0];
    if (goals.contains(*s->name)) {
        auto* p = static_cast<solution *>(::malloc(sizeof(solution)));
        p->goal = s;
        closed[*s->name] = s->cost;
        return p;
    }
    opened.erase(opened.begin());
    if (closed.contains(*s->name) && closed[*s->name] < s->cost) {
        solution* ss = ucs();
        return ss;
    }
    closed[*s->name] = s->cost;
    vector<transition> trans = transitions[*s->name];
    for ([[maybe_unused]] auto & tran : trans) {
        float acc_cost = s->cost + tran.cost;
        if (closed.contains(tran.to) && closed[tran.to] < acc_cost) {
            continue;
        }

        auto* p = static_cast<state *>(::malloc(sizeof(state)));
        p->parent = s;
        auto* sp = static_cast<string *>(::malloc(sizeof(string)));
        *sp = tran.to;
        p->name = sp;
        p->cost = acc_cost;
        opened.push_back(p);
    }

    std::sort(opened.begin(), opened.end(), &sort_opened);
    solution* ss = ucs();
    return ss;
}

solution* searchUcs() {
    auto* p = static_cast<state *>(malloc(sizeof(state)));
    p->name = &initial;
    opened.push_back(p);
    solution* s = ucs();
    return s;
}

bool sort_opened_heur(const state * left, const state * right) {
    return right->cost + heuristics[*right->name] >= left->cost + heuristics[*left->name];
}

solution* astar() {
    if (opened.empty()) {
        return nullptr;
    }

    state* s = opened[0];
    if (goals.contains(*s->name)) {
        auto* p = static_cast<solution *>(::malloc(sizeof(solution)));
        p->goal = s;
        closed[*s->name] = s->cost;
        return p;
    }
    opened.erase(opened.begin());
    if (closed.contains(*s->name) && closed[*s->name] < s->cost) {
        solution* ss = astar();
        return ss;
    }
    closed[*s->name] = s->cost;
    vector<transition> trans = transitions[*s->name];
    for ([[maybe_unused]] auto & tran : trans) {
        float acc_cost = s->cost + tran.cost;
        if (closed.contains(tran.to) && closed[tran.to] < acc_cost) {
            continue;
        }

        auto* p = static_cast<state *>(::malloc(sizeof(state)));
        p->parent = s;
        auto* sp = static_cast<string *>(::malloc(sizeof(string)));
        *sp = tran.to;
        p->name = sp;
        p->cost = acc_cost;
        opened.push_back(p);
    }

    std::sort(opened.begin(), opened.end(), &sort_opened_heur);
    solution* ss = astar();
    return ss;
}

solution* searchAstar() {
    auto* p = static_cast<state *>(malloc(sizeof(state)));
    p->name = &initial;
    opened.push_back(p);
    solution* s = astar();
    return s;
}

int main(int argc, char *argv[]) {
    string alg;
    string states;
    string heuristics_location;
    string check_optimistic;
    string check_consistent;

    if(argc < 2){
        std::cout << "Not enough arguments";
        return 0;
    }
    std::string s1;
    map<string, int> args;
    args["--alg"] = 0;
    args["--ss"] = 1;
    args["--h"] = 2;
    args["--check-optimistic"] = 3;
    args["--check-consistent"] = 4;

    for (int i = 1; i < argc; i++) {
        switch (args[argv[i]]) {
            case 0: {
                alg = argv[++i];
                break;
            }
            case 1: {
                states = argv[++i];
                break;
            }
            case 2: {
                heuristics_location = argv[++i];
                break;
            }
            case 3: {
                check_optimistic = argv[++i];
                break;
            }
            case 4: {
                check_consistent = argv[++i];
                break;
            }
        }
    }

    cout << states;
    std::ifstream infile(states);

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

    if (!heuristics_location.empty()) {
        std::ifstream heur(heuristics_location);
        while (std::getline(heur, input)) {
            if (input.starts_with('#')) {
                continue;
            }
            if (input.empty()) {
                break;
            }
            unsigned long idx = input.find(':');
            string state = input.substr(0, idx);

            float cost = std::stof(input.substr(idx + 1));
            heuristics[state] = cost;
        }
    }

    auto* s = static_cast<solution *>(::malloc(sizeof(solution)));
    if (alg == "bfs") {
        s = searchBfs();
    } else if (alg == "ucs") {
        s = searchUcs();
    } else if (alg == "astar") {
        s = searchAstar();
    }

    string found = !s->goal->name->empty() ? "yes" : "no";
    cout<< "[FOUND_SOLUTION]: " + found + "\n";
    if (!s->goal->name->empty()) {
        printf("[STATES_VISITED]: %zu\n", closed.size());
        vector<string> p = reconstruct_path(s->goal);
        string full_path;
        for (int i = 0; i < p.size(); i++) {
            full_path += p[i] + " => ";
            if (i != 0 && i == p.size() - 2) {
                full_path += p[i+1];
                break;
            }
        }
        printf("[PATH_LENGTH]: %zu\n", p.size());
        printf("[TOTAL_COST]: %f\n", s->goal->cost);
        cout<<"[PATH]: " + full_path + "\n";
    }

    return 0;
}

vector<string> reconstruct_path(state* goal) {
    vector<string> res;
    state* cur = goal;
    while (cur != nullptr) {
        if (cur->parent != nullptr) {
            vector<transition> trans = transitions[*cur->parent->name];
            for (auto &t: trans) {
                if (t.to == *cur->name) {
                    break;
                }
            }
        }

        res.push_back(*cur->name);
        cur = cur->parent;
    }
    std::reverse(res.begin(), res.end());

    return res;
}

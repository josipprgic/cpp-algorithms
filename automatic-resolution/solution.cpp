#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <numeric>
#include <execinfo.h>
#include <unistd.h>

using namespace std;

struct token {
    string value;
    bool negated;
};

struct clause {
    int first{};
    int second{};
    vector<token> tokens;
};

//string initial; TODO del
//map<string, float> closed;
//set<string> goals;
//map<string, float> heuristics;
clause goal;
vector<clause> clauses;
int original_clauses;
set<string> found;

token create_token(const string& s) {
    unsigned long idx = s.find('~', 0);
    if (idx == 0) {
        return token{s.substr(1, s.length()), true};
    }

    return token{s, false};
}

vector<token> factorize(vector<token> toks) {
    for (int i = 0; i < toks.size(); i++) {
        for (int j = 0; j < toks.size(); j++) {
            if (i==j) {
                continue;
            }
            if (toks[i].value == toks[j].value && toks[i].negated == toks[j].negated) {
                toks.erase(toks.begin() + j);
                j -= 1;
            }
        }
    }

    return toks;
}

clause extract_clause(const string& line) {
    unsigned long idx = line.find(' ', 0);
    if (idx > line.length()) {
        token tok = create_token(line);
    }

    string inp = line;
    vector<token> toks;

    while (idx <= inp.length()) {
        string first = inp.substr(0, idx);
        token tok  = create_token(first);
        toks.push_back(tok);
        inp = inp.substr(idx + 3, inp.length());
        idx = inp.find(' ', 0);
    }

    token tok  = create_token(inp);
    toks.push_back(tok);

    return clause{-1, -1, factorize(toks)};
}

vector<clause> negate_clause(const clause& c) {
    vector<clause> css;
    for (auto & t : c.tokens) {
        token tok = token{t.value, !t.negated};
        vector<token> toks;
        toks.push_back(tok);
        css.push_back(clause{c.first, c.second, toks});
    }

    return css;
}

bool is_tautology(const clause& cl) {
    for (const auto & token : cl.tokens) {
        for (const auto & j : cl.tokens) {
            if (j.value == token.value && j.negated == !token.negated) {
                return true;
            }
        }
    }
    return false;
}

bool is_covered(const clause& first, const clause& second) {
    for (const auto & token : first.tokens) {
        bool has_this = false;
        for (const auto & j : second.tokens) {
            if (j.value == token.value && (j.negated && token.negated || !j.negated && !token.negated)) {
                has_this = true;
                break;
            }
        }

        if (!has_this) {
            return false;
        }
    }

    return true;
}

void delete_redundant() {
    set<int> to_del;
    for (int i = 0; i < clauses.size(); i++) {
        for (int j = 0; j < clauses.size(); j++) {
            if (i == j) {
                continue;
            }
            bool is_cov = is_covered(clauses[i], clauses[j]);
            if (is_cov) {
                to_del.insert(j);
            }
        }
    }

    for (int i = 0; i < clauses.size(); i++) {
        if (is_tautology(clauses[i])) {
            to_del.insert(i);
        }
    }

    vector<clause> tmp;
    for (int i = 0; i < clauses.size(); i++) {
        if (to_del.find(i) != to_del.end()) {
            continue;
        }

        tmp.push_back(clauses[i]);
    }

    clauses = tmp;
}

string stringify(token t) {
    string res;
    if (t.negated) {
        res += "~";
    }
    res += t.value;

    return res;
}

string cnf(clause c) {
    string res;

    for (int i = 0; i < c.tokens.size() - 1; i++) {
        res += stringify(c.tokens[i]);
        res += " v ";
    }
    res += stringify(c.tokens[c.tokens.size()-1]);

    return res;
}

vector<clause> resolvec(int f, int s) {
    clause first = clauses[f];
    clause second = clauses[s];
    vector<token> remainder_f;
    vector<token> second_toks = second.tokens;
    bool found_all = false;
    for (int i = 0; i < first.tokens.size(); i++) {
        bool ff = false;
        for (int j = 0; j < second_toks.size(); j++) {
            if (first.tokens[i].value == second_toks[j].value && first.tokens[i].negated != second_toks[j].negated) {
                ff = true;
                second_toks.erase(second_toks.begin() + j);
                found_all = true;
                break;
            }
        }

        if (!ff) {
            remainder_f.push_back(first.tokens[i]);
        }
    }
    vector<clause> res;

    if (found_all) {
        vector<token> res_toks;
        for (auto const & t : remainder_f) {
            res_toks.push_back(t);
        }
        for (auto const & t : second_toks) {
            res_toks.push_back(t);
        }
        if (!res_toks.empty()) {
            clause c = clause{f, s, res_toks};
            if (is_tautology(c)) {
                return res;
            }
            res_toks = factorize(res_toks);
            if (!res_toks.empty()) {
                res.push_back(clause{f, s, res_toks});
            }
        }
    } else {
        res.push_back(clause{f, s, remainder_f});
        res.push_back(clause{f, s, second_toks});
    }

    return res;
}

vector<clause> used;
map<int, int> added;

void print_sol(clause* c, int i, int j) {
    if (used.empty()) {
        for (int k = 0; k < original_clauses; k++) {
            used.push_back(clauses[k]);
            added[k] = k;
        }
    }
    if (i < 0 || j < 0) {
        return;
    }

    clause first = clauses[i];
    clause second = clauses[j];
    print_sol(&first, first.first, first.second);
    print_sol(&second, second.first, second.second);
    if (i >= original_clauses && added.find(i) == added.end()) {
        clause c1 = clauses[i];
        clause s = clause{added[c1.first], added[c1.second], c1.tokens};
        used.push_back(s);
        added[i] = original_clauses++;
    }
    if (j >= original_clauses && added.find(j) == added.end()) {
        clause c1 = clauses[j];
        clause s = clause{added[c1.first], added[c1.second], c1.tokens};
        used.push_back(s);
        added[j] = original_clauses++;
    }
}

void resolve() {
    for (int i = 0; i < clauses.size(); i++) {
        found.insert(cnf(clauses[i]));
    }
        for (int i = original_clauses; i < clauses.size(); i++) {
            int cr = 0;
            int n = clauses.size();
            for (int j = 0; j < n; j++) {
                if (i == j) {
                    continue;
                }

                vector<clause> res = resolvec(i, j);
                if (res.empty()) {
                    int oc = original_clauses;
                    print_sol(nullptr, i, j);
                    vector<token> toks;
                    toks.push_back(token{"NIL", false});
                    clause s = clause{added[i], added[j], toks};
                    used.push_back(s);
                    for (int coun = oc +1; coun < used.size(); coun++) {
                        clause cc = used[coun];
                        ::printf("%d. %s(%d, %d)\n", coun+1, cnf(cc).c_str(), cc.first+1, cc.second+1);
                    }
                    cout<<"===============\n";
                    ::printf("[CONCLUSION]: %s is true\n", cnf(goal).c_str());
                    return;
                }
                for (const auto& re : res) {
                    string ccc = cnf(re);
                    if (found.find(ccc) == found.end()) {
                        clauses.push_back(re);
                        found.insert(ccc);
                        cr++;
                    }
                }
            }
        }
    ::printf("[CONCLUSION]: %s is unknown\n", cnf(goal).c_str());
}

int main(int argc, char *argv[]) {
    string alg;
    string clauses_src;
    string commands;

    if(argc < 2){
        std::cout << "Not enough arguments";
        return 0;
    }

    alg = argv[1];
    clauses_src = argv[2];
    if (argc >= 4) {
        commands = argv[3];
    }

    std::ifstream infile(clauses_src);
    string input;
    while (std::getline(infile, input)) {
        if (input.find('#', 0) == 0) {
            continue;
        }

        clause cl = extract_clause(input);
        clauses.push_back(cl);
    }

    goal = clauses[clauses.size() - 1];
    vector<clause> goals = negate_clause(goal);
    clauses.erase(clauses.end() - 1);
    for (const auto & g : goals) {
        clauses.push_back(g);
    }

    if (!commands.empty()) {
        std::ifstream infile2(commands);
        while (std::getline(infile2, input)) {
            if (input.find('#', 0) == 0) {
                continue;
            }
            if (input.empty()) {
                break;
            }
            cout << input;
        }
    }

    delete_redundant();
    clauses.erase(clauses.end() - goals.size());
    original_clauses = clauses.size();
    for (auto const & c : goals) {
        clauses.push_back(c);
    }

    for (int i = 0; i < clauses.size(); i++) {
        ::printf("%d. %s\n", i+1, cnf(clauses[i]).c_str());
    }
    cout<<"===============\n";

    if (alg == "resolution") {
        resolve();
    }
//
//    if (check_optimistic) {
//        cout<<"# HEURISTIC-OPTIMISTIC " + heuristics_location + "\n";
//        check_optimisticF();
//        return 0;
//    }
//    if (check_consistent) {
//        cout<<"# HEURISTIC-CONSISTENT " + heuristics_location + "\n";
//        check_consistentF();
//        return 0;
//    }
//    auto* s = static_cast<solution *>(::malloc(sizeof(solution)));
//    if (alg == "bfs") {
//        cout<<"# BFS\n";
//        s = searchBfs();
//    } else if (alg == "ucs") {
//        cout<<"# UCS\n";
//        s = searchUcs();
//    } else if (alg == "astar") {
//        cout<<"# A-STAR " + heuristics_location + "\n";
//        s = searchAstar();
//    }
//
//    string found = !s->goal->name->empty() ? "yes" : "no";
//    cout<< "[FOUND_SOLUTION]: " + found + "\n";
//    if (!s->goal->name->empty()) {
//        printf("[STATES_VISITED]: %zu\n", closed.size());
//        vector<string> p = reconstruct_path(s->goal);
//        string full_path;
//        for (int i = 0; i < p.size(); i++) {
//            full_path += p[i] + " => ";
//            if (i != 0 && i == p.size() - 2) {
//                full_path += p[i+1];
//                break;
//            }
//        }
//        printf("[PATH_LENGTH]: %zu\n", p.size());
//        printf("[TOTAL_COST]: %.1f\n", s->goal->cost);
//        cout<<"[PATH]: " + full_path + "\n";
//    }
}
#include <iostream>
#include <fstream>
#include <utility>
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

struct resolution_inp {
    clause goal;
    vector<clause> clauses;
    int original_clauses;
    set<string> found;
};

struct printable_sol {
    int original_clauses;
    vector<clause> used;
    map<int, int> added;
};

token create_token(const string& s) {
    unsigned long idx = s.find('~', 0);
    if (idx == 0) {
        return token{s.substr(1, s.length()), true};
    }

    return token{s, false};
}

vector<token> factorize(vector<token> tokens) {
    vector<token> toks = std::move(tokens);
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
        token tok = create_token(first);
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

vector<clause> delete_redundant(vector<clause> clauses) {
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

    return tmp;
}

string stringify(const token& t) {
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

vector<clause> resolvec(vector<clause> clauses, int f, int s) {
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
            res_toks = factorize(res_toks);
            res.push_back(clause{f, s, res_toks});
        }
    } else {
        res.push_back(clause{f, s, remainder_f});
    }

    return res;
}

printable_sol sol;

void print_sol(vector<clause> clauses, int i, int j) {
    if (sol.used.empty()) {
        for (int k = 0; k < sol.original_clauses; k++) {
            sol.used.push_back(clauses[k]);
            sol.added[k] = k;
        }
    }
    if (i < 0 || j < 0) {
        return;
    }

    clause first = clauses[i];
    clause second = clauses[j];
    print_sol(clauses, first.first, first.second);
    print_sol(clauses, second.first, second.second);
    if (i >= sol.original_clauses && sol.added.find(i) == sol.added.end()) {
        clause c1 = clauses[i];
        clause s = clause{sol.added[c1.first], sol.added[c1.second], c1.tokens};
        sol.used.push_back(s);
        sol.added[i] = sol.original_clauses++;
    }
    if (j >= sol.original_clauses && sol.added.find(j) == sol.added.end()) {
        clause c1 = clauses[j];
        clause s = clause{sol.added[c1.first], sol.added[c1.second], c1.tokens};
        sol.used.push_back(s);
        sol.added[j] = sol.original_clauses++;
    }
}

void resolve(resolution_inp inp) {
    for (const auto & clause : inp.clauses) {
        inp.found.insert(cnf(clause));
    }

    // search from the beginning of support set - starts with the goal and is constantly expanded by new clauses
    for (int i = inp.original_clauses; i < inp.clauses.size(); i++) {
        int cr = 0;
        int n = inp.clauses.size();
        for (int j = 0; j < n; j++) {
            if (i == j) {
                continue;
            }
            vector<clause> res = resolvec(inp.clauses, i, j);

            // FOUND THE SOLUTION
            if (res.empty()) {
                int oc = inp.original_clauses;
                sol = printable_sol();
                sol.original_clauses = inp.original_clauses;
                print_sol(inp.clauses, i, j);

                vector<token> toks;
                toks.push_back(token{"NIL", false});
                clause s = clause{sol.added[i], sol.added[j], toks};
                sol.used.push_back(s);

                // print steps
                for (int coun = oc +1; coun < sol.used.size(); coun++) {
                    clause cc = sol.used[coun];
                    ::printf("%d. %s(%d, %d)\n", coun+1, cnf(cc).c_str(), cc.first+1, cc.second+1);
                }
                cout<<"===============\n";
                ::printf("[CONCLUSION]: %s is true\n", cnf(inp.goal).c_str());
                return;
            }

            // Add to found and to clauses if not already found
            for (const auto& re : res) {
                string ccc = cnf(re);
                if (inp.found.find(ccc) == inp.found.end()) {
                    inp.clauses.push_back(re);
                    inp.found.insert(ccc);
                    cr++;
                }
            }
        }
    }
    ::printf("[CONCLUSION]: %s is unknown\n", cnf(inp.goal).c_str());
}

int main(int argc, char *argv[]) {
    string alg;
    string clauses_src;
    string commands;

    clause goal;
    vector<clause> clauses;
    int original_clauses;

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

    if (alg == "resolution") {
        goal = clauses[clauses.size() - 1];
        vector<clause> goals = negate_clause(goal);
        clauses.erase(clauses.end() - 1);
        original_clauses = clauses.size();
        for (const auto & g : goals) {
            clauses.push_back(g);
        }

        clauses = delete_redundant(clauses);

        for (int i = 0; i < clauses.size(); i++) {
            ::printf("%d. %s\n", i+1, cnf(clauses[i]).c_str());
        }
        cout<<"===============\n";

        if (alg == "resolution") {
            resolve(resolution_inp{goal, clauses, original_clauses});
        }
    } else if (alg == "cooking") {
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
}
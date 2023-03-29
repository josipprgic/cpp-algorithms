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
    vector<token> tokens;
};

//string initial; TODO del
//map<string, float> closed;
//set<string> goals;
//map<string, float> heuristics;
clause goal;
vector<clause> clauses;
vector<clause> support_set;
set<long> conjectured;

token create_token(const string& s) {
    unsigned long idx = s.find('~', 0);
    if (idx == 0) {
        return token{s.substr(1, s.length()), true};
    }

    return token{s, false};
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

    return clause{toks};
}

vector<clause> negate_clause(const clause& c) {
    vector<clause> css;
    for (auto & t : c.tokens) {
        token tok = token{t.value, !t.negated};
        vector<token> toks;
        toks.push_back(tok);
        css.push_back(clause{toks});
    }

    return css;
}

string cnf(clause c) {
    return "";
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

    vector<clause> goals = negate_clause(clauses[clauses.size() - 1]);
    for (const auto & g : goals) {
        clauses.push_back(g);
    }

    for (int i = 0; i < clauses.size(); i++) {
        string s = cnf(clauses[i]);
        ::printf("%d. %s", i, s.c_str());
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

//    simplify_clauses();

    if (alg == "resolution") {

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
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <math.h>

using namespace std;

vector<string> traits;
map<int, set<string>> poss_val;
vector<vector<string>> dataset;
vector<vector<string>> test_dataset;
map<string, int> labels;

int main(int argc, char *argv[]) {
    int generationSize, elitism, maxIteration = 0;
    double mutationProbability, mutationScale = 0;
    string arch, train, test;

    std::string s1;
    map<string, int> args;
    args["--train"] = 0;
    args["--test"] = 1;
    args["--nn"] = 2;
    args["--popsize"] = 3;
    args["--elitism"] = 4;
    args["--p"] = 5;
    args["--K"] = 6;
    args["--iter"] = 7;
    for (int i = 1; i < argc; i++) {
        switch (args[argv[i]]) {
            case 0: {
                train = argv[++i];
                break;
            }
            case 1: {
                test = argv[++i];
                break;
            }
            case 2: {
                arch = argv[++i];
                break;
            }
            case 3: {
                generationSize = ::atoi(argv[++i]);
                break;
            }
            case 4: {
                elitism = ::atoi(argv[++i]);
                break;
            }
            case 5: {
                mutationProbability = ::atof(argv[++i]);
                break;
            }
            case 6: {
                mutationScale = ::atof(argv[++i]);
                break;
            }
            case 7: {
                maxIteration = ::atoi(argv[++i]);
                break;
            }
        }
    }

    ifstream infile(train);

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

    ifstream testfile(test);
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
}

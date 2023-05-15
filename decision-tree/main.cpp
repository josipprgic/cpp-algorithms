#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>

using namespace std;

vector<string> traits;
string label;
vector<vector<string>> dataset;
vector<vector<string>> test_dataset;

int main(int argc, char *argv[]) {
    int depth = 0;
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
    while (getline(infile, input)) {
        vector<string> data;
        for (unsigned long idx = input.find(','); idx > 0 && idx < input.length(); idx = input.find(',')) {
            string s = input.substr(0, idx);
            data.push_back(s);

            input = input.substr(idx + 1);
        }
        data.push_back(input);
        dataset.push_back(data);
    }

    ifstream testfile(test_path);
    getline(infile, input);

    while (getline(testfile, input)) {
        vector<string> data;
        for (unsigned long idx = input.find(','); idx > 0 && idx < input.length(); idx = input.find(',')) {
            string s = input.substr(0, idx);
            data.push_back(s);

            input = input.substr(idx + 1);
        }
        data.push_back(input);
        test_dataset.push_back(data);
    }

    cout << dataset.size();
    cout << test_dataset.size();

}

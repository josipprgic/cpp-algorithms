#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <math.h>
#include <random>

using namespace std;

std::default_random_engine generator;
std::normal_distribution<double> distribution(0.0,0.1);
unsigned long nn_count = 0;

struct nn {
    vector<vector<vector<double>>> nn;
    unsigned long id;
    double fitness = 0;
};

vector<vector<double>> train_set;
vector<vector<double>> test_set;

double activation(double x) {
    return 1 / (1 + std::exp(-x));
}

nn create_nn(vector<int>& arch);
void extract_datasets(const string& train, const string& test);
vector<int> extract_architecture(string arch);
double calc(nn& net, vector<double>& vals);
double mean_sqr_err(nn& net);
nn fit(vector<int> arch, int gen_size, int max_iter, int elitism, double mut_prob, double mut_scale);

int main(int argc, char *argv[]) {

    int gen_size, elitism, max_iter = 0;
    double mut_prob, mut_scale = 0;
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
                gen_size = ::atoi(argv[++i]);
                break;
            }
            case 4: {
                elitism = ::atoi(argv[++i]);
                break;
            }
            case 5: {
                mut_prob = ::atof(argv[++i]);
                break;
            }
            case 6: {
                mut_scale = ::atof(argv[++i]);
                break;
            }
            case 7: {
                max_iter = ::atoi(argv[++i]);
                break;
            }
        }
    }
    std::srand( std::time(nullptr) );
    extract_datasets(train, test);
    vector<int> architecture = extract_architecture(arch);

    nn best = fit(architecture, gen_size, max_iter, elitism, mut_prob, mut_scale);
}

bool sort_fitness(const nn& left, const nn& right) {
    return left.fitness > right.fitness;
}

int select_one(vector<nn>& nets, double total_fit) {
    double random = ((double)std::rand()/ RAND_MAX) * total_fit;
    double counter = 0.;
    for (int i = nets.size() -1; i > 0; i--) {
        counter += nets[i].fitness;
        if (counter > random) {
            return i - 1;
        }
    }

    return 0;
}

nn breed(vector<nn>& nets, int idf, int ids) {
    vector<vector<vector<double>>> res(nets[idf].nn.size());
    for (int i = 0; i < nets[idf].nn.size(); i++) {
        vector<vector<double>> layer(nets[idf].nn[i].size());
        for (int j = 0, jn = nets[idf].nn[i].size(); j < jn; j++) {
            vector<double> neuron(nets[idf].nn[i][j].size());
            for (int k = 0, ns = nets[idf].nn[i][j].size(); k < ns; k++) {
                neuron[k] = (nets[idf].nn[i][j][k] + nets[ids].nn[i][j][k]) / 2.;
            }
            layer[j] = neuron;
        }
        res[i] = layer;
    }

    return nn{res, nn_count++};
}

void mutate(nn& n, double mut_prob, double mut_scale) {
    for (int i = 0; i < n.nn.size(); i++) {
        for (int j = 0, jn = n.nn[i].size(); j < jn; j++) {
            for (int k = 0, ns = n.nn[i][j].size(); k < ns; k++) {
                if ((double)std::rand()/ RAND_MAX < mut_prob) {
                    n.nn[i][j][k] += distribution(generator) * mut_scale;
                }
            }
        }
    }
}

nn fit(vector<int> arch, int gen_size, int max_iter, int elitism, double mut_prob, double mut_scale) {
    vector<nn> networks (gen_size);
    for (int i = 0; i < gen_size; i++) {
        nn net = create_nn(arch);
        networks[i] = net;
    }

    for (int i = 1; i <= max_iter; i++) {

        vector<nn> next_gen(gen_size);
        double total_fit = 0;
        for (auto & net : networks) {
//            if (net.fitness != 0.0) {
//                total_fit += net.fitness;
//                continue;
//            }
            net.fitness = 1. / mean_sqr_err(net);
            total_fit += net.fitness;
        }
        std::sort(networks.begin(), networks.end(), &sort_fitness);
        if (i % 2000 == 0) {
            ::printf("[Train error @%d]: %10.6f\n", i, 1./networks[0].fitness);
        }
        for (int j = 0; j < elitism; j++) {
            next_gen[j] = networks[j];
        }

        for (int j = 0, ln = gen_size - elitism; j < ln; j++) {
            int idf = select_one(networks, total_fit);
            int ids = select_one(networks, total_fit);
            while (idf == ids) {
                ids = select_one(networks, total_fit);
            }

//            ::printf("%d %d \n", idf, ids);
            nn child = breed(networks, idf, ids);
//            double f = 1. / mean_sqr_err(child);
            mutate(child, mut_prob, mut_scale);
//            double s = 1. / mean_sqr_err(child);
//            if (s < networks[0].fitness) {
//                ::printf("KRIVOO %d %d\n", idf, ids);
//            } else {
//                ::printf("DOBROO %d %d\n", idf, ids);
//            }
            next_gen[elitism + j] = child;
        }

        networks = next_gen;
    }

    return networks[0];
}

double calc(nn& net, vector<double>& vals) {
    vector<double> prev = vals;
    for (int i = 0; i < net.nn.size(); i++) {
        vector<double> l_out;
        for (int j = 0; j < net.nn[i].size(); j++) {
            double out = 0;
            for (int k = 0; k < prev.size(); k++) {
                out += net.nn[i][j][k] * prev[k];
            }
            out += net.nn[i][j][prev.size()];
            if (i == net.nn.size()-1) {
                return out;
            }
            l_out.push_back(activation(out));
        }
        prev = l_out;
    }
    return 0.0;
}

double mean_sqr_err(nn& net) {
    double sse = 0;
    for (auto & l : train_set) {
        double expected = l[l.size()-1];

        double predicted = 0.0;
        vector<double> prev = l;
        for (int i = 0, ns = net.nn.size(); i < ns; i++) {
            vector<vector<double>> layer = net.nn[i];
            int ls = layer.size();
            vector<double> l_out(ls);
            for (int j = 0; j < ls; j++) {
                vector<double> neuron = layer[j];
                double out = 0;
                int ps = neuron.size() -1;
                for (int k = 0; k < ps; k++) {
                    out += neuron[k] * prev[k];
                }
                out += neuron[ps];
                if (i == net.nn.size()-1) {
                    predicted = out;
                    break;
                }
                l_out[j] = activation(out);
            }
            prev = l_out;
        }
        double diff = predicted - expected;
        sse += diff * diff;
    }

//    ::printf("ERROR %f\n", sse / train_set.size());
    return sse / train_set.size();
}

nn create_nn(vector<int>& arch) {
    vector<vector<vector<double>>> nn_v(arch.size()+1);
    int prev_size = train_set[0].size()-1;
    for (int i = 0; i < arch.size(); i++) {
        vector<vector<double>> layer(arch[i]);
        for (int j = 0; j < arch[i]; j++) {
            vector<double> neuron(prev_size + 1);
            for (int k = 0; k <= prev_size; k++) {
                neuron[k] = distribution(generator);
            }
            layer[j] = neuron;
        }
        nn_v[i] = layer;
        prev_size = arch[i];
    }

    // OUTPUT layer
    vector<double> neuron(prev_size + 1);
    for (int k = 0; k <= prev_size; k++) {
        neuron[k] = distribution(generator);
    }
    vector<vector<double>> layer(1);
    layer[0] = neuron;
    nn_v[arch.size()] = layer;

    return nn{nn_v, nn_count++};
}


vector<int> extract_architecture(string arch) {
    vector<int> res;
    for (unsigned long idx = arch.find('s'); idx > 0 && idx <= arch.length(); idx = arch.find('s')) {
        string s = arch.substr(0, idx);
        res.push_back(::atoi(s.c_str()));
        arch = arch.substr(idx + 1);
    }

    return res;
}

void extract_datasets(const string& train, const string& test) {
    ifstream infile(train);

    string headers;
    getline(infile,headers);

    if (headers.empty()) {
        cout << "Broken file - no header";
    }

    string input;
    while (getline(infile, input)) {
        if (input.empty()) {
            break;
        }
        vector<double> data;
        for (unsigned long idx = input.find(','); idx > 0 && idx < input.length(); idx = input.find(',')) {
            string s = input.substr(0, idx);
            data.push_back(::atof(s.c_str()));
            input = input.substr(idx + 1);
        }
        data.push_back(::atof(input.c_str()));
        train_set.push_back(data);
    }

    ifstream testfile(test);
    getline(testfile,headers);
    if (headers.empty()) {
        cout << "Broken file - no header";
    }
    while (getline(testfile, input)) {
        if (input.empty()) {
            break;
        }
        vector<double> data;
        for (unsigned long idx = input.find(','); idx > 0 && idx < input.length(); idx = input.find(',')) {
            string s = input.substr(0, idx);
            data.push_back(::atof(s.c_str()));
            input = input.substr(idx + 1);
        }
        data.push_back(::atof(input.c_str()));
        test_set.push_back(data);
    }
}
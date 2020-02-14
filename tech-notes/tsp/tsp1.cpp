#include "cities.h"

#include <fstream>
#include <cmath>
#include <limits>

using namespace std;

/*************************
 * O(n!) with starting position fixed and distance matrix
 * g++ -o main tsp1.cpp -std=gnu++2a -O3
 *************************/

/******* Performance (O3) ******
$ time ./main out10 
shortest dist is 287.077.
permutations 362880.

real	0m0.018s
user	0m0.014s
sys	0m0.002s

$ time ./main out11
shortest dist is 284.96.
permutations 3628800.

real	0m0.113s
user	0m0.110s
sys	0m0.002s

$ time ./main out12
shortest dist is 329.309.
permutations 39916800.

real	0m1.104s
user	0m1.099s
sys	0m0.003s

$ time ./main out13
shortest dist is 400.13.
permutations 479001600.

real	0m13.557s
user	0m13.509s
sys	0m0.025s
 */

class TSPMatrixAndFixedStart {
  private:
    double getDistance(const City& one, const City& two) {
        return distances[one.id * seq.size() + two.id];
    }

    void swap(int i, int j) {
        City temp = seq[j];
        seq[j] = seq[i];
        seq[i] = temp;
    }

    void traverse(int length) {
        if (!length) {
            double totalLength = getDistance(seq[seq.size() - 1], seq[0]);
            // cout << "visit " << seq[seq.size() - 1].id << "\n";
            for (int i = 0; i < seq.size() - 1; ++i) {
                totalLength += getDistance(seq[i], seq[i + 1]);
                // cout << "visit " << seq[i].id << "\n";
            }
            // cout << "\n";
            if (totalLength < shortest) {
                shortest = totalLength;
            }
            permutations += 1;
        } else {
            for (int i = seq.size() - length; i < seq.size(); ++i) {
                swap(i, seq.size() - length);
                traverse(length - 1);
                swap(seq.size() - length, i);
            }
        }
    }

    void loadDistanceMatrix() {
        for (int i = 0; i < seq.size(); ++i) {
            for (int j = i; j < seq.size(); ++j) {
                const City& one = seq[i];
                const City& two = seq[j];
                double val = std::sqrt((one.x - two.x) * (one.x - two.x) + (one.y - two.y) * (one.y - two.y));
                distances[j * seq.size() + i] = val;
                distances[i * seq.size() + j] = val;
            }
        }
    }

    double shortest = std::numeric_limits<double>::max();
    long permutations = 0;
    vector<City> seq;
    double* distances = nullptr;

  public:
    TSPMatrixAndFixedStart(const vector<City>& in) : seq(in) {
        distances = new double[seq.size() * seq.size()];
        loadDistanceMatrix();
    }

    ~TSPMatrixAndFixedStart() { delete[] distances; }

    TSPMatrixAndFixedStart(const TSPMatrixAndFixedStart&) = delete;
    TSPMatrixAndFixedStart& operator=(const TSPMatrixAndFixedStart&) = delete;

    TSPMatrixAndFixedStart(TSPMatrixAndFixedStart&&) = delete;
    TSPMatrixAndFixedStart& operator=(TSPMatrixAndFixedStart&&) = delete;

    double tsp() {
        traverse(seq.size() - 1);
        return shortest;
    }

    long getPermutations() const { return permutations; }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }
    ifstream infile(argv[1], ios_base::in);
    vector<City> cities = readCities(infile);
    infile.close();
    
    TSPMatrixAndFixedStart naive(cities);
    double shortest = naive.tsp();
    cout << "shortest dist is " << shortest << ".\n";
    cout << "permutations " << naive.getPermutations() << ".\n";

    return 0;
}


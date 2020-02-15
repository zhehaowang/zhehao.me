#include "cities.h"

#include <fstream>
#include <cmath>
#include <limits>

using namespace std;

/*************************
 * O(n!) with starting position fixed, distance matrix, early termination pruning
 * g++ -o main tsp2.cpp -std=gnu++2a -O3
 *************************/

/******* Performance (O3) ******
$ time ./main out10 
shortest dist is 287.077.
permutations 458.

real	0m0.004s
user	0m0.002s
sys	0m0.001s

$ time ./main out11
shortest dist is 284.96.
permutations 979.

real	0m0.010s
user	0m0.007s
sys	0m0.002s

$ time ./main out12
shortest dist is 329.309.
permutations 1061.

real	0m0.039s
user	0m0.035s
sys	0m0.002s

$ time ./main out13 (I overwrote the input after this one :( )
shortest dist is 400.13.
permutations 3026.

real	0m0.283s
user	0m0.279s
sys	0m0.002s

...

$ time ./main out16
shortest dist is 352.308.
permutations 24094.

real	0m8.729s
user	0m8.673s
sys	0m0.027s
 */

class TSPEarlyTerminationPruning {
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
            for (int i = 0; i < seq.size() - 1; ++i) {
                totalLength += getDistance(seq[i], seq[i + 1]);
            }
            if (totalLength < shortest) {
                shortest = totalLength;
            }
            permutations += 1;
        } else {
            for (int i = seq.size() - length; i < seq.size(); ++i) {
                currentTotalDistance += getDistance(seq[i], seq[seq.size() - length - 1]);
                if (currentTotalDistance < shortest) {
                    swap(i, seq.size() - length);
                    traverse(length - 1);
                    swap(seq.size() - length, i);
                }
                currentTotalDistance -= getDistance(seq[i], seq[seq.size() - length - 1]);
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
    double currentTotalDistance = 0;

  public:
    TSPEarlyTerminationPruning(const vector<City>& in) : seq(in) {
        distances = new double[seq.size() * seq.size()];
        loadDistanceMatrix();
    }

    ~TSPEarlyTerminationPruning() { delete[] distances; }

    TSPEarlyTerminationPruning(const TSPEarlyTerminationPruning&) = delete;
    TSPEarlyTerminationPruning& operator=(const TSPEarlyTerminationPruning&) = delete;

    TSPEarlyTerminationPruning(TSPEarlyTerminationPruning&&) = delete;
    TSPEarlyTerminationPruning& operator=(TSPEarlyTerminationPruning&&) = delete;

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
    
    TSPEarlyTerminationPruning naive(cities);
    double shortest = naive.tsp();
    cout << "shortest dist is " << shortest << ".\n";
    cout << "permutations " << naive.getPermutations() << ".\n";

    return 0;
}


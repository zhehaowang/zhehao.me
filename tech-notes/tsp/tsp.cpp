#include "cities.h"

#include <fstream>
#include <cmath>
#include <limits>

using namespace std;

/*************************
 * naive O(n!)
 * g++ -o main tsp.cpp -std=gnu++2a
 *************************/

/******* Performance (O0) ******
$ time ./main out10
shortest dist is 287.077.
permutations 3628800.

real	0m1.005s
user	0m0.999s
sys	0m0.003s

$ time ./main out11
shortest dist is 284.96.
permutations 39916800.

real	0m11.604s
user	0m11.555s
sys	0m0.022s

$ time ./main out12
shortest dist is 329.309.
permutations 479001600.

real	2m24.765s
user	2m24.277s
sys	0m0.210s
 */

/******* Performance (O3) ******
 $ time ./main out10
shortest dist is 287.077.
permutations 3628800.

real	0m0.134s
user	0m0.127s
sys	0m0.002s
 
shortest dist is 284.96.
permutations 39916800.

real	0m1.389s
user	0m1.378s
sys	0m0.004s
 
$ time ./main out12
shortest dist is 329.309.
permutations 479001600.

real	0m17.199s
user	0m17.139s
sys	0m0.026s
 */

class TSPNaive {
  private:
    double getDistance(const City& one, const City& two) {
        return std::sqrt((one.x - two.x) * (one.x - two.x) + (one.y - two.y) * (one.y - two.y));
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

    double shortest = std::numeric_limits<double>::max();
    long permutations = 0;
    vector<City> seq;

  public:
    TSPNaive(const vector<City>& in) : seq(in) {}

    double tsp() {
        traverse(seq.size());
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
    
    TSPNaive naive(cities);
    double shortest = naive.tsp();
    cout << "shortest dist is " << shortest << ".\n";
    cout << "permutations " << naive.getPermutations() << ".\n";

    return 0;
}


#include "cities.h"

#include <fstream>
#include <cmath>
#include <limits>

using namespace std;

/*************************
 * O(n!) with starting position fixed, distance matrix, early termination pruning and MST heuristic
 * g++ -o main tsp3.cpp -std=gnu++2a -O3
 *************************/

/******* Performance (O3) ******

 */

class TSPMSTHeuristic {
  private:
    // get the MST total distance for nodes [seq.size() - length, seq.size()]
    // Prim, greedy algorithm
    double getMST(int length) const {
        int startIdx = seq.size() - length;
        vector<pair<double, bool>> dists(length, make_pair(std::numeric_limits<double>::max(), false));

        double totalMSTLength = 0;
        int visitedCnt = 1;
        dists[0] = {0, true};
        while (visitedCnt < length) {
            for (int i = 0; i < length; ++i) {
                if (dists[i].second) {
                    continue;
                }
                
            }
        }


        return totalMSTLength;
    }

    double getDistance(const City& one, const City& two) const {
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
                swap(i, seq.size() - length);
                if (currentTotalDistance + getMST(length) < shortest) {
                    traverse(length - 1);
                }
                swap(seq.size() - length, i);
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
    TSPMSTHeuristic(const vector<City>& in) : seq(in) {
        distances = new double[seq.size() * seq.size()];
        loadDistanceMatrix();
    }

    ~TSPMSTHeuristic() { delete[] distances; }

    TSPMSTHeuristic(const TSPMSTHeuristic&) = delete;
    TSPMSTHeuristic& operator=(const TSPMSTHeuristic&) = delete;

    TSPMSTHeuristic(TSPMSTHeuristic&&) = delete;
    TSPMSTHeuristic& operator=(TSPMSTHeuristic&&) = delete;

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
    
    TSPMSTHeuristic naive(cities);
    double shortest = naive.tsp();
    cout << "shortest dist is " << shortest << ".\n";
    cout << "permutations " << naive.getPermutations() << ".\n";

    return 0;
}


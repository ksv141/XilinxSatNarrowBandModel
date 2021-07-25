#ifndef MLS_H
#define MLS_H

#include <vector>
#include <deque>
#include <cmath>
#include <ctime>
#include <unordered_map>
#include <cstdlib>
#include <random>
#include <limits>
#include <stdint.h>

using namespace std;

// генератор M-последовательностей
class mls
{
private:
    unordered_map<int, vector<int>> Taps;
    vector<int> taps;
    deque<bool> state;
    int length;
    int nbits;
    mt19937 gen;
    uniform_int_distribution<> d;
    int symbAlphSize;

    // смещение начального seed для ГСЧ.
    // Используется при одновременном создании нескольких генераторов
    static unsigned int seedShift;

public:
    mls(int nbits = 32, int m = 2);
    void setBits(int n_bits);
    int size();
    int nextSample();
    bool eof();
    int nextSymbol();
};

#endif // MLS_H

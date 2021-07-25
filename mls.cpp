#include "mls.h"

unsigned int mls::seedShift = 0;

// n_bits detemines the length of each MLS sequence (n_bits=10 produces sequences of 1023 bits)
mls::mls(int n_bits, int m)
{
    symbAlphSize = m;
    setBits(n_bits);
    taps = Taps[nbits];
    state.resize(nbits,false);
	length = (1 << nbits) - 1; //pow(2,nbits) - 1;

    seedShift += 1000;
    gen.seed((unsigned long)time(0) + seedShift);
    d.param(uniform_int_distribution<>::param_type(0, numeric_limits<int>::max()));


    uint32_t randomNum;
    randomNum = d(gen);                      //use device entropy to genrate random number
    for (int i = 0;i< nbits;i++)
        state[i] = (randomNum >> i) & 1;

}

void mls::setBits(int n_bits)
{
    if(n_bits < 2)
        nbits = 2;
    else if(n_bits > 32)
        nbits = 32;
    else
        nbits = n_bits;

    //define tap points
    Taps[2] = {1};
    Taps[3] = {2};
    Taps[4] = {3};
    Taps[5] = {3};
    Taps[6] = {5};
    Taps[7] = {6};
    Taps[8] = {7, 6, 1};
    Taps[9] = {5};
    Taps[10] = {7};
    Taps[11] = {9};
    Taps[12] = {11, 10, 4};
    Taps[13] = {12, 11, 8};
    Taps[14] = {13, 12, 2};
    Taps[15] = {14};
    Taps[16] = {15, 13, 4};
    Taps[17] = {14};
    Taps[18] = {11};
    Taps[19] = {18, 17, 14};
    Taps[20] = {17};
    Taps[21] = {19};
    Taps[22] = {21};
    Taps[23] = {18};
    Taps[24] = {23, 22, 17};
    Taps[25] = {22};
    Taps[26] = {25, 24, 20};
    Taps[27] = {26, 25, 22};
    Taps[28] = {25};
    Taps[29] = {27};
    Taps[30] = {29, 28, 7};
    Taps[31] = {28};
    Taps[32] = {31, 30, 10};
}

int mls::size()
{
    return length;
}

int mls::nextSample()
{
    bool feedback;

    feedback = state.back();
	for (int ti = 0; ti < taps.size(); ti++)
	{
		int t = taps[ti];
		int nt = t % nbits;
		feedback ^= state[taps[ti] % nbits];
	}
	state.pop_back();
	state.push_front(feedback);

    return static_cast<int>(feedback);
}

bool mls::eof()
{
    return false;
}

int mls::nextSymbol()
{
    int out = 0;
    if (symbAlphSize == 2)
        out = nextSample();
    else if (symbAlphSize == 4) {
        int x0 = nextSample();
        int x1 = nextSample();
        out = x0 + (x1 << 1);
    }
    else if (symbAlphSize == 8) {
        int x0 = nextSample();
        int x1 = nextSample();
        int x2 = nextSample();
        out = x0 + (x1 << 1) + (x2 << 2);
    }
    else if (symbAlphSize == 16) {
        int x0 = nextSample();
        int x1 = nextSample();
        int x2 = nextSample();
        int x3 = nextSample();
        out = x0 + (x1 << 1) + (x2 << 2) + (x3 << 3);
    }
    return out;
}

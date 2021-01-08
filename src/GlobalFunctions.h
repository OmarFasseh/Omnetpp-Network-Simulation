#include <bitset>
#include "MyMessage_m.h"
#include "Node.h"
#include <omnetpp.h>

using namespace std;
string BitsToString(vector<bool> &receiverBits);
void StringToBits(string payload, vector<bool> &payloadBits);
vector<bool> removePadding(string payload, int &charCount, int paddingSize);
vector<bool> checkHamming(vector<bool> &ham, int charCount);
string unHam(string payload, int paddingSize);
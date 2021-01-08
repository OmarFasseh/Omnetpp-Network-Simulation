/*
 * GlobalFunctions.cc
 *
 *  Created on: Dec 30, 2020
 *      Author: Omar
 */
#include "GlobalFunctions.h"
using namespace omnetpp;

string BitsToString(vector<bool> &receiverBits)
{
    char character = 0;
    int j = 0;
    string data = "";
    //General functions
    for (int i = 0; i < receiverBits.size(); i++)
    {
        if (j == 8)
        {
            data += character;
            character = receiverBits[i];
            j = 0;
        }
        else
        {
            character = character << 1;
            character |= receiverBits[i];
        }
        j++;
    }
    if (j == 8)
    {
        data += character;
    }
    return data;
}

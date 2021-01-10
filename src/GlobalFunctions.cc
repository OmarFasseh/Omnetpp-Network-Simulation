/*
 * GlobalFunctions.cc
 *
 *  Created on: Dec 30, 2020
 *      Author: Omar
 */
#include "GlobalFunctions.h"
using namespace omnetpp;
#include <bits/stdc++.h> /* reverse*/

string BitsToStringDecode(vector<bool> &receiverBits)
{
    char character = 0;
    int j = 0;
    string data = "";
    for (int i = receiverBits.size() - 1; i >= 0; i--)
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
    reverse(data.begin(), data.end());
    return data;
}
void StringToBits(string payload, vector<bool> &payloadBits)
{
    int k = 0;
    for (int i = 0; i < payload.size(); i++)
    {
        bitset<8> newChar(payload[i]);
        for (int z = 0; z < 8; z++)
        {
            if (newChar[z])
            {
                payloadBits[k] = true;
            }
            k++;
        }
    }
}

vector<bool> removePadding(string payload, int payloadSize, int &charCount, int paddingSize)
{
    // EV << "STR ReC : " << payload << endl;
    charCount = (int)(payload[0]);
    int m = payloadSize - 1;
    vector<bool> payloadBits(m * 8, false);
    int k = m * 8 - 1;
    for (int i = payloadSize - 1; i > 0; i--)
    {
        bitset<8> newChar(payload[i]);
        for (int z = 0; z < 8; z++)
        {
            if (newChar[z])
            {
                payloadBits[k] = true;
            }
            k--;
        }
    }

    //for (int i = 0; i < payloadBits.size(); i++)
    //    EV << " " << payloadBits[i];
    int payloadSize1 = m * 8 - paddingSize;
    vector<bool> messageWithHamming(payloadSize1);

    for (int i = 0; i < payloadSize1; i++)
    {
        messageWithHamming[i] = payloadBits[i + paddingSize];
    }
    return messageWithHamming;
}
vector<bool> checkHamming(vector<bool> &ham, int charCount)
{
    int r = 0;
    charCount--;
    while ((charCount * 8) + r + 1 > pow(2, r))
    {
        r++;
    }

    // EV << endl
    //    << "Message received with hamming: ";
    // for (int i = 0; i < ham.size(); i++)
    // {
    //     EV << ham[i] << " ";
    // }
    // EV << endl;

    int j = 0;
    vector<bool> errorBits(r, 0);
    int errorPos = 0;

    for (int i = 0; i < r; i++)
    {
        int step = pow(2, i);
        for (j = step - 1; j < charCount * 8 + r; j += (2 * step))
        {
            for (int z = 0; z < step; z++)
            {
                if (ham[step - 1] == ham[j + z] && j + z != step - 1)
                {
                    ham[step - 1] = false;
                }
                else if (j + z != step - 1)
                {
                    ham[step - 1] = true;
                }
            }
        }
        errorBits[i] = ham[step - 1];
    }
    for (int i = r; i >= 0; i--)
    {
        errorPos = errorPos << 1;
        errorPos |= errorBits[i];
    }
     if (errorPos != 0)
     {
    // EV << "Error at " << errorPos - 1 << endl;
     ham[errorPos - 1] = !ham[errorPos - 1];
    // EV << "Message after correction : " << endl;
    // for (int i = 0; i < ham.size(); i++)
    // {
    //     EV << ham[i] << " ";
     }
    // EV << endl;
    // }
    // else
    // {
    // EV << "No error found." << endl;
    // }
    j = 0;
    vector<bool> payloadBits(charCount * 8, false);
    for (int i = 1; i < charCount * 8 + r; i++)
    {
        if ((int)log2(i) != log2(i))
        {
            payloadBits[j++] = ham[i - 1];
        }
    }
    /*
    EV << "Payload after correction: ";
    for (int i = 0; i < payloadBits.size(); i++)
    {
        EV << payloadBits[i] << " ";
    }
    EV << endl;
    */
    return payloadBits;
}
string unHam(const char *payload, int payloadSize, int paddingSize)
{
    int charCount;
    vector<bool> ham = removePadding(payload, payloadSize, charCount, paddingSize);
    //vector<bool> payloadBits = checkHamming(ham, charCount);
    int r = 0;
    charCount--;
    while ((charCount * 8) + r + 1 > pow(2, r))
    {
        r++;
    }
    vector<bool> payloadBits(charCount * 8, false);
    int j = 0;
    for (int i = 1; i < charCount * 8 + r; i++)
    {
        if ((int)log2(i) != log2(i))
        {
            payloadBits[j++] = ham[i - 1];
        }
    }
    string recMsg = BitsToStringDecode(payloadBits);

    return recMsg;
}

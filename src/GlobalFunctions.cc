/*
 * GlobalFunctions.cc
 *
 *  Created on: Dec 30, 2020
 *      Author: Omar
 */
#include "GlobalFunctions.h"
using namespace omnetpp;
void csend(MyMessage * msg, int dest, int source)
{
    std:: string s ="hi there!";
    std::bitset<8> p{0};
    for(int i=0;i<s.size();i++)
    {
        std::bitset<8> y(s[i]);
        p=p^y;
    }
    //msg->setMycheckbits(p);
    int rand,rand2;
    //int rand=uniform(0,1)*10;
    if(rand>6)
    {
        //int rand2=uniform(0,1)*s.size();
        s[rand2]=s[rand2]+1;
    }
    msg->setM_Payload(s.c_str());
    //msg->setM_Type(0);
    msg->setSeq_Num(0);
    msg->setReciver(dest);
    msg->setSender(source);
    //send(msg,"out");
}





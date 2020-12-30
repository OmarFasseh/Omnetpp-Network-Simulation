//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Node.h"
Define_Module(Node);

void Node::initialize()
{
    double interval = exponential(1 / par("lambda").doubleValue());
    scheduleAt(simTime() + interval, new cMessage(""));
}

void Node::cSend(MyMessage * msg, int dest, int source)
{
    std:: string s ="hi there!";
    std::bitset<8> p{0};
    for(int i=0;i<s.size();i++)
    {
        std::bitset<8> y(s[i]);
        p=p^y;
    }
    msg->setMycheckbits(p);
    int rand=uniform(0,1)*10;
    if(rand>6)
    {
        int rand2=uniform(0,1)*s.size();
        s[rand2]=s[rand2]+1;
    }
    msg->setM_Payload(s.c_str());
    msg->setM_Type(0);
    msg->setSeq_Num(0);
    msg->setReciver(dest);
    msg->setSender(source);
    send(msg,"out");
}

void Node::handleMessage(cMessage *msg)
{
    //MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
    if (msg->isSelfMessage()) { //Host wants to send

        int rand;
        do { //Avoid sending to yourself
            rand = uniform(0, getParentModule()->par("n").intValue());
        } while(rand == getIndex());
        delete msg;
        std::stringstream ss ;
        ss << rand;
        MyMessage *mmsg = new MyMessage(ss.str().c_str());
        cSend(mmsg,rand,getIndex());
        /*
        std::stringstream ss ;
        ss << rand;
        EV << "Sending "<< ss.str() <<" from source " << getIndex() << "\n";
        delete msg;
        msg = new cMessage(ss.str().c_str());
        send(msg, "out");*/

        double interval = exponential(1 / par("lambda").doubleValue());
        EV << ". Scheduled a new packet after " << interval << "s";
        scheduleAt(simTime() + interval, new cMessage(""));
    }
    else {
        //atoi functions converts a string to int
        //Check if this is the proper destination
        MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
        if (mmsg->getReciver() == getIndex())
            bubble(mmsg->getM_Payload());
        else
            bubble("Wrong destination");
        //TODO: delete msg;
    }
}





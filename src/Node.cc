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
    //double interval = exponential(1 / par("lambda").doubleValue());
   // scheduleAt(simTime() + interval, new cMessage(""));
    int n = getIndex();
    //read file n
    std::string file_name="../txtFiles/"+std::to_string(n)+".txt";
    my_file.open(file_name, std::ios::in);
    if(!my_file)
        finished=true;
    else
        finished = false;
}

void Node::mSend(MyMessage * msg,int ack)
{

    std:: string s;
    //new line only on inc
    getline(my_file, s);
    if(my_file.eof()||s.size()==0)
    {
        my_file.close();
        finished=true;
        return;
    }
    std::bitset<8> p{0};
    for(int i=0;i<s.size();i++)
    {
        std::bitset<8> y(s[i]);
        p=p^y;
    }
    msg->setMycheckbits(p);

    msg->setM_Type(ack);
    msg->setSeq_Num(0);
    msg->setReciver(reciver);
    msg->setSender(n);

    // Error

    int modE=uniform(0,1)*100;
    if(modE>par("modPercent").intValue()) //ini
    {
        int rand2=uniform(0,1)*s.size();
        int errorVal = uniform(-1,1)*par("modVal").intValue(); //ini
        s[rand2]=s[rand2]+errorVal;
    }
    msg->setM_Payload(s.c_str());
    int errored=uniform(0,1)*100;
    if(errored<par("chanPercent").intValue()){ //ini
        int rand=uniform(0,1)*3;
        if(rand==0)
            sendDelayed(msg, 0.01, "out");
        else if(rand==1){
            MyMessage * copyMsg = msg->dup();
            sendDelayed(copyMsg,0.002,"out");
            sendDelayed(msg,0.0021,"out");
        }
        //2 lost msg
    }
    else
        sendDelayed(msg,0.002,"out");
}

void Node::handleMessage(cMessage *msg)
{
    MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
    if (mmsg->getM_Type()==20) { //Host wants to send
        delete msg;
        std::stringstream ss ;
        ss << reciver;
        MyMessage *mmsg = new MyMessage(ss.str().c_str());
        mSend(mmsg,0);
        if(finished)
        {
            MyMessage *mmsg = new MyMessage("control message");
            mmsg->setM_Type(20);// 20 hub control
            mmsg->setReciver(reciver);
            mmsg->setM_Payload("error");
            send(mmsg,"out");
            return;
        }
     //   double interval = exponential(1 / par("lambda").doubleValue());
      //  EV << ". Scheduled a new packet after " << interval << "s";
     //   scheduleAt(simTime() + interval, new cMessage(""));
    }
    else {
        //MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
        bubble(mmsg->getM_Payload());
        std::stringstream ss ;
        ss << reciver;
        MyMessage *mmsg = new MyMessage(ss.str().c_str());
        mSend(mmsg,0);
        if(finished)
            return;

    }
}





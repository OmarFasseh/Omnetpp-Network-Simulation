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
#include "MyMessage_m.h"
Define_Module(Node);



void Node::initialize()
{
    counter = 0;
    // TODO - Generated method body
    if ( strcmp(getName(),"Tic")==0)
    {
        MyMessage_Base * msg = new MyMessage_Base("Hello");
        std::string inp = "hi there!";

        msg->setM_Payload(inp.c_str());
        msg->setM_Type(0);
//        msg->setSeq_Num(counter);

        std::bitset<8> checkSum(0);
        for(int i = 0;i<inp.size();i++){
                std::bitset<8> bs(inp[i]);
                checkSum= checkSum^  bs;
            }
        charCountSend(msg);

        std::bitset<8> ffs(255);
        std::bitset<8> CS(ffs.to_ulong() - checkSum.to_ulong());
        msg->setMycheckbits(checkSum);


       int rand=uniform(0,1)*10;
       if(rand>6) // prob to delay the message
       {
           int charrand=uniform(0,inp.size());
            std::string mypayload= msg->getM_Payload();
            mypayload[charrand]=mypayload[charrand]+5;
           msg->setM_Payload(mypayload.c_str());
       }
       send(msg,"out");
    }
}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    MyMessage_Base *mmsg = check_and_cast<MyMessage_Base *>(msg);
    if(mmsg->getM_Type() == 0){// check id msg correct or not
        EV << "char count: " << charCountRec(mmsg);
        bits sum = mmsg->getMycheckbits();
        std::bitset<8> checkSum(0);
        std::string pl =  mmsg->getM_Payload();
        for(int i = 0;i<pl.size();i++){
            std::bitset<8> bs(pl[i]);
            checkSum= checkSum^  bs;
        }
        if(checkSum.to_ulong() == sum.to_ulong()){// if check sum with no error

            MyMessage_Base * nmsg = new MyMessage_Base("Ack");
            nmsg->setM_Type(1);
            nmsg->setSeq_Num(mmsg->getSeq_Num());
            send(nmsg,"out");
        }
        else{// if check sum with error

            MyMessage_Base * nmsg = new MyMessage_Base("Nack");
            nmsg->setM_Type(2);
            nmsg->setSeq_Num(mmsg->getSeq_Num());
            send(nmsg,"out");
        }
    }
    else if(mmsg->getM_Type() == 1){// print if the sent msg is correct or not
        EV<<"received positive ack at sender  with sequence number ...   ";
        EV << mmsg->getSeq_Num();

        MyMessage_Base * nmsg = new MyMessage_Base("Hello");
        std::string inp = "hi there!";

        nmsg->setM_Payload(inp.c_str());
        nmsg->setM_Type(0);
        counter = counter + 1;
        nmsg->setSeq_Num(counter);

        std::bitset<8> checkSum(0);
        for(int i = 0;i<inp.size();i++){
                std::bitset<8> bs(inp[i]);
                checkSum= checkSum^  bs;
            }
        nmsg->setMycheckbits(checkSum);


        int rand=uniform(0,1)*10;
        if(rand>6) // prob to delay the message
        {
            int charrand=uniform(0,inp.size());
            std::string mypayload= nmsg->getM_Payload();
            mypayload[charrand]=mypayload[charrand]+5;
            nmsg->setM_Payload(mypayload.c_str());
        }
      send(nmsg,"out");
    }
    else if(mmsg->getM_Type() == 2){// print if the sent msg is correct or not
        EV<<"received negative ack at sender  with sequence number ...   ";
        EV << mmsg->getSeq_Num();

        MyMessage_Base * nmsg = new MyMessage_Base("Hello");
        std::string inp = "hi there!";

        nmsg->setM_Payload(inp.c_str());
        nmsg->setM_Type(0);
        nmsg->setSeq_Num(counter);

        std::bitset<8> checkSum(0);
        for(int i = 0;i<inp.size();i++){
                std::bitset<8> bs(inp[i]);
                checkSum= checkSum^  bs;
            }
        nmsg->setMycheckbits(checkSum);


        int rand=uniform(0,1)*10;
        if(rand>6) // prob to delay the message
        {
            int charrand=uniform(0,inp.size());
            std::string mypayload= nmsg->getM_Payload();
            mypayload[charrand]=mypayload[charrand]+5;
            nmsg->setM_Payload(mypayload.c_str());
        }
      send(nmsg,"out");
    }



}




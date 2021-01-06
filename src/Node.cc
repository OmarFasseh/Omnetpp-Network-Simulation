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
#include <stdlib.h>     /* atoi */
Define_Module(Node);

void Node::initialize()
{
    double interval = exponential(1 / par("lambda").doubleValue());
    scheduleAt(simTime() + interval, new cMessage(""));
    int n = getIndex();
    //read file n
    std::string file_name="../txtFiles/"+std::to_string(n)+".txt";
    my_file.open(file_name, std::ios::in);
    if(!my_file)
        finished=true;
    else
        finished = false;
}

void Node::mSend(MyMessage * msg)
{

    std:: string s;
    //new line only on inc
    getline(my_file, s);
    if(my_file.eof())
    {
        my_file.close();
        finished=true;
    }
    //s="abcdefg";
    //EV << "Sender " << s << endl;
    vector<bool> messageWithHaming= setHamming(s);
    int paddingSize = 0;
    string msgToSend = setMessagePayload(messageWithHaming, s.size()+1, paddingSize);
    msg->setPaddingSize(paddingSize);

    int charCount;
    vector<bool> receiverBits = removePadding(msgToSend, charCount, msg->getPaddingSize());
    checkHamming(receiverBits,charCount);
    //checkHamming(out, 1);


    //msg->setM_Type(0);
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
            sendDelayed(msg, 2.0, "out");
        else if(rand==1){
            MyMessage * copyMsg = msg->dup();
            send(copyMsg,   "out");
            send(msg,       "out");
        }
        //2 lost msg
    }
    else
        send(msg, "out");
}

void Node::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) { //Host wants to send
        delete msg;
        std::stringstream ss ;
        ss << reciver;
        MyMessage *mmsg = new MyMessage(ss.str().c_str());
        mSend(mmsg);
        if(finished) return;
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


vector<bool> Node::setHamming(string payload)
{

    int m=payload.size();
    int r=0;

    while((m*8)+r+1>pow(2,r))
    {
        r++;
    }
    vector<bool> ham(m*8+r,false);
    vector<bool> payloadBits(m*8,false);
    int k=0;
    for(int i=0;i<payload.size();i++)
    {
        bitset<8> newChar (payload[i]);
        for(int z=0;z<8;z++)
        {
            if(newChar[z])
            {
                payloadBits[k] = true;
            }
            k++;
        }
     }
    EV << "Payload to be sent (without hamming): ";
    for(int i=0;i<payloadBits.size();i++){
            EV << payloadBits[i]<<" ";
        }
        EV << endl;


    int j=0;
    for(int i=1;i<m*8+r;i++)
    {
        if((int)log2(i) ==log2(i)) continue;
        ham[i-1] = payloadBits[j++];
    }
    for(int i=0;i<r;i++)
    {
        int step=pow(2,i);
        for(j=step-1;j<m*8+r;j+=(2*step))
        {
            for(int z=0;z<step;z++)
            {
                if(ham[step-1] == ham[j+z])
                {
                    ham[step-1]=false;
                }
                else
                {
                    ham[step-1]=true;
                }

            }
        }
    }

    EV << endl << "Payload after hamming: ";
    for(int i=0;i<ham.size();i++){
        EV << ham[i]<<" ";
    }
    EV << endl;
    ham[5] = !ham[5];
    return ham;
}

string Node::setMessagePayload(vector<bool> &payloadBits, int charCount, int &paddingSize)
{
    int stringBitsSize = ceil(payloadBits.size()/8.0f)*8;
    paddingSize = stringBitsSize-payloadBits.size();
    for(int i = 0; i < paddingSize; i++)
        payloadBits.insert(payloadBits.begin(), 0);

    string msgPayload = to_string(charCount);
    char character=0;
    int j = 0;
    EV << "Message sent in bits : ";
    for(int i = 0; i < payloadBits.size(); i++)
    {
        if(j==8)
        {
            msgPayload+=character;
            character = payloadBits[i];
            j = 0;

        }
        else
        {
            character = character << 1;
            character |=payloadBits[i];
        }
        j++;
    }
    if(j==8)
    {
        msgPayload+=character;
    }
    for (int i = 0; i < payloadBits.size(); i++)
    {
        EV << payloadBits[i];
    }
    EV << "done, string is : " <<msgPayload << endl;
    return msgPayload;
}


vector<bool> Node::checkHamming(vector<bool> &ham, int charCount)
{
    int r=0;
    charCount--;
    while((charCount*8)+r+1>pow(2,r))
    {
        r++;
    }

    EV << endl << "Message received with hamming: ";
    for(int i=0;i<ham.size();i++){
        EV << ham[i]<<" ";
    }
    EV << endl;
    int j = 0;
    vector<bool> errorBits(r,0);
    int errorPos = 0;

    for(int i=0;i<r;i++)
    {
        int step=pow(2,i);
        for(j=step-1;j<charCount*8+r;j+=(2*step))
        {
            for(int z=0;z<step;z++)
            {
                if(ham[step-1] == ham[j+z] && j+z != step-1)
                {
                    ham[step-1]=false;
                }
                else if(j+z != step-1)
                {
                    ham[step-1]=true;
                }

            }
        }
        errorBits[i] = ham[step-1];
    }
    for (int i = r; i >= 0; i--)
    {
        errorPos = errorPos << 1;
        errorPos |= errorBits[i];
    }
    if(errorPos!=0)
    {
        EV << "Error at " << errorPos-1 << endl;
        ham[errorPos-1]=!ham[errorPos-1];
        EV << "Message after correction : " << endl;
        for(int i=0;i<ham.size();i++){
            EV << ham[i]<<" ";
        }
        EV << endl;

    }
    else
    {
        EV << "No error found." << endl;
    }
    j = 0;
    vector<bool> payloadBits(charCount*8,false);
    for(int i=1;i<charCount*8+r;i++)
    {
       if((int)log2(i) !=log2(i))
       {
           payloadBits[j++] = ham[i-1];
       }
    }
    EV << "Payload after correction: ";
    for(int i=0;i<payloadBits.size();i++){
        EV << payloadBits[i]<<" ";
    }
    EV << endl;
    return payloadBits;
}

vector<bool> Node::removePadding(string payload, int &charCount, int paddingSize)
{
    EV << "STR ReC : " << payload << endl;
    charCount = atoi(&payload[0]);
    int m=payload.size()-1;
    vector<bool> payloadBits(m*8,false);
    int k=m*8-1;
    for(int i=payload.size()-1;i>0;i--)
    {
        bitset<8> newChar (payload[i]);
        for(int z=0;z<8;z++)
        {
            if(newChar[z])
            {
                payloadBits[k] = true;
            }
            k--;
        }
     }
    for (int i = 0; i < payloadBits.size(); i++)
        EV <<" "<< payloadBits[i];
    int payloadSize = m*8-paddingSize;
    vector<bool> messageWithHamming(payloadSize);
    EV<<"Message with hamming rec :" ;
    for(int i = 0; i < payloadSize; i++)
    {
        messageWithHamming[i] = payloadBits[i+paddingSize];
    }
    return messageWithHamming;
}
string Node::decodeHamming(vector<bool> &msg)
{


}


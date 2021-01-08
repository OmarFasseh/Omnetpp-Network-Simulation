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
#include <stdlib.h> /* atoi */
Define_Module(Node);

void Node::initialize()
{
    //double interval = exponential(1 / par("lambda").doubleValue());
    // scheduleAt(simTime() + interval, new cMessage(""));
    int n = getIndex();
    receiverR = 0;
    senderWindowSize = 3;
    sequenceNumber = 0;
    timers = vector<MyMessage *>(senderWindowSize, nullptr);
    //read file n
    std::string file_name = "../txtFiles/" + std::to_string(n) + ".txt";
    my_file.open(file_name, std::ios::in);
    if (!my_file)
        finished = true;
    else
        finished = false;
}

void Node::mSend(int ack)
{

    //If window slided and we dont have the window full of data, read data if possible.
    while (senderWindowSize != senderData.size())
    {
        string s;
        //new line only on inc
        getline(my_file, s);
        if (my_file.eof() || s.size() == 0)
        {
            my_file.close(); //Might be removed to be able to add to file while running.
            finished = true;
            break;
        }
        senderData.push(s);
    }

    //Sending data with ack
    for (int i = 0; i < senderData.size(); i++)
    {
        string messageType = "0"; //0 Means message has data
        string tmp = messageType;
        tmp += (char)(receiverR + '0'); //Ack for receiver to slide the receriver's sending window
        //
        tmp += (char)((sequenceNumber + i) % (senderWindowSize + 1) + '0'); //To tell receiver the sequence of message being sent
        tmp += senderData.front();
        const char *de5ra1 = tmp.c_str();
        senderData.pop();
        senderData.push(tmp);
        //Hamming and framing
        vector<bool> messageWithHaming = setHamming(tmp);
        int paddingSize = 0;
        string msgToSend = setMessagePayload(messageWithHaming, tmp.size() + 1, paddingSize);
        MyMessage *msg = new MyMessage("message");
        //->setM_Payload(msgToSend.c_str());
        msg->setPaddingSize(paddingSize);
        msg->setM_Type(99);
        msg->setReciver(reciver);
        msg->setSender(n);
        MyMessage *timer = new MyMessage("timer");
        timers[(sequenceNumber + i) % (senderWindowSize + 1)] = timer;
        timer->setM_Type(50);
        scheduleAt(simTime() + 0.1 * (i + 1), timer);
        const char *de5raaaaaaaa = msgToSend.c_str();
        errorAndSendWithDelay(msg, msgToSend, 0.001 * (i + 1));
    }

    //Sending ack only
    if (senderData.size() == 0 && ack)
    {
        string messageType = "1"; //1 Means message has no data (only ack)
        string tmp = messageType;
        tmp += (char)(receiverR + '0'); //Ack for receiver to slide the receriver's sending window

        //Hamming and framing
        vector<bool> messageWithHaming = setHamming(tmp);
        int paddingSize = 0;
        string msgToSend = setMessagePayload(messageWithHaming, tmp.size() + 1, paddingSize);
        MyMessage *msg = new MyMessage();
        //msg->setM_Payload(msgToSend);
        msg->setPaddingSize(paddingSize);
        msg->setM_Type(99);
        msg->setReciver(reciver);
        msg->setSender(n);
        errorAndSendWithDelay(msg, msgToSend, 0.001);
        //Send with/without error
    }
}
void Node::errorAndSendWithDelay(MyMessage *msg, string s, double delay)
{
    first = false;
    int modE = uniform(0, 1) * 100;
    if (modE < par("modPercent").intValue()) //ini
    {
        int rand2 = uniform(0, 1) * s.size();
        int errorVal = uniform(-1, 1) * par("modVal").intValue(); //ini
        s[rand2] = s[rand2] + errorVal;
    }
    msg->setM_Payload(s.c_str()); //TODO: remove cast
    const char * de5ra = s.c_str();
    int errored = uniform(0, 1) * 100;
    if (errored < par("chanPercent").intValue())
    { //ini
        int rand = uniform(0, 1) * 3;
        if (rand == 0)
            sendDelayed(msg, 0.01 + delay, "out");
        else if (rand == 1)
        {
            MyMessage *copyMsg = msg->dup();
            sendDelayed(copyMsg, 0.002 + delay, "out");
            sendDelayed(msg, 0.0021 + delay, "out");
        }
        //2 lost msg
    }
    else
        sendDelayed(msg, 0.002 + delay, "out");
}
void Node::handleMessage(cMessage *msg)
{
    MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
    if (mmsg->getM_Type() == 20)
    { //Host wants to send
        delete msg;
        stringstream ss;
        ss << reciver;
        mSend(0);
        if (finished && first)
        {
            first = false;
            MyMessage *mmsg = new MyMessage("control message");
            mmsg->setM_Type(20); // 20 hub control
            mmsg->setReciver(reciver);
            mmsg->setM_Payload("Control");
            send(mmsg, "out");
            return;
        }
        //   double interval = exponential(1 / par("lambda").doubleValue());
        //  EV << ". Scheduled a new packet after " << interval << "s";
        //   scheduleAt(simTime() + 0.1, new cMessage(""));
    }
    else if (mmsg->getM_Type() == 99)
    {
        int charCount;
        //General functions
        string s;

        const char * shit = mmsg->getM_Payload();
        vector<bool> receiverBits = removePadding(mmsg->getM_Payload(), charCount, mmsg->getPaddingSize());
        receiverBits = checkHamming(receiverBits, charCount);
        string recMsg = BitsToString(receiverBits);
        char messageType = recMsg[0];
        int rec = (int)recMsg[1] - '0';
        int i = 0;
        while (rec != sequenceNumber)
        {
            if (timers[(sequenceNumber + i) % (senderWindowSize + 1)])
            {
                cancelAndDelete(timers[(sequenceNumber + i) % (senderWindowSize + 1)]);
            }
            timers[(sequenceNumber + i) % (senderWindowSize + 1)] = NULL;
            sequenceNumber = (sequenceNumber + 1) % (senderWindowSize + 1);
            if (!senderData.empty())
                senderData.pop();
            i++;
        }

        if (messageType == '0')
        {
            if (recMsg[2] - '0' == receiverR)
            {
                receiverR++;
            }
            recMsg = recMsg.substr(3, recMsg.size() - 3);
            bubble(recMsg.c_str());
        }
        mSend(1);
        if (finished)
        {
            //no more
        }
        delete mmsg;
        //MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
        // bubble(mmsg->getM_Payload());
        // std::stringstream ss;
        // ss << reciver;
        // MyMessage *mmsg = new MyMessage(ss.str().c_str());
        // mSend(mmsg, 0);
    }
    else if (mmsg->getM_Type() == 50)
    {
        delete mmsg;
        for (int i = 0; i < senderWindowSize; i++)
        {
            if (timers[i])
            {
                cancelAndDelete(timers[i]);
                timers[i] = nullptr;
            }
        }
        mSend(1);
    }
}

vector<bool> Node::setHamming(string payload)
{

    int m = payload.size();
    int r = 0;

    while ((m * 8) + r + 1 > pow(2, r))
    {
        r++;
    }
    vector<bool> ham(m * 8 + r, false);
    vector<bool> payloadBits(m * 8, false);
    //General functions
    StringToBits(payload, payloadBits);

    EV << "Payload to be sent (without hamming): ";
    for (int i = 0; i < payloadBits.size(); i++)
    {
        EV << payloadBits[i] << " ";
    }
    EV << endl;

    int j = 0;
    for (int i = 1; i < m * 8 + r; i++)
    {
        if ((int)log2(i) == log2(i))
            continue;
        ham[i - 1] = payloadBits[j++];
    }
    for (int i = 0; i < r; i++)
    {
        int step = pow(2, i);
        for (j = step - 1; j < m * 8 + r; j += (2 * step))
        {
            for (int z = 0; z < step; z++)
            {
                if (ham[step - 1] == ham[j + z])
                {
                    ham[step - 1] = false;
                }
                else
                {
                    ham[step - 1] = true;
                }
            }
        }
    }

    EV << endl
       << "Payload after hamming: ";
    for (int i = 0; i < ham.size(); i++)
    {
        EV << ham[i] << " ";
    }
    EV << endl;
    return ham;
}

string Node::setMessagePayload(vector<bool> &payloadBits, int charCount, int &paddingSize)
{
    int stringBitsSize = ceil(payloadBits.size() / 8.0f) * 8;
    paddingSize = stringBitsSize - payloadBits.size();
    for (int i = 0; i < paddingSize; i++)
        payloadBits.insert(payloadBits.begin(), 0);

    string msgPayload = to_string(charCount);
    EV << "Message sent in bits : ";
    //General functions
    string s = BitsToString(payloadBits);
    msgPayload += s;

    for (int i = 0; i < payloadBits.size(); i++)
    {
        EV << payloadBits[i];
    }
    EV << "done, string is : " << msgPayload << endl;
    return msgPayload;
}

string Node::decodeHamming(vector<bool> &msg)
{
    string s = "";
    return s;
}

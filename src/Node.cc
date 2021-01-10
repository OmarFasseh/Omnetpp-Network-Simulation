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
    timers = vector<MyMessage *>(senderWindowSize + 2, nullptr);
    //read file n
    std::string file_name = "../txtFiles/" + std::to_string(n) + ".txt";
    my_file.open(file_name, std::ios::in);
    if (!my_file)
        finished = true;
    else
        finished = false;
}

void Node::mSend(int notControl, int windowSkip)
{

    //If window slided and we dont have the window full of data, read data if possible.
    while (senderWindowSize != senderData.size())
    {
        string s;
        //new line only on inc
        bool eof = my_file.eof();
        getline(my_file, s);
        if (eof || finished)
        {
            my_file.close(); //Might be removed to be able to add to file while running.
            finished = true;
            break;
        }
        usefulData++;
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
        senderData.push(senderData.front());
        senderData.pop();
        if (windowSkip > i)
        {
            continue;
        }
        //Hamming and framing
        vector<bool> messageWithHaming = setHamming(tmp);
        int paddingSize = 0;
        string msgToSend = setMessagePayload(messageWithHaming, tmp.size() + 1, paddingSize);
        MyMessage *msg = new MyMessage("message");
        msg->setM_Payload(msgToSend.c_str());
        msg->setPaddingSize(paddingSize);
        msg->setM_Type(99);
        msg->setReceiver(receiver);
        msg->setSender(n);
        msg->setPayloadSize(msgToSend.size());
        MyMessage *timer = new MyMessage("timer");
        timers[(sequenceNumber + i) % (senderWindowSize + 1)] = timer;
        timer->setM_Type(50);
        scheduleAt(simTime() + 0.1 * (i + 1), timer);
        errorAndSendWithDelay(msg, msgToSend, i);
    }

    //Sending ack only
    if (senderData.size() == windowSkip && notControl)
    {
        string messageType = "1"; //1 Means message has no data (only ack)
        string tmp = messageType;
        tmp += (char)(receiverR + '0'); //Ack for receiver to slide the receriver's sending window

        //Hamming and framing
        vector<bool> messageWithHaming = setHamming(tmp);
        int paddingSize = 0;
        string msgToSend = setMessagePayload(messageWithHaming, tmp.size() + 1, paddingSize);
        MyMessage *msg;
        //msg->setM_Payload(msgToSend);
        if (finished & (senderData.size() == 0))
        {
            msg = new MyMessage("Finished + ack");
            msg->setM_Type(98); //finished file
        }
        else
        {
            msg = new MyMessage("ack");
            msg->setM_Type(99); //not finished
        }
        msg->setPaddingSize(paddingSize);
        msg->setReceiver(receiver);
        msg->setSender(n);
        msg->setPayloadSize(msgToSend.size());
        errorAndSendWithDelay(msg, msgToSend, 0);
    }
}
void Node::errorAndSendWithDelay(MyMessage *msg, string s, double delay)
{
    first = false;

    double simDelay = par("simDelay").doubleValue();
    delay *= simDelay / 5;

    int modE = uniform(0, 1) * 100;
    if (modE < par("modPercent").intValue()) //ini
    {
        int rand2 = uniform(0, 1) * (s.size());
        if (rand2 == 0)
            rand2++;
        int errorBit = uniform(0, 1) * 8; //ini
        bitset<8> bs(s.c_str()[rand2]);
        bs[errorBit] = !bs[errorBit];
        unsigned long i = bs.to_ulong();
        unsigned char c = static_cast<unsigned char>(i);
        s[rand2] = c;
    }
    msg->setM_Payload(s.c_str());
    int errored = uniform(0, 1) * 100;
    if (errored < par("chanPercent").intValue())
    { //ini
        int rand = uniform(0, 1) * 3;
        if (rand == 0 && par("del").boolValue())
        {
            //send delayed
            sendDelayed(msg, simDelay * 5 + delay, "out");
            generatedFrames++;
        }
        else if (rand == 1 && par("dup").boolValue())
        {
            //send dup
            MyMessage *copyMsg = msg->dup();
            sendDelayed(copyMsg, simDelay + delay, "out");
            sendDelayed(msg, simDelay + simDelay / 10 + delay, "out");
            generatedFrames += 2;
        }
        else if (rand == 2 && par("loss").boolValue())
        {
            //lost msg
            EV << "Lost message";
            droppedFrames++;
        }
        else
        {
            sendDelayed(msg, simDelay + delay, "out");
            generatedFrames++;
        }
    }
    else
    {
        //no channel error
        sendDelayed(msg, simDelay + delay, "out");
        generatedFrames++;
    }
}
void Node::handleMessage(cMessage *msg)
{
    if (veryFinished)
    {
        delete msg;
        return;
    }
    MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
    if (mmsg->getM_Type() == 20)
    { //Host wants to send
        delete msg;
        stringstream ss;
        ss << receiver;
        mSend(0, 0);
        if (finished && first) //didn't find a file must retransmit control msg
        {
            first = false;
            MyMessage *mmsg = new MyMessage("control message");
            mmsg->setM_Type(20); // 20 hub control
            mmsg->setReceiver(receiver);
            mmsg->setM_Payload("Control");
            send(mmsg, "out");
            return;
        }
    }
    else if (mmsg->getM_Type() == 99 || mmsg->getM_Type() == 98)
    {

        int charCount;
        //General functions
        vector<bool> receiverBits = removePadding(mmsg->getM_Payload(), mmsg->getPayloadSize(), charCount, mmsg->getPaddingSize());
        vector<bool> receiverBits2 = checkHamming(receiverBits, charCount);
        string recMsg = BitsToStringDecode(receiverBits2);
        char messageType = recMsg[0];
        int rec = (int)recMsg[1] - '0';

        while (rec != sequenceNumber)
        {
            if (timers[(sequenceNumber) % (senderWindowSize + 1)])
            {
                cancelAndDelete(timers[(sequenceNumber) % (senderWindowSize + 1)]);
            }
            timers[(sequenceNumber) % (senderWindowSize + 1)] = nullptr;

            sequenceNumber = (sequenceNumber + 1) % (senderWindowSize + 1);

            if (!senderData.empty())
            {
                senderData.pop();
            }
        }
        if (messageType == '0' && mmsg->getM_Type() == 99)
        {
            if (recMsg[2] - '0' == receiverR)
            {
                receiverR = (receiverR + 1) % (senderWindowSize + 1);
            }
            recMsg = recMsg.substr(3, recMsg.size() - 3);
            bubble(recMsg.c_str());
        }
        if (senderData.size() == 0 && mmsg->getM_Type() == 98)
        {
            //no more
            veryFinished = true;
            delete mmsg;
            return;
        }
        int windowSkip = senderData.size();
        mSend(1, windowSkip);
    }
    else if (mmsg->getM_Type() == 50)
    {
        retransmittedFrames += senderData.size();
        for (int i = 0; i < senderWindowSize; i++)
        {
            if (timers[i])
            {
                cancelAndDelete(timers[i]);
                timers[i] = nullptr;
            }
        }
        mSend(1, 0);
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

    // EV << "Payload to be sent (without hamming): ";
    // for (int i = 0; i < payloadBits.size(); i++)
    // {
    //     EV << payloadBits[i] << " ";
    // }
    // EV << endl;

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

    // EV << endl
    //    << "Payload after hamming: ";
    // for (int i = 0; i < ham.size(); i++)
    // {
    //     EV << ham[i] << " ";
    // }
    // EV << endl;
    return ham;
}

string Node::setMessagePayload(vector<bool> &payloadBits, int charCount, int &paddingSize)
{
    int stringBitsSize = ceil(payloadBits.size() / 8.0f) * 8;
    paddingSize = stringBitsSize - payloadBits.size();
    for (int i = 0; i < paddingSize; i++)
        payloadBits.insert(payloadBits.begin(), 1);

    char c = (char)charCount;
    string msgPayload = "";
    msgPayload += c;
    //EV << "Message sent in bits : ";
    //General functions
    char character = 0;
    int j = 0;
    //EV << "Message sent in bits : ";
    for (int i = 0; i < payloadBits.size(); i++)
    {
        if (j == 8)
        {
            msgPayload += character;
            character = payloadBits[i];
            j = 0;
        }
        else
        {
            character = character << 1;
            character |= payloadBits[i];
        }
        j++;
    }
    if (j == 8)
    {
        msgPayload += character;
    }

    // for (int i = 0; i < payloadBits.size(); i++)
    // {
    //     EV << payloadBits[i];
    // }
    // EV << "done, string is : " << msgPayload << endl;
    return msgPayload;
}

string Node::decodeHamming(vector<bool> &msg)
{
    string s = "";
    return s;
}

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

#include "hub.h"
#include "Node.h"

#include "MyMessage_m.h"
Define_Module(Hub);
#include <fstream>
void Hub::initialize()
{
    //pairs + start time

    int n = par("n").intValue();
    int pairsDelay = par("pairsDelay").doubleValue();
    std::vector<int> pool(n, 0);
    int pair1, pair2;

    for (int i = 0; i < n; i++)
    {
        pool[i] = i;
    }
    for (int i = 0; i < n / 2; i++)
    {
        int rand = uniform(0, 1) * pool.size();
        pair1 = pool[rand];
        pool.erase(pool.begin() + rand);

        rand = uniform(0, 1) * pool.size();
        pair2 = pool[rand];
        pool.erase(pool.begin() + rand);
        //((Hub *)getParentModule()->getSubmodule("hub"))->test();
        ((Node *)getParentModule()->getSubmodule("nodes", pair1))->receiver = pair2;
        ((Node *)getParentModule()->getSubmodule("nodes", pair2))->receiver = pair1;
        EV << pair1 << pair2 << " ";
        pairs.push_back(std::make_pair(pair1, pair2));

        MyMessage *mmsg = new MyMessage("control_message");
        mmsg->setM_Type(20); // 20 hub control
        sendDelayed(mmsg, i * pairsDelay + simTime(), "outs", pair1);
    }
    MyMessage *mmsg = new MyMessage("END");
    mmsg->setM_Type(95); //simulation finished
    scheduleAt(n * pairsDelay + simTime(), mmsg);
}

void Hub::handleMessage(cMessage *msg)
{

    MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
    if (mmsg->getM_Type() == 95)
    {
        int n = par("n").intValue();
        std::fstream f;
        f.open("../txtFiles/outputStats.txt", fstream::out);
        int totalDF = 0, totalGF = 0, totalRF = 0, totalUseful = 0;
        for (int i = 0; i < n; i++)
        {
            Node *n = ((Node *)getParentModule()->getSubmodule("nodes", i));
            totalDF += n->droppedFrames;
            totalGF += n->generatedFrames;
            totalRF += n->retransmittedFrames;
            totalUseful += n->usefulData;
            f << "Node " << i << "\n";
            f << "Dropped frames = " << n->droppedFrames << "\n";
            f << "Generated frames = " << n->generatedFrames << "\n";
            f << "Retransmitted frames = " << n->retransmittedFrames << "\n";
            f << "Useful data = " << n->usefulData << "\n";
            f << "------------------------\n";
        }
        f << "Stats: \n";
        f << "Total Dropped frames = " << totalDF << "\n";
        f << "Total Generated frames = " << totalGF << "\n";
        f << "Total Retransmitted frames = " << totalRF << "\n";
        f << "Total useful data = " << totalUseful << "\n";
        f << "Total Useful/Transmitted = " << ((double)totalUseful) / totalGF * 100.0 << "%\n";
        return;
    }

    if (mmsg->getM_Type() == 99)
    {
        string recMsg = unHam(mmsg->getM_Payload(), mmsg->getPayloadSize(), mmsg->getPaddingSize());
        bubble(recMsg.c_str());
        if (recMsg[0] == '0')
        {
            recMsg = recMsg.substr(2, recMsg.size() - 3);
            bubble(recMsg.c_str());
        }
        else
            bubble(to_string(recMsg[0]).c_str());
    }
    send(msg, "outs", mmsg->getReceiver());
}

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

#ifndef __MESH_NODE_H_
#define __MESH_NODE_H_

#include "MyMessage_m.h"
#include <bitset>
#include "GlobalFunctions.h"
#include <fstream>
#include <cmath>
#include <vector>
#include <queue>

using namespace omnetpp;
using namespace std;
/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{

protected:
  int senderNum;
  int n = -1;
  std::fstream my_file;
  bool finished = false;
  bool first = true;
  int senderWindowSize;
  queue<string> senderData;
  int senderQueueSize;
  int receiverR;
  int sequenceNumber;
  vector<MyMessage *> timers;
  void mSend(int ack);
  void errorAndSendWithDelay(MyMessage *msg, string s, double delay);
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);

public:
  int reciver;
  vector<bool> setHamming(string payload);
  string setMessagePayload(vector<bool> &payloadBits, int charCount, int &paddingSize);
  string decodeHamming(vector<bool> &msg);
};

#endif

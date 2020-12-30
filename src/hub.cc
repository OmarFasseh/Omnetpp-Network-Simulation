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
#include "MyMessage_m.h"
Define_Module(Hub);

void Hub::initialize()
{
    // TODO - Generated method body
}

void Hub::handleMessage(cMessage *msg)
{

    MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
    int modE=uniform(0,1)*10;
    if(modE>6) //30%
    {
        std:: string s = mmsg->getM_Payload();
        int rand2=uniform(0,1)*s.size();
        s[rand2]=s[rand2]+1;
        mmsg->setM_Payload(s.c_str());
     }
    int errored=uniform(0,1)*2;
    if(errored){
        int rand=uniform(0,1)*3;
        if(rand==0)
            sendDelayed(msg, 2.0, "outs",mmsg->getReciver());
        else if(rand==1){
            MyMessage * copyMsg = mmsg->dup();
            send(copyMsg,   "outs",mmsg->getReciver());
            send(msg,       "outs",mmsg->getReciver());
        }
    }
    else
        send(msg, "outs", mmsg->getReciver());

}

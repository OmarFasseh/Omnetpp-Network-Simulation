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
#include "vector"
#include <fstream>
void Hub::initialize()
{
    //pairs + start time

    int n=par("n").intValue();
    std::vector<int> pool (n,0);
    int pair1,pair2;

    for(int i =0;i<n;i++){
        pool[i]=i;
    }
    for(int i =0;i<n/2;i++){
        int rand=uniform(0,1)*pool.size();
        pair1=pool[rand];
        pool.erase(pool.begin()+rand);

        rand=uniform(0,1)*pool.size();
        pair2=pool[rand];
        pool.erase(pool.begin()+rand);

        //pair1,pair2
        /*
        std::fstream my_file;
        std::string file_name="../txtFiles/"+std::to_string(n)+;
        my_file.open(file_name, std::ios::out);
        if(!my_file)
            finished=true;
      */
    }

}

void Hub::handleMessage(cMessage *msg)
{

    MyMessage *mmsg = check_and_cast<MyMessage *>(msg);
    bubble(mmsg->getM_Payload());
    send(msg, "outs", mmsg->getReciver());

}

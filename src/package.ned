package mesh;

@license(LGPL);

import ned.IdealChannel;

network Mesh
{
    parameters:
        int n = default(5);
        @display("bgb=327,200");
    submodules:
        nodes[n]: Node;
        hub: Hub;
    connections:
        for i=0..sizeof(nodes)-1 {
            nodes[i].out --> IdealChannel --> hub.ins++;
            nodes[i].in <-- IdealChannel <-- hub.outs++;
        }
}

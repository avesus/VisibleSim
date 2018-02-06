/*
 * pathHop.cpp
 *
 *  Created on: 19/01/2018
 *      Author: pthalamy
 */

#include "pathHop.hpp"

#include <algorithm>

#include "utils.h"
#include "catoms3DWorld.h"
#include "catoms3DBlock.h"
#include "lattice.h"

using namespace Catoms3D;

bool
PathHop::getConnectors(std::set<short>& connectors) const {
    transform(conDistanceMap.begin(), conDistanceMap.end(),
              std::inserter<set<short>>(connectors, connectors.end()),
              [](const std::pair<short,int>& pair) { return pair.first; });

    return !connectors.empty();
}       

bool
PathHop::getConnectorsByIncreasingDistance(vector<short>& sortedCons) {
    // Fill vector with connectors
    for (const auto& pair : conDistanceMap)
        sortedCons.push_back(pair.first);

    // Lambda sort relative to distance to path target
    sort(sortedCons.begin(), sortedCons.end(),
         [=](const short& a, const short& b) {
             return conDistanceMap.at(a) < conDistanceMap.at(b);
         });

    return !sortedCons.empty();
}

int
PathHop::getDistance(short con) const {
    auto pair = conDistanceMap.find(con);
    if (pair != conDistanceMap.end())
        return pair->second;
    else
        return -1;
}

short
PathHop::getHopConnectorAtPosition(const Cell3DPosition &pos) const {
    Catoms3DBlock *pivot = static_cast<Catoms3DBlock*>(
        Catoms3DWorld::getWorld()->lattice->getBlock(position));
    
    return pivot->getConnectorId(pos);
}

bool
PathHop::isInVicinityOf(const Cell3DPosition &pos) const {
    short conToFind = getHopConnectorAtPosition(pos);
    
    return conDistanceMap.find(conToFind) != conDistanceMap.end();  
}

bool
PathHop::finalTargetReached() const {
    return conDistanceMap.empty();
}

void
PathHop::prune(short connector) {
    assert(conDistanceMap.find(connector) != conDistanceMap.end());
    int dPrune = conDistanceMap[connector];

    cout << "Prune " << connector << "(" << dPrune << ")" << endl
         << *this << endl;
    
    utils::erase_if(conDistanceMap, [&](std::pair<short, int> pair) {
            return (pair.second >= dPrune); // (pair.first != connector) && ?
        });

    cout << "Res: " << *this << endl;    
}

std::ostream& operator<<(std::ostream &stream, PathHop const& hop) {   
    stream << "Position: " << hop.position << endl
           << "Orientation: " << hop.orientationCode << endl
           << "Connectors: " << endl;

    for (auto const& pair : hop.conDistanceMap) {
        stream << pair.first << " (d = " << pair.second << ")" << endl;
    }
    
    return stream;
}

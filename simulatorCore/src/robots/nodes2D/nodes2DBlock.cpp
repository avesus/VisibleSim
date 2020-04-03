/**
 * @file   nodes2DBlock.cpp
 * @author pthalamy <pthalamy@p3520-pthalamy-linux>
 * @date   Wed Jun 19 13:47:36 2019
 *
 * @brief
 *
 *
 */

#include <iostream>

#include "robots/nodes2D/nodes2DBlock.h"
#include "base/buildingBlock.h"
#include "robots/nodes2D/nodes2DWorld.h"
#include "robots/nodes2D/nodes2DSimulator.h"
#include "utils/trace.h"

using namespace std;

//! \namespace Nodes2D
namespace Nodes2D {

Nodes2DBlock::Nodes2DBlock(int bId, BlockCodeBuilder bcb)
    : BaseSimulator::BuildingBlock(bId, bcb, SLattice::MAX_NB_NEIGHBORS) {
#ifdef DEBUG_OBJECT_LIFECYCLE
    OUTPUT << "Nodes2DBlock constructor" << endl;
#endif
}

Nodes2DBlock::~Nodes2DBlock() {
    OUTPUT << "Nodes2DBlock destructor " << blockId << endl;
}

int Nodes2DBlock::getDirection(P2PNetworkInterface *given_interface) const {
    if( !given_interface) {
        return -1;
    }

    for( int i(0); i < SLattice::MAX_NB_NEIGHBORS; ++i) {
        if(P2PNetworkInterfaces[i] == given_interface) return i;
    }

    return -1;
}

std::ostream& operator<<(std::ostream &stream, Nodes2DBlock const& bb) {
    stream << bb.blockId << "\tcolor: " << bb.color;
    return stream;
}

bool Nodes2DBlock::getNeighborPos(short connectorDir,Cell3DPosition &pos) const {
    Nodes2DWorld *wrl = getWorld();
        const Vector3D bs = wrl->lattice->gridScale;
        pos = ((SLattice*)(wrl->lattice))->getNeighborRelativePos(SLattice::Direction(connectorDir));
        Vector3D realPos(pos[0]*bs[0],pos[1]*bs[1],pos[2]*bs[2],1.0);
        realPos = ((Nodes2DGlBlock*)ptrGlBlock)->mat*realPos;
        pos = wrl->lattice->worldToGridPosition(realPos);
        return wrl->lattice->isInGrid(pos);
}

void Nodes2DBlock::addNeighbor(P2PNetworkInterface *ni, BuildingBlock* target) {
#ifdef DEBUG_NEIGHBORHOOD
    OUTPUT << "Simulator: "<< blockId << " add neighbor " << target->blockId << " on "
           << getWorld()->lattice->getDirectionString(getDirection(ni)) << endl;
#endif
    getScheduler()->schedule(
        new AddNeighborEvent(getScheduler()->now(), this,
                             getWorld()->lattice->getOppositeDirection(getDirection(ni)),
                             target->blockId));
}

void Nodes2DBlock::removeNeighbor(P2PNetworkInterface *ni) {
#ifdef DEBUG_NEIGHBORHOOD
    OUTPUT << "Simulator: "<< blockId << " remove neighbor on "
           << getWorld()->lattice->getDirectionString(getDirection(ni)) << endl;
#endif
    getScheduler()->schedule(new RemoveNeighborEvent(getScheduler()->now(), this,getWorld()->lattice->getOppositeDirection(getDirection(ni))));
}

bool Nodes2DBlock::hasANeighbor(SLattice::Direction n, bool groundIsNeighbor) const {
    return hasANeighbor(getInterface(n),groundIsNeighbor);
}

bool Nodes2DBlock::hasANeighbor(P2PNetworkInterface *p2p, bool groundIsNeighbor) const {
    Cell3DPosition p = getPosition(p2p);
    if(p2p->connectedInterface) {
        return true;
    } else if (groundIsNeighbor && (p[2]<0)) {
        return true;
    }
    return false;
}

Cell3DPosition Nodes2DBlock::getPosition(SLattice::Direction d) const {
    World *wrl = getWorld();
    const vector<Cell3DPosition>& nCells = wrl->lattice->getRelativeConnectivity(position);
    return position + nCells[d];
}

// PTHY: TODO: Can be genericized in BuildingBlocks
Cell3DPosition Nodes2DBlock::getPosition(P2PNetworkInterface *p2p) const{
    return getPosition((SLattice::Direction)getDirection(p2p));
}

void Nodes2DBlock::setPosition(const Cell3DPosition &p) {
    setPositionAndOrientation(p, orientationCode);
}

void Nodes2DBlock::setPositionAndOrientation(const Cell3DPosition &pos, short code) {
    orientationCode = code;
    position = pos;

    Matrix M=getMatrixFromPositionAndOrientation(pos,code);
    getWorld()->updateGlData(this,M);
    getWorld()->updateGlData(this,position);
}

short Nodes2DBlock::getOrientationFromMatrix(const Matrix &mat) {
    static short tab[3][3]={{-1,2,-1},{3,-1,1},{-1,0,-1}};
    Vector3D V=mat * Vector3D(1,0,0);
    int cx=(int)(V[0]+1.5);
    int cy=(int)(V[1]+1.5);

    /*1,0 -> 0
    0,1 -> 1
    -1,0 -> 2
    0,-1 -> 3*/

    //OUTPUT << "result =" << current << endl;
    return (tab[cx][cy]);
}

Matrix Nodes2DBlock::getMatrixFromPositionAndOrientation(const Cell3DPosition &pos, short code) {
    short orientation = code;

    Matrix M1,M2,M;
    M1.setRotationZ(-orientation*90.0);
    Vector3D V=getWorld()->lattice->gridToWorldPosition(pos)-Vector3D(-12.5,-12.5,0);
    M2.setTranslation(V);
    M = M2*M1;
    return M;
}

}
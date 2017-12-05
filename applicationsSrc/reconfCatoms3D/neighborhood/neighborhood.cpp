#include "neighborhood.h"
#include "neighborRestriction.h"
#include "catoms3DWorld.h"

#define MSG_TIME rand()%10
#define MSG_TIME_ADD 100+rand()%10

int Neighborhood::numberBlockedModules = 0;
int Neighborhood::numberMessagesToAddBlock = 0;

Neighborhood::Neighborhood(Catoms3D::Catoms3DBlock *c, Reconf *r, SyncNext *sn, SyncPrevious *sp, BlockCodeBuilder bcb)
{
    catom = c;
    reconf = r;
    syncNext = sn;
    syncPrevious = sp;
    blockCodeBuilder = bcb;
}

void Neighborhood::addNeighborToLeft()
{
    Cell3DPosition pos = catom->position.addX(-1);
    if (BlockCode::target->isInTarget(pos) && !catom->getInterface(pos)->isConnected()) {
        if (!Scheduler::getScheduler()->hasEvent(ADDLEFTBLOCK_EVENT_ID, catom->blockId))
            getScheduler()->schedule(new AddLeftBlock_event(getScheduler()->now()+MSG_TIME_ADD, catom));
    }
}

void Neighborhood::addNeighborToRight()
{
    Cell3DPosition pos = catom->position.addX(1);
    if (BlockCode::target->isInTarget(pos) && !catom->getInterface(pos)->isConnected()) {
        if (!Scheduler::getScheduler()->hasEvent(ADDRIGHTBLOCK_EVENT_ID, catom->blockId))
            getScheduler()->schedule(new AddRightBlock_event(getScheduler()->now()+MSG_TIME_ADD, catom));
    }
}

void Neighborhood::addNextLineNeighbor()
{
    addNeighbor(catom->position.addY(1));
}

void Neighborhood::addPreviousLineNeighbor()
{
    addNeighbor(catom->position.addY(-1));
}

void Neighborhood::addNeighborToNextPlane()
{
    addNeighbor(catom->position.addZ(1));
}

void Neighborhood::addNeighborToPreviousPlane()
{
    addNeighbor(catom->position.addZ(-1));
}

bool Neighborhood::isFirstCatomOfLine()
{
    if (catom->getInterface(catom->position.addX(-1))->connectedInterface == NULL &&
            catom->getInterface(catom->position.addX(1))->connectedInterface == NULL)
        return true;
    return false;
}

bool Neighborhood::isFirstCatomOfPlane()
{
    if (isFirstCatomOfLine() &&
            catom->getInterface(catom->position.addY(-1))->connectedInterface == NULL &&
            catom->getInterface(catom->position.addY(1))->connectedInterface == NULL)
        return true;
    return false;
}


void Neighborhood::addNeighbors()
{
    addNext();
    addPrevious();
    addLeft();
    addRight();
}

void Neighborhood::addLeft() {
    if(BlockCode::target->isInTarget(catom->position.addX(-1)) &&
       !catom->getInterface(catom->position.addX(-1))->isConnected()) {
        if (syncNext->needSyncToLeft()) {
            syncNext->sync();
            //catom->setColor(RED);
        }
        else if (!BlockCode::target->isInTarget(catom->position.addY(-1).addX(-1)) || !catom->getInterface(catom->position.addY(-1))->isConnected())
            addNeighborToLeft();
    }
}

void Neighborhood::addRight() {
    if(BlockCode::target->isInTarget(catom->position.addX(1)) &&
       !catom->getInterface(catom->position.addX(1))->isConnected()) {
        if (syncPrevious->needSyncToRight()) {
            syncPrevious->sync();
            //catom->setColor(RED);
        }
        else if (!BlockCode::target->isInTarget(catom->position.addY(1).addX(1)) || !catom->getInterface(catom->position.addY(1))->isConnected())
            addNeighborToRight();
    }
}

void Neighborhood::addNext() {
    if (reconf->isSeedNext() && !syncNext->needSyncToRight()) {
        addEventAddNextLineNeighbor();
        //catom->setColor(CYAN);
    }
}
void Neighborhood::addPrevious() {
    if (reconf->isSeedPrevious() && !syncPrevious->needSyncToLeft()) {
        //catom->setColor(GOLD);
        addEventAddPreviousLineNeighbor();
    }
}

void Neighborhood::addEventAddNextLineNeighbor() {
    if (!Scheduler::getScheduler()->hasEvent(ADDNEXTLINE_EVENT_ID, catom->blockId)
            && !catom->getInterface(catom->position.addY(1))->isConnected()) {
        AddNextLine_event *evt = new AddNextLine_event(getScheduler()->now()+MSG_TIME_ADD, catom);
        getScheduler()->schedule(evt);
    }
}

void Neighborhood::addEventAddPreviousLineNeighbor() {
    if (!Scheduler::getScheduler()->hasEvent(ADDPREVIOUSLINE_EVENT_ID, catom->blockId)
            && !catom->getInterface(catom->position.addY(-1))->isConnected()) {
        AddPreviousLine_event *evt = new AddPreviousLine_event(getScheduler()->now()+MSG_TIME_ADD, catom);
        getScheduler()->schedule(evt);
    }
}

void Neighborhood::sendMessageToAddLeft() {
    if (catom->getInterface(catom->position.addY(-1))->isConnected()
            && BlockCode::target->isInTarget(catom->position.addY(-1).addX(-1))) {
        CanFillLeft_message *msg = new CanFillLeft_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(catom->position.addY(-1))));
        numberMessagesToAddBlock++;
        return;
    }

    addNeighborToLeft();
}

void Neighborhood::sendMessageToAddRight() {
    if (catom->getInterface(catom->position.addY(1))->isConnected()
            && BlockCode::target->isInTarget(catom->position.addY(1).addX(1))) {
        CanFillRight_message *msg = new CanFillRight_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(catom->position.addY(1))));
        numberMessagesToAddBlock++;
        return;
    }

    addNeighborToRight();
}

void Neighborhood::sendResponseMessageToAddLeft() {
    if (syncNext->needSyncToLeft() &&
            syncNext->isInternalBorder(1))
        return;

    if (catom->getInterface(catom->position.addY(1))->isConnected() &&
            catom->getInterface(catom->position.addX(-1))->isConnected()) {
        CanFillLeftResponse_message *msg = new CanFillLeftResponse_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(catom->position.addY(1))));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::sendResponseMessageToAddRight() {
    if (syncNext->needSyncToRight() &&
            syncNext->isInternalBorder(1))
        return;

    if (catom->getInterface(catom->position.addX(1))->isConnected() &&
            catom->getInterface(catom->position.addY(-1))->isConnected()) {
        CanFillRightResponse_message *msg = new CanFillRightResponse_message();
        getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent(getScheduler()->now() + MSG_TIME, msg, catom->getInterface(catom->position.addY(-1))));
        numberMessagesToAddBlock++;
    }
}

void Neighborhood::addAllNeighbors()
{
    for (int i = 0; i < 12; i++) {
        Cell3DPosition neighborGridPos = catom->position;
        int *neighborPosPointer = (catom->position[2]%2) ? NeighborRestriction::neighborDirectionsOdd[i] : NeighborRestriction::neighborDirectionsEven[i];

        neighborGridPos.pt[0] += neighborPosPointer[0];
        neighborGridPos.pt[1] += neighborPosPointer[1];
        neighborGridPos.pt[2] += neighborPosPointer[2];
        //if (neighborGridPos[2] != catom->position[2])
            //continue;
        addNeighbor(neighborGridPos);
    }
}

bool Neighborhood::addFirstNeighbor()
{
    for (int i = 0; i < 12; i++) {
        Cell3DPosition neighborGridPos = catom->position;
        int *neighborPosPointer = (catom->position[2]%2) ? NeighborRestriction::neighborDirectionsOdd[i] : NeighborRestriction::neighborDirectionsEven[i];

        neighborGridPos.pt[0] += neighborPosPointer[0];
        neighborGridPos.pt[1] += neighborPosPointer[1];
        neighborGridPos.pt[2] += neighborPosPointer[2];
        //if (neighborGridPos[2] != catom->position[2])
            //continue;
        if (addNeighbor(neighborGridPos))
            return true;
    }
    return false;
}

bool Neighborhood::addNeighbor(Cell3DPosition pos)
{
    Catoms3D::Catoms3DWorld *world = Catoms3D::Catoms3DWorld::getWorld();
    NeighborRestriction neighbors;
    if (world->lattice->isFree(pos) && BlockCode::target->isInTarget(pos)) {
        Color c = BlockCode::target->getTargetColor(pos);
        if (neighbors.isPositionBlockable(pos))
            //world->addBlock(0, blockCodeBuilder, pos, LIGHTGREY, 0, false);
            world->addBlock(0, blockCodeBuilder, pos, c, 0, false);
        else if (neighbors.isPositionBlocked(pos)) {
            world->addBlock(0, blockCodeBuilder, pos, c, 0, false);
            numberBlockedModules++;
            cout << "number of blocked modules = " << numberBlockedModules << endl;
            cout << "---- ERROR ----\nPosition " << pos << " blocked" << endl;

            //std::this_thread::sleep_for(std::chrono::milliseconds(100000));
        }
        else {
            //world->addBlock(0, blockCodeBuilder, pos, LIGHTGREY, 0, false);
            world->addBlock(0, blockCodeBuilder, pos, c, 0, false);
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        world->linkBlock(pos);
        return true;
    }
    return false;
}

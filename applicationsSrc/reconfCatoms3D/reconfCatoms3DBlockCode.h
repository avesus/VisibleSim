/*
 *  reconfCatoms3DBlockCode.h
 *
 *  Created on: 17 October 2016
 *  Author: Thadeu
 */

#ifndef RECONFCATOMS3DBLOCKCODE_H_
#define RECONFCATOMS3DBLOCKCODE_H_

#define NEW_CATOM_MSG_ID	9001
#define NEW_CATOM_RESPONSE_MSG_ID	9002
#define RIGHT_SIDE_COMPLETED_MSG_ID	9003
#define LEFT_SIDE_COMPLETED_MSG_ID	9004

#include <queue>
#include <set>
#include "catoms3DBlockCode.h"
#include "catoms3DSimulator.h"
#include "catoms3DBlock.h"
#include "scheduler.h"
#include "events.h"
#include "csgUtils.h"
#include "target.h"
#include "seed.h"
#include "syncRequest.h"
#include "syncResponse.h"
#include "directions.h"

enum RECONF_STATUS { WAITING, READY };
enum SIDE_COMPLETED { LEFT, RIGHT };

class SyncRoute;

class ReconfCatoms3DBlockCode : public Catoms3D::Catoms3DBlockCode {
public:
	Scheduler *scheduler;
	Catoms3D::Catoms3DBlock *catom;
    Vector3D worldPosition;

    // CSG variables
    static CSGNode *csgRoot;
    static BoundingBox boundingBox;
    CsgUtils csgUtils;
    
    // Reconfiguration Variables
    static Seed *root;
    set<bID> lineSeeds;
    int numberSeedsLeft;
    int numberSeedsRight;
    bID lineParent;
    bool leftCompleted;
    bool rightCompleted;
    int currentLine;
    SyncRequest syncRequest;
    SyncResponse syncResponse;
    map<bID, SyncRoute> syncRoutes;

	ReconfCatoms3DBlockCode(Catoms3D::Catoms3DBlock *host);
	~ReconfCatoms3DBlockCode();

	void startup();
	void processLocalEvent(EventPtr pev);
    Vector3D getWorldPosition(Cell3DPosition gridPosition);
    void createCSG();
    void sendCSGMessage();
    void addNeighbor(Cell3DPosition pos);
    void addNeighbors();
    void addNeighborsOnXAxis();
    bool isSeed();
    void tryAddNextLineNeighbor();
    bool needSync();
    void checkLineCompleted();

    void sendMessageLineCompleted(SIDE_COMPLETED side);
    void sendMessageToGetNeighborInformation();

	static BlockCode *buildNewBlockCode(BuildingBlock *host);
private:
    bool cellHasBlock(const Cell3DPosition &pos);
};

class New_catom_message : public Message {
public:
    New_catom_message() { id = NEW_CATOM_MSG_ID; };
};

class New_catom_response_message : public Message {
public:
    int currentLine;
    bID lineParent;
    set<bID> lineSeeds;
    bool leftCompleted, rightCompleted;
    int numberSeedsLeft, numberSeedsRight;
    New_catom_response_message() { id = NEW_CATOM_RESPONSE_MSG_ID; };
};

class Right_side_completed_message : public Message {
public:
    set<bID> seeds;
    int numberSeedsRight;
    Right_side_completed_message(set<bID> s, int nSeedsRight) : seeds(s), numberSeedsRight(nSeedsRight) { id = RIGHT_SIDE_COMPLETED_MSG_ID; };
};

class Left_side_completed_message : public Message {
public:
    set<bID> seeds;
    int numberSeedsLeft;
    Left_side_completed_message(set<bID> s, int nSeedsLeft) : seeds(s), numberSeedsLeft(nSeedsLeft) { id = LEFT_SIDE_COMPLETED_MSG_ID; };
};

#endif /* RECONFCATOMS3DBLOCKCODE_H_ */

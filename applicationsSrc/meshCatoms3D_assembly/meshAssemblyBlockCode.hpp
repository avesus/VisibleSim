/**
 * @file   meshCatoms3DBlockCode_mvmt.hpp
 * @author pthalamy <pthalamy@p3520-pthalamy-linux>
 * @date   Tue Jul 10 13:47:48 2018
 * 
 * @brief  
 * 
 * 
 */


#ifndef MESHCATOMS3DBLOCKCODE_H_
#define MESHCATOMS3DBLOCKCODE_H_

#include <queue>

#include "catoms3DBlockCode.h"
#include "catoms3DSimulator.h"
#include "catoms3DMotionRules.h"
#include "rotation3DEvents.h"
#include "catoms3DBlock.h"
#include "cell3DPosition.h"

#include "meshAssemblyMessages.hpp"
#include "meshRuleMatcher.hpp"

#include "meshAssemblyLocalRules.hpp"

#define IT_MODE_TILEROOT_ACTIVATION 1 

class MeshAssemblyBlockCode : public Catoms3D::Catoms3DBlockCode {
private:
    static constexpr std::array<Cell3DPosition, 12> entryPointRelativePos =
    {
        Cell3DPosition(-1,-1,-1), // RevZ_EPL
        Cell3DPosition(0,-1,-1), // RevZ_R_EPL
        Cell3DPosition(1,-1,-1), // RZ_L_EPL
        Cell3DPosition(2,-1,-1), // RZ_EPL

        Cell3DPosition(2,0,-1), // RZ_R_EPL
        Cell3DPosition(2,1,-1), // Z_R_EPL
        Cell3DPosition(2,2,-1), // Z_EPL

        Cell3DPosition(1,2,-1), // Z_L_EPL
        Cell3DPosition(0,2,-1), // LZ_R_EPL
        Cell3DPosition(-1,2,-1), // LZ_EPL

        Cell3DPosition(-1,1,-1), // LZ_L_EPL
        Cell3DPosition(-1,0,-1) // RevZ_L_EPL
    };
public:
    static const uint B = 6;
    static uint X_MAX, Y_MAX, Z_MAX; // const
    static constexpr Cell3DPosition meshSeedPosition = Cell3DPosition(3,3,3);    
    static int nbCatomsInPlace;
    static int nbMessages;
    static Time t0;

    // For stats export
    pair<int, string> maxBitrate;    
    pair<Time, int> rate = { 0, 0 };
    
    static constexpr std::array<Cell3DPosition, 6> incidentTipRelativePos =
    {
        Cell3DPosition(0,0,-1), // ZBranch
        Cell3DPosition(1,1,-1), // RevZBranch
        Cell3DPosition(1,0,-1), // LZBranch
        Cell3DPosition(0,1,-1), // RZBranch
        Cell3DPosition(-1,0,0), // XBranch
        Cell3DPosition(0,-1,0) // YBranch
    };
               
    inline static Time getRoundDuration() {
        return (Rotations3D::ANIMATION_DELAY * Rotations3D::rotationDelayMultiplier
                + Rotations3D::COM_DELAY) + 20128;// + (getScheduler()->now() / 1000);
    }

    inline const Cell3DPosition& getEntryPointRelativePos(MeshComponent mc) const {
        VS_ASSERT_MSG(mc >= RevZ_EPL, "Attempting to getEntryPointRelativePos for non EPL component");
        return entryPointRelativePos[mc - RevZ_EPL];
    }
    
    inline const Cell3DPosition getTileRelativePosition() const {
        return catom->position - coordinatorPos;
    }
    
    int debugColorIndex = 0;
    
    Scheduler *scheduler;
    World *world;
    Lattice *lattice;
    Catoms3D::Catoms3DBlock *catom;
    MeshCoating::MeshRuleMatcher *ruleMatcher;

    /** MOTION COORDINATION **/
    bool greenLightIsOn = true;
    bool moduleAwaitingGo = false;
    Cell3DPosition awaitingModulePos = Cell3DPosition(-1, -1, -1);
    Cell3DPosition actuationTargetPos;
    Cell3DPosition stepTargetPos;
    
    bool rotating = false;

    bool isAdjacentToPosition(const Cell3DPosition& pos) const;
    // NOTE: what if there is more than 1?
    Catoms3DBlock* findTargetLightAmongNeighbors(const Cell3DPosition& targetPos) const;
    void setGreenLightAndResumeFlow();
    /**                     **/

    
    BranchIndex branch;
    AgentRole role;
    Cell3DPosition coordinatorPos;
    Cell3DPosition targetPosition;
    
    Time startTime;
    
    // Free Agent Vars
    short step = 1; // For moving FreeAgents
    bool tileInsertionAckGiven = false; // for tile insertion coordination at HBranch tips

    // Support Agent Vars
    Cell3DPosition branchTipPos; // For Support role only        
    
    // Coordinator Vars
    std::array<int, 4> catomsSpawnedToVBranch = { 0, 0, 0, 0 }; // Number of catoms fed to each vertical branch
    std::array<int, 6> catomsReqByBranch = {-1,-1,-1,-1,-1,-1}; // We could have -1 if branch should not be grown
    std::array<bool, 6> moduleReadyOnEPL = {0}; //<! keeps track of modules which arrived on Tile Entry Point Locations

    
    /** 
     * Finds the next target position that a module arriving at EPL epl should move to, 
     *  and return it
     * @param epl entry point location to evaluate
     * @return the next target cell that should be filled by catom at epl
     */
    const Cell3DPosition getNextTargetForEPL(MeshComponent epl);
    
    // TargetCSG *target;
    MeshAssemblyBlockCode(Catoms3D::Catoms3DBlock *host);
    ~MeshAssemblyBlockCode();          
    
    /**
     * \brief Global message handler for this instance of the blockcode
     * \param msg Message received b
y the module
     * \param sender Connector that has received the message and hence that is connected to the sender */
    void processReceivedMessage(MessagePtr msg, P2PNetworkInterface* sender);
    
    void startup();
    void processLocalEvent(EventPtr pev);
    void onBlockSelected();

    static BlockCode *buildNewBlockCode(BuildingBlock *host) {
        return (new MeshAssemblyBlockCode((Catoms3DBlock*)host));
    }

    /** 
     * Add initial sandbox modules to the lattice
     */
    void initializeSandbox();
    static bool sandboxInitialized;
    
    /** 
     * Transforms a shifted grid position into a mesh absolute position.
     * @note This has to be used due to the mesh seed being offsetted in order to leave space 
     *  for spawning modules 
     * @param pos position to normalize
     * @return the corresponding position of pos in the coordinate system of the mesh
     */
    static const Cell3DPosition norm(const Cell3DPosition& pos);

    /** 
     * Inverse function of norm
     * @note This has to be used due to the mesh seed being offsetted in order to leave space 
     *  for spawning modules 
     * @param pos position to denormalize
     * @return the corresponding position of pos in the absolute coordinate system 
     */
    static const Cell3DPosition denorm(const Cell3DPosition& pos);

    /** 
     * Transforms an absolute position into a tile-relative position
     * @param pos position to relatify
     * @return the corresponding position of pos in the tile-relative coordinate system 
     */
    const Cell3DPosition relatify(const Cell3DPosition& pos);

    /** 
     * Inverse operation of relatify
     */
    const Cell3DPosition derelatify(const Cell3DPosition& pos);
    
    void updateOpenPositions();
        
    /** 
     * Computes the entry point direction identifier corresponding to neighbor cell pos
     * @param pos a cell that is adjacent to the current module 
     * @attention pos is not a relative position but an absolute one!
     * @return entry point direction id corresponding to pos or -1 if pos not adjacent to module or if cell is not on a module's lower connectors
     */
    short getEntryPointLocationForCell(const Cell3DPosition& pos);

    /** 
     * Adds a new mesh component to the grid at its predetermined entry point, 
     *  and prepare to send it its coordinates
     * @param mc mesh component of catom to add
     */
    void handleMeshComponentInsertion(MeshComponent mc);

    bool handleModuleInsertionToBranch(BranchIndex bid);
    const Cell3DPosition getEntryPointForModuleOnBranch(BranchIndex bid);
    
    /** 
     * Finds an entry point index for a catom required to fill component mc
     * @param mc mesh component type of the catom
     * @return entry point position / spawn location for component mc
     */
    const Cell3DPosition getEntryPointForMeshComponent(MeshComponent mc);

    /** 
     * Checks if all the incident branches around a tile root are complete
     * @param pos position of the tile root
     * @return true if all incident branches are complete, false otherwise
     */
    bool incidentBranchesToRootAreComplete(const Cell3DPosition& pos);

    /** 
     * Checks if branch tip from branch bi is in place around tile root at trp
     * @param trp tile root position
     * @param bi index of the incident branch that needs checking
     * @return true if the tip of branch bi is in place next ot trp
     */
    bool isIncidentBranchTipInPlace(const Cell3DPosition& trp, BranchIndex bi);

    void scheduleRotationTo(const Cell3DPosition& pos);

    void matchRulesAndRotate();
    void matchRulesAndProbeGreenLight();

    void initializeTileRoot();
    void initializeSupportModule();

    /** 
     * @param epl Entry point location name of the desired entry point
     * @return absolute position of the requested entry point location
     */
    const Cell3DPosition getEntryPointPosition(MeshComponent epl) const;

    /** 
     * Indicates whether position pos is an entry point location
     * @param pos position to evaluate
     * @return true if pos is an EPL, false otherwise
     */
    bool isOnEntryPoint(const Cell3DPosition& pos) const;

    /** 
     * Locates a route for communicating with the tile root and sends 
     *  a RequestTargetCell message through that route
     * @return true if a message could be sent, false otherwise
     */
    bool requestTargetCellFromTileRoot();

    /** 
     * Removes the next element in the target cell queue of the EPL for component comp
     * @param comp component for which the EPL queue must be popped
     */
    void discardNextTargetForComponent(MeshComponent comp);

    // TODO:
    void feedBranches();

    /** 
     * @return true if catom is on the lowest tile layer, false otherwise
     */
    bool isAtGroundLevel();
    /**
     * @copydoc BlockCode::sendMessage
     * @note This is only used for logging sent messages, it calls 
     *  BlockCode::sendMessage immediatly after 
     */
    virtual int sendMessage(HandleableMessage *msg,P2PNetworkInterface *dest,
                            Time t0,Time dt);

    /**
     * @copydoc BlockCode::sendMessage
     * @note This is only used for logging sent messages, it calls 
     *  BlockCode::sendMessage immediatly after 
     */
    virtual int sendMessage(Message *msg,P2PNetworkInterface *dest,
                            Time t0,Time dt);

    void log_send_message() const;
    void updateMsgRate();

    /** 
     * @return true if module at entry point location epl is immediately required for the construction, or false if it should wait 
     */
    bool moduleAtEPLIsRequiredAtOnce(MeshComponent epl);

    /** 
     * Checks for every module awaiting at an EPL, if that module is required for the next construction steps,and if so sends it a PROVIDE_TARGET_CELL message to resume the flow. 
     */
    void awakenPausedModules();
    std::array<bool, 12> moduleAwaitingOnEPL = {0};

    std::queue<MeshComponent> targetQueueForEPL[12] = {
        queue<MeshComponent>({
                RevZ_1, RevZ_2, RevZ_3, RevZ_4, RevZ_5,
                Z_R_EPL, Z_L_EPL, Z_R_EPL
            }), // RevZ_EPL
        {}, // RevZ_R_EPL
        {},  // RZ_L_EPL
        queue<MeshComponent>({
                S_RevZ,
                X_2, X_3, X_4, X_5,
                RZ_1, RZ_2, RZ_3, RZ_4, RZ_5,
                LZ_R_EPL
            }), // RZ__EPL x
        queue<MeshComponent>({ S_RZ }), // RZ__R_EPL
        queue<MeshComponent>({ X_1 }), // Z__R_EPL x 
        queue<MeshComponent>({
                Z_1, Z_2, Z_3, Z_4, Z_5
            }), // Z__EPL 
        queue<MeshComponent>({ Y_1 }), // Z__L_EPL x
        queue<MeshComponent>({ S_RZ }), // LZ__R_EPL x
        queue<MeshComponent>({
                S_Z,
                Y_2, Y_3, Y_4, Y_5,
                LZ_1, LZ_2, LZ_3, LZ_4, LZ_5,
                RZ_R_EPL
            }), // LZ__EPL x
        {}, // LZ__L_EPL x
        {} // RevZ__L_EPL x   
    };
};

#endif /* MESHCATOMS3DBLOCKCODE_H_ */
    

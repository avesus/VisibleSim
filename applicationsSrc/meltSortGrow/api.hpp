
#ifndef __DESIGN_HPP_
#define __DESIGN_HPP_

#include <set>
#include <list>

#include "catoms3DMotionRules.h"
#include "catoms3DWorld.h"

#include "pathHop.hpp"

using namespace Catoms3D;

/**
   \brief blablabla
   \param x 
   \return */
// void function(...);

class API { 
    static inline Catoms3DMotionRules* getMotionRules() {
        return Catoms3DWorld::getWorld()->getMotionRules();
    }
public:

    /** 
        \brief Add a new path entry at the back of the path hop list
        \param catom the module for which a hop entry needs to be created
        \param path the path hop list to which the hop entry will be appended
        \return false is the catom cannot be connected to the path, true otherwise
        \attention path must be initialized and thus contain at least one entry
        \todo PTHA
    **/
    static bool addModuleToPath(Catoms3DBlock *catom, list<PathHop>& path);

    
    /** 
        \brief Builds a sequence of rotation that goes from a catom's current location to the target of the path
        \param catom the module that will be performing the rotations
        \param pivotCon id of the connector of the pivot that is connected to the catom module
        \param path the path hop list that will be used to build the motion sequence
        \param rotations the output motion sequence 
        \return false is the catom cannot be reach the path target, true otherwise
        \attention path must be initialized and thus contain at least one entry
        \todo PTHA
    **/
    static bool buildRotationSequenceToTarget(Catoms3DBlock *pivot,
                                              short pivotCon,
                                              list<PathHop>& path,
                                              list<Catoms3DMotionRulesLink*>& rotations);
    
/**
   \brief For a given Catoms3D module acting as a pivot, enumerates all the possible catom surface links that
   a catom connected to the pivot on any connector could use to reach connector conId
   \param pivot The module acting as a pivot for the link search, which will be the point-of-reference for the connector search
   \param conId Identifier of the target connector of the all the link in the results set 
   \param links Reference to the container that will hold the set of all individual surface links around pivot that lead from one connector to connector conId. 
   \return true if at least one link exists, false otherwise
   \remarks This function might not be of any use, as \ref{getAllLinks} might need to be used by default 
   \remarks Changer signature pour renvoyer pathStruct plutot que les links **/
    static bool getAllLinksToConnector(const Catoms3DBlock *pivot,
                                       short conId,
                                       vector<Catoms3DMotionRulesLink*>&links);
    
/**
   \brief For a given Catoms3D module acting as a pivot, enumerates all the possible catom surface links that
   a catom connected to the pivot on any connector could use
   \param pivot The module acting as a pivot for the link search, which will be the point-of-reference for the connector search
   \param links Reference to the container that will hold the set of all individual surface links around pivot that lead from any connector to any other. 
   \return true if at least one link exists, false otherwise
   It can be seen as a graph, where the resulting set is the list of all the edges connecting the connectors of the pivot module, acting as vertices. 
   \remarks This is the fallback function that can be used to find an indirect link from any connector to a target one in case no direct link could be found using \ref{getAllLinksToConnector}  
 */
    static bool getAllLinks(const Catoms3DBlock *pivot,
                            vector<Catoms3DMotionRulesLink*>&links);

/**
   \brief Given a set of motion rules link passed as argument, searches a path (sequence of individual rotations) that leads from connector conFrom to connector conTo
   \param motionRulesLinks a set of surface links between connectors of a pivot module that another module can follow to rotate
   \param conFrom the source connector of the desired connector path
   \param conTo the destination connector of the desired connector path
   \return an ordered list of individual links that can be followed by a module to move from conFrom to conTo, or list.end() if no path has been found 
   \remarks The best option would be to find the fastest path from conFrom to conTo */
    static bool findConnectorsPath(const vector<Catoms3DMotionRulesLink*>& motionRulesLinks,
                                   short conFrom,
                                   short conTo,
                                   list<Catoms3DMotionRulesLink*>& paths);

/**
   \brief Given a catom used as pivot, searches a path (sequence of individual rotations) that leads from connector conFrom to connector any connector of the input set
   \param motionRulesLinks a set of surface links between connectors of a pivot module that another module can follow to rotate
   \param conFrom the source connector of the desired connector path
   \param consTo the destinations connector of the desired connector path \attention{sorted in increasing distance to the global path target}
   \param shortestPath  container that will populated as an ordered list of individual links that can be followed by a module to move from conFrom to a connector of conTo, or list.end() if no path has been found 
   \return true if a path exists, false otherwise
   \remarks The best option would be to find the fastest path from conFrom to a conTo connector, and also consider the distance of conFrom to the global target 
   \attention the consTo input vector should be ordered by distance to some target in order for this function to return the shortest path to that target */
    static bool findConnectorsPath(const vector<Catoms3DMotionRulesLink*>& motionRulesLinks,
                                   short conFrom,
                                   const std::vector<short>& consTo,
                                   list<Catoms3DMotionRulesLink*>& shortestPath);

/**
   \brief Given a set of motion rules link passed as argument, deduce a set of all connectors for which a path exists to connector conTo
   \param motionRulesLinks a set of surface links between connectors of a pivot module that another module can follow to rotate
   \param conTo the desired destination of the connector path for which we want to find all potential motion sources
   \return a set of all connectors for which a path to conTo exists 
   \remarks An enhancement could be to return a list of connectors sorted by increasing distance to conTo */
    static bool findPathConnectors(const vector<Catoms3DMotionRulesLink*>& motionRulesLinks,
                                   short conTo,
                                   set<short>& pathConnectors);

/**
   \brief Given a catom used as pivot, deduce a set of all connectors for which a path exists to connector conTo
   \param conTo the desired destination of the connector path for which we want to find all potential motion sources
   \return a set of all connectors for which a path to conTo exists 
   \remarks An enhancement could be to return a list of connectors sorted by increasing distance to conTo */
    static bool findPathConnectors(const Catoms3DBlock *pivot, short conTo,
                                   set<short>& pathConnectors);

/**
   \brief Given a catom used as pivot, deduce a set of all connectors for which a path exists to any connector of input set consTo
   \param consTo the desired destinations of the connector path for which we want to find all potential motion sources
   \return a set of all connectors for which a path to conTo exists 
   \remarks An enhancement could be to return a list of connectors sorted by increasing distance to conTo */
    static bool findPathConnectors(const Catoms3DBlock *pivot,
                                   const set<short>& consTo,
                                   set<short>& pathConnectors);

/**
   \brief Given a set of connector IDs and the orientation of the module to which they belong, determine which connectors of the current module are adjacent to those contained in the input set. 
   \remarks Two connectors of opposing modules are adjacent if a third module could fill a position where both connectors are connected to that module at the same time
   \param pathConnectors a set of connector IDs belonging to a path to some target
   \param pathOrientationCode the orientation code of the module to which the path connectors belong
   \param orientationCode the orientation code of the module for which we wish to determine the connectors adjacent to the input set
   \return a set of all connectors of module with orientation orientationCode that are adjacent to the connectors in the input set, set.end() if none exist */
    static bool findAdjacentConnectors(const PathHop hop,
                                       short pathOrientationCode,
                                       short orientationCode,
                                       set<short>& adjacentConnectors);

// NOT MSG SPECIFIC

/**
   \brief Returns the ID of the connector of a given catom that is adjacent to the input lattice position
   \remarks Two connectors of opposing modules are adjacent if a third module could fill a position where both connectors are connected to that module at the same time
   \param catom the module for which we want to find which connector is adjacent to the input position
   \param cell a cell position that belongs to the simulated lattice
   \return the ID of the connector of module catom that is adjacent to the input position, or -1 if that position is not in the neighborhood of catom 
   \todo BPIR? */
    static bool getConnectorsAdjacentToCell(Catoms3DBlock *catom,
                                            const Cell3DPosition cell,
                                            set<short>& pathConnectors);

/**
   \brief Returns the ID of the connector of a given catom that is connected to the input lattice position
   \param catom the module for which we want to find which connector is adjacent to the input position
   \param cell a cell cell that belongs to the simulated lattice
   \return the ID of the connector of module catom that is adjacent to the input cell, or -1 if that cell is not in the neighborhood of catom */
    static short getConnectorForCell(Catoms3DBlock *catom, const Cell3DPosition cell);
// Already implemented in Catoms3DBlock::getConnectorId(cell);
};

#endif

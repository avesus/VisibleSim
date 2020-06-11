/*
 * smartBlocksBlock.h
 *
 *  Created on: 12 avril 2013
 *      Author: ben
 */

#ifndef SMARTBLOCKSBLOCK_H_
#define SMARTBLOCKSBLOCK_H_

#include "base/buildingBlock.h"
#include "robots/smartBlocks/smartBlocksBlockCode.h"
#include "robots/smartBlocks/smartBlocksGlBlock.h"
#include "comm/network.h"
#include "grid/lattice.h"
#include "replay/replayExporter.h"

namespace SmartBlocks {

class SmartBlocksBlockCode;

class SmartBlocksBlock : public BaseSimulator::BuildingBlock {
public:
    bool wellPlaced,_isBorder,_isTrain,_isSingle;

    SmartBlocksBlock(int bId, BlockCodeBuilder bcb);
    ~SmartBlocksBlock();

    inline P2PNetworkInterface *getInterface(SLattice::Direction d) const {
        return P2PNetworkInterfaces[d];
    }
    P2PNetworkInterface *getP2PNetworkInterfaceByRelPos(const Cell3DPosition &pos) const;
    P2PNetworkInterface *getP2PNetworkInterfaceByDestBlockId(bID id) const;

    Cell3DPosition getPosition(SLattice::Direction d) const;
    int getDirection( P2PNetworkInterface*) const override;
    inline void getGridPosition(int &x,int &y) const {
        x = int(position[0]); y=int(position[1]);
    };

    /**
     * @copydoc BuildingBlock::addNeighbor
     */
    virtual void addNeighbor(P2PNetworkInterface *ni, BuildingBlock* target) override;
    /**
     * @copydoc BuildingBlock::removeNeighbor
     */
    virtual void removeNeighbor(P2PNetworkInterface *ni) override;

    void setDisplayedValue(int n);

    /**
     * @param dest
     * @return true if the module can move to position dest
     *         (it is in the grid, adjacent and free)
     */
    bool canMoveTo(const Cell3DPosition& dest) const override;

    /**
     * @brief Moves the current module to dest if possible (it is in the grid, adjacent and free)
     * @param dest
     * @return true module has initiated a motion to dest, false if it is not possible
     */
    bool moveTo(const Cell3DPosition& dest) override;

    /**
     * Serializes (converts to a stream of bits) relevant data from the SmartBlock object
     *  for the purpose of simulation replay.
     *
     *  Overrides BuildingBlock serializer, serializes as:
     *   <id><position><orientation><color><display_value>
     *
     * @see BuildingBlock::serialize
     * @param bStream output binary stream
     */
    virtual void serialize(std::ofstream &bStream) override;

    /**
     * Clear-text equivalent of the BuildingBlock::serialize function, for debugging purpose
     *  Overrides BuildingBlock serializer_cleartext function
     *
     * @see SmartBlock::serialize
     * @param dbStream output binary stream
     */
    virtual void serialize_cleartext(std::ofstream &dbStream)  override;
};

}

#endif /* SMARTBLOCKSBLOCK_H_ */

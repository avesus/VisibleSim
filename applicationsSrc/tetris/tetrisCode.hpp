#ifndef tetrisCode_H_
#define tetrisCode_H_

#include "robots/blinkyBlocks/blinkyBlocksSimulator.h"
#include "robots/blinkyBlocks/blinkyBlocksWorld.h"
#include "robots/blinkyBlocks/blinkyBlocksBlockCode.h"

static const int HEIGHTMSG_MSG_ID = 1001;
static const int WIDTHMSG_MSG_ID = 1002;

using namespace BlinkyBlocks;

class TetrisCode : public BlinkyBlocksBlockCode {
private:
	BlinkyBlocksBlock *module = nullptr;
public :
	TetrisCode(BlinkyBlocksBlock *host);
	~TetrisCode() {};

/**
  * This function is called on startup of the blockCode, it can be used to perform initial
  *  configuration of the host or this instance of the program.
  * @note this can be thought of as the main function of the module
  */
    void startup() override;

/**
  * @brief Message handler for the message 'heightMsg'
  * @param _msg Pointer to the message received by the module, requires casting
  * @param sender Connector of the module that has received the message and that is connected to the sender
  */
   void myHeightMsgFunc(std::shared_ptr<Message>_msg,P2PNetworkInterface *sender);

/**
  * @brief Message handler for the message 'widthMsg'
  * @param _msg Pointer to the message received by the module, requires casting
  * @param sender Connector of the module that has received the message and that is connected to the sender
  */
   void myWidthMsgFunc(std::shared_ptr<Message>_msg,P2PNetworkInterface *sender);

/**
  * @brief Handler for all events received by the host block
  * @param pev pointer to the received event
  */
    void processLocalEvent(std::shared_ptr<Event> pev) override;

/**
  * User-implemented keyboard handler function that gets called when
  *  a key press event could not be caught by openglViewer
  * @param c key that was pressed (see openglViewer.cpp)
  * @param x location of the pointer on the x axis
  * @param y location of the pointer on the y axis
  * @note call is made from GlutContext::keyboardFunc (openglViewer.h)
  */
    void onUserKeyPressed(unsigned char c, int x, int y) override;

/*****************************************************************************/
/** needed to associate code to module                                      **/
	static BlockCode *buildNewBlockCode(BuildingBlock *host) {
	    return(new TetrisCode((BlinkyBlocksBlock*)host));
	};
/*****************************************************************************/
};

#endif /* tetrisCode_H_ */
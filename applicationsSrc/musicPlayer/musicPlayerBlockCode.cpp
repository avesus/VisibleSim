/*
 * BbCycleBlockCode.cpp
 *
 *  Created on: 26 mars 2013
 *      Author: dom
 */

#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <stdio.h> 
#include "scheduler.h"
#include "network.h"
#include "musicPlayerBlockCode.h"
#include "musicPlayerEvents.h"
#include "trace.h"
#include <vector>

using namespace std;
using namespace BlinkyBlocks;

#define SYNC_PERIOD (2*1000*1000)
#define COLOR_CHANGE_PERIOD_USEC (2*1000*1000)
#define SIMULATION_DURATION_USEC (10*60*1000*1000)

MusicPlayerBlockCode::MusicPlayerBlockCode(BlinkyBlocksBlock *host): BlinkyBlocksBlockCode(host) {
	OUTPUT << "musicPlayerBlockCode constructor" << endl;
}

MusicPlayerBlockCode::~MusicPlayerBlockCode() {
	OUTPUT << "musicPlayerBlockCode destructor" << endl;
}

void MusicPlayerBlockCode::init() {
	BlinkyBlocksBlock *bb = (BlinkyBlocksBlock*) hostBlock;
	stringstream info;
	std::vector<Note> song(Score());
	Color c = PINK;
	block2Answer=NULL;
	cycle=true;
	if(hostBlock->blockId==1){
		idMessage=0;
		BlinkyBlocks::getScheduler()->schedule(new SpreadSongEvent(BlinkyBlocks::getScheduler()->now(),hostBlock));	
		BlinkyBlocks::getScheduler()->schedule(new SynchronizeEvent(BlinkyBlocks::getScheduler()->now()+SYNC_PERIOD,hostBlock));	
		info << "This block is the Master Block" << endl;
	}
	BlinkyBlocks::getScheduler()->schedule(new SetColorEvent(COLOR_CHANGE_PERIOD_USEC,bb,c));
	BlinkyBlocks::getScheduler()->trace(info.str(),hostBlock->blockId);
}

void MusicPlayerBlockCode::startup() {
	stringstream info;
	delay=0;
	info << "  Starting MusicPlayerBlockCode in block " << hostBlock->blockId;
	init();
}

void MusicPlayerBlockCode::processLocalEvent(EventPtr pev) {
	stringstream info;
	MessagePtr message;
	BlinkyBlocksBlock *bb = (BlinkyBlocksBlock*) hostBlock;
	info.str("");
	
	OUTPUT << bb->blockId << " processLocalEvent: date: "<< BaseSimulator::getScheduler()->now() << " process event " << pev->getEventName() << "(" << pev->eventType << ")" << ", random number : " << pev->randomNumber << endl;
	
	switch (pev->eventType) {
		case EVENT_SET_COLOR:
			{
			Color color = (boost::static_pointer_cast<SetColorEvent>(pev))->color;
			bb->setColor(color);
			info << "set color "<< color << endl;
			if (cycle){
				color = BLUE;
				cycle = false;
			}
			else{
				color = RED;
				cycle = true;
			}
			BlinkyBlocks::getScheduler()->schedule(new SetColorEvent(bb->getTime()+COLOR_CHANGE_PERIOD_USEC+delay,bb,color));
			info << "Setcolor scheduled" << endl;
			}
			break;
		case EVENT_SPREAD_SONG:
			{
				
			}
			break;
		case EVENT_NI_RECEIVE:
			{
			message = (boost::static_pointer_cast<NetworkInterfaceReceiveEvent>(pev))->message; 
			P2PNetworkInterface * recvInterface = message->destinationInterface;
			switch(message->id){
				case SYNC_MSG_ID: 
					{
					SynchroMessage_ptr recvMessage = boost::static_pointer_cast<SynchroMessage>(message);
					if (!received[recvMessage->idSync]){
						received[recvMessage->idSync]=true;
						block2Answer=recvInterface;
						sendClockToNeighbors(block2Answer,recvMessage->nbhop+1,recvMessage->time,recvMessage->idSync); 	
						delay = recvMessage->time - bb->getTime() + 6000*recvMessage->nbhop;
						info<<"synchronized with delay : "<< delay << endl;
						}
					}
					break;					
				default:
					break;
				}
			}
			break;
		case EVENT_SYNC:
			{
			received[idMessage]=true;
			sendClockToNeighbors(NULL,1,bb->getTime(),idMessage);
			idMessage++;
			uint64_t nextSync = bb->getTime()+SYNC_PERIOD;
			BlinkyBlocks::getScheduler()->schedule(new SynchronizeEvent(nextSync,bb));
			info << "scheduled synchro" << endl;
			}
			break;
		default:
			ERRPUT << "*** ERROR *** : unknown local event" << endl;
			break;
		}
		BlinkyBlocks::getScheduler()->trace(info.str(),hostBlock->blockId);
}

BlinkyBlocks::BlinkyBlocksBlockCode* MusicPlayerBlockCode::buildNewBlockCode(BlinkyBlocksBlock *host) {
	return(new MusicPlayerBlockCode(host));
}

void MusicPlayerBlockCode::sendClockToNeighbors (P2PNetworkInterface *p2pExcept, int hop, uint64_t clock, int id){
	P2PNetworkInterface * p2p;
	BlinkyBlocksBlock *bb = (BlinkyBlocksBlock*) hostBlock;
	
	for (int i=0; i<6 ; i++) {
	p2p = bb->getInterface(NeighborDirection::Direction(i));
		if (p2p->connectedInterface && p2p!=p2pExcept){	
			SynchroMessage *message = new SynchroMessage(clock, hop, id);
			BlinkyBlocks::getScheduler()->schedule(new NetworkInterfaceEnqueueOutgoingEvent (BlinkyBlocks::getScheduler()->now(), message, p2p));
		}
	}
}

void MusicPlayerBlockCode::sendSongToNeighbors (P2PNetworkInterface *p2pExcept){
	P2PNetworkInterface * p2p;
	BlinkyBlocksBlock *bb = (BlinkyBlocksBlock*) hostBlock;


}

std::vector<Note> MusicPlayerBlockCode::Score(){ //We can take a midi file and break it down or just fill the score manually
	std::vector<Note> score;
	score.push_back(Note(1000,523.25,2000));
	score.push_back(Note(2500,523.25,3500));
	score.push_back(Note(4000,587.33,5000));
	score.push_back(Note(5500,659.26,6500)); 
	return score;
}

SynchroMessage::SynchroMessage(uint64_t t, int hop, int ids) :Message(){
	id = SYNC_MSG_ID;
	idSync=ids;
	time = t;
	nbhop = hop;
}

SynchroMessage::~SynchroMessage(){
}

Note::Note(int sTime, unsigned int freq, unsigned int time){
	startTime=sTime;
	frequency=freq;
	duration=time;
}

Note::~Note(){}

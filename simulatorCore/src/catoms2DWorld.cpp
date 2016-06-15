

/*
 * robotBlockWorld.cpp
 *
 *  Created on: 12 janvier 2014
 *      Author: Benoît
 */

#include <iostream>
#include <string>
#include <stdlib.h>
#include "catoms2DWorld.h"
#include "catoms2DBlock.h"
#include "catoms2DScheduler.h"
#include "trace.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include "catoms2DSimulator.h"

using namespace std;

namespace Catoms2D {
    
    const double M_SQRT3_2 = sqrt(3.0)/2.0;
    
    Catoms2DWorld::Catoms2DWorld(int slx,int sly,int slz, int argc, char *argv[]):World() {
        OUTPUT << "\033[1;31mCatoms2DWorld constructor\033[0m" << endl;
        gridSize[0]=slx;
        gridSize[1]=sly;
        gridSize[2]=slz;
        gridPtrBlocks = new Catoms2DBlock*[slx*sly*slz];
        
        // initialise grid of blocks
        int i=slx*sly*slz;
        Catoms2DBlock **ptr = gridPtrBlocks;
        while (i--) {
            *ptr=NULL;
            ptr++;
        }
        targetGrid=NULL;
        
        GlutContext::init(argc,argv);
        idTextureHexa=0;
        idTextureLines=0;
        blockSize[0]=1.0;
        blockSize[1]=5.0;
        blockSize[2]=1.0;
        objBlock = new ObjLoader::ObjLoader("../../simulatorCore/catoms2DTextures","catom2D.obj");
        objBlockForPicking = new ObjLoader::ObjLoader("../../simulatorCore/catoms2DTextures","catom2Dpicking.obj");
        objRepere = new ObjLoader::ObjLoader("../../simulatorCore/smartBlocksTextures","repere25.obj");
        camera = new Camera(-M_PI/2.0,M_PI/3.0,750.0);
        camera->setLightParameters(Vector3D(0,0,0),45.0,80.0,800.0,45.0,10.0,1500.0);
        camera->setTarget(Vector3D(0,0,1.0));
        
        menuId=0;
        numSelectedFace=0;
        numSelectedBlock=0;
    }
    
    Catoms2DWorld::~Catoms2DWorld() {
        OUTPUT << "Catoms2DWorld destructor" << endl;
        /*	block linked are deleted by world::~world() */
        delete [] gridPtrBlocks;
        delete [] targetGrid;
        delete objBlock;
        delete objBlockForPicking;
        delete objRepere;
        delete camera;
    }
    
    void Catoms2DWorld::createWorld(int slx,int sly,int slz, int argc, char *argv[]) {
        world = new Catoms2DWorld(slx,sly,slz,argc,argv);
    }
    
    void Catoms2DWorld::deleteWorld() {
        delete((Catoms2DWorld*)world);
    }
    
    void Catoms2DWorld::addBlock(int blockId, Catoms2DBlockCode *(*robotBlockCodeBuildingFunction)(Catoms2DBlock*),const Vector3D &pos,const Color &color,bool master) {
        
        if (blockId == -1) {
            map<int, BaseSimulator::BuildingBlock*>::iterator it;
            for(it = buildingBlocksMap.begin();
                it != buildingBlocksMap.end(); it++) {
                Catoms2DBlock* bb = (Catoms2DBlock*) it->second;
                if (it->second->blockId > blockId) blockId = bb->blockId;
            }
            blockId++;
        }
        Catoms2DBlock *robotBlock = new Catoms2DBlock(blockId,robotBlockCodeBuildingFunction);
        buildingBlocksMap.insert(std::pair<int,BaseSimulator::BuildingBlock*>(robotBlock->blockId, (BaseSimulator::BuildingBlock*)robotBlock));
        
        getScheduler()->schedule(new CodeStartEvent(getScheduler()->now(), robotBlock));
        
        Catoms2DGlBlock *glBlock = new Catoms2DGlBlock(blockId);
        tabGlBlocks.push_back(glBlock);
        robotBlock->setGlBlock(glBlock);
        
        robotBlock->setPosition(pos);
        robotBlock->setColor(color);
        robotBlock->isMaster=master;
        /*
********************************************************************
A ECRIRE AVEC LE MAILLAGE HEXAGONAL
        */
        int ix,iy,iz;
        ix = int(robotBlock->position.pt[0]);
        iy = int(robotBlock->position.pt[1]);
        iz = int(robotBlock->position.pt[2]);
        if (ix>=0 && ix<gridSize[0] &&
            iy>=0 && iy<gridSize[1] &&
            iz>=0 && iz<gridSize[2]) {
            setGridPtr(ix,iy,iz,robotBlock);
        } else {
            ERRPUT << "ERROR : BLOCK #" << blockId << " out of the grid !!!!!" << endl;
            exit(1);
        }
    }
    
    void Catoms2DWorld::connectBlock(Catoms2DBlock *block) {
        int ix,iy,iz;
        
        ix = int(block->position.pt[0]);
        iy = int(block->position.pt[1]);
        iz = int(block->position.pt[2]);
        setGridPtr(ix,iy,iz,block);
        OUTPUT << "Reconnection " << block->blockId << " pos ="<< ix << "," << iy << "," << iz << endl;
        
        linkBlock(ix,iy,iz);
        
        if (ix<gridSize[0]-1) linkBlock(ix+1,iy,iz);
        if (ix>0) linkBlock(ix-1,iy,iz);
        
        if (iz<gridSize[2]-1) linkBlock(ix,iy,iz+1);
        if (iz>0) linkBlock(ix,iy,iz-1);
        
        if (iz%2 == 1) {
            if (ix<gridSize[0]-1) {
                if (iz<gridSize[2]-1) linkBlock(ix+1,iy,iz+1);
                if (iz>0) linkBlock(ix+1,iy,iz-1);
            }
        } else {
            if (ix>0) {
                // x-1
                if (iz<gridSize[2]-1) linkBlock(ix-1,iy,iz+1);
                if (iz>0) linkBlock(ix-1,iy,iz-1);
            }
        }
    }
    
    void Catoms2DWorld::disconnectBlock(Catoms2DBlock *block) {
        P2PNetworkInterface *fromBlock,*toBlock;
        
        for(int i=0; i<6; i++) {
            fromBlock = block->getInterface(NeighborDirection::Direction(i));
            if (fromBlock && fromBlock->connectedInterface) {
                toBlock = fromBlock->connectedInterface;
                fromBlock->connectedInterface=NULL;
                toBlock->connectedInterface=NULL;
            }
        }
        int ix,iy,iz;
        ix = int(block->position.pt[0]);
        iy = int(block->position.pt[1]);
        iz = int(block->position.pt[2]);
        setGridPtr(ix,iy,iz,NULL);
        OUTPUT << getScheduler()->now() << " : Disconnection " << block->blockId <<
            " pos ="<< ix << "," << iy << "," << iz << endl;
    }
    
    void Catoms2DWorld::linkBlock(int ix, int iy, int iz) {
        Catoms2DBlock *ptrBlock = getGridPtr(ix,iy,iz);

        if (ptrBlock) {
            OUTPUT << "link block " << ptrBlock->blockId << endl;

            if (ix<gridSize[0]-1 && getGridPtr(ix+1,iy,iz)) {
                (ptrBlock)->getInterface(NeighborDirection::Right)->
                    connect(getGridPtr(ix+1,iy,iz)->getInterface(NeighborDirection::Left));
                OUTPUT << "connection #" << (ptrBlock)->blockId <<
                    " to #" << getGridPtr(ix+1,iy,iz)->blockId << endl;
            } else {
                (ptrBlock)->getInterface(NeighborDirection::Right)->connect(NULL);
            }

            if (ix>0 && getGridPtr(ix-1,iy,iz)) {
                (ptrBlock)->getInterface(NeighborDirection::Left)->
                    connect(getGridPtr(ix-1,iy,iz)->getInterface(NeighborDirection::Right));
                OUTPUT << "connection #" << (ptrBlock)->blockId <<
                    " to #" << getGridPtr(ix-1,iy,iz)->blockId << endl;
            } else {
                (ptrBlock)->getInterface(NeighborDirection::Left)->connect(NULL);
            }

            if (iz%2 == 1) {
                if (iz<gridSize[2]-1 && getGridPtr(ix,iy,iz+1)) {
                    (ptrBlock)->getInterface(NeighborDirection::TopLeft)->
                        connect(getGridPtr(ix,iy,iz+1)->getInterface(NeighborDirection::BottomRight));
                    OUTPUT << "connection #" << (ptrBlock)->blockId <<
                        " to #" << getGridPtr(ix,iy,iz+1)->blockId << endl;
                } else {
                    (ptrBlock)->getInterface(NeighborDirection::TopLeft)->connect(NULL);
                }
                if (iz>0 && getGridPtr(ix,iy,iz-1)) {
                    (ptrBlock)->getInterface(NeighborDirection::BottomLeft)->
                        connect(getGridPtr(ix,iy,iz-1)->getInterface(NeighborDirection::TopRight));
                    OUTPUT << "connection #" << (ptrBlock)->blockId <<
                        " to #" << getGridPtr(ix,iy,iz-1)->blockId << endl;
                } else {
                    (ptrBlock)->getInterface(NeighborDirection::BottomLeft)->connect(NULL);
                }
            } else {
                if (iz<gridSize[2]-1 && getGridPtr(ix,iy,iz+1)) {
                    (ptrBlock)->getInterface(NeighborDirection::TopRight)->
                        connect(getGridPtr(ix,iy,iz+1)->getInterface(NeighborDirection::BottomLeft));
                    OUTPUT << "connection #" << (ptrBlock)->blockId <<
                        " to #" << getGridPtr(ix,iy,iz+1)->blockId << endl;
                } else {
                    (ptrBlock)->getInterface(NeighborDirection::TopRight)->connect(NULL);
                }
                if (iz>0 && getGridPtr(ix,iy,iz-1)) {
                    (ptrBlock)->getInterface(NeighborDirection::BottomRight)->
                        connect(getGridPtr(ix,iy,iz-1)->getInterface(NeighborDirection::TopLeft));
                    OUTPUT << "connection #" << (ptrBlock)->blockId <<
                        " to #" << getGridPtr(ix,iy,iz-1)->blockId << endl;
                } else {
                    (ptrBlock)->getInterface(NeighborDirection::BottomRight)->connect(NULL);
                }
            }

            if (iz%2 == 1) {
                if (ix<gridSize[0]-1) {
                    // x+1
                    if (iz<gridSize[2]-1 && getGridPtr(ix+1,iy,iz+1)) {
                        (ptrBlock)->getInterface(NeighborDirection::TopRight)->
                            connect(getGridPtr(ix+1,iy,iz+1)->getInterface(NeighborDirection::BottomLeft));
                        OUTPUT << "connection #" << (ptrBlock)->blockId <<
                            " to #" << getGridPtr(ix+1,iy,iz+1)->blockId << endl;
                    } else {
                        (ptrBlock)->getInterface(NeighborDirection::TopRight)->connect(NULL);
                    }

                    if (iz>0 && getGridPtr(ix+1,iy,iz-1)) {
                        (ptrBlock)->getInterface(NeighborDirection::BottomRight)->
                            connect(getGridPtr(ix+1,iy,iz-1)->getInterface(NeighborDirection::TopLeft));
                        OUTPUT << "connection #" << (ptrBlock)->blockId <<
                            " to #" << getGridPtr(ix+1,iy,iz-1)->blockId << endl;
                    } else {
                        (ptrBlock)->getInterface(NeighborDirection::BottomRight)->connect(NULL);
                    }
                }
            } else {
                if (ix>0) {
                    // x-1
                    if (iz<gridSize[2]-1 && getGridPtr(ix-1,iy,iz+1)) {
                        (ptrBlock)->getInterface(NeighborDirection::TopLeft)->
                            connect(getGridPtr(ix-1,iy,iz+1)->getInterface(NeighborDirection::BottomRight));
                        OUTPUT << "connection #" << (ptrBlock)->blockId <<
                            " to #" << getGridPtr(ix-1,iy,iz+1)->blockId << endl;
                    } else {
                        (ptrBlock)->getInterface(NeighborDirection::TopLeft)->connect(NULL);
                    }

                    if (iz>0 && getGridPtr(ix-1,iy,iz-1)) {
                        (ptrBlock)->getInterface(NeighborDirection::BottomLeft)->
                            connect(getGridPtr(ix-1,iy,iz-1)->getInterface(NeighborDirection::TopRight));
                        OUTPUT << "connection #" << (ptrBlock)->blockId <<
                            " to #" << getGridPtr(ix-1,iy,iz-1)->blockId << endl;
                    } else {
                        (ptrBlock)->getInterface(NeighborDirection::BottomLeft)->connect(NULL);
                    }
                }
            }
        }
    }


    void Catoms2DWorld::linkBlocks() {
        int ix,iy,iz;
        for (iz=0; iz<gridSize[2]; iz++) {
            for (iy=0; iy<gridSize[1]; iy++) {
                for(ix=0; ix<gridSize[0]; ix++) {
                    linkBlock(ix,iy,iz);
                }
            }
        }
    }

    void Catoms2DWorld::deleteBlock(Catoms2DBlock *bb) {
        if (bb->getState() >= Catoms2DBlock::ALIVE ) {
            // cut links between bb and others
            for(int i=0; i<6; i++) {
                P2PNetworkInterface *bbi = bb->getInterface(NeighborDirection::Direction(i));
                if (bbi->connectedInterface) {
                    //bb->removeNeighbor(bbi); //Useless
                    bbi->connectedInterface->hostBlock->removeNeighbor(bbi->connectedInterface);
                    bbi->connectedInterface->connectedInterface=NULL;
                    bbi->connectedInterface=NULL;
                }
            }
            // free grid cell
            int ix,iy,iz;
            ix = int(bb->position.pt[0]);
            iy = int(bb->position.pt[1]);
            iz = int(bb->position.pt[2]);
            setGridPtr(ix,iy,iz,NULL);
        
            disconnectBlock(bb);
        }
        if (selectedBlock == bb->ptrGlBlock) {
            selectedBlock = NULL;
            GlutContext::mainWindow->select(NULL);
        }
        // remove the associated glBlock
        std::vector<GlBlock*>::iterator cit=tabGlBlocks.begin();
        if (*cit==bb->ptrGlBlock) tabGlBlocks.erase(cit);
        else {
            while (cit!=tabGlBlocks.end() && (*cit)!=bb->ptrGlBlock) {
                cit++;
            }
            if (*cit==bb->ptrGlBlock) tabGlBlocks.erase(cit);
        }
        delete bb->ptrGlBlock;
    }

    void Catoms2DWorld::glDraw() {
        glPushMatrix();
        glTranslatef(0.5*blockSize[0],0,0.5*blockSize[2]);
        glDisable(GL_TEXTURE_2D);
        vector <GlBlock*>::iterator ic=tabGlBlocks.begin();
        lock();
        while (ic!=tabGlBlocks.end()) {
            ((Catoms2DGlBlock*)(*ic))->glDraw(objBlock);
            ic++;
        }
        unlock();
        glPopMatrix();
    
        static const GLfloat white[]={0.8f,0.8f,0.8f,1.0f},gray[]={0.2f,0.2f,0.2f,1.0f};
    
        glMaterialfv(GL_FRONT,GL_AMBIENT,gray);
        glMaterialfv(GL_FRONT,GL_DIFFUSE,white);
        glMaterialfv(GL_FRONT,GL_SPECULAR,gray);
        glMaterialf(GL_FRONT,GL_SHININESS,40.0);
        glPushMatrix();
        enableTexture(true);
        glBindTexture(GL_TEXTURE_2D,idTextureLines);
        glScalef(gridSize[0]*blockSize[0],gridSize[1]*blockSize[1],blockSize[2]+(gridSize[2]-1)*blockSize[2]*M_SQRT3_2);
        glBegin(GL_QUADS);
        // bottom
        glNormal3f(0,0,1.0f);
        glTexCoord2f(1.0f,0.25f);
        glVertex3f(0.0f,0.0f,0.0f);
        glTexCoord2f(1.0f,gridSize[0]+0.25f);
        glVertex3f(1.0f,0.0f,0.0f);
        glTexCoord2f(0,gridSize[0]+0.25f);
        glVertex3f(1.0,1.0,0.0f);
        glTexCoord2f(0,0.25f);
        glVertex3f(0.0,1.0,0.0f);
        // top
        glNormal3f(0,0,-1.0f);
        glTexCoord2f(1.0f,0.25f);
        glVertex3f(0.0f,0.0f,1.0f);
        glTexCoord2f(0,0.25f);
        glVertex3f(0.0,1.0,1.0f);
        glTexCoord2f(0,gridSize[0]+0.25f);
        glVertex3f(1.0,1.0,1.0f);
        glTexCoord2f(1.0f,gridSize[0]+0.25f);
        glVertex3f(1.0f,0.0f,1.0f);
        // left
        glNormal3f(1.0f,0,0);
        glTexCoord2f(0,0.25f*M_SQRT3_2);
        glVertex3f(0.0f,0.0f,0.0f);
        glTexCoord2f(1.0f,0.25f*M_SQRT3_2);
        glVertex3f(0.0f,1.0f,0.0f);
        glTexCoord2f(1.0f,(gridSize[2]+0.25f)*M_SQRT3_2);
        glVertex3f(0.0,1.0,1.0f);
        glTexCoord2f(0.0f,(gridSize[2]+0.25f)*M_SQRT3_2);
        glVertex3f(0.0,0.0,1.0f);
        // right
        glNormal3f(-1.0f,0,0);
        glTexCoord2f(0,0.25f*M_SQRT3_2);
        glVertex3f(1.0f,0.0f,0.0f);
        glTexCoord2f(0.0f,(gridSize[2]+0.25f)*M_SQRT3_2);
        glVertex3f(1.0,0.0,1.0f);
        glTexCoord2f(1.0f,(gridSize[2]+0.25f)*M_SQRT3_2);
        glVertex3f(1.0,1.0,1.0f);
        glTexCoord2f(1.0f,0.25f*M_SQRT3_2);
        glVertex3f(1.0f,1.0f,0.0f);
        glEnd();
        glPopMatrix();
        // draw hexa
        glPushMatrix();
        glBindTexture(GL_TEXTURE_2D,idTextureHexa);
        glScalef(gridSize[0]*blockSize[0],gridSize[1]*blockSize[1],blockSize[2]+(gridSize[2]-1)*blockSize[2]*M_SQRT3_2);
        float h=((gridSize[2]-1)+1.0/M_SQRT3_2)/2.0;
        glBegin(GL_QUADS);
        // back
        glNormal3f(0,-1.0,0);
        glTexCoord2f(0,0);
        glVertex3f(0.0f,1.0f,0.0f);
        glTexCoord2f(gridSize[0]/3.0f,0);
        glVertex3f(1.0f,1.0f,0.0f);
        glTexCoord2f(gridSize[0]/3.0f,h);
        glVertex3f(1.0f,1.0,1.0f);
        glTexCoord2f(0,h);
        glVertex3f(0.0,1.0,1.0f);
        // front
        glNormal3f(0,1.0,0);
        glTexCoord2f(0,0);
        glVertex3f(0.0f,0.0f,0.0f);
        glTexCoord2f(0,h);
        glVertex3f(0.0,0.0,1.0f);
        glTexCoord2f(gridSize[0]/3.0f,h);
        glVertex3f(1.0f,0.0,1.0f);
        glTexCoord2f(gridSize[0]/3.0f,0);
        glVertex3f(1.0f,0.0f,0.0f);
        glEnd();
        glPopMatrix();
        // draw the axes
        glPushMatrix();
        glScalef(0.05f,0.05f,0.05f);
        objRepere->glDraw();
        glPopMatrix();
    }

    void Catoms2DWorld::glDrawId() {
        glPushMatrix();
        glTranslatef(0.5*blockSize[0],0,0.5*blockSize[2]);
        glDisable(GL_TEXTURE_2D);
        vector <GlBlock*>::iterator ic=tabGlBlocks.begin();
        int n=1;
        lock();
        while (ic!=tabGlBlocks.end()) {
            ((Catoms2DGlBlock*)(*ic))->glDrawId(objBlock,n);
            ic++;
        }
        unlock();
        glPopMatrix();
    }

    void Catoms2DWorld::glDrawIdByMaterial() {
        glPushMatrix();
        glTranslatef(0.5*blockSize[0],0.5*blockSize[1],0.5*blockSize[2]);
    
        glDisable(GL_TEXTURE_2D);
        vector <GlBlock*>::iterator ic=tabGlBlocks.begin();
        int n=1;
        lock();
        while (ic!=tabGlBlocks.end()) {
            ((Catoms2DGlBlock*)(*ic))->glDrawIdByMaterial(objBlockForPicking,n);
            ic++;
        }
        unlock();
        glPopMatrix();
    }


    void Catoms2DWorld::loadTextures(const string &str) {
        string path = str+"//hexa.tga";
        int lx,ly;
        idTextureHexa = GlutWindow::loadTexture(path.c_str(),lx,ly);
        path = str+"//lignes.tga";
        idTextureLines = GlutWindow::loadTexture(path.c_str(),lx,ly);
    }

    void Catoms2DWorld::updateGlData(Catoms2DBlock*blc) {
        //cout << "update posgrid:" << blc->position << endl;
        updateGlData(blc,gridToWorldPosition(blc->position));
    }

    void Catoms2DWorld::updateGlData(Catoms2DBlock*blc, const Vector3D &position) {
        Catoms2DGlBlock *glblc = blc->getGlBlock();
        if (glblc) {
            lock();
            //cout << "update pos:" << position << endl;
            glblc->setPosition(position);
            glblc->setColor(blc->color);
            unlock();
        }
    }

    void Catoms2DWorld::updateGlData(Catoms2DBlock*blc, const Vector3D &position, double angle) {
        Catoms2DGlBlock *glblc = blc->getGlBlock();
        if (glblc) {
            lock();
            //cout << "update pos:" << position << endl;
            glblc->setAngle(angle);
            glblc->setPosition(position);
            glblc->setColor(blc->color);
            unlock();
        }
    }

    Vector3D Catoms2DWorld::worldToGridPosition(Vector3D &pos) {
        Vector3D res;
        res.pt[2] = round(pos[2] / (M_SQRT3_2 * blockSize[2]));
        res.pt[1] = 0;
        res.pt[0] = (int) (pos[0]/blockSize[0] - ((int)res.pt[2]%2)*0.5);
        /*cout << pos[0]/blockSize[0] << endl;
          cout << res.pt[0] << endl;
          cout << res.pt[2] << endl;
          cout << pos[2] << endl;*/
        /*
          cout << "------------computation worldToGridPosition--------------" << endl;
          cout << pos << endl;
          cout << res << endl;
          cout << "---------------------------------------------------------" << endl;*/
        return res;
    }

    Vector3D Catoms2DWorld::gridToWorldPosition(Vector3D &pos) {
        Vector3D res;
    
        res.pt[2] = M_SQRT3_2*pos[2]*blockSize[2];
        res.pt[1] = blockSize[1]/2.0;
        res.pt[0] = (pos[0]+((int)(pos[2]+0.01)%2)*0.5)*blockSize[0]; // +0.01 because of round problem
        /*cout << "------------computation gridToWorldPosition--------------" << endl;
          cout << pos << endl;
          cout << ((int)pos[2]%2)*0.5 << endl;
          cout << (int)pos[2]%2 << endl;
          cout << pos[2] << endl;
          cout << (int)(pos[2]+0.01) << endl;
          cout << res << endl;
          cout << "---------------------------------------------------------" << endl;*/
        return res;
    }

    bool Catoms2DWorld::areNeighborsWorldPos(Vector3D &pos1, Vector3D &pos2) {
        float distance = 0;
        for (int i = 0; i < 3; i++) {
            distance += powf(pos2[i] - pos1[i],2);
        }
        distance = sqrt(distance);
        return (ceil(distance) == blockSize[0]);
    }

    bool Catoms2DWorld::areNeighborsGridPos(Vector3D &pos1, Vector3D &pos2) {
        Vector3D wpos1 = gridToWorldPosition(pos1);
        Vector3D wpos2 = gridToWorldPosition(pos2);
        return areNeighborsWorldPos(wpos1,wpos2);
    }

    void Catoms2DWorld::menuChoice(int n) {
        Catoms2DBlock *bb = (Catoms2DBlock *)getBlockById(tabGlBlocks[numSelectedBlock]->blockId);
        
        switch (n) {
        case 1 : {
            OUTPUT << "ADD block link to : " << bb->blockId << "     num Face : " << numSelectedFace << endl;

            Vector3D pos = bb->getPosition(NeighborDirection::Direction(numSelectedFace));

            addBlock(-1, bb->buildNewBlockCode, pos, bb->color);
            linkBlocks();
        } break;
        case 2 : {
            OUTPUT << "DEL num block : " << tabGlBlocks[numSelectedBlock]->blockId << endl;
            deleteBlock(bb);
        } break;
        case 3 : {
            tapBlock(getScheduler()->now(), bb->blockId);
        } break;
        case 4:                 // Save current configuration
            exportConfiguration();
            break;
        case 5: {                 // Move Left
			// Identify pivot
			int pivotId = bb->getCCWMovePivotId();
			Catoms2DBlock *pivot = (Catoms2DBlock *)getBlockById(pivotId);
			Catoms2DMove move = Catoms2DMove(pivot, RelativeDirection::CCW);
			bb->startMove(move);
        } break;
        case 6:                 // Move Right
			// Identify pivot
			int pivotId = bb->getCWMovePivotId();
			Catoms2DBlock *pivot = (Catoms2DBlock *)getBlockById(pivotId);
			Catoms2DMove move = Catoms2DMove(pivot, RelativeDirection::CW);
			bb->startMove(move);
        break;
        }

    }

    bool Catoms2DWorld::canAddBlockToFace(int numSelectedBlock, int numSelectedFace) {
        Catoms2DBlock *bb = (Catoms2DBlock *)getBlockById(tabGlBlocks[numSelectedBlock]->blockId);

        // cerr << "tabGlBlocks:" << endl;
        // for(auto b: tabGlBlocks) {
        //     cerr << b->blockId << " | ";
        // }
        // cerr << endl;
        // cerr << "bb->blockId = " << bb->blockId << endl;
        
        switch (numSelectedFace) {
        case NeighborDirection::Left :
            return (bb->position[0] > 0
                    && getGridPtr(int(bb->position[0]) - 1,
                                  int(bb->position[1]),
                                  int(bb->position[2])) == NULL);
            break;
        case NeighborDirection::Right :
            return (bb->position[0] < gridSize[0] - 1
                    && getGridPtr(int(bb->position[0]) + 1,
                                  int(bb->position[1]),
                                  int(bb->position[2])) == NULL);
            break;
        case NeighborDirection::BottomLeft :            
            if (IS_EVEN((int)bb->position[2]))
                return (bb->position[2] > 0
                        && bb->position[0] > 0
                        && getGridPtr(int(bb->position[0]) - 1,
                                      int(bb->position[1]),
                                      int(bb->position[2]) - 1) == NULL);
            else
                return (bb->position[2] > 0
                        && getGridPtr(int(bb->position[0]),
                                      int(bb->position[1]),
                                      int(bb->position[2]) - 1) == NULL);                
                
            break;
        case NeighborDirection::TopLeft :
            if (IS_EVEN((int)bb->position[2]))
                return (bb->position[2] < gridSize[2] - 1
                        && bb->position[0] > 0
                        && getGridPtr(int(bb->position[0]) - 1,
                                      int(bb->position[1]),
                                      int(bb->position[2]) + 1) == NULL);
            else
                return (bb->position[2] < gridSize[2] - 1
                        && getGridPtr(int(bb->position[0]),
                                      int(bb->position[1]),
                                      int(bb->position[2]) + 1) == NULL);                

            break;
        case NeighborDirection::BottomRight :
            if (IS_ODD((int)bb->position[2]))
                return (bb->position[2] > 0
                        && bb->position[0] < gridSize[0] - 1
                        && getGridPtr(int(bb->position[0]) + 1,
                                      int(bb->position[1]),
                                      int(bb->position[2]) - 1) == NULL);
            else
                return (bb->position[2] > 0
                        && getGridPtr(int(bb->position[0]),
                                      int(bb->position[1]),
                                      int(bb->position[2]) - 1) == NULL);                
            break;
        case NeighborDirection::TopRight :
            if (IS_ODD((int)bb->position[2]))
                return (bb->position[2] < gridSize[2] - 1
                        && bb->position[0] < gridSize[0] - 1
                        && getGridPtr(int(bb->position[0]) + 1,
                                      int(bb->position[1]),
                                      int(bb->position[2]) + 1) == NULL);
            else
                return (bb->position[2] < gridSize[2] - 1
                        && getGridPtr(int(bb->position[0]),
                                      int(bb->position[1]),
                                      int(bb->position[2]) + 1) == NULL);                
            break;
        }
        
        return false;
    }

    
    void Catoms2DWorld::setSelectedFace(int n) {
        // cerr << "n = " << n << endl;
        // cerr << "n/7 = " << n/7 << endl;
        // cerr << "n%7 = " << n%7 << endl;
        
        numSelectedBlock = n / numPickingTextures;
        string name = objBlockForPicking->getObjMtlName(n % numPickingTextures);
        // cerr << "tabGlBlocks:" << endl;
        // for(int n1 = 0; n1 < 7; n1++) {
        //     cerr << objBlockForPicking->getObjMtlName(n1) << " | ";
        // }
        // cerr << endl;

        numSelectedFace = numPickingTextures;   // Undefined NeighborDirection

        if (name == "face_0") numSelectedFace = NeighborDirection::Right;
        else if (name == "face_1") numSelectedFace = NeighborDirection::TopRight;
        else if (name == "face_2") numSelectedFace = NeighborDirection::TopLeft;
        else if (name == "face_3") numSelectedFace = NeighborDirection::Left;
        else if (name == "face_4") numSelectedFace = NeighborDirection::BottomLeft;
        else if (name == "face_5") numSelectedFace = NeighborDirection::BottomRight;            
        
        // cerr << name << " = " << NeighborDirection::getString(numSelectedFace) << endl;
    }

    void Catoms2DWorld::createHelpWindow() {
        if (GlutContext::helpWindow)
            delete GlutContext::helpWindow;
        GlutContext::helpWindow = new GlutHelpWindow(NULL,10,40,540,500,"../../simulatorCore/genericHelp.txt");
    }

    void Catoms2DWorld::getPresenceMatrix(const PointRel3D &pos,PresenceMatrix &pm) {
        presence *gpm=pm.grid;
        Catoms2DBlock **grb;
    
        //memset(pm.grid,wall,27*sizeof(presence));
    
        for (int i=0; i<27; i++) { *gpm++ = wallCell; };
    
        int ix0 = (pos.x<1)?1-pos.x:0,
            ix1 = (pos.x>gridSize[0]-2)?gridSize[0]-pos.x+1:3,
            iy0 = (pos.y<1)?1-pos.y:0,
            iy1 = (pos.y>gridSize[1]-2)?gridSize[1]-pos.y+1:3,
            iz0 = (pos.z<1)?1-pos.z:0,
            iz1 = (pos.z>gridSize[2]-2)?gridSize[2]-pos.z+1:3,
            ix,iy,iz;
        for (iz=iz0; iz<iz1; iz++) {
            for (iy=iy0; iy<iy1; iy++) {
                gpm = pm.grid+((iz*3+iy)*3+ix0);
                grb = gridPtrBlocks+(ix0+pos.x-1+(iy+pos.y-1+(iz+pos.z-1)*gridSize[1])*gridSize[0]);
                for (ix=ix0; ix<ix1; ix++) {
                    *gpm++ = (*grb++)?fullCell:emptyCell;
                }
            }
        }
    }

    /**
     * Displays a popup menu at coordinates (ix, iy)
     *  Overriden for catoms2D to allow for c2d movements
     *
     * @param ix 
     * @param iy 
     * @return 
     */
     void Catoms2DWorld::createPopupMenu(int ix, int iy) {
         if (!GlutContext::popupMenu) {
             GlutContext::popupMenu = new GlutPopupMenuWindow(NULL,0,0,200,180);
             GlutContext::popupMenu->addButton(1,"../../simulatorCore/menuTextures/menu_add.tga");
             GlutContext::popupMenu->addButton(2,"../../simulatorCore/menuTextures/menu_del.tga");
             GlutContext::popupMenu->addButton(3,"../../simulatorCore/menuTextures/menu_tap.tga");
             GlutContext::popupMenu->addButton(4,"../../simulatorCore/menuTextures/menu_save.tga");
             GlutContext::popupMenu->addButton(5,"../../simulatorCore/menuTextures/menu_tap.tga");
             GlutContext::popupMenu->addButton(6,"../../simulatorCore/menuTextures/menu_tap.tga");
             GlutContext::popupMenu->addButton(7,"../../simulatorCore/menuTextures/menu_cancel.tga");
         }

         if (iy < GlutContext::popupMenu->h) iy = GlutContext::popupMenu->h;

         cerr << "Block " << numSelectedBlock << ":" << numSelectedFace << " selected" << endl;
		 Catoms2DBlock *bb = (Catoms2DBlock *)getBlockById(tabGlBlocks[numSelectedBlock]->blockId);
		 
         GlutContext::popupMenu->activate(1, canAddBlockToFace((int)numSelectedBlock, (int)numSelectedFace));
		 GlutContext::popupMenu->activate(5, bb->getCCWMovePivotId() != -1);
		 GlutContext::popupMenu->activate(6, bb->getCWMovePivotId() != -1);
         GlutContext::popupMenu->setCenterPosition(ix,GlutContext::screenHeight-iy);
         GlutContext::popupMenu->show(true);
     }    

    void Catoms2DWorld::exportConfiguration() {
        throw "configuration export not yet implemented";
    }

    
    void Catoms2DWorld::initTargetGrid() {
        if (targetGrid) delete [] targetGrid;
        int sz = gridSize[0]*gridSize[1]*gridSize[2];
        targetGrid = new presence[gridSize[0]*gridSize[1]*gridSize[2]];
        memset(targetGrid,emptyCell,sz*sizeof(presence));
    }

} // RobotBlock namespace


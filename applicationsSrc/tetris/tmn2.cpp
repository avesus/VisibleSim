#include "tetrisCode.hpp"

void TetrisCode::sendTmn2(bool reinit, bool blocked)
{
    TmnData data = TmnData(update, rotation, position, color, nbReinit, nbFree);
    ReinitData rData = ReinitData(nbReinit, tmn, movement);

    if (rotation == NORTH)
    {
        itf[northId] = topItf;
        itf[eastId] = rightItf;
        itf[southId] = bottomItf;
        itf[westId] = leftItf;
        northBool = roleInPixel == TOP_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == ALONE;
        eastBool = roleInPixel == RIGHT_BORDER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        southBool = roleInPixel == BOTTOM_BORDER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        westBool = roleInPixel == LEFT_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == ALONE;
    }
    else if (rotation == EAST)
    {
        itf[northId] = rightItf;
        itf[eastId] = bottomItf;
        itf[southId] = leftItf;
        itf[westId] = topItf;
        northBool = roleInPixel == RIGHT_BORDER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        eastBool = roleInPixel == BOTTOM_BORDER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        southBool = roleInPixel == LEFT_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == ALONE;
        westBool = roleInPixel == TOP_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == ALONE;
    }
    else if (rotation == SOUTH)
    {
        itf[northId] = bottomItf;
        itf[eastId] = leftItf;
        itf[southId] = topItf;
        itf[westId] = rightItf;
        northBool = roleInPixel == BOTTOM_BORDER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        eastBool = roleInPixel == LEFT_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == ALONE;
        southBool = roleInPixel == TOP_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == ALONE;
        westBool = roleInPixel == RIGHT_BORDER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
    }
    else if (rotation == WEST)
    {
        itf[northId] = leftItf;
        itf[eastId] = topItf;
        itf[southId] = rightItf;
        itf[westId] = bottomItf;
        northBool = roleInPixel == LEFT_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == ALONE;
        eastBool = roleInPixel == TOP_BORDER || roleInPixel == TOP_LEFT_CORNER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == ALONE;
        southBool = roleInPixel == RIGHT_BORDER || roleInPixel == TOP_RIGHT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
        westBool = roleInPixel == BOTTOM_BORDER || roleInPixel == BOTTOM_LEFT_CORNER || roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE;
    }
    if (northBool)
    {
        if (position == 1)
        {
            data.position = 2;
        }
        else if (position == 3)
        {
            data.position = 1;
        }
        else if (position == 4)
        {
            data.position = 3;
        }
        if (position != 2)
        {
            P2PNetworkInterface *i = itf[northId];
            if (reinit && i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Reinit Tmn 2 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
                nbReinitBackMsg += 1;
            }
            else if (blocked && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn Blocked Msg", new Message(BLOCKED_MSG_ID), i, 0, 0);
            }
            else
            {
                if (i != parent && i != nullptr and i->isConnected())
                {
                    sendMessage("Tmn 2 Message", new MessageOf<TmnData>(TMN2_MSG_ID, data), i, 0, 0);
                    nbTmnBackMsg += 1;
                }
            }
        }
    }
    else
    {
        P2PNetworkInterface *i = itf[northId];
        if (reinit && i != parent && i != nullptr and i->isConnected())
        {
            sendMessage("Reinit Tmn 2 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
            nbReinitBackMsg += 1;
        }
        else if (blocked && i != nullptr and i->isConnected())
        {
            sendMessage("Tmn Blocked Msg", new Message(BLOCKED_MSG_ID), i, 0, 0);
        }
        else
        {
            if (i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn 2 Message", new MessageOf<TmnData>(TMN2_MSG_ID, data), i, 0, 0);
                nbTmnBackMsg += 1;
            }
        }
    }
    data.position = position;
    if (southBool)
    {
        if (position == 1)
        {
            data.position = 3;
        }
        else if (position == 2)
        {
            data.position = 1;
        }
        else if (position == 3)
        {
            data.position = 4;
        }
        if (position != 4)
        {
            P2PNetworkInterface *i = itf[southId];
            if (reinit && i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Reinit Tmn 2 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
                nbReinitBackMsg += 1;
            }
            else if (blocked && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn Blocked Msg", new Message(BLOCKED_MSG_ID), i, 0, 0);
            }
            else
            {
                if (i != parent && i != nullptr and i->isConnected())
                {
                    sendMessage("Tmn 2 Message", new MessageOf<TmnData>(TMN2_MSG_ID, data), i, 0, 0);
                    nbTmnBackMsg += 1;
                }
            }
        }
    }
    else
    {
        P2PNetworkInterface *i = itf[southId];
        if (reinit && i != parent && i != nullptr and i->isConnected())
        {
            sendMessage("Reinit Tmn 2 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
            nbReinitBackMsg += 1;
        }
        else if (blocked && i != nullptr and i->isConnected())
        {
            sendMessage("Tmn Blocked Msg", new Message(BLOCKED_MSG_ID), i, 0, 0);
        }
        else
        {
            if (i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn 2 Message", new MessageOf<TmnData>(TMN2_MSG_ID, data), i, 0, 0);
                nbTmnBackMsg += 1;
            }
        }
    }
    data.position = position;
    if (!eastBool)
    {
        P2PNetworkInterface *i = itf[eastId];
        if (reinit && i != parent && i != nullptr and i->isConnected())
        {
            sendMessage("Reinit Tmn 2 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
            nbReinitBackMsg += 1;
        }
        else if (blocked && i != nullptr and i->isConnected())
        {
            sendMessage("Tmn Blocked Msg", new Message(BLOCKED_MSG_ID), i, 0, 0);
        }
        else
        {
            if (i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn 2 Message", new MessageOf<TmnData>(TMN2_MSG_ID, data), i, 0, 0);
                nbTmnBackMsg += 1;
            }
        }
    }
    if (!westBool)
    {
        P2PNetworkInterface *i = itf[westId];
        if (reinit && i != parent && i != nullptr and i->isConnected())
        {
            sendMessage("Reinit Tmn 2 Message", new MessageOf<ReinitData>(REINITPIX_MSG_ID, rData), i, 0, 0);
            nbReinitBackMsg += 1;
        }
        else if (blocked && i != nullptr and i->isConnected())
        {
            sendMessage("Tmn Blocked Msg", new Message(BLOCKED_MSG_ID), i, 0, 0);
        }
        else
        {
            if (i != parent && i != nullptr and i->isConnected())
            {
                sendMessage("Tmn 2 Message", new MessageOf<TmnData>(TMN2_MSG_ID, data), i, 0, 0);
                nbTmnBackMsg += 1;
            }
        }
    }
}

void TetrisCode::myTmn2Func(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{

    MessageOf<TmnData> *msg = static_cast<MessageOf<TmnData> *>(_msg.get());
    TmnData msgData = *msg->getData();
    if (update < msgData.nbupdate && (tmn != 2 || rotation != msgData.rotation || position != msgData.position || color != msgData.color))
    {
        tmn = 2;
        update = msgData.nbupdate;
        rotation = msgData.rotation;
        position = msgData.position;
        color = msgData.color;
        nbReinit = msgData.nbReinit;
        nbFree = msgData.nbFree;
        parent = sender;
        nbTmnBackMsg = 0;
        module->setColor(Colors[color]);
        sendTmn2(false, false);
        if (nbTmnBackMsg == 0 && parent != nullptr && parent->isConnected())
        {
            sendMessage("Tmn Back Message Parent", new MessageOf<int>(TMNBACK_MSG_ID, update), parent, 0, 0);
        }
    }
    else if (update == msgData.nbupdate)
    {
        sendMessage("Tmn Back Message", new MessageOf<int>(TMNBACK_MSG_ID, update), sender, 0, 0);
    }
};

void TetrisCode::myRestartTmn2Func(std::shared_ptr<Message> _msg, P2PNetworkInterface *sender)
{
    MessageOf<TmnData> *msg = static_cast<MessageOf<TmnData> *>(_msg.get());
    TmnData msgData = *msg->getData();
    if (roleInPixel == BOTTOM_RIGHT_CORNER || roleInPixel == ALONE)
    {
        parent = nullptr;
        leaderBlockCode = this;
        tmn = 2;
        update = msgData.nbupdate;
        nbReinit = msgData.nbReinit;
        nbFree = msgData.nbFree;
        rotation = msgData.rotation;
        position = msgData.position;
        color = msgData.color;
        goingRight = msgData.goingR;
        goingLeft = msgData.goingL;
        turnCK = msgData.rotCW;
        turnCounterCK = msgData.rotCCW;
        module->setColor(Colors[color]);
        sendTmn2(false, false);
    }
    else
    {
        P2PNetworkInterface *i = nullptr;
        if (sender == topItf)
        {
            i = bottomItf;
        }
        else if (sender == bottomItf)
        {
            i = topItf;
        }
        else if (sender == rightItf)
        {
            i = leftItf;
        }
        else if (sender == leftItf)
        {
            i = rightItf;
        }
        if (i != nullptr && i->isConnected())
        {
            sendMessage("Restart Tmn 2 Message", new MessageOf<TmnData>(START_TMN2_MSG_ID, msgData), i, 0, 0);
        }
    }
}

void TetrisCode::verifTmn2()
{
    if (movement == GO_LEFT || movement == GO_RIGHT || movement == DOWN)
    {
        verifications.clear();
        int rot1 = 0;
        int rot2 = 0;
        int rot3 = 0;
        int rot4 = 0;
        int dir = 0;
        P2PNetworkInterface *i = nullptr; //for direct verifications
        if (movement == DOWN)
        {
            rot1 = NORTH;
            rot2 = SOUTH;
            rot3 = EAST;
            rot4 = WEST;
            dir = SOUTH;
            i = bottomItf;
        }
        else if (movement == GO_RIGHT)
        {
            rot1 = WEST;
            rot2 = EAST;
            rot3 = NORTH;
            rot4 = SOUTH;
            dir = EAST;
            i = rightItf;
        }
        else if (movement == GO_LEFT)
        {
            rot1 = EAST;
            rot2 = WEST;
            rot3 = SOUTH;
            rot4 = NORTH;
            dir = WEST;
            i = leftItf;
        }
        if (rotation == rot1)
        {
            nbFree += 1;
            verifications.push_back(freeAnswer(4, dir));
            isFreeData data = isFreeData(nbFree, 4, dir);
            sendVerifTmn2(false, data);
        }
        else if (rotation == rot2)
        {
            nbFree += 1;
            verifications.push_back(freeAnswer(2, dir));
            isFreeData data = isFreeData(nbFree, 2, dir);
            sendVerifTmn2(false, data);
        }
        else if (rotation == rot3 || rotation == rot4)
        {
            verifications.push_back(freeAnswer(2, dir));
            verifications.push_back(freeAnswer(3, dir));
            verifications.push_back(freeAnswer(4, dir));
            if (movement == GO_LEFT)
            {
                nbFree += 1;
                verifications.push_back(freeAnswer(1, dir));
                isFreeData data = isFreeData(nbFree, 1, dir);
                sendVerifTmn2(false, data);
            }
            else
            {
                //the following verification can be made directly by the deciding module
                if (i != nullptr && i->isConnected())
                {
                    sendMessage("Direct Verification Message", new Message(FREEMSG_ID), i, 0, 0);
                }
                else
                {
                    //the tetramino is blocked
                    blocked = true;
                    sendTmn2(false, true);
                    if (leftItf != nullptr && leftItf->isConnected())
                    {
                        sendMessage("Counting blocked neighbors", new MessageOf<int>(COUNT_BCK_MSG_ID, 1), leftItf, 0, 0);
                    }
                    if (rightItf != nullptr && rightItf->isConnected())
                    {
                        sendMessage("Counting blocked neighbors", new MessageOf<int>(COUNT_BCK_MSG_ID, 1), rightItf, 0, 0);
                    }
                    nbTmn += 1;
                    sendMessageToAllNeighbors("New Tetramino Message", new MessageOf<int>(NEWTMNMSG_ID, nbTmn), 0, 0, 0);
                }
            }
        }
    }
    else if (movement == ROT_CK || movement == ROT_COUNTER_CK)
    {
        farVerifications.clear();
        std::vector<int> dirs;
        farVerif v;
        if (movement == ROT_CK)
        {
            dirs.push_back(EAST);
            farVerifications.push_back(farVerif(0, 0, dirs, rotation));
            dirs.clear();
            dirs.push_back(WEST);
            farVerifications.push_back(farVerif(0, 0, dirs, rotation));
            dirs.push_back(WEST);
            v = farVerif(0, 0, dirs, rotation);
            farVerifications.push_back(v);
        }
        else if (movement == ROT_COUNTER_CK)
        {
            dirs.push_back(WEST);
            farVerifications.push_back(farVerif(0, 0, dirs, rotation));
            dirs.clear();
            dirs.push_back(EAST);
            farVerifications.push_back(farVerif(0, 0, dirs, rotation));
            dirs.push_back(EAST);
            v = farVerif(0, 0, dirs, rotation);
            farVerifications.push_back(v);
        }
        nbFree += 1;
        v.id = nbFree;
        sendFarVerif(v, nullptr);
    }
}

void TetrisCode::sendVerifTmn2(bool answer, isFreeData data)
{
    P2PNetworkInterface *i = itf[westId];
    if (!westBool && i != nullptr && i->isConnected())
    {
        if (answer)
        {
            sendMessage("Back Free Message", new MessageOf<isFreeData>(BACKFREEMSG_ID, data), i, 0, 0);
        }
        else
        {
            sendMessage("Verification Message", new MessageOf<isFreeData>(ISFREE_MSG_ID, data), i, 0, 0);
        }
    }
    i = itf[eastId];
    if (!eastBool && i != nullptr && i->isConnected())
    {
        if (answer)
        {
            sendMessage("Back Free Message", new MessageOf<isFreeData>(BACKFREEMSG_ID, data), i, 0, 0);
        }
        else
        {
            sendMessage("Verification Message", new MessageOf<isFreeData>(ISFREE_MSG_ID, data), i, 0, 0);
        }
    }
    i = itf[northId];
    if ((position != 2 || !northBool) && i != nullptr && i->isConnected())
    {
        if (answer)
        {
            sendMessage("Back Free Message", new MessageOf<isFreeData>(BACKFREEMSG_ID, data), i, 0, 0);
        }
        else
        {
            sendMessage("Verification Message", new MessageOf<isFreeData>(ISFREE_MSG_ID, data), i, 0, 0);
        }
    }
    i = itf[southId];
    if ((position != 4 || !southBool) && i != nullptr && i->isConnected())
    {
        if (answer)
        {
            sendMessage("Back Free Message", new MessageOf<isFreeData>(BACKFREEMSG_ID, data), i, 0, 0);
        }
        else
        {
            sendMessage("Verification Message", new MessageOf<isFreeData>(ISFREE_MSG_ID, data), i, 0, 0);
        }
    }
}
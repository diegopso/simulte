//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "apps/d2dMultihop/eventGenerator/EventGenerator.h"
#include "common/LteCommon.h"
#include "stack/phy/layer/LtePhyBase.h"

Define_Module(EventGenerator);

EventGenerator::EventGenerator()
{
    selfMessage_ = NULL;
    eventId_ = 0;
}

EventGenerator::~EventGenerator()
{
    cancelAndDelete(selfMessage_);
}

void EventGenerator::initialize()
{
    EV_TRACE << "EventGenerator initialize "<< endl;

    selfMessage_ = new cMessage("selfMessage");
    eventPeriod_ = par("eventPeriod");
    binder_ = getBinder();
    singleEventSource_ = par("singleEventSource").boolValue();

    simtime_t startTime = par("startTime");
    if (startTime >= 0)
    {
        // this conversion is made in order to obtain ms-aligned start time, even in case of random generated ones
        simtime_t offset = (round(SIMTIME_DBL(startTime)*1000)/1000)+simTime();

        scheduleAt(offset, selfMessage_);
        EV_TRACE << "\t sending event notification in " << offset << " seconds " << endl;
    }
}

void EventGenerator::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        if (!strcmp(msg->getName(), "selfMessage"))
            notifyEvent();
        else
            throw cRuntimeError("Unrecognized self message");
    }
}

void EventGenerator::notifyEvent()
{
    // randomly pick one node and notify it the event
    int r = intuniform(0,appVector_.size()-1,0);
    appVector_[r]->handleEvent(eventId_);

    if (!singleEventSource_)
    {
        // select a second originator, close to the first one

        LtePhyBase* uePhy = lteNodePhy_[r+UE_MIN_ID];
        inet::Coord uePos = uePhy->getCoord();

        // get references to all UEs in the cell
        // they are useful later to retrieve UE positions

        // find all UEs within a 20m-radius
        std::vector<MacNodeId> tmp;
        std::set<MacNodeId>::iterator it = lteNodeIdSet_.begin();
        for (; it != lteNodeIdSet_.end(); ++it)
        {
            MacNodeId nodeId = *it;
            if (r + UE_MIN_ID != nodeId)
            {
                LtePhyBase* phy = lteNodePhy_[nodeId];
                inet::Coord pos = phy->getCoord();
                double d = uePos.distance(pos);
                if (d < 20.0)
                    tmp.push_back(nodeId);
            }
        }

        // select one neighbor randomly
        if (!tmp.empty())
        {
            int r = intuniform(0,tmp.size()-1,1);
            MacNodeId id = tmp.at(r);
            appVector_[id-UE_MIN_ID]->handleEvent(eventId_);
        }
    }

    // TODO use a realistic model for generating events

    simtime_t offset = eventPeriod_ + simTime();
    scheduleAt(offset, selfMessage_);

    eventId_++;
}

void EventGenerator::computeTargetNodeSet(std::set<MacNodeId>& targetSet, MacNodeId sourceId, double maxBroadcastRadius)
{
    if (maxBroadcastRadius < 0.0)
    {
        // default behavior: the message must be delivered to all MultihopD2D apps in the network
        targetSet = lteNodeIdSet_;
    }
    else
    {
        // send the message to all UEs within the area defined by the maxBroadcastRadius parameter

        // get references to all UEs in the cell
        // they are useful later to retrieve UE positions
        std::map<MacNodeId, Coord> uePos;
        std::set<MacNodeId>::iterator it = lteNodeIdSet_.begin();
        for (; it != lteNodeIdSet_.end(); ++it)
        {
            MacNodeId nodeId = *it;
            LtePhyBase* uePhy = lteNodePhy_[nodeId];
            uePos[nodeId] = uePhy->getCoord();
        }

        // get the coordinates of the source node
        Coord srcCoord = uePos[sourceId];

        // compute the distance for each UE
        std::map<MacNodeId, Coord>::iterator mit = uePos.begin();
        for (; mit != uePos.end(); ++mit)
        {
            if (mit->second.distance(srcCoord) < maxBroadcastRadius)
            {
                EV_TRACE << " - " << mit->first << endl;
                targetSet.insert(mit->first);
            }
        }
    }
}

void EventGenerator::registerNode(MultihopD2D* app, MacNodeId lteNodeId)
{
    appVector_.push_back(app);
    lteNodeIdSet_.insert(lteNodeId);
    lteNodePhy_[lteNodeId] = check_and_cast<LtePhyBase*>((getSimulation()->getModule(binder_->getOmnetId(lteNodeId)))->getSubmodule("lteNic")->getSubmodule("phy") );
}

void EventGenerator::unregisterNode(MultihopD2D* app, MacNodeId lteNodeId)
{
    std::vector<MultihopD2D*>::iterator it = appVector_.begin();
    for (; it != appVector_.end(); ++it)
    {
        if (*it == app) // compare pointers
        {
            appVector_.erase(it);
            break;
        }
    }

    std::set<MacNodeId>::iterator sit = lteNodeIdSet_.find(lteNodeId);
    if (sit != lteNodeIdSet_.end())
        lteNodeIdSet_.erase(sit);
}

//
//                           SimuLTE
// Copyright (C) 2012 Antonio Virdis, Daniele Migliorini, Giovanni
// Accongiagioco, Generoso Pagano, Vincenzo Pii.
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef GTP_COMMON_H
#define GTP_COMMON_H

#include <map>
#include <list>
#include "IPvXAddress.h"

enum EpcNodeType
{
    ENB,
    PGW,
    SGW
};

#define LOCAL_ADDRESS_TEID -1

#define UNSPECIFIED_PORT 65536

#define UNSPECIFIED_TFT 65536

//===================== GTP-U tunnel management =====================
typedef int TunnelEndpointIdentifier;

struct ConnectionInfo
{
    ConnectionInfo(TunnelEndpointIdentifier id, IPvXAddress hop) :
        teid(id), nextHop(hop)
    {
    }

    TunnelEndpointIdentifier teid;
    IPvXAddress nextHop;
};

typedef std::map<TunnelEndpointIdentifier, ConnectionInfo> LabelTable;
//===================================================================

//=================== Traffic filters management ====================
// identifies a traffic flow template
typedef int TrafficFlowTemplateId;

struct TrafficFlowTemplate
{
    TrafficFlowTemplate(IPvXAddress ad, unsigned int src, unsigned int dest) :
        addr(ad), srcPort(src), destPort(dest)
    {
    }
    IPvXAddress addr;

    unsigned int srcPort;
    unsigned int destPort;

    TrafficFlowTemplateId tftId;

    bool operator==(const TrafficFlowTemplate & b)
    {
        if ((b.addr == addr) && (b.srcPort == srcPort) && (b.destPort == destPort))
            return true;
        else
            return false;
    }
};

// contains a list of traffic flow templates associated to a src/dest address. It is used By the Traffic Flow Filter
typedef std::list<TrafficFlowTemplate> TrafficFilterTemplateList;

typedef std::map<IPvXAddress, TrafficFilterTemplateList> TrafficFilterTemplateTable;
//===================================================================

char * const * loadXmlTable(char const * attributes[], unsigned int numAttributes);

#endif

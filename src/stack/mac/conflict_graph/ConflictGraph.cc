//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "stack/mac/conflict_graph/ConflictGraph.h"

/*!
 * \fn ConflictGraph()
 * \memberof ConflictGraph
 * \brief class constructor;
 */
ConflictGraph::ConflictGraph(LteMacEnbD2D* macEnb, bool reuseD2D, bool reuseD2DMulti)
{
    conflictGraph_.clear();
    macEnb_ = macEnb;
    cellInfo_ = macEnb_->getCellInfo();

    reuseD2D_ = reuseD2D;
    reuseD2DMulti_ = reuseD2DMulti;
}

/*!
 * \fn ~ConflictGraph()
 * \memberof ConflictGraph
 * \brief class destructor
 */
ConflictGraph::~ConflictGraph()
{
    clearConflictGraph();
}

// reset Conflict Graph
void ConflictGraph::clearConflictGraph()
{
    conflictGraph_.clear();
}

void ConflictGraph::computeConflictGraph()
{
    EV_TRACE << " ConflictGraph::computeConflictGraph - START "<<endl;

    // --- remove the old one --- //
    clearConflictGraph();

    // --- find the vertices of the graph by scanning the peering map --- //
    std::vector<CGVertex> vertices;
    findVertices(vertices);
    EV_TRACE << " ConflictGraph::computeConflictGraph - " << vertices.size() << " vertices found" << endl;

    // --- for each CGVertex, find the interfering vertices --- //
    findEdges(vertices);

    EV_TRACE << " ConflictGraph::computeConflictGraph - END "<<endl;

}

void ConflictGraph::printConflictGraph()
{
    EV_TRACE << " ConflictGraph::printConflictGraph "<<endl;

    if (conflictGraph_.empty())
    {
        EV_TRACE << " ConflictGraph::printConflictGraph - No reuse enabled "<<endl;
        return;
    }

    EV_TRACE << "              ";
    CGMatrix::iterator it = conflictGraph_.begin(), et = conflictGraph_.end();
    for (; it != et; ++it)
    {
        if (it->first.isMulticast())
            EV_TRACE << "| (" << it->first.srcId << ", *  ) ";
        else
            EV_TRACE << "| (" << it->first.srcId << "," << it->first.dstId <<") ";
    }
    EV_TRACE << endl;

    it = conflictGraph_.begin();
    for (; it != et; ++it)
    {
        if (it->first.isMulticast())
            EV_TRACE << "| (" << it->first.srcId << ", *  ) ";
        else
            EV_TRACE << "| (" << it->first.srcId << "," << it->first.dstId <<") ";
        std::map<CGVertex, bool>::iterator jt = it->second.begin();
        for (; jt != it->second.end(); ++jt)
        {
            if (it->first == jt->first)
            {
                EV_TRACE << "|      -      ";
            }
            else
            {
                EV_TRACE << "|      " << jt->second << "      ";
            }
        }
        EV_TRACE << endl;
    }
    EV_TRACE << endl;
}

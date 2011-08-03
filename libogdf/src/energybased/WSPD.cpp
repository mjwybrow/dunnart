#include "WSPD.h"
#include "FastUtils.h"

namespace ogdf {

WSPD::WSPD(__uint32 maxNumNodes) : m_maxNumNodes(maxNumNodes)
{
	m_maxNumPairs = maxNumNodes*2;
	m_numPairs = 0;
	allocate();
	clear();
}


WSPD::~WSPD(void)
{
	deallocate();
}

unsigned long WSPD::sizeInBytes() const
{
	return m_maxNumNodes*sizeof(WSPDNodeInfo) +
		   m_maxNumPairs*sizeof(WSPDPairInfo);
}

void WSPD::allocate()
{
	m_nodeInfo = (WSPDNodeInfo*)MALLOC_16(m_maxNumNodes*sizeof(WSPDNodeInfo));
	m_pairs = (WSPDPairInfo*)MALLOC_16(m_maxNumPairs*sizeof(WSPDPairInfo));
}

void WSPD::deallocate()
{
	FREE_16(m_nodeInfo);
	FREE_16(m_pairs);
}

void WSPD::clear()
{
	for (__uint32 i = 0; i < m_maxNumNodes; i++)
	{
		m_nodeInfo[i].numWSNodes = 0;
	};	
	m_numPairs = 0;
}

} // end of namespace ogdf

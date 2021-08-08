#ifndef _INCLUDE_WRAPPERS_H_
#define _INCLUDE_WRAPPERS_H_

#include "extension.h"

typedef CHandle<CBaseEntity> EHANDLE;

//Part of the code from here - https://github.com/shqke/sourcetvsupport/blob/master/extension/wrappers.h#L246
// :D
class CLagCompensationManager
{
public:
	static int offset_m_AdditionalEntitiesPool;
	static CLagCompensationManager* LagCompInstance;
	
	CUtlRBTree<EHANDLE>& m_AdditionalEntitiesPool()
	{
		return *reinterpret_cast<CUtlRBTree<EHANDLE>*>(reinterpret_cast<byte*>(this) + offset_m_AdditionalEntitiesPool);
	}
};

#endif // _INCLUDE_WRAPPERS_H_
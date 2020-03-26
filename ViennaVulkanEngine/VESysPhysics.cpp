/**
*
* \file
* \brief
*
* Details
*
*/


#include "VEDefines.h"
#include "VESysMessages.h"
#include "VESysEngine.h"
#include "VESysWindow.h"
#include "VESysPhysics.h"

namespace vve::sysphy {

	VeHandle g_updateHandle;
	VeHandle g_closeHandle;

	void init() {
		syseng::registerEntity(VE_SYSTEM_NAME);
		VE_SYSTEM_HANDLE = syseng::getEntityHandle(VE_SYSTEM_NAME);

		g_updateHandle = sysmes::addHandler(std::bind(update, std::placeholders::_1));
		sysmes::subscribeMessage(	syseng::VE_SYSTEM_HANDLE, VE_NULL_HANDLE, g_updateHandle,
									sysmes::VeMessageType::VE_MESSAGE_TYPE_UPDATE);

		g_closeHandle = sysmes::addHandler(std::bind(close, std::placeholders::_1));
		sysmes::subscribeMessage(syswin::VE_SYSTEM_HANDLE, VE_NULL_HANDLE, g_closeHandle,
								sysmes::VeMessageType::VE_MESSAGE_TYPE_CLOSE);


	}

	void update(sysmes::VeMessageTableEntry e) {

	}


	void close(sysmes::VeMessageTableEntry e) {

	}





}
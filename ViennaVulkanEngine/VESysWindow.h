#pragma once


/**
*
* \file
* \brief
*
* Details
*
*/


namespace vve::syswin {

	inline const std::string VE_SYSTEM_NAME = "VE SYSTEM WINDOW";
	inline VeHandle VE_SYSTEM_HANDLE = VE_NULL_HANDLE;

	std::vector<const char*> getRequiredInstanceExtensions();
	bool createSurface(VkInstance instance, VkSurfaceKHR* pSurface);

	void init();
	void update(sysmes::VeMessageTableEntry e);
	void close(sysmes::VeMessageTableEntry e);
	void closeWin();

	void windowSizeChanged();
}



#pragma once

/**
*
* \file
* \brief
*
* Details
*
*/


namespace vve {


	namespace syseng {

		inline const std::string VE_SYSTEM_NAME = "VE SYSTEM ENGINE";
		inline VeHandle VE_SYSTEM_HANDLE = VE_NULL_HANDLE;

		void		registerEntity(const std::string& name);
		VeHandle	getEntityHandle(const std::string& name);

		void		registerTablePointer(VeTable* ptr);
		VeTable*	getTablePointer(const std::string name);

		VeHandle getGUID();

		void		createHeaps(uint32_t num);
		VeHeapMemory* getHeap();
		VeHeapMemory* getTmpHeap();

		void		forwardTime();


		using namespace std::chrono;

		duration<double, std::micro>	  getTimeDelta();
		time_point<high_resolution_clock> getNowTime();
		time_point<high_resolution_clock> getCurrentUpdateTime();
		time_point<high_resolution_clock> getNextUpdateTime();
		time_point<high_resolution_clock> getReachedTime();

		void init();
		void runGameLoop();
		void computeOneFrame();
		void close();
	}

	inline VeHandle getGUID() {
		return syseng::getGUID();
	};

	inline 	VeHeapMemory* getHeap() {
		return syseng::getHeap();
	};

	inline 	VeHeapMemory* getTmpHeap() {
		return syseng::getTmpHeap();
	};

}


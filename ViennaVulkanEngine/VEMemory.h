#pragma once



namespace mem {

	//------------------------------------------------------------------------------------------------------

	class VeMap {
	protected:

	public:
		VeMap() {};
		virtual ~VeMap() {};
		virtual bool		getMappedIndex(		VeHandle& key, VeIndex& index) { assert(false); return false; };
		virtual uint32_t	getMappedIndices(	VeHandle& key, std::vector<VeIndex>& result) { assert(false); return 0; };
		virtual bool		getMappedIndex(		std::pair<VeHandle, VeHandle> &key, VeIndex& index) { assert(false); return false; };
		virtual uint32_t	getMappedndices(	std::pair<VeHandle, VeHandle> &key, std::vector<VeIndex>& result) { assert(false); return 0; };
		virtual bool		getMappedIndex(		std::string& key, VeIndex& index) { assert(false); return false; };
		virtual uint32_t	getMappedIndices(	std::string& key, std::vector<VeIndex>& result) { assert(false); return 0; };
		virtual uint32_t	getAllIndices(		std::vector<VeIndex>& result) { assert(false); return 0; };
		virtual void		insertIntoMap(		void *entry, VeIndex &dir_index ) { assert(false); };

		VeHandle getIntFromEntry(void* entry, VeIndex offset, VeIndex num_bytes ) {
			uint8_t* ptr = (uint8_t*)entry + offset;

			if (num_bytes == 4) {
				uint32_t* k1 = (uint32_t*)ptr;
				return (VeHandle)*k1;
			}
			uint64_t* k2 = (uint64_t*)ptr;
			return (VeHandle)*k2;
		};

		void getKey( void* entry, VeIndex offset, VeIndex num_bytes, VeHandle& key) {
			key = getIntFromEntry( entry, offset, num_bytes );
		};

		void getKey(	void* entry, std::pair<VeIndex, VeIndex> offset, 
						std::pair<VeIndex, VeIndex> num_bytes, std::pair<VeHandle, VeHandle>& key) {

			key = std::pair<VeHandle, VeHandle>(getIntFromEntry(entry, offset.first,  num_bytes.first), 
												getIntFromEntry(entry, offset.second, num_bytes.second));
		}

		void getKey( void* entry, VeIndex offset, VeIndex num_bytes, std::string &key) {
			uint8_t* ptr = (uint8_t*)entry + offset;
			std::string* pstring = (std::string*)ptr;
			key = *pstring;
		}

	};


	template <typename M, typename K, typename I>
	class VeTypedMap : public VeMap {
	protected:

		I	m_offset;		///
		I	m_num_bytes;	///
		M	m_map;			///key value pairs - value is the index of the entry in the table

	public:
		VeTypedMap(	I offset, I num_bytes ) : VeMap(), m_offset(offset), m_num_bytes(num_bytes) {};
		virtual ~VeTypedMap() {};

		virtual bool getMappedIndex( K & key, VeIndex &index ) override {
			auto search = m_map.find(key);
			if (search == m_map.end()) return false;
			index = search->second;
			return true;
		}

		virtual uint32_t getMappedIndices(K & key, std::vector<VeIndex>& result) override {
			uint32_t num = 0;
			auto range = m_map.equal_range( key );
			for (auto it = range.first; it != range.second; ++it, ++num) result.push_back( it->second );
			return num;
		};

		virtual uint32_t getAllIndices( std::vector<VeIndex>& result) override {
			uint32_t num = 0;
			for (auto entry : m_map) { ++num; result.emplace_back(entry.second); }
			return num;
		}

		virtual void insertIntoMap( void* entry, VeIndex& dir_index ) override {
			K key;
			getKey( entry, m_offset, m_num_bytes, key);
			m_map.try_emplace( key, dir_index );
		};
	};



	//------------------------------------------------------------------------------------------------------

	class VeDirectory {
	protected:

		struct VeDirectoryEntry {
			VeIndex	m_auto_id = VE_NULL_INDEX;
			VeIndex	m_table_index = VE_NULL_INDEX;	///index into the entry table
			VeIndex	m_next_free = VE_NULL_INDEX;	///index of next free entry in directory

			VeDirectoryEntry( VeIndex auto_id, VeIndex table_index, VeIndex next_free) : 
				m_auto_id(auto_id), m_table_index(table_index), m_next_free(next_free) {}
		};

		VeIndex							m_auto_counter = 0;				///
		std::vector<VeDirectoryEntry>	m_dir_entries;					///1 level of indirection, idx into the entry table
		VeIndex							m_first_free = VE_NULL_INDEX;	///index of first free entry in directory

		VeHandle addNewEntry(VeIndex table_index ) {
			VeIndex auto_id = ++m_auto_counter;
			m_dir_entries.emplace_back( auto_id, table_index, m_first_free );
			VeIndex dir_index = (VeIndex)m_dir_entries.size() - 1;
			return (VeHandle) auto_id << 32 & dir_index;
		}

		VeHandle writeOverOldEntry(VeIndex table_index) {
			VeIndex auto_id				= ++m_auto_counter;
			VeIndex dir_index			= m_first_free;
			VeIndex next_free			= m_dir_entries[dir_index].m_next_free;
			m_dir_entries[dir_index]	= { auto_id, table_index, VE_NULL_INDEX };
			m_first_free				= next_free;
			return (VeHandle)auto_id << 32 & dir_index;
		}

	public:
		VeDirectory() {};
		~VeDirectory() {};

		VeHandle addEntry( VeIndex table_index ) {
			VeHandle handle;
			if (m_first_free == VE_NULL_INDEX) handle = addNewEntry(table_index);
			else handle = writeOverOldEntry(table_index);
			return handle;
		};

		VeDirectoryEntry& getEntry( VeIndex dir_index) { return m_dir_entries[dir_index];  };

		void removeEntry( VeIndex dir_index ) {
			m_dir_entries[m_first_free].m_next_free = m_first_free;
			m_first_free = dir_index;
		}
	};


	//------------------------------------------------------------------------------------------------------

	class VeFixedSizeTable {
	protected:
		VeIndex	m_thread_id;	///id of thread that accesses to this table are scheduled to

	public:
		VeFixedSizeTable( VeIndex thread_id ) : m_thread_id(thread_id) {};
		virtual ~VeFixedSizeTable() {};
		VeIndex	getThreadId() { return m_thread_id; };
	};


	//------------------------------------------------------------------------------------------------------

	template <typename T>
	class VeFixedSizeTypedTable : public VeFixedSizeTable {
	protected:
		std::vector<VeMap*>		m_maps;				///vector of maps for quickly finding or sorting entries
		VeDirectory				m_directory;		///
		std::vector<T>			m_data;				///growable entry data table
		std::vector<VeIndex>	m_tbl2dir;

	public:

		VeFixedSizeTypedTable( std::vector<VeMap*> &&maps, VeIndex thread_id = VE_NULL_INDEX) : VeFixedSizeTable( thread_id ) {
			m_maps = std::move(maps);	
		};

		~VeFixedSizeTypedTable() { for (uint32_t i = 0; i < m_maps.size(); ++i ) delete m_maps[i]; };

		std::vector<T>& getData() { return m_data; };

		VeHandle	addEntry(T& te);
		bool		getEntryFromHandle(VeHandle key, T& entry);
		VeIndex		getIndexFromHandle(VeHandle key);
		VeHandle	getHandleFromIndex(VeIndex table_index);
		bool		deleteEntryByHandle(VeHandle key);

		bool		getEntryFromMap(VeIndex num_map, VeHandle key, T& entry);
		bool		getEntryFromMap(VeIndex num_map, std::pair<VeHandle,VeHandle> &key, T& entry);
		bool		getEntryFromMap(VeIndex num_map, std::string key, T& entry);

		uint32_t	getEntriesFromMap(VeIndex num_map, VeHandle key, std::vector<T>& result);
		uint32_t	getEntriesFromMap(VeIndex num_map, std::pair<VeHandle, VeHandle>& key, std::vector<T>& result);
		uint32_t	getEntriesFromMap(VeIndex num_map, std::string& key, std::vector<T>& result);

		uint32_t	getTableIndices(VeIndex num_map, std::vector<VeIndex>& result);
		uint32_t	getTableEntries(VeIndex num_map, std::vector<T>& result);

		uint32_t	deleteEntriesByMap(VeIndex num_map, VeHandle key );
		uint32_t	deleteEntriesByMap(VeIndex num_map, std::pair<VeHandle, VeHandle>& key);
		uint32_t	deleteEntriesByMap(VeIndex num_map, std::string& key);
	};

	//--------------------------------------------------------------------------------------------------------------------------

	template<typename T> inline VeHandle VeFixedSizeTypedTable<T>::addEntry(T& te) {
		VeIndex table_index = (VeIndex)m_data.size();
		m_data.emplace_back(te);

		VeHandle handle = m_directory.addEntry( table_index );
		VeIndex dir_index = handle & VE_NULL_INDEX;
		m_tbl2dir.emplace_back(dir_index);
		for (auto map : m_maps) map->insertIntoMap( (void*)&te, dir_index );
		return handle;
	};

	template<typename T> inline bool VeFixedSizeTypedTable<T>::getEntryFromHandle( VeHandle key, T& entry ) {
		VeIndex dir_index	= (VeIndex)(key & VE_NULL_INDEX);
		VeIndex auto_id		= (VeIndex)(key >> 32);
		if (auto_id != m_data[m_directory.getEntry(dir_index).m_auto_id]) return false;
		entry = m_data[m_directory.getEntry(dir_index).m_table_index];
		return true;
	};

	template<typename T> inline VeIndex VeFixedSizeTypedTable<T>::getIndexFromHandle(VeHandle key) {
		VeIndex dir_index = (VeIndex)(key & VE_NULL_INDEX);
		VeIndex auto_id = (VeIndex)(key >> 32);
		if (auto_id != m_data[m_directory.getEntry(dir_index).m_auto_id]) return VE_NULL_INDEX;
		return m_data[m_directory.getEntry(dir_index).m_table_index];
	};

	template<typename T> inline VeHandle VeFixedSizeTypedTable<T>::getHandleFromIndex(VeIndex table_index) {
		VeIndex dir_index = m_tbl2dir[table_index];
		VeIndex auto_id = m_data[m_directory.getEntry(dir_index).m_auto_id];
		return (VeHandle)auto_id << 32 & dir_index;
	};

	template<typename T> inline bool VeFixedSizeTypedTable<T>::deleteEntryByHandle(VeHandle key) {
		VeIndex dir_index = (VeIndex)(key & VE_NULL_INDEX);
		VeIndex auto_id = (VeIndex)(key >> 32);
		if (auto_id != m_data[m_directory.getEntry(dir_index).m_auto_id]) return false;

		return true;
	};

	//--------------------------------------------------------------------------------------------------------------------------

	template<typename T> inline bool VeFixedSizeTypedTable<T>::getEntryFromMap(VeIndex num_map, VeHandle key, T& entry) {
		VeIndex dir_index;
		if (!m_maps[num_map]->getMappedIndex(key, dir_index)) return false;
		entry = m_data[m_directory.getEntry(dir_index).m_table_index];
		return true;
	};

	template<typename T> inline bool VeFixedSizeTypedTable<T>::getEntryFromMap(VeIndex num_map, std::pair<VeHandle, VeHandle>& key, T& entry) {
		VeIndex dir_index;
		if (!m_maps[num_map]->getMappedIndex(key, dir_index)) return false;
		entry = m_data[m_directory.getEntry(dir_index).m_table_index];
		return true;
	};

	template<typename T> inline	bool VeFixedSizeTypedTable<T>::getEntryFromMap(VeIndex num_map, std::string key, T& entry) {
		VeIndex dir_index;
		if (!m_maps[num_map]->getMappedIndex(key, dir_index)) return false;
		entry = m_data[m_directory.getEntry(dir_index).m_table_index];
		return true;
	};

	//--------------------------------------------------------------------------------------------------------------------------

	template<typename T> inline	uint32_t VeFixedSizeTypedTable<T>::getEntriesFromMap(VeIndex num_map, VeHandle key, std::vector<T>& result) {
		std::vector<VeIndex> dir_indices;
		uint32_t num = m_maps[num_map]->getMappedIndices(key, dir_indices);
		for (auto dir_index : dir_indices) result.emplace_back( m_data[m_directory.getEntry(dir_index).m_table_index] );
		return num;
	};

	template<typename T> inline	uint32_t VeFixedSizeTypedTable<T>::getEntriesFromMap(	VeIndex num_map,
																				std::pair<VeHandle, VeHandle>& key, 
																				std::vector<T>& result) {
		std::vector<VeIndex> dir_indices;
		uint32_t num = m_maps[num_map]->getMappedIndices(key, dir_indices);
		for (auto dir_index : dir_indices) result.emplace_back(m_data[m_directory.getEntry(dir_index).m_table_index]);
		return num;
	};

	template<typename T> inline	uint32_t VeFixedSizeTypedTable<T>::getEntriesFromMap(VeIndex num_map, std::string& key, std::vector<T>& result) {
		std::vector<VeIndex> dir_indices;
		uint32_t num = m_maps[num_map]->getMappedIndices(key, dir_indices);
		for (auto dir_index : dir_indices) result.emplace_back(m_data[m_directory.getEntry(dir_index).m_table_index]);
		return num;
	};

	//--------------------------------------------------------------------------------------------------------------------------

	template<typename T> inline	uint32_t VeFixedSizeTypedTable<T>::getTableIndices( VeIndex num_map, std::vector<VeIndex>& result) {
		std::vector<VeIndex> dir_indices;
		uint32_t num = m_maps[num_map]->getAllIndices(dir_indices);
		for (auto dir_index : dir_indices) result.emplace_back( m_directory.getEntry(dir_index).m_table_index );
		return num;
	};

	template<typename T> inline	uint32_t VeFixedSizeTypedTable<T>::getTableEntries(VeIndex num_map, std::vector<T>& result) {
		std::vector<VeIndex> dir_indices;
		uint32_t num = m_maps[num_map]->getAllIndices(dir_indices);
		for (auto dir_index : dir_indices) result.emplace_back( m_data[m_directory.getEntry(dir_index).m_table_index] );
		return num;
	};

	//--------------------------------------------------------------------------------------------------------------------------

	template<typename T> inline	uint32_t VeFixedSizeTypedTable<T>::deleteEntries(VeIndex num_map, VeHandle key) {


		return 0;
	};

	template<typename T> inline	uint32_t VeFixedSizeTypedTable<T>::deleteEntries(VeIndex num_map, std::pair<VeHandle, VeHandle>& key) {
		return 0;
	};

	template<typename T> inline	uint32_t VeFixedSizeTypedTable<T>::deleteEntries(VeIndex num_map, std::string& key) {
		return 0;
	};



	///-------------------------------------------------------------------------------
	struct VariableSizeTable {
		//TableSortIndex		m_indices;
		std::vector<uint8_t>	m_data;
	};


}



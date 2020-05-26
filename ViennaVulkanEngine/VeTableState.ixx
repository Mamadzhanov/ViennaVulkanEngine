export module VVE:VeTableState;

import std.core;
import :VeTypes;
import :VeMap;
import :VeMemory;
import :VeTableChunk;


export namespace vve {


	//----------------------------------------------------------------------------------
	// Delare VeTableState
	template< typename... Types> struct VeTableState;

	#define VeTableStateType VeTableState< Typelist < TypesOne... >, Typelist < TypesTwo... > >

	//----------------------------------------------------------------------------------
	// Specialization of VeTableState
	template< typename... TypesOne, typename... TypesTwo>
	struct VeTableStateType {

		using tuple_type = std::tuple<TypesOne...>;
		using chunk_type = VeTableChunk<TypesOne...>;
		static_assert(sizeof(chunk_type) <= VE_TABLE_CHUNK_SIZE);

		std::vector<std::unique_ptr<chunk_type>>	m_chunks;				///pointers to table chunks
		std::set<VeChunkIndex32>					m_free_chunks;			///chunks that are not full
		std::set<VeChunkIndex32>					m_deleted_chunks;		///chunks that have been deleted -> empty slot

		using map_type = std::tuple<VeHashMap<0>, TypesTwo...>;
		map_type m_maps;

	public:
		VeTableState();
		~VeTableState() = default;

		//-------------------------------------------------------------------------------
		//read operations


		//-------------------------------------------------------------------------------
		//write operations

		void operator=(const VeTableStateType& rhs);
		void clear();
	};

	//----------------------------------------------------------------------------------
	template<typename... TypesOne, typename... TypesTwo>
	VeTableStateType::VeTableState() : m_maps() {
		m_chunks.emplace_back(std::make_unique<chunk_type>());
	};


	//-------------------------------------------------------------------------------
	//write operations

	template<typename... TypesOne, typename... TypesTwo>
	void VeTableStateType::operator=(const VeTableStateType& rhs) {

	}

	template<typename... TypesOne, typename... TypesTwo>
	void VeTableStateType::clear() {

	}



};



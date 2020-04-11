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


namespace vve::sysmes {


	//--------------------------------------------------------------------------------------------------
	//messages 

	std::vector<VeMap*> maps1 = {
		new VeHashedMultimap< VeKey, VeIndex >((VeIndex)offsetof(VeMessage, m_senderID), (VeIndex)sizeof(VeMessage::m_senderID)),
		new VeHashedMultimap< VeKey, VeIndex >((VeIndex)offsetof(VeMessage, m_receiverID), (VeIndex)sizeof(VeMessage::m_receiverID)),
		new VeHashedMultimap< VeKeyPair, VeIndexPair >(
			VeIndexPair((VeIndex)offsetof(VeMessage, m_senderID), (VeIndex)offsetof(VeMessage, m_receiverID) ),
			VeIndexPair((VeIndex)sizeof(VeMessage::m_senderID), (VeIndex)sizeof(VeMessage::m_receiverID) ) )
	};
	VeFixedSizeTableMT<VeMessage> g_messages_table( "Messages Table", maps1, true, true, 0, 0);
	VeFixedSizeTableMT<VeMessage> g_messages_table2(g_messages_table);


	std::vector<VeMap*> maps2 = {
		new VeHashedMultimap< VeKeyPair, VeIndexPair >(
			VeIndexPair((VeIndex)offsetof(VeMessage, m_senderID), (VeIndex)offsetof(VeMessage, m_receiverID)),
			VeIndexPair((VeIndex)sizeof(VeMessage::m_senderID), (VeIndex)sizeof(VeMessage::m_receiverID)))
	};
	VeFixedSizeTableMT<VeMessage> g_continuous_messages_table("Continuous Messages Table", maps2, true, false, 0, 0);
	VeFixedSizeTableMT<VeMessage> g_continuous_messages_table2(g_continuous_messages_table);


	//--------------------------------------------------------------------------------------------------
	//message handler

	struct VeMessageHandlerTableEntry {
		std::function<void(VeHandle)> m_handler;
	};
	VeFixedSizeTableMT<VeMessageHandlerTableEntry> g_handler_table("Message Handler Table", false, false, 0, 0);
	VeFixedSizeTableMT<VeMessageHandlerTableEntry> g_handler_table2(g_handler_table);


	//--------------------------------------------------------------------------------------------------
	//subscribers

	struct VeMessageSubscribeTableEntry {
		VeHandle				m_senderID;		//handle of the sending entity
		VeHandle				m_receiverID;	//handle of the receiver entity
		VeMessageType			m_type;			//filter on type of message
		VeHandle				m_handlerID;	//handler of this message
		vgjs::VgjsThreadIndex	m_thread_idx;	//run handler function in this thread or NULL
	};
	std::vector<VeMap*> maps4 = {
		new VeHashedMultimap< VeKeyPair, VeIndexPair >(
			VeIndexPair((VeIndex)offsetof(VeMessageSubscribeTableEntry, m_receiverID),	(VeIndex)offsetof(VeMessageSubscribeTableEntry, m_type)),
			VeIndexPair((VeIndex)sizeof(VeMessageSubscribeTableEntry::m_receiverID),	(VeIndex)sizeof(VeMessageSubscribeTableEntry::m_type))),
		new VeHashedMultimap< VeKey, VeIndex >((VeIndex)offsetof(VeMessageSubscribeTableEntry, m_handlerID), (VeIndex)sizeof(VeMessageSubscribeTableEntry::m_handlerID)),
		new VeHashedMultimap< VeKeyPair, VeIndexPair >(
			VeIndexPair((VeIndex)offsetof(VeMessageSubscribeTableEntry, m_senderID),	(VeIndex)offsetof(VeMessageSubscribeTableEntry, m_type)),
			VeIndexPair((VeIndex)sizeof(VeMessageSubscribeTableEntry::m_senderID),		(VeIndex)sizeof(VeMessageSubscribeTableEntry::m_type))),
	};
	VeFixedSizeTableMT<VeMessageSubscribeTableEntry> g_subscribe_table("Message Subscribe Table", maps4, true, false, 0, 0);
	VeFixedSizeTableMT<VeMessageSubscribeTableEntry> g_subscribe_table2(g_subscribe_table);


	//--------------------------------------------------------------------------------------------------
	//received messages

	struct VeMessageReceiveTableEntry {
		VeMessagePhase	m_phase;
		VeHandle		m_receiverID;	//handle of the receiver entity
		VeHandle		m_messageID;	//handle of message
	};
	std::vector<VeMap*> maps5 = {
		new VeHashedMultimap< VeKey, VeIndex >((VeIndex)offsetof(VeMessageReceiveTableEntry, m_receiverID), (VeIndex)sizeof(VeMessageReceiveTableEntry::m_receiverID))
	};
	VeFixedSizeTableMT<VeMessageReceiveTableEntry> g_receive_table("Message Receive Table", maps5, true, true, 0, 0);
	VeFixedSizeTableMT<VeMessageReceiveTableEntry> g_receive_table2(g_receive_table);


	//--------------------------------------------------------------------------------------------------
	//handler calls

	struct VeMessagrCallTableEntry {
		VeMessagePhase			m_phase;
		VeHandle				m_receiverID;	//handle of the receiver entity
		VeHandle				m_handlerID;	//handler of message
		vgjs::VgjsThreadIndex	m_thread_idx;	//special thread to run on
	};
	std::vector<VeMap*> maps6 = {
		new VeHashedMap< VeKeyPair, VeIndexPair >(
			VeIndexPair((VeIndex)offsetof(VeMessagrCallTableEntry, m_receiverID),(VeIndex)offsetof(VeMessagrCallTableEntry, m_handlerID)),
			VeIndexPair((VeIndex)sizeof(VeMessagrCallTableEntry::m_receiverID),  (VeIndex)sizeof(VeMessagrCallTableEntry::m_handlerID) ) )
	};
	VeFixedSizeTableMT<VeMessagrCallTableEntry> g_calls_table("Message Calls Table", maps6, true, true, 0, 0);
	VeFixedSizeTableMT<VeMessagrCallTableEntry> g_calls_table2(g_calls_table);


	//--------------------------------------------------------------------------------------------------
	//functions

	void init() {
		syseng::registerEntity(VE_SYSTEM_NAME);
		VE_SYSTEM_HANDLE = syseng::getEntityHandle(VE_SYSTEM_NAME);

		syseng::registerTablePointer(&g_messages_table);
		syseng::registerTablePointer(&g_continuous_messages_table);
		syseng::registerTablePointer(&g_handler_table);
		syseng::registerTablePointer(&g_subscribe_table);
		syseng::registerTablePointer(&g_receive_table);
		syseng::registerTablePointer(&g_calls_table);
	}

	void callAllMessages() {
		for (auto call : g_calls_table.data()) {
			VeMessageHandlerTableEntry handlerData;
			g_handler_table.getEntry(call.m_handlerID, handlerData);
			JADDT( handlerData.m_handler(call.m_receiverID), vgjs::TID(call.m_thread_idx) );
		}
	}

	void preupdate() {
		//swap tables
	}

	void update() {
		callAllMessages();
	}

	void postupdate() {
		for (auto message : g_continuous_messages_table.data())
			recordMessage(message);
	}

	void close() {
	}


	VeHandle recordMessage(VeMessage message ) {
		if (JIDX != g_messages_table.getThreadIdx()) {
			JADDT(recordMessage(message), vgjs::TID(g_messages_table.getThreadIdx()));
			return VE_NULL_HANDLE;
		};

		//std::cout << "add message type " << (VeIndex)message.m_type << " action " << message.m_action << " key/button " << message.m_key_button << std::endl;

		//insert into messages table and get a messageID
		VeHandle messageID = g_messages_table.insert(message);

		//find all subscriptions fitting to this message
		std::vector<VeHandle, custom_alloc<VeHandle>> result(getThreadTmpHeap());
		if (message.m_receiverID == VE_NULL_HANDLE) {
			g_subscribe_table.getHandlesEqual(VeKeyPair{ (VeKey)message.m_senderID,   (VeKey)message.m_type }, 2, result);
		} else {
			g_subscribe_table.getHandlesEqual(VeKeyPair{ (VeKey)message.m_receiverID, (VeKey)message.m_type }, 0, result);
		}

		for (VeHandle& handle : result) {
			VeMessageSubscribeTableEntry subscribeData;
			g_subscribe_table.getEntry(handle, subscribeData);
			g_receive_table.insert({ getPhaseFromType(message.m_type), messageID, subscribeData.m_receiverID }); //accumulate messages for each receiver
			g_calls_table.insert({ getPhaseFromType(message.m_type), subscribeData.m_receiverID, subscribeData.m_handlerID, subscribeData.m_thread_idx }); //will be added only once due to map
		}

		return messageID;
	}


	void receiveMessages(VeHandle receiverID, std::vector<VeHandle, custom_alloc<VeHandle>>& result) {
		g_messages_table.getHandlesEqual((VeKey)receiverID, 1, result);
	}

	bool getMessage(VeHandle messageID, VeMessage& entry) {
		return g_messages_table.getEntry(messageID, entry);
	}


	VeHandle addContinuousMessage(VeMessage message ) {
		//std::cout << "add cont message type " << message.m_type << " action " << message.m_action << " key/button " << message.m_key_button << std::endl;
		return g_continuous_messages_table.insert(message);
	}

	void removeContinuousMessage(VeMessage message) {
		//std::cout << "remove cont message type " << message.m_type << " action " << message.m_action << " key/button " << message.m_key_button << std::endl;

		VeHandle messageID = g_continuous_messages_table.find(VeKeyPair{ (VeKey)message.m_senderID, (VeKey)message.m_receiverID }, 0);
		assert(messageID != VE_NULL_HANDLE);
		g_continuous_messages_table.erase(messageID);
	}

	VeHandle addHandler(std::function<void(VeHandle)> handler) {
		return g_handler_table.insert({ handler });
	}

	void removeHandler(VeHandle handlerID) {
		std::vector<VeHandle, custom_alloc<VeHandle>> result(getThreadTmpHeap());
		g_subscribe_table.getHandlesEqual((VeKey)handlerID, 1, result);
		for (auto handle : result) {
			g_subscribe_table.erase(handle);
		}
		g_handler_table.erase(handlerID);
	}

	void subscribeMessage(VeHandle senderID, VeHandle receiverID, VeHandle handlerID, VeMessageType type, vgjs::VgjsThreadIndex thread_idx) {
		g_subscribe_table.insert({ senderID, receiverID, type, handlerID, thread_idx });
	}

	void unsubscribeMessage(VeHandle senderID, VeHandle receiverID, VeMessageType type) {
		VeHandle subID = g_subscribe_table.find(VeKeyTriple{ (VeKey)senderID, (VeKey)receiverID, (VeKey)type }, 2);
		g_subscribe_table.erase(subID);
	}

}


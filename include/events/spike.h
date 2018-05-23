/**
 * @file spike.h
 *
 * Spike Events format definition and handling functions.
 * This contains spikes generated by a neuron-array chip.
 */

#ifndef LIBCAER_EVENTS_SPIKE_H_
#define LIBCAER_EVENTS_SPIKE_H_

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shift and mask values for spike information associated
 * with a Spike event.
 * 32 core IDs, 64 chip IDs and up to a million neuron IDs
 * are supported.
 * Bit 0 is the valid mark, see 'common.h' for more details.
 */
//@{
#define SPIKE_SOURCE_CORE_ID_SHIFT 1
#define SPIKE_SOURCE_CORE_ID_MASK 0x0000001F
#define SPIKE_CHIP_ID_SHIFT 6
#define SPIKE_CHIP_ID_MASK 0x0000003F
#define SPIKE_NEURON_ID_SHIFT 12
#define SPIKE_NEURON_ID_MASK 0x000FFFFF
//@}

/**
 * Spike event data structure definition.
 * This contains the core ID, the neuron ID and the timestamp
 * of the received spike, together with the usual validity mark.
 * Signed integers are used for fields that are to be interpreted
 * directly, for compatibility with languages that do not have
 * unsigned integer types, such as Java.
 */
PACKED_STRUCT(
struct caer_spike_event {
	/// Event information. First because of valid mark.
	uint32_t data;
	/// Event timestamp.
	int32_t timestamp;
});

/**
 * Type for pointer to Spike event data structure.
 */
typedef struct caer_spike_event *caerSpikeEvent;
typedef const struct caer_spike_event *caerSpikeEventConst;

/**
 * Spike event packet data structure definition.
 * EventPackets are always made up of the common packet header,
 * followed by 'eventCapacity' events. Everything has to
 * be in one contiguous memory block.
 */
PACKED_STRUCT(
struct caer_spike_event_packet {
	/// The common event packet header.
	struct caer_event_packet_header packetHeader;
	/// The events array.
	struct caer_spike_event events[];
});

/**
 * Type for pointer to Spike event packet data structure.
 */
typedef struct caer_spike_event_packet *caerSpikeEventPacket;
typedef const struct caer_spike_event_packet *caerSpikeEventPacketConst;

/**
 * Allocate a new Spike events packet.
 * Use free() to reclaim this memory.
 *
 * @param eventCapacity the maximum number of events this packet will hold.
 * @param eventSource the unique ID representing the source/generator of this packet.
 * @param tsOverflow the current timestamp overflow counter value for this packet.
 *
 * @return a valid SpikeEventPacket handle or NULL on error.
 */
static inline caerSpikeEventPacket caerSpikeEventPacketAllocate(int32_t eventCapacity, int16_t eventSource, int32_t tsOverflow) {
	return ((caerSpikeEventPacket) caerEventPacketAllocate(eventCapacity, eventSource, tsOverflow, SPIKE_EVENT,
		sizeof(struct caer_spike_event), offsetof(struct caer_spike_event, timestamp)));
}

/**
 * Transform a generic event packet header into a Spike event packet.
 * This takes care of proper casting and checks that the packet type really matches
 * the intended conversion type.
 *
 * @param header a valid event packet header pointer. Cannot be NULL.
 * @return a properly converted, typed event packet pointer.
 */
static inline caerSpikeEventPacket caerSpikeEventPacketFromPacketHeader(caerEventPacketHeader header) {
	if (caerEventPacketHeaderGetEventType(header) != SPIKE_EVENT) {
		return (NULL);
	}

	return ((caerSpikeEventPacket) header);
}

/**
 * Transform a generic read-only event packet header into a read-only Spike event packet.
 * This takes care of proper casting and checks that the packet type really matches
 * the intended conversion type.
 *
 * @param header a valid read-only event packet header pointer. Cannot be NULL.
 * @return a properly converted, read-only typed event packet pointer.
 */
static inline caerSpikeEventPacketConst caerSpikeEventPacketFromPacketHeaderConst(caerEventPacketHeaderConst header) {
	if (caerEventPacketHeaderGetEventType(header) != SPIKE_EVENT) {
		return (NULL);
	}

	return ((caerSpikeEventPacketConst) header);
}

/**
 * Get the Spike event at the given index from the event packet.
 *
 * @param packet a valid SpikeEventPacket pointer. Cannot be NULL.
 * @param n the index of the returned event. Must be within [0,eventCapacity[ bounds.
 *
 * @return the requested Spike event. NULL on error.
 */
static inline caerSpikeEvent caerSpikeEventPacketGetEvent(caerSpikeEventPacket packet, int32_t n) {
	// Check that we're not out of bounds.
	if (n < 0 || n >= caerEventPacketHeaderGetEventCapacity(&packet->packetHeader)) {
		caerLogEHO(CAER_LOG_CRITICAL, "Spike Event",
			"Called caerSpikeEventPacketGetEvent() with invalid event offset %" PRIi32 ", while maximum allowed value is %" PRIi32 ".",
			n, caerEventPacketHeaderGetEventCapacity(&packet->packetHeader) - 1);
		return (NULL);
	}

	// Return a pointer to the specified event.
	return (packet->events + n);
}

/**
 * Get the Spike event at the given index from the event packet.
 * This is a read-only event, do not change its contents in any way!
 *
 * @param packet a valid SpikeEventPacket pointer. Cannot be NULL.
 * @param n the index of the returned event. Must be within [0,eventCapacity[ bounds.
 *
 * @return the requested read-only Spike event. NULL on error.
 */
static inline caerSpikeEventConst caerSpikeEventPacketGetEventConst(caerSpikeEventPacketConst packet, int32_t n) {
	// Check that we're not out of bounds.
	if (n < 0 || n >= caerEventPacketHeaderGetEventCapacity(&packet->packetHeader)) {
		caerLogEHO(CAER_LOG_CRITICAL, "Spike Event",
			"Called caerSpikeEventPacketGetEventConst() with invalid event offset %" PRIi32 ", while maximum allowed value is %" PRIi32 ".",
			n, caerEventPacketHeaderGetEventCapacity(&packet->packetHeader) - 1);
		return (NULL);
	}

	// Return a pointer to the specified event.
	return (packet->events + n);
}

/**
 * Get the 32bit event timestamp, in microseconds.
 * Be aware that this wraps around! You can either ignore this fact,
 * or handle the special 'TIMESTAMP_WRAP' event that is generated when
 * this happens, or use the 64bit timestamp which never wraps around.
 * See 'caerEventPacketHeaderGetEventTSOverflow()' documentation
 * for more details on the 64bit timestamp.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 *
 * @return this event's 32bit microsecond timestamp.
 */
static inline int32_t caerSpikeEventGetTimestamp(caerSpikeEventConst event) {
	return (le32toh(event->timestamp));
}

/**
 * Get the 64bit event timestamp, in microseconds.
 * See 'caerEventPacketHeaderGetEventTSOverflow()' documentation
 * for more details on the 64bit timestamp.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 * @param packet the SpikeEventPacket pointer for the packet containing this event. Cannot be NULL.
 *
 * @return this event's 64bit microsecond timestamp.
 */
static inline int64_t caerSpikeEventGetTimestamp64(caerSpikeEventConst event, caerSpikeEventPacketConst packet) {
	return (I64T(
		(U64T(caerEventPacketHeaderGetEventTSOverflow(&packet->packetHeader)) << TS_OVERFLOW_SHIFT) | U64T(caerSpikeEventGetTimestamp(event))));
}

/**
 * Set the 32bit event timestamp, the value has to be in microseconds.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 * @param timestamp a positive 32bit microsecond timestamp.
 */
static inline void caerSpikeEventSetTimestamp(caerSpikeEvent event, int32_t timestamp) {
	if (timestamp < 0) {
		// Negative means using the 31st bit!
		caerLogEHO(CAER_LOG_CRITICAL, "Spike Event", "Called caerSpikeEventSetTimestamp() with negative value!");
		return;
	}

	event->timestamp = htole32(timestamp);
}

/**
 * Check if this Spike event is valid.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 *
 * @return true if valid, false if not.
 */
static inline bool caerSpikeEventIsValid(caerSpikeEventConst event) {
	return (GET_NUMBITS32(event->data, VALID_MARK_SHIFT, VALID_MARK_MASK));
}

/**
 * Validate the current event by setting its valid bit to true
 * and increasing the event packet's event count and valid
 * event count. Only works on events that are invalid.
 * DO NOT CALL THIS AFTER HAVING PREVIOUSLY ALREADY
 * INVALIDATED THIS EVENT, the total count will be incorrect.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 * @param packet the SpikeEventPacket pointer for the packet containing this event. Cannot be NULL.
 */
static inline void caerSpikeEventValidate(caerSpikeEvent event, caerSpikeEventPacket packet) {
	if (!caerSpikeEventIsValid(event)) {
		SET_NUMBITS32(event->data, VALID_MARK_SHIFT, VALID_MARK_MASK, 1);

		// Also increase number of events and valid events.
		// Only call this on (still) invalid events!
		caerEventPacketHeaderSetEventNumber(&packet->packetHeader,
			caerEventPacketHeaderGetEventNumber(&packet->packetHeader) + 1);
		caerEventPacketHeaderSetEventValid(&packet->packetHeader,
			caerEventPacketHeaderGetEventValid(&packet->packetHeader) + 1);
	}
	else {
		caerLogEHO(CAER_LOG_CRITICAL, "Spike Event", "Called caerSpikeEventValidate() on already valid event.");
	}
}

/**
 * Invalidate the current event by setting its valid bit
 * to false and decreasing the number of valid events held
 * in the packet. Only works with events that are already
 * valid!
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 * @param packet the SpikeEventPacket pointer for the packet containing this event. Cannot be NULL.
 */
static inline void caerSpikeEventInvalidate(caerSpikeEvent event, caerSpikeEventPacket packet) {
	if (caerSpikeEventIsValid(event)) {
		CLEAR_NUMBITS32(event->data, VALID_MARK_SHIFT, VALID_MARK_MASK);

		// Also decrease number of valid events. Number of total events doesn't change.
		// Only call this on valid events!
		caerEventPacketHeaderSetEventValid(&packet->packetHeader,
			caerEventPacketHeaderGetEventValid(&packet->packetHeader) - 1);
	}
	else {
		caerLogEHO(CAER_LOG_CRITICAL, "Spike Event", "Called caerSpikeEventInvalidate() on already invalid event.");
	}
}

/**
 * Get the source core ID.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 *
 * @return the Spike's source core ID.
 */
static inline uint8_t caerSpikeEventGetSourceCoreID(caerSpikeEventConst event) {
	return U8T(GET_NUMBITS32(event->data, SPIKE_SOURCE_CORE_ID_SHIFT, SPIKE_SOURCE_CORE_ID_MASK));
}

/**
 * Set the source core ID.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 * @param sourceCoreID the Spike's source core ID.
 */
static inline void caerSpikeEventSetSourceCoreID(caerSpikeEvent event, uint8_t sourceCoreID) {
	CLEAR_NUMBITS32(event->data, SPIKE_SOURCE_CORE_ID_SHIFT, SPIKE_SOURCE_CORE_ID_MASK);
	SET_NUMBITS32(event->data, SPIKE_SOURCE_CORE_ID_SHIFT, SPIKE_SOURCE_CORE_ID_MASK, sourceCoreID);
}

/**
 * Get the chip ID.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 *
 * @return the Spike's chip ID.
 */
static inline uint8_t caerSpikeEventGetChipID(caerSpikeEventConst event) {
	return U8T(GET_NUMBITS32(event->data, SPIKE_CHIP_ID_SHIFT, SPIKE_CHIP_ID_MASK));
}

/**
 * Set the chip ID.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 * @param chipID the Spike's chip ID.
 */
static inline void caerSpikeEventSetChipID(caerSpikeEvent event, uint8_t chipID) {
	CLEAR_NUMBITS32(event->data, SPIKE_CHIP_ID_SHIFT, SPIKE_CHIP_ID_MASK);
	SET_NUMBITS32(event->data, SPIKE_CHIP_ID_SHIFT, SPIKE_CHIP_ID_MASK, chipID);
}

/**
 * Get the neuron ID.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 *
 * @return the Spike's neuron ID.
 */
static inline uint32_t caerSpikeEventGetNeuronID(caerSpikeEventConst event) {
	return U32T(GET_NUMBITS32(event->data, SPIKE_NEURON_ID_SHIFT, SPIKE_NEURON_ID_MASK));
}

/**
 * Set the neuron ID.
 *
 * @param event a valid SpikeEvent pointer. Cannot be NULL.
 * @param neuronID the Spike's neuron ID.
 */
static inline void caerSpikeEventSetNeuronID(caerSpikeEvent event, uint32_t neuronID) {
	CLEAR_NUMBITS32(event->data, SPIKE_NEURON_ID_SHIFT, SPIKE_NEURON_ID_MASK);
	SET_NUMBITS32(event->data, SPIKE_NEURON_ID_SHIFT, SPIKE_NEURON_ID_MASK, neuronID);
}

/**
 * Iterator over all Spike events in a packet.
 * Returns the current index in the 'caerSpikeIteratorCounter' variable of type
 * 'int32_t' and the current event in the 'caerSpikeIteratorElement' variable
 * of type caerSpikeEvent.
 *
 * SPIKE_PACKET: a valid SpikeEventPacket pointer. Cannot be NULL.
 */
#define CAER_SPIKE_ITERATOR_ALL_START(SPIKE_PACKET) \
	for (int32_t caerSpikeIteratorCounter = 0; \
		caerSpikeIteratorCounter < caerEventPacketHeaderGetEventNumber(&(SPIKE_PACKET)->packetHeader); \
		caerSpikeIteratorCounter++) { \
		caerSpikeEvent caerSpikeIteratorElement = caerSpikeEventPacketGetEvent(SPIKE_PACKET, caerSpikeIteratorCounter);

/**
 * Const-Iterator over all Spike events in a packet.
 * Returns the current index in the 'caerSpikeIteratorCounter' variable of type
 * 'int32_t' and the current read-only event in the 'caerSpikeIteratorElement' variable
 * of type caerSpikeEventConst.
 *
 * SPIKE_PACKET: a valid SpikeEventPacket pointer. Cannot be NULL.
 */
#define CAER_SPIKE_CONST_ITERATOR_ALL_START(SPIKE_PACKET) \
	for (int32_t caerSpikeIteratorCounter = 0; \
		caerSpikeIteratorCounter < caerEventPacketHeaderGetEventNumber(&(SPIKE_PACKET)->packetHeader); \
		caerSpikeIteratorCounter++) { \
		caerSpikeEventConst caerSpikeIteratorElement = caerSpikeEventPacketGetEventConst(SPIKE_PACKET, caerSpikeIteratorCounter);

/**
 * Iterator close statement.
 */
#define CAER_SPIKE_ITERATOR_ALL_END }

/**
 * Iterator over only the valid Spike events in a packet.
 * Returns the current index in the 'caerSpikeIteratorCounter' variable of type
 * 'int32_t' and the current event in the 'caerSpikeIteratorElement' variable
 * of type caerSpikeEvent.
 *
 * SPIKE_PACKET: a valid SpikeEventPacket pointer. Cannot be NULL.
 */
#define CAER_SPIKE_ITERATOR_VALID_START(SPIKE_PACKET) \
	for (int32_t caerSpikeIteratorCounter = 0; \
		caerSpikeIteratorCounter < caerEventPacketHeaderGetEventNumber(&(SPIKE_PACKET)->packetHeader); \
		caerSpikeIteratorCounter++) { \
		caerSpikeEvent caerSpikeIteratorElement = caerSpikeEventPacketGetEvent(SPIKE_PACKET, caerSpikeIteratorCounter); \
		if (!caerSpikeEventIsValid(caerSpikeIteratorElement)) { continue; } // Skip invalid Spike events.

/**
 * Const-Iterator over only the valid Spike events in a packet.
 * Returns the current index in the 'caerSpikeIteratorCounter' variable of type
 * 'int32_t' and the current read-only event in the 'caerSpikeIteratorElement' variable
 * of type caerSpikeEventConst.
 *
 * SPIKE_PACKET: a valid SpikeEventPacket pointer. Cannot be NULL.
 */
#define CAER_SPIKE_CONST_ITERATOR_VALID_START(SPIKE_PACKET) \
	for (int32_t caerSpikeIteratorCounter = 0; \
		caerSpikeIteratorCounter < caerEventPacketHeaderGetEventNumber(&(SPIKE_PACKET)->packetHeader); \
		caerSpikeIteratorCounter++) { \
		caerSpikeEventConst caerSpikeIteratorElement = caerSpikeEventPacketGetEventConst(SPIKE_PACKET, caerSpikeIteratorCounter); \
		if (!caerSpikeEventIsValid(caerSpikeIteratorElement)) { continue; } // Skip invalid Spike events.

/**
 * Iterator close statement.
 */
#define CAER_SPIKE_ITERATOR_VALID_END }

/**
 * Reverse iterator over all spike events in a packet.
 * Returns the current index in the 'caerSpikeIteratorCounter' variable of type
 * 'int32_t' and the current event in the 'caerSpikeIteratorElement' variable
 * of type caerSpikeEvent.
 *
 * SPIKE_PACKET: a valid SpikeEventPacket pointer. Cannot be NULL.
 */
#define CAER_SPIKE_REVERSE_ITERATOR_ALL_START(SPIKE_PACKET) \
	for (int32_t caerSpikeIteratorCounter = caerEventPacketHeaderGetEventNumber(&(SPIKE_PACKET)->packetHeader) - 1; \
		caerSpikeIteratorCounter >= 0; \
		caerSpikeIteratorCounter--) { \
		caerSpikeEvent caerSpikeIteratorElement = caerSpikeEventPacketGetEvent(SPIKE_PACKET, caerSpikeIteratorCounter);
/**
 * Const-Reverse iterator over all spike events in a packet.
 * Returns the current index in the 'caerSpikeIteratorCounter' variable of type
 * 'int32_t' and the current read-only event in the 'caerSpikeIteratorElement' variable
 * of type caerSpikeEventConst.
 *
 * SPIKE_PACKET: a valid SpikeEventPacket pointer. Cannot be NULL.
 */
#define CAER_SPIKE_CONST_REVERSE_ITERATOR_ALL_START(SPIKE_PACKET) \
	for (int32_t caerSpikeIteratorCounter = caerEventPacketHeaderGetEventNumber(&(SPIKE_PACKET)->packetHeader) - 1; \
		caerSpikeIteratorCounter >= 0; \
		caerSpikeIteratorCounter--) { \
		caerSpikeEventConst caerSpikeIteratorElement = caerSpikeEventPacketGetEventConst(SPIKE_PACKET, caerSpikeIteratorCounter);

/**
 * Reverse iterator close statement.
 */
#define CAER_SPIKE_REVERSE_ITERATOR_ALL_END }

/**
 * Reverse iterator over only the valid spike events in a packet.
 * Returns the current index in the 'caerSpikeIteratorCounter' variable of type
 * 'int32_t' and the current event in the 'caerSpikeIteratorElement' variable
 * of type caerSpikeEvent.
 *
 * SPIKE_PACKET: a valid SpikeEventPacket pointer. Cannot be NULL.
 */
#define CAER_SPIKE_REVERSE_ITERATOR_VALID_START(SPIKE_PACKET) \
	for (int32_t caerSpikeIteratorCounter = caerEventPacketHeaderGetEventNumber(&(SPIKE_PACKET)->packetHeader) - 1; \
		caerSpikeIteratorCounter >= 0; \
		caerSpikeIteratorCounter--) { \
		caerSpikeEvent caerSpikeIteratorElement = caerSpikeEventPacketGetEvent(SPIKE_PACKET, caerSpikeIteratorCounter); \
		if (!caerSpikeEventIsValid(caerSpikeIteratorElement)) { continue; } // Skip invalid spike events.

/**
 * Const-Reverse iterator over only the valid spike events in a packet.
 * Returns the current index in the 'caerSpikeIteratorCounter' variable of type
 * 'int32_t' and the current read-only event in the 'caerSpikeIteratorElement' variable
 * of type caerSpikeEventConst.
 *
 * SPIKE_PACKET: a valid SpikeEventPacket pointer. Cannot be NULL.
 */
#define CAER_SPIKE_CONST_REVERSE_ITERATOR_VALID_START(SPIKE_PACKET) \
	for (int32_t caerSpikeIteratorCounter = caerEventPacketHeaderGetEventNumber(&(SPIKE_PACKET)->packetHeader) - 1; \
		caerSpikeIteratorCounter >= 0; \
		caerSpikeIteratorCounter--) { \
		caerSpikeEventConst caerSpikeIteratorElement = caerSpikeEventPacketGetEventConst(SPIKE_PACKET, caerSpikeIteratorCounter); \
		if (!caerSpikeEventIsValid(caerSpikeIteratorElement)) { continue; } // Skip invalid spike events.

/**
 * Reverse iterator close statement.
 */
#define CAER_SPIKE_REVERSE_ITERATOR_VALID_END }

#ifdef __cplusplus
}
#endif

#endif /* LIBCAER_EVENTS_SPIKE_H_ */

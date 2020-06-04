/*
 * BFD Data Plane protocol messages header.
 *
 * Copyright (C) 2020 Network Device Education Foundation, Inc. ("NetDEF")
 *                    Rafael F. Zalamena
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file bfddp_packet.h
 */
#ifndef BFD_DP_PACKET_H
#define BFD_DP_PACKET_H

#include <netinet/in.h>

#include <stdint.h>

/** BFD data plane protocol version. */
#define BFD_DP_VERSION 1

/** BFD data plane message types. */
enum bfddp_message_type {
	/** Ask for BFD daemon or data plane for echo packet. */
	ECHO_REQUEST = 0,
	/** Answer a ECHO_REQUEST packet. */
	ECHO_REPLY = 1,
	/** Add or update BFD peer session. */
	DP_ADD_SESSION = 2,
	/** Delete BFD peer session. */
	DP_DELETE_SESSION = 3,
	/** Tell BFD daemon state changed: timer expired or session down. */
	BFD_STATE_CHANGE = 4,

	/** Ask for BFD session counters. */
	DP_REQUEST_SESSION_COUNTERS = 5,
	/** Tell BFD daemon about counters values. */
	BFD_SESSION_COUNTERS = 6,
};

/**
 * `ECHO_REQUEST`/`ECHO_REPLY` data payload.
 *
 * Data plane might use whatever precision it wants for `dp_time`
 * field, however if you want to be able to tell the delay between
 * data plane packet send and BFD daemon packet processing you should
 * use `gettimeofday()` and have the data plane clock sincronized with
 * BFD daemon (not a problem if data plane runs in the same system).
 *
 * Normally data plane will only check the timestamp it sent to determine
 * the whole packet trip time.
 */
struct bfddp_echo {
	/** Filled by data plane. */
	uint64_t dp_time;
	/** Filled by BFD daemon. */
	uint64_t bfdd_time;
};


/** BFD session flags. */
enum bfddp_session_flag {
	/** Set when using multihop. */
	SESSION_MULTIHOP = (1 << 0),
	/** Set when using demand mode. */
	SESSION_DEMAND = (1 << 1),
	/** Set when using cbit (Control Plane Independent). */
	SESSION_CBIT = (1 << 2),
	/** Set when using echo mode. */
	SESSION_ECHO = (1 << 3),
	/** Set when using IPv6. */
	SESSION_IPV6 = (1 << 4),
	/** Set when using passive mode. */
	SESSION_PASSIVE = (1 << 5),
};

/**
 * `DP_ADD_SESSION`/`DP_DELETE_SESSION` data payload.
 *
 * `lid` is unique in BFD daemon so it might be used as key for data
 * structures lookup.
 */
struct bfddp_session {
	/** Important session flags. \see bfddp_session_flag. */
	uint32_t flags;
	/**
	 * Session source address.
	 *
	 * Check `flags` field for `SESSION_IPV6` before using as IPv6.
	 */
	struct in6_addr src;
	/**
	 * Session destination address.
	 *
	 * Check `flags` field for `SESSION_IPV6` before using as IPv6.
	 */
	struct in6_addr dst;

	/** Local discriminator. */
	uint32_t lid;
	/**
	 * Minimum desired transmission interval (in microseconds) without
	 * jitter.
	 */
	uint32_t min_tx;
	/**
	 * Required minimum receive interval rate (in microseconds) without
	 * jitter.
	 */
	uint32_t min_rx;
	/**
	 * Required minimum echo receive interval rate (in microseconds)
	 * without jitter.
	 */
	uint32_t min_echo_rx;
	/**
	 * Amount of milliseconds to wait before starting sending session
	 * packets (only works when `SESSION_PASSIVE` flag is not set).
	 */
	uint32_t hold_time;

	/** Minimum TTL. */
	uint8_t ttl;
	/** Detection multiplier. */
	uint8_t detect_mult;
	/** Reserved / zeroed. */
	uint16_t zero;

	/** Interface index (set to `-1` when unavailable). */
	int32_t ifindex;
	/** Interface name (empty when unavailable). */
	char ifname[64];

	/* TODO: missing authentication. */
};

/** BFD packet state values as defined in RFC 5880, Section 4.1. */
enum bfd_state_value {
	/** Session is administratively down. */
	STATE_ADMINDOWN = 0,
	/** Session is down or went down. */
	STATE_DOWN = 1,
	/** Session is initializing. */
	STATE_INIT = 2,
	/** Session is up. */
	STATE_UP = 3,
};

/** BFD diagnostic field values as defined in RFC 5880, Section 4.1. */
enum bfd_diagnostic_value {
	/** Nothing was diagnosed. */
	DIAG_NOTHING = 0,
	/** Control detection time expired. */
	DIAG_CONTROL_EXPIRED = 1,
	/** Echo function failed. */
	DIAG_ECHO_FAILED = 2,
	/** Neighbor signaled down. */
	DIAG_DOWN = 3,
	/** Forwarding plane reset. */
	DIAG_FP_RESET = 4,
	/** Path down. */
	DIAG_PATH_DOWN = 5,
	/** Concatenated path down. */
	DIAG_CONCAT_PATH_DOWN = 6,
	/** Administratively down. */
	DIAG_ADMIN_DOWN = 7,
	/** Reverse concatenated path down. */
	DIAG_REV_CONCAT_PATH_DOWN = 8,
};

/** BFD remote state flags. */
enum bfd_remote_flags {
	/** Control Plane Independent bit. */
	RBIT_CPI = (1 << 0),
	/** Demand mode bit. */
	RBIT_DEMAND = (1 << 1),
	/** Multipoint bit. */
	RBIT_MP = (1 << 2),
};

/**
 * `BFD_STATE_CHANGE` data payload.
 */
struct bfddp_state_change {
	/** Local discriminator. */
	uint32_t lid;
	/** Remote discriminator. */
	uint32_t rid;
	/** Remote configurations/bits set. \see bfd_remote_flags. */
	uint32_t remote_flags;
	/** Remote state. \see bfd_state_values.*/
	uint8_t state;
	/** Remote diagnostics (if any) */
	uint8_t diagnostics;
};

/**
 * BFD control packet.
 */
struct bfddp_control_packet {
	/** (3 bits version << 5) | (5 bits diag). */
	uint8_t version_diag;
	/**
	 * (2 bits state << 6) | (6 bits flags)
	 *
	 * Flag bits:
	 * - `(1 << 4)`: Poll bit.
	 * - `(1 << 3)`: Final bit.
	 * - `(1 << 2)`: Control Plane Independent bit.
	 * - `(1 << 1)`: Authentication present bit.
	 * - `(1 << 0)`: Demand mode bit.
	 */
	uint8_t state_bits;
	/** Detection multiplier. */
	uint8_t detection_multiplier;
	/** Packet length in bytes. */
	uint8_t length;
	/** Our discriminator. */
	uint32_t local_id;
	/** Remote system discriminator. */
	uint32_t remote_id;
	/** Desired minimun send interval in microseconds. */
	uint32_t desired_tx;
	/** Desired minimun receive interval in microseconds. */
	uint32_t required_rx;
	/** Desired minimun echo receive interval in microseconds. */
	uint32_t required_echo_rx;
};

/**
 * The protocol wire message header structure.
 */
struct bfddp_message_header {
	/** Protocol version format. \see BFD_DP_VERSION. */
	uint8_t version;
	/** Reserved / zero field. */
	uint8_t zero;
	/** Message contents type. \see bfddp_message_type. */
	uint16_t type;
	/* Reserved / zero field. */
	uint16_t zero2;
	/** Message length. */
	uint16_t length;
};

/**
 * Data plane session counters request.
 *
 * Message type: `DP_REQUEST_SESSION_COUNTERS`.
 */
struct bfddp_request_counters {
	/** Session local discriminator. */
	uint32_t lid;
};

/**
 * BFD session counters reply.
 *
 * Message type: `BFD_SESSION_COUNTERS`.
 */
struct bfddp_session_counters {
	/** Session local discriminator. */
	uint32_t lid;

	/** Control packet bytes input. */
	uint64_t control_input_bytes;
	/** Control packets input. */
	uint64_t control_input_packets;
	/** Control packet bytes output. */
	uint64_t control_output_bytes;
	/** Control packets output. */
	uint64_t control_output_packets;

	/** Echo packet bytes input. */
	uint64_t echo_input_bytes;
	/** Echo packets input. */
	uint64_t echo_input_packets;
	/** Echo packet bytes output. */
	uint64_t echo_output_bytes;
	/** Echo packets output. */
	uint64_t echo_output_packets;
};

/**
 * The protocol wire messages structure.
 */
struct bfddp_message {
	/** Message header. \see bfddp_message_header. */
	struct bfddp_message_header header;

	/** Message payload. \see bfddp_message_type. */
	union {
		struct bfddp_echo echo;
		struct bfddp_session session;
		struct bfddp_state_change state;
		struct bfddp_control_packet control;
		struct bfddp_request_counters counters_req;
		struct bfddp_session_counters session_counters;
	} data;
};

#endif /* BFD_DP_PACKET_H */

/* Main API
 *
 * Boss ME-5 Editor
 *
 * Copyright (C) 2012 Kim Taylor <kmtaylor@gmx.com>
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 * 
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 * 
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: me_editor.h,v 1.18 2012/07/18 05:42:11 kmtaylor Exp $
 */

#define me_editor_top_midi_class	    me_editor_midi_class_0
#define DEFAULT_DEVICE_ID 0x10
#define DEFAULT_MODEL_ID 0x4C

#ifndef ME_EDITOR_DEBUG
#define ME_EDITOR_DEBUG 1
#endif

#define TIMEOUT_TIME 3000
#define MAX_SET_NAME_SIZE 16
#define NUM_USER_PATCHES 256

extern void *__common_allocate(size_t size, char *func_name);

typedef const struct s_midi_class MidiClass;
typedef struct s_midi_class_member MidiClassMember;

struct s_midi_class_member {
	const char	    *name;
	const uint32_t	    sysex_addr_base;
	const MidiClass	    *class;
	int		    blacklisted;
};

struct s_midi_class {
	const char		*name;
	MidiClassMember		*members;
	const int		size;
	const MidiClass		**parents;
};

enum midi_address_flags {
	M_ADDRESS_FETCHED		= 0x01,
	M_ADDRESS_BLACKLISTED		= 0x10,
};

typedef struct s_midi_address {
	uint32_t		sysex_addr;
	uint32_t		sysex_size;
	const MidiClass		*class;
	enum midi_address_flags	flags;
	uint32_t		value;
} midi_address;

enum init_flags {
	ME_EDITOR_READ		= 0x01,
	ME_EDITOR_WRITE		= 0x02,
	ME_EDITOR_ACK			= 0x04,
};

extern int me_editor_init(const char *client_name, enum init_flags flags);
extern int me_editor_close(void);

extern void me_editor_set_timeout(int timeout_time);

extern midi_address *me_editor_match_midi_address(uint32_t sysex_addr);
extern MidiClass *me_editor_match_class_name(char *class_name);

extern const char *me_editor_get_desc(uint32_t sysex_addr);

extern uint32_t me_editor_add_addresses(uint32_t address1,
				uint32_t address2);

extern char **me_editor_get_parents(uint32_t sysex_addr, int *num);

extern uint32_t me_editor_get_sysex_size(uint32_t sysex_addr);

extern int me_editor_class_num_parents(MidiClass *class);
extern uint32_t me_editor_get_sysex_value(uint8_t *data, uint32_t size);

/* Block, waiting for a new incoming sysex event
 * If successful, DATA contains a newly allocated buffer of the received
 * sysex data, and the return value is the number of bytes received.
 * If unsuccessful, DATA is set to NULL
 * On timeout, me_editor_listen_sysex_event returns -1 */
extern int me_editor_listen_sysex_event(uint8_t *command_id, 
		                uint32_t *address, uint8_t **data);

extern void me_editor_send_bulk_sysex(midi_address m_addresses[],
		const int num);

extern void me_editor_send_sysex(uint32_t sysex_addr,
		                uint32_t sysex_size, uint8_t *data);

extern void me_editor_send_sysex_value(uint32_t sysex_addr,
				uint32_t sysex_size, uint32_t sysex_value);

extern int me_editor_get_bulk_sysex(midi_address m_addresses[],
		const int num);

/* Requests sysex data and blocks waiting for a response.
 * If a response is received, DATA points to a newly allocated buffer
 * containing the raw sysex data.
 * If a timeout occurs, an unrecognised response arrives or a checksum
 * error occurs, me_editor_get_sysex returns -1 with DATA set to NULL.
 * If BLACKLISTING is enabled, an attempt to read a blacklisted SYSEX_ADDR
 * will result in a return value of -2 with DATA set to NULL */
extern int me_editor_get_sysex(uint32_t sysex_addr,
		                uint32_t sysex_size, uint8_t **data);

extern char *me_editor_get_patch_name(uint32_t sysex_addr);
extern char *me_editor_get_copy_patch_name(void);
extern int me_editor_refresh_patch_names(void);

/* Class refers to the parent class */
extern int me_editor_copy_class(MidiClass *class, uint32_t sysex_addr,
				int *depth);
extern int me_editor_paste_class(MidiClass *class, uint32_t sysex_addr,
		                int *depth);
extern int me_editor_paste_layer_to_part(MidiClass *class,
		uint32_t sysex_addr, int *depth, int layer, int part);
extern void me_editor_flush_copy_data(int *depth);
extern int me_editor_write_copy_data_to_file(char *filename, int *depth);
extern int me_editor_read_copy_data_from_file(char *filename, int *depth);

#ifdef ME_EDITOR_PRIVATE

#define MAX_SYSEX_PACKET_SIZE 120

#define ADDRESS_GROUP	    "Addresses"
#define GENERAL_GROUP	    "General"
#define SIZE_KEY	    "Size"
#define CLASS_KEY	    "Class"
#define ADDRESS_BASE_KEY    "BaseAddress"

typedef struct s_class_data Class_data;
struct s_class_data {
	midi_address	    *m_addresses;
	uint32_t	    sysex_addr_base;
	MidiClass	    *class;
	Class_data	    *next;
	unsigned int	    size;
};

#endif

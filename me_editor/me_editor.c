/* me_editor
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
 * $Id: me_editor.c,v 1.19 2012/07/18 05:42:11 kmtaylor Exp $
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <libgen.h>

#define ME_EDITOR_PRIVATE
#include <me_editor.h>
#include "midi_addresses.h"
#include "sysex.h"

#if ME_EDITOR_DEBUG
#include "log.h"
#include <malloc.h>
#endif

#define allocate(t, num) __common_allocate(sizeof(t) * num, "me_editor")
#define NUM_ADDRESSES me_editor_num_addresses
#define NUM_CLASSES me_editor_num_classes

static MePatch me_editor_patches[NUM_USER_PATCHES];

static uint8_t device_id = DEFAULT_DEVICE_ID;
static uint32_t model_id = DEFAULT_MODEL_ID;

static Class_data *copy_paste_data;

#ifdef BLACKLISTING
static int match_member_entry(MidiClass *class, uint32_t address) {
	int i;
	for (i = 0; i < class->size; i++) {
            if (class->members[i].sysex_addr_base > address)
                return i - 1;
        }
	return (i - 1);
}

#define BLACKLIST_CLASS_MEMBER(class, address) { \
	i = match_member_entry(&class, address);    \
	class.members[i].blacklisted = 1;	    \
}

static void blacklist_class_members(void) {
	int i;
	BLACKLIST_CLASS_MEMBER(me_editor_midi_class_0,  0x00);
}

#define BLACKLIST_ADDRESS(address) { \
	m_address = me_editor_match_midi_address(address);	\
	m_address->flags |= M_ADDRESS_BLACKLISTED;		\
}

static void blacklist_addresses(void) {
	midi_address *m_address;
	BLACKLIST_ADDRESS(0x10000000);
}
#endif

int me_editor_init(const char *client_name, enum init_flags flags) {
	int retval;

	retval = sysex_init(client_name, TIMEOUT_TIME, flags);

#ifdef BLACKLISTING
	blacklist_class_members();
	blacklist_addresses();
#endif

	return retval;
}

int me_editor_close(void) {
	int retval;

	retval = sysex_close();

	return retval;
}

uint32_t me_editor_add_addresses(uint32_t address1, uint32_t address2) {
	uint8_t a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4;
	a1 = (address1 & 0xff000000) >> 24; b1 = (address2 & 0xff000000) >> 24;
	a2 = (address1 & 0x00ff0000) >> 16; b2 = (address2 & 0x00ff0000) >> 16;
	a3 = (address1 & 0x0000ff00) >> 8;  b3 = (address2 & 0x0000ff00) >> 8;
	a4 = (address1 & 0x000000ff);	    b4 = (address2 & 0x000000ff);

	c4 = a4 + b4;
	c3 = a3 + b3 + ((c4 & 0x80)>>7);
	c2 = a2 + b2 + ((c3 & 0x80)>>7);
	c1 = a1 + b1 + ((c2 & 0x80)>>7);

	c4 &= 0x7f;
	c3 &= 0x7f;
	c2 &= 0x7f;
	c1 &= 0x7f;

	return ( c1<<24 | c2<<16 | c3<<8 | c4 );
}

midi_address *me_editor_match_midi_address(uint32_t sysex_addr) {
	int i;
        for (i = 0; i < NUM_ADDRESSES; i++) {
                if (me_editor_midi_addresses[i].sysex_addr == sysex_addr) {
                        return &me_editor_midi_addresses[i];
                }
        }
        return NULL;
}

MidiClass *me_editor_match_class_name(char *class_name) {
	int i;
        for (i = 0; i < NUM_CLASSES; i++) {
                if (!strcmp(me_editor_midi_classes[i]->name, class_name)) {
                        return me_editor_midi_classes[i];
                }
        }
        return NULL;
}

static uint32_t class_member_base(MidiClass *class, uint32_t sysex_addr) {
	int i;
	for (i = 0; i < class->size; i++) {
            if (class->members[i].sysex_addr_base > sysex_addr)
                return class->members[i - 1].sysex_addr_base;
        }
	return class->members[i - 1].sysex_addr_base;
}

int me_editor_class_num_parents(MidiClass *class) {
	int num_parents = 0;
	MidiClass *cur_parent;
	
	if (!class->parents) return 0;

	cur_parent = class->parents[0];
	while(cur_parent) {
	    cur_parent = class->parents[++num_parents];
	}

	return num_parents;
}

/* Once each match is made, we can subtract its base address */
static uint32_t match_class_member(uint32_t sysex_addr, MidiClass *class,
		int depth) {
	int i;
	uint32_t sub_addr = 0;
	int num_parents;
	MidiClass *cur_parent = NULL;
	
	if (!class)
	    class = me_editor_match_midi_address(sysex_addr)->class;

	if (depth)
	    class = class->parents[depth - 1];

	num_parents = me_editor_class_num_parents(class);

	for (i = 0; i < num_parents; i++) {
	    cur_parent = class->parents[num_parents - 1 - i];
	    sub_addr += class_member_base(cur_parent, sysex_addr - sub_addr);
	}

	sysex_addr -= sub_addr;

	for (i = 0; i < class->size; i++) {
            if (class->members[i].sysex_addr_base > sysex_addr)
                return i - 1;
        }
	return (i - 1);
}

const char *me_editor_get_desc(uint32_t sysex_addr) {
	midi_address *m_address = me_editor_match_midi_address(sysex_addr);
	if (!m_address) return NULL;
	int i = match_class_member(sysex_addr, m_address->class, 0);
	if (i < 0) return NULL;
	return (m_address->class->members[i].name);
}

char **me_editor_get_parents(uint32_t sysex_addr, int *num_parents) {
	midi_address *m_address = me_editor_match_midi_address(sysex_addr);
	int depth, i, j;
	char **parents_array;
	
	if (!m_address) return NULL;

	*num_parents = me_editor_class_num_parents(m_address->class);

	if (*num_parents == 0) return NULL;

	parents_array = allocate(char *, *num_parents);

	for (depth = *num_parents, j = 0; depth > 0; depth--, j++) {
	    i = match_class_member(sysex_addr, m_address->class, depth);
	    asprintf(&parents_array[j], "%s",
		    m_address->class->parents[depth - 1]->members[i].name);
	}

	return parents_array;
}

uint32_t me_editor_get_sysex_size(uint32_t sysex_addr) {
	midi_address *m_address = me_editor_match_midi_address(sysex_addr);
	if (!m_address) return -1;
	return m_address->sysex_size;
}

uint32_t me_editor_get_sysex_value(uint8_t *data, uint32_t size) {
	int i, j;
	uint32_t retval = 0;
	j = size - 1;
	for (i = 0; i < size; i++, j--) {
		retval |= data[j] << (i*4);
	}
	return retval;
}

static void me_editor_write_sysex_value(uint32_t sysex_value,
			    uint32_t sysex_size, uint8_t *data) {
	int i, j;
	if (sysex_size == 1) {
	    data[0] = sysex_value & 0x7f;
	} else {
	    for (i = 0, j = sysex_size - 1; i < sysex_size; i++, j--) {
		data[j] = (sysex_value >> (i*4)) & 0xf;
	    }
	}
}

void me_editor_set_device_id(uint8_t id) {
	device_id = id;
}

void me_editor_set_model_id(uint32_t id) {
	model_id = id;
}

static int address_sort(const void *va, const void *vb) {
	const midi_address **a = (const midi_address **) va;
	const midi_address **b = (const midi_address **) vb;
	if ((*a)->sysex_addr == (*b)->sysex_addr) return 0;
	if ((*a)->sysex_addr > (*b)->sysex_addr) return 1;
	else return -1;
}

static int build_blocks(uint32_t block_addresses[], uint32_t block_sizes[],
		int block_offsets[], int *total_size, const int num,
		midi_address **s_addresses) {
	int i, blocks;
	block_addresses[0] = s_addresses[0]->sysex_addr;
	block_offsets[0] = 0;
	for (i = 0; i < num; i++) block_sizes[i] = 0;
	*total_size = 0;
	for (i = 1, blocks = 1; i < num; i++) {
	    if (s_addresses[i - 1]->sysex_addr + 
		    s_addresses[i - 1]->sysex_size ==
			s_addresses[i]->sysex_addr) {
		block_sizes[blocks - 1] += s_addresses[i - 1]->sysex_size;
		*total_size += s_addresses[i - 1]->sysex_size;
		if (block_sizes[blocks - 1] < MAX_SYSEX_PACKET_SIZE) {
		    continue;
		} else {
		    block_sizes[blocks - 1] -= s_addresses[i - 1]->sysex_size;
		    *total_size -= s_addresses[i - 1]->sysex_size;
		}
	    }
	    block_sizes[blocks - 1] += s_addresses[i - 1]->sysex_size;
	    *total_size += s_addresses[i - 1]->sysex_size;
	    if (block_sizes[blocks - 1] > MAX_SYSEX_PACKET_SIZE) {
		block_sizes[blocks - 1] -= s_addresses[i - 1]->sysex_size;
		*total_size -= s_addresses[i - 1]->sysex_size;
		i--;
	    }
	    block_offsets[blocks] = i;
	    block_addresses[blocks++] = s_addresses[i]->sysex_addr;
	}
	block_sizes[blocks - 1] += s_addresses[i - 1]->sysex_size;
	*total_size += s_addresses[i - 1]->sysex_size;
	if (block_sizes[blocks - 1] > MAX_SYSEX_PACKET_SIZE) {
	    block_sizes[blocks - 1] -= s_addresses[i - 1]->sysex_size;
	    block_sizes[blocks] += s_addresses[i - 1]->sysex_size;
	    block_offsets[blocks] = i - 1;
	    block_addresses[blocks++] = s_addresses[i - 1]->sysex_addr;
	}
	return blocks;
}

int me_editor_get_bulk_sysex(midi_address m_addresses[], const int num) {
	int i, j, blocks;
	int retval = 0;
	int total_size;
	uint32_t block_addresses[num];
	uint32_t block_sizes[num];
	int block_offsets[num];
	uint8_t **data;
	int data_offset;
	midi_address *s_address;

	midi_address **s_addresses = allocate(midi_address *, num);
	for (i = 0; i < num; i++) {
		s_addresses[i] = &m_addresses[i];
	}
	qsort(s_addresses, num, sizeof(midi_address *), address_sort);

	blocks = build_blocks(block_addresses, block_sizes, block_offsets, 
			&total_size, num, s_addresses);

	data = allocate(uint8_t *, blocks);
	for (i = 0; i < blocks; i++) data[i] = NULL;
	for (i = 0; i < blocks; i++) {
	    retval = me_editor_get_sysex(
			    block_addresses[i], block_sizes[i], &data[i]);
	    if (retval < 0) goto cleanup;
	}
	
	for (i = 0; i < blocks; i++) {
	    data_offset = 0;
	    for (j = 0; j < block_sizes[i]; j++) {
		s_address = s_addresses[block_offsets[i] + j];
		s_address->value = me_editor_get_sysex_value(
			    &data[i][data_offset],
			    s_address->sysex_size);
		s_address->flags |= M_ADDRESS_FETCHED;
		data_offset += s_address->sysex_size;
		if (data_offset >= block_sizes[i]) break;
	    }
	}

cleanup:
	for (i = 0; i < blocks; i++) {
	    if (data[i]) free(data[i]);
	}
	free(data);
	free(s_addresses);
	return retval;
}

void me_editor_send_sysex(uint32_t sysex_addr,
			    uint32_t sysex_size, uint8_t *data) {
	sysex_send(device_id, model_id, sysex_addr, sysex_size, data);
}

void me_editor_send_bulk_sysex(midi_address m_addresses[], const int num) {
	int i, blocks;
	int total_size;
	uint32_t block_addresses[num];
	uint32_t block_sizes[num];
	int block_offsets[num];
	uint8_t *data;
	int data_offset = 0;

	midi_address **s_addresses = allocate(midi_address *, num);
	for (i = 0; i < num; i++) {
		s_addresses[i] = &m_addresses[i];
	}
	qsort(s_addresses, num, sizeof(midi_address *), address_sort);

	blocks = build_blocks(block_addresses, block_sizes, block_offsets, 
			&total_size, num, s_addresses);

	data = allocate(uint8_t *, total_size);
	for (i = 0; i < num; i++) {
	    me_editor_write_sysex_value(s_addresses[i]->value,
			s_addresses[i]->sysex_size, &data[data_offset]);
	    data_offset += s_addresses[i]->sysex_size;
	}

	data_offset = 0;
	for (i = 0; i < blocks; i++) {
	    me_editor_send_sysex(block_addresses[i], block_sizes[i],
			    data + data_offset);
	    data_offset += block_sizes[i];
	}
	free(data);
	free(s_addresses);
}

void me_editor_send_sysex_value(uint32_t sysex_addr,
			    uint32_t sysex_size, uint32_t sysex_value) {
	uint8_t data[4];
	me_editor_write_sysex_value(sysex_value, sysex_size, data);
	me_editor_send_sysex(sysex_addr, sysex_size, data);
}

int me_editor_get_sysex(uint32_t sysex_addr,
		                uint32_t sysex_size, uint8_t **data) {
	int retval;
	
#ifdef BLACKLISTING
	*data = NULL;
	midi_address *m_address = me_editor_match_midi_address(sysex_addr);
	if (!m_address) return -2;
	int i = match_class_member(sysex_addr, m_address->class, 0);
	if (m_address->flags & M_ADDRESS_BLACKLISTED) return -2;
	if (m_address->class->members[i].blacklisted) return -2;
#endif

	retval = sysex_recv(device_id, model_id, sysex_addr, sysex_size, data);

	if (retval < 0) {
#ifdef BLACKLISTING
	    m_address->flags |= M_ADDRESS_BLACKLISTED;
#endif
#if ME_EDITOR_DEBUG
	    char *msg;
	    asprintf(&msg,
		    "Timeout while attempting to read from address 0x%08X",
		    sysex_addr);
	    common_log(1, msg);
	    free(msg);
#endif
	}

	return retval;
}

void me_editor_set_timeout(int timeout_time) {
	sysex_set_timeout(timeout_time);
}

/* This function will block, returns the number of data bytes collected */
int me_editor_listen_sysex_event(uint8_t *command_id, 
		uint32_t *address, uint8_t **data) {
	int sum;
	return sysex_listen_event(command_id, address, data, &sum);
}

static int me_editor_get_patch_index(uint32_t sysex_addr) {
        int i;
        for (i = 0; i < NUM_USER_PATCHES; i++) {
                if (me_editor_patches[i].sysex_base_addr == sysex_addr) {
                        return i;
                }
        }
        return -1;
}

char *me_editor_get_patch_name(uint32_t sysex_addr) {
	int i = me_editor_get_patch_index(sysex_addr);

	if (i < 0) return NULL;
        if (me_editor_patches[i].flags & ME_PATCH_NAME_KNOWN)
            return me_editor_patches[i].name;
        return NULL;
}

#define UNKNOWN_PATCH_NAME "Unknown"
int me_editor_read_patch_names(char *filename) {
	int retval, p_error, i;
	GKeyFile *key_file;
	GError *error = NULL;
	gsize length;
	gchar **name_keys;
	gchar *cur_key;
	uint32_t sysex_base_addr;
	uint32_t cur_sysex_base = FIRST_USER_PATCH_ADDR;
	char *patch_name;
	int patch_name_len;

	/* Open file */
	key_file = g_key_file_new();
	if (g_key_file_load_from_file(key_file, filename,
				G_KEY_FILE_NONE, NULL) == FALSE) {
	    /* Fill up local cache with "Unknown" */
	    for (i = 0; i < NUM_USER_PATCHES; i++) {
		me_editor_patches[i].sysex_base_addr = cur_sysex_base;
		memcpy(me_editor_patches[i].name, UNKNOWN_PATCH_NAME,
				strlen(UNKNOWN_PATCH_NAME) + 1);
		me_editor_patches[i].flags |= ME_PATCH_NAME_KNOWN;
		cur_sysex_base = me_editor_add_addresses(cur_sysex_base,
				USER_PATCH_DELTA);
	    }
	    return -1;
	}

	name_keys = g_key_file_get_keys(key_file, PATCH_GROUP,
			&length, &error);
        if (g_error_matches(error, G_KEY_FILE_ERROR,
			        G_KEY_FILE_ERROR_GROUP_NOT_FOUND)) {
	    g_clear_error(&error);
	    goto parse_error;
	}

	if (length != NUM_USER_PATCHES) {
	    g_strfreev(name_keys);
	    goto parse_error;
	}

	for (i = 0; i < NUM_USER_PATCHES; i++) {
	    cur_key = name_keys[i];
	    if (sscanf(cur_key, "0x%04X", &sysex_base_addr) != 1) continue;
	    patch_name = g_key_file_get_string(key_file,
			    PATCH_GROUP, cur_key, NULL);
	    patch_name_len = strlen(patch_name);
	    if (patch_name_len > MAX_SET_NAME_SIZE)
		patch_name_len = MAX_SET_NAME_SIZE;
	    memcpy(me_editor_patches[i].name, patch_name,
			    patch_name_len + 1);
	    me_editor_patches[i].name[MAX_SET_NAME_SIZE] = '\0';
	    me_editor_patches[i].flags |= ME_PATCH_NAME_KNOWN;
	    me_editor_patches[i].sysex_base_addr = sysex_base_addr;
	}

	g_strfreev(name_keys);
	g_key_file_free(key_file);
	return 0;

parse_error:
	return -2;
}

int me_editor_write_patch_names(char *filename) {
	int i, retval = 0;
	FILE *fp;
	GKeyFile *key_file;
	gsize length;
	gchar key_name[11];
	gchar *data;

	fp = fopen(filename, "w");
	if (!fp) return -2;

	key_file = g_key_file_new();

	for (i = 0; i < NUM_USER_PATCHES; i++) {
	    if (!(me_editor_patches[i].flags & ME_PATCH_NAME_KNOWN)) {
		retval = -1;
		goto failed;
	    }
	    sprintf(key_name, "0x%04X", me_editor_patches[i].sysex_base_addr);
	    g_key_file_set_string(key_file, PATCH_GROUP,
					key_name, me_editor_patches[i].name);
	}

	data = g_key_file_to_data(key_file, &length, NULL);
	size_t s = fwrite(data, sizeof(gchar), length, fp);

	free(data);
failed:
	fclose(fp);
	g_key_file_free(key_file);
	return retval;
}

static int transfer_addresses_under_member(MidiClassMember *class_member,
		uint32_t sysex_addr, int pasting) {
	int i, retval;
	MidiClass *class = class_member->class;

	if (!class) return 0;

	if (!class->members[0].class && pasting) {
	    me_editor_send_bulk_sysex(
		    me_editor_match_midi_address(sysex_addr),
		    class->size);
	}

	if (!class->members[0].class && !pasting) {
	    retval = me_editor_get_bulk_sysex(
		    me_editor_match_midi_address(sysex_addr),
		    class->size);
	    if (retval) return retval;
	}

	for (i = 0; i < class->size; i++) {
	    retval = transfer_addresses_under_member(&class->members[i],
			    sysex_addr + class->members[i].sysex_addr_base,
			    pasting);
	    if (retval) return retval;
	}
	return retval;
}

static int count_addresses_under_member(MidiClassMember *class_member,
		uint32_t sysex_addr) {
	int i, total = 0;
	MidiClass *class = class_member->class;

	if (!class) return 1;

	for (i = 0; i < class->size; i++) {
	    total += count_addresses_under_member(&class->members[i],
			    sysex_addr + class->members[i].sysex_addr_base);
	}
	return total;
}

static void cp_addresses_under_member(MidiClassMember *class_member,
		Class_data *cur_class_data,
		int *index, uint32_t sysex_addr, int pasting) {
	int i;
	MidiClass *class = class_member->class;
	midi_address *to, *from;

	/* Copy a single address */
	if (!class) {
	    if (pasting) {
		from = &cur_class_data->m_addresses[*index];
		to = me_editor_match_midi_address(sysex_addr);
	    } else {
		to = &cur_class_data->m_addresses[*index];
		from = me_editor_match_midi_address(sysex_addr);
	    }

	    memcpy(to, from, sizeof(midi_address));

	    if (pasting) {
		to->sysex_addr += cur_class_data->sysex_addr_base;
	    } else {
		to->sysex_addr -= cur_class_data->sysex_addr_base;
	    }

	    (*index)++;
	    return;
	}

	for (i = 0; i < class->size; i++) {
	    cp_addresses_under_member(&class->members[i],
			    cur_class_data, index,
			    sysex_addr + class->members[i].sysex_addr_base,
			    pasting);
	}
	return;
}

int me_editor_copy_class(MidiClass *class, uint32_t sysex_addr, int *depth) {
	int num_addresses, retval;
	int index = 0;
	MidiClassMember *class_member;
        Class_data *cur_class_data, *last_class_data = NULL;

        if (!copy_paste_data) {
            copy_paste_data = allocate(struct s_class_data, 1);
            cur_class_data = copy_paste_data;
	    *depth = 1;
        } else {
            cur_class_data = copy_paste_data;
            while (cur_class_data->next) cur_class_data = cur_class_data->next;
            cur_class_data->next = allocate(struct s_class_data, 1);
	    last_class_data = cur_class_data;
            cur_class_data = cur_class_data->next;
	    (*depth)++;
        }
        cur_class_data->next = NULL;
	cur_class_data->m_addresses = NULL;

	if (!me_editor_match_midi_address(sysex_addr)) {
	    retval = -1;
	    goto failed;
	}

	class_member = &class->members[match_class_member(sysex_addr,
								class, 0)];
	num_addresses = count_addresses_under_member(class_member, sysex_addr);
	retval = transfer_addresses_under_member(class_member, sysex_addr, 0);
	if (retval) goto failed;

	cur_class_data->size = num_addresses;
	cur_class_data->m_addresses = allocate(midi_address, num_addresses);
	cur_class_data->class = class_member->class;
	cur_class_data->sysex_addr_base = sysex_addr;
	cur_class_data->name = me_editor_get_patch_name(sysex_addr);

	cp_addresses_under_member(class_member,
					cur_class_data, &index, sysex_addr, 0);
	return 0;

failed:
	if (copy_paste_data == cur_class_data)
	    copy_paste_data = NULL;
	else
	    last_class_data->next = NULL;
	if (cur_class_data->m_addresses)
	    free(cur_class_data->m_addresses);
	free(cur_class_data);
	(*depth)--;
	return retval;
}

int me_editor_paste_class(MidiClass *class, uint32_t sysex_addr,
		int *depth) {
	int dummy, i, retval = 0, patch_num;
	Class_data *cur_class_data;
	Class_data *verify_class_data;
	Class_data *last_class_data;
	MidiClassMember *class_member;
	int index = 0;
	int *broken_address_data;
	int broken_addresses = 0, loops;
	midi_address *cur_broken_addr;
	uint8_t *data;

	cur_class_data = copy_paste_data;
	if (!cur_class_data) return -1;

	if (!me_editor_match_midi_address(sysex_addr)) return -1;

	class_member = &class->members[match_class_member(sysex_addr,
								class, 0)];
	/* Sanity check */
	if (class_member->class != cur_class_data->class)
	    return -2;

	cur_class_data->sysex_addr_base = sysex_addr;
	cp_addresses_under_member(class_member,
			                cur_class_data, &index, sysex_addr, 1);
	transfer_addresses_under_member(class_member, sysex_addr, 1);

	/* Verify that copy was perfect */
	sysex_wait_write();
	retval = me_editor_copy_class(class,
			cur_class_data->sysex_addr_base, &dummy);

	if (retval < 0) return -4;

	broken_address_data = allocate(int, cur_class_data->size);

	verify_class_data = copy_paste_data;
	last_class_data = copy_paste_data;
	while (last_class_data->next->next)
		last_class_data = last_class_data->next;
	while (verify_class_data->next)
		verify_class_data = verify_class_data->next;
	last_class_data->next = NULL;

	for (i = 0; i < cur_class_data->size; i++) {
	    if (verify_class_data->m_addresses[i].value !=
			cur_class_data->m_addresses[i].value) {
		broken_address_data[broken_addresses] = i; 
		broken_addresses ++;
	    }
	}

	loops = broken_addresses;
	/* Attempt one by one copy of any addresses that failed */
	for (i = 0; i < loops; i++) {
	    cur_broken_addr =
		    &cur_class_data->m_addresses[broken_address_data[i]];
	    me_editor_send_sysex_value(
				    cur_broken_addr->sysex_addr + 
				    cur_class_data->sysex_addr_base,
				    cur_broken_addr->sysex_size,
				    cur_broken_addr->value);
	    if (me_editor_get_sysex(
				    cur_broken_addr->sysex_addr +
				    cur_class_data->sysex_addr_base,
				    cur_broken_addr->sysex_size, &data) < 0) {
		continue;
	    }
	    if (cur_broken_addr->value == me_editor_get_sysex_value(data,
						cur_broken_addr->sysex_size)) {
		broken_addresses--;
	    }
	    free(data);
	}
	
	if (broken_addresses) retval = -3;

	patch_num = me_editor_get_patch_index(sysex_addr);
	if (patch_num >= 0) {
	    memcpy(me_editor_patches[patch_num].name, cur_class_data->name,
			    strlen(cur_class_data->name) + 1);
	}

	copy_paste_data = copy_paste_data->next;
	if (cur_class_data->m_addresses)
		free(cur_class_data->m_addresses);
	if (verify_class_data->m_addresses)
		free(verify_class_data->m_addresses);
	free(verify_class_data);
	free(cur_class_data);
	free(broken_address_data);
	(*depth) -= 1;
	return retval;
}

void me_editor_flush_copy_data(int *depth) {
        Class_data *cur_class_data;
        while (copy_paste_data) {
            cur_class_data = copy_paste_data;
            copy_paste_data = copy_paste_data->next;
	    if (cur_class_data->m_addresses) free(cur_class_data->m_addresses);
            free(cur_class_data);
        }
	*depth = 0;
}

#define CHECK_ERROR(val) \
	if (!val) goto parse_error;					    \
	p_error = 0;							    \
	retval = g_error_matches(error, G_KEY_FILE_ERROR,		    \
				G_KEY_FILE_ERROR_KEY_NOT_FOUND);	    \
	if (retval) p_error = 1;					    \
	retval = g_error_matches(error, G_KEY_FILE_ERROR,		    \
				G_KEY_FILE_ERROR_INVALID_VALUE);	    \
	if (retval) p_error = 1;					    \
	g_clear_error(&error);						    \
	if (p_error) goto parse_error

char *me_editor_get_copy_data_name(void) {
	if (!copy_paste_data) return NULL;
	return copy_paste_data->name;
}

int me_editor_read_copy_data_from_file(char *filename, int *depth) {
	int retval, p_error, i, patch_name_len, patch_num;
	GKeyFile *key_file;
	GError *error;
	gsize length;
	gchar *class_name;
	gchar **address_keys;
	gchar *cur_key;
        Class_data *cur_class_data;
	int num_addresses;
	uint32_t addr_base, sysex_addr;
	MidiClass *class;
	midi_address *cur_address;
	midi_address *lib_address;

	key_file = g_key_file_new();
	if (g_key_file_load_from_file(key_file, filename, 
				G_KEY_FILE_NONE, NULL) == FALSE) return -1;
	error = NULL;

	num_addresses = g_key_file_get_uint64(key_file, GENERAL_GROUP,
			SIZE_KEY, &error);
	CHECK_ERROR(num_addresses);

	addr_base = g_key_file_get_uint64(key_file, GENERAL_GROUP,
			ADDRESS_BASE_KEY, &error);
	CHECK_ERROR(addr_base);

	class_name = g_key_file_get_string(key_file, GENERAL_GROUP,
			CLASS_KEY, &error);
	CHECK_ERROR(class_name);

	class = me_editor_match_class_name(class_name);
	free(class_name);
	if (!class) goto parse_error;

	address_keys = g_key_file_get_keys(key_file, ADDRESS_GROUP,
			&length, &error);
        if (g_error_matches(error, G_KEY_FILE_ERROR,
			        G_KEY_FILE_ERROR_GROUP_NOT_FOUND)) {
	    g_clear_error(&error);
	    goto parse_error;
	}

	if (length != num_addresses) {
	    g_strfreev(address_keys);
	    goto parse_error;
	}

        if (!copy_paste_data) {
            copy_paste_data = allocate(struct s_class_data, 1);
            cur_class_data = copy_paste_data;
	    *depth = 1;
        } else {
            cur_class_data = copy_paste_data;
            while (cur_class_data->next) cur_class_data = cur_class_data->next;
            cur_class_data->next = allocate(struct s_class_data, 1);
            cur_class_data = cur_class_data->next;
	    (*depth)++;
        }
        cur_class_data->next = NULL;
	cur_class_data->m_addresses = NULL;

	cur_class_data->size = num_addresses;
	cur_class_data->m_addresses = allocate(midi_address, num_addresses);
	cur_class_data->class = class;
	cur_class_data->sysex_addr_base = addr_base;
	
	patch_num = me_editor_get_patch_index(addr_base);
	if (patch_num >= 0) {
	    patch_name_len = strlen(basename(filename));
	    if (patch_name_len > MAX_SET_NAME_SIZE)
		patch_name_len = MAX_SET_NAME_SIZE;
	    memcpy(me_editor_patches[patch_num].name, basename(filename),
			    patch_name_len + 1);
	    me_editor_patches[i].name[MAX_SET_NAME_SIZE] = '\0';
	    cur_class_data->name = me_editor_patches[patch_num].name;
	} else {
	    cur_class_data->name = NULL;
	}

	for (i = 0; i < num_addresses; i++) {
	    cur_key = address_keys[i];
	    if (sscanf(cur_key, "0x%08X", &sysex_addr) != 1) continue;
	    lib_address = me_editor_match_midi_address(
					    sysex_addr + addr_base);
	    cur_address = &cur_class_data->m_addresses[i];

	    cur_address->sysex_addr = sysex_addr;
	    cur_address->value = g_key_file_get_uint64(key_file,
			    ADDRESS_GROUP, cur_key, NULL);
	    cur_address->class = lib_address->class;
	    cur_address->sysex_size = lib_address->sysex_size;
	    cur_address->flags = 0;
	}

	g_strfreev(address_keys);
	g_key_file_free(key_file);
	return 0;

parse_error:
	return -2;
}

int me_editor_write_copy_data_to_file(char *filename, int *depth) {
	int i;
	FILE *fp;
	GKeyFile *key_file;
	gsize length;
	gchar key_name[11];
	gchar *data;
        Class_data *cur_class_data;
	midi_address *m_address;

	cur_class_data = copy_paste_data;
	if (!cur_class_data) return -1;
	
	fp = fopen(filename, "w");
	if (!fp) return -2;

	key_file = g_key_file_new();
	
	g_key_file_set_string(key_file, GENERAL_GROUP,
			    CLASS_KEY, cur_class_data->class->name);
	g_key_file_set_uint64(key_file, GENERAL_GROUP,
			    ADDRESS_BASE_KEY, cur_class_data->sysex_addr_base);
	g_key_file_set_uint64(key_file, GENERAL_GROUP,
			    SIZE_KEY, cur_class_data->size);

	for (i = 0; i < cur_class_data->size; i++) {
	    m_address = &cur_class_data->m_addresses[i];
	    sprintf(key_name, "0x%08X", m_address->sysex_addr);
	    g_key_file_set_uint64(key_file, ADDRESS_GROUP,
					key_name, m_address->value);
	}

	data = g_key_file_to_data(key_file, &length, NULL);
	size_t s = fwrite(data, sizeof(gchar), length, fp);

	fclose(fp);
	free(data);
	g_key_file_free(key_file);
	copy_paste_data = copy_paste_data->next;
	if (cur_class_data->m_addresses)
		free(cur_class_data->m_addresses);
	free(cur_class_data);
	(*depth)--;
	return 0;
}

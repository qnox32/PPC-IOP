#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

/* Temporary file for common functions and structs in binary */

typedef struct event_t
{
	struct event_t *next_entry;
	struct event_t *prev_entry;
	int trigger_time;
	void (*callback)(uint32_t param);
	int param;
}event_t;

extern event_t* (*add_event)(int trigger_time, void *callback, int param);

extern int (*printf)(const char *fmt, ...);

int valid_addr(uint32_t addr);

#endif
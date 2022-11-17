//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016-2022 Julian Nechaevsky
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     OPL callback queue.
//


#pragma once

#include "opl.h"


typedef struct opl_callback_queue_s opl_callback_queue_t;

opl_callback_queue_t *OPL_Queue_Create(void);
int OPL_Queue_IsEmpty(opl_callback_queue_t *queue);
void OPL_Queue_Clear(opl_callback_queue_t *queue);
void OPL_Queue_Destroy(opl_callback_queue_t *queue);
void OPL_Queue_Push(opl_callback_queue_t *queue,
                    opl_callback_t callback, void *data,
                    uint64_t time);
int OPL_Queue_Pop(opl_callback_queue_t *queue,
                  opl_callback_t *callback, void **data);
uint64_t OPL_Queue_Peek(opl_callback_queue_t *queue);
void OPL_Queue_AdjustCallbacks(opl_callback_queue_t *queue,
                               uint64_t time, float factor);


/* $Id$ */
/*
** Copyright (C) 2009-2017 Quadrant Information Security <quadrantsec.com>
** Copyright (C) 2009-2017 Champ Clark III <cclark@quadrantsec.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* sagan-ipc.c
 *
 * This allows Sagan to share data with other Sagan processes. This is for
 * Inter-process communications (IPC).
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"             /* From autoconf */
#endif

void IPC_Init(void);
sbool Clean_IPC_Object( int );
void IPC_Check_Object(char *, sbool, char *);


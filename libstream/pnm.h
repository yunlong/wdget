/*
 * Copyright (C) 2002-2003 the xine project
 *
 * This file is part of xine, a free video player.
 *
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 * $Id: pnm.h,v 1.5 2003/12/09 00:02:30 f1rmb Exp $
 *
 * pnm util functions header by joschka
 */
 
#ifndef HAVE_PNM_H
#define HAVE_PNM_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct pnm_s pnm_t;

extern pnm_t*
pnm_connect (const char *url);

extern int
pnm_read (pnm_t *session, char *data, int len);

extern void
pnm_close (pnm_t *session);

extern int
pnm_peek_header(pnm_t *session, char *data, int maxsize);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef HAVE_PNM_H */



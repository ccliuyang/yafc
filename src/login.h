/* login.h -- connect and login
 *
 * This file is part of Yafc, an ftp client.
 * This program is Copyright (C) 1998, 1999, 2000 Martin Hedenfalk
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _login_h_included
#define _login_h_included

#define OP_NOKRB 1
#define OP_YESKRB 2
#define OP_ANON 4
#define OP_NOAUTO 8
#define OP_NOALIAS 16
#define OP_NOPROXY 32

void yafc_open(const char *host, unsigned int opt);

#endif

/*
 * Copyright (c) 2010, 2012, 2013, 2021 Ali Polatel <alip@exherbo.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <pinktrace/private.h>
#include <pinktrace/pink.h>

inline enum pink_event pink_event_decide(int status)
{
	return (unsigned)status >> 16;
}

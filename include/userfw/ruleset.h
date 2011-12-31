/*-
 * Copyright (C) 2011 by Maxim Ignatenko <gelraen.ua@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef USERFW_RULESET_H
#define USERFW_RULESET_H

#ifdef _KERNEL
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/rwlock.h>
#include <userfw/module.h>

typedef struct __userfw_rule
{
	struct __userfw_rule *next;

	uint16_t	number;
	userfw_action	action;
	userfw_match	match;
} userfw_rule;

typedef struct __userfw_ruleset
{
	userfw_rule	*rule;
	struct rwlock	mtx;
} userfw_ruleset;

extern userfw_ruleset global_rules;

#define USERFW_RLOCK(p)	rw_rlock(&((p)->mtx))
#define USERFW_WLOCK(p)	rw_wlock(&((p)->mtx))
#define USERFW_RUNLOCK(p)	rw_runlock(&((p)->mtx))
#define USERFW_WUNLOCK(p)	rw_wunlock(&((p)->mtx))
#define USERFW_INIT_LOCK(p, s)	rw_init(&((p)->mtx), (s))
#define USERFW_UNINIT_LOCK(p)	rw_destroy(&((p)->mtx))

void init_ruleset(userfw_ruleset *, const char *name);
void delete_ruleset(userfw_ruleset *);

int check_packet(struct mbuf **mb, userfw_chk_args *args, userfw_ruleset *ruleset);

#endif /* _KERNEL */
#endif /* USERFW_RULESET_H */

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


#include "userfw.h"
#include "userfw_dev.h"
#include "userfw_pfil.h"
#include "userfw_module.h"
#include "userfw_domain.h"
#include "userfw_util.h"
#include <userfw/ruleset.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/sctp.h>

int check_packet(struct mbuf **mb, int global, userfw_chk_args *args, userfw_ruleset *ruleset);

MALLOC_DEFINE(M_USERFW, "userfw", "Memory for userfw rules and cache");

int userfw_init()
{
	int err = 0;

	SLIST_INIT(&userfw_modules_list);
	rw_init(&userfw_modules_list_mtx, "userfw modules list lock");

#if 0
	err = userfw_dev_register();
#endif

	init_ruleset(&global_rules, "userfw global ruleset lock");

	if (!err)
		err = userfw_pfil_register();

	if (!err)
		err = userfw_domain_init();

	return err;
}

int userfw_uninit()
{
	int err = 0;

	err = userfw_domain_uninit();
	
	if (!err)
		err = userfw_pfil_unregister();

	if (!err)
	{
		rw_destroy(&userfw_modules_list_mtx);

		delete_ruleset(&global_rules);
	}

#if 0
	if (!err)
		err = userfw_dev_unregister();
#endif

	return err;
}

int
userfw_chk(struct mbuf **mb, userfw_chk_args *args)
{
	return check_packet(mb, 1, args, &global_rules);
}

int
check_packet(struct mbuf **mb, int global, userfw_chk_args *args, userfw_ruleset *ruleset)
{
	userfw_rule *rule = ruleset->rule;
	userfw_cache cache;
	int ret, matched = 0;

	userfw_cache_init(&cache);

	USERFW_RLOCK(ruleset);

	while(rule != NULL)
	{
		if (rule->match.do_match(mb, args, &(rule->match), &cache))
		{
			if ((*mb) == NULL)
				return EACCES;
			ret = rule->action.do_action(mb, args, &(rule->action), &cache);
			matched = 1;
			break;
		}
		if ((*mb) == NULL)
			return EACCES;
		rule = rule->next;
	}

	USERFW_RUNLOCK(ruleset);

	if (!matched)
#ifdef USERFW_DEFAULT_TO_DENY
		ret = EACCES;
#else
		ret = 0;
#endif

	userfw_cache_cleanup(&cache);

	return ret;
}
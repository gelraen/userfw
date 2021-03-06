#include <sys/param.h>
#include <sys/systm.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <userfw/module.h>
#include <userfw/io.h>
#include "dummy.h"

enum dummy_cache_entries
{
	TAG
};

static int
action_tag(struct mbuf **mb, userfw_chk_args *args, userfw_action *action, userfw_cache *cache, int *continue_, uint32_t flags)
{
	uint32_t val;

	val = action->args[0].uint32.value;
	if (cache != NULL && (flags & USERFW_ACTION_FLAG_SECOND_PASS) == 0)
	{
		userfw_cache_write(cache, USERFW_DUMMY_MOD, TAG, (void*)(int)val, NULL);
	}
	*continue_ = 1;
	return 0;
}

static userfw_action_descr dummy_actions[] =
{
	{A_TAG,	1,	{T_UINT32},	"tag",	action_tag}
};

static int
match_tag(struct mbuf **mb, userfw_chk_args *args, userfw_match *match, userfw_cache *cache, userfw_arg *marg)
{
	uint32_t val = match->args[0].uint32.value;
	if (cache != NULL)
		return val == (uint32_t)(int)userfw_cache_read(cache, USERFW_DUMMY_MOD, TAG) ? 1 : 0;
	return 0;
};

static userfw_match_descr dummy_matches[] =
{
	{M_TAGGED,	1,	{T_UINT32},	"tagged",	match_tag}
};

static int
cmd_echo(opcode_t op, uint32_t cookie, userfw_arg *args, struct socket *so, struct thread *td)
{
	struct userfw_io_block *msg;
	unsigned char *buf;
	size_t len;
	int err;

	/* Allocate container for reply */
	msg = userfw_msg_alloc_container(T_CONTAINER, ST_MESSAGE, 2, M_USERFW);

	/* Insert cookie as first sub-block */
	userfw_msg_insert_uint32(msg, ST_COOKIE, cookie, 0, M_USERFW);

	/* Insert received string as second sub-block */
	userfw_msg_insert_string(msg, ST_UNSPEC, args[0].string.data, args[0].string.length, 1, M_USERFW);

	/* Serialize constructed message into buffer */
	len = userfw_msg_calc_size(msg);
	buf = malloc(len, M_USERFW, M_WAITOK);
	err = userfw_msg_serialize(msg, buf, len);

	/* Free now unneeded msg */
	userfw_msg_free(msg, M_USERFW);

	/* Send data from buffer */
	userfw_domain_send_to_socket(so, buf, len);

	/* Free buffer */
	free(buf, M_USERFW);
	return 0;
}

static userfw_cmd_descr dummy_cmds[] =
{
	{CMD_ECHO,	1,	{T_STRING}, "echo", cmd_echo}
};

static userfw_modinfo dummy_modinfo =
{
	.id = USERFW_DUMMY_MOD,
	.name = "dummy",
	.nactions = sizeof(dummy_actions)/sizeof(dummy_actions[0]),
	.nmatches = sizeof(dummy_matches)/sizeof(dummy_matches[0]),
	.ncmds = sizeof(dummy_cmds)/sizeof(dummy_cmds[0]),
	.actions = dummy_actions,
	.matches = dummy_matches,
	.cmds = dummy_cmds
};

static int
dummy_modevent(module_t mod, int type, void *p)
{
	int err = 0;
	switch(type)
	{
	case MOD_LOAD:
		err = userfw_mod_register(&dummy_modinfo);
		break;
	case MOD_UNLOAD:
		err = userfw_mod_unregister(USERFW_DUMMY_MOD);
		break;
	default:
		err = EOPNOTSUPP;
		break;
	}
	return err;
}

static moduledata_t dummy_mod =
{
	"userfw_dummy",
	dummy_modevent,
	0
};

MODULE_VERSION(userfw_dummy, 1);
DEPEND_ON_USERFW_CORE(userfw_dummy);

DECLARE_MODULE(userfw_dummy, dummy_mod, SI_SUB_USERFW, SI_ORDER_USERFW_MOD);

#ifndef DUMMY_H
#define DUMMY_H

/* In most cases you should use `date +%s` as module id */
#define USERFW_DUMMY_MOD	1325523542

enum dummy_cmds
{
	CMD_ECHO
};

enum dummy_actions
{
	A_TAG
};

enum dummy_matches
{
	M_TAGGED
};

#endif /* DUMMY_H */

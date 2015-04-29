#include <netinet/in.h>

typedef struct
{
	unsigned rule;
	struct in_addr dst;
	struct in_addr src;
	_Bool protocol;
	short port;
} pc_set;

typedef void pc_root;

pc_root * pc_init();
void pc_destroy(pc_root * root);
_Bool pc_add(pc_root * root, pc_set set[], unsigned count);
_Bool pc_update(pc_root * root, pc_set old, pc_set new);
void pc_remove(pc_root * root, pc_set set);


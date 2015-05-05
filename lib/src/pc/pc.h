#include <netinet/in.h>
#include <inttypes.h>

typedef struct
{
	unsigned rule;
	struct in_addr dst;
	uint8_t dst_length;
	struct in_addr src;
	uint8_t src_length;
	_Bool protocol;
	short port_start;
	short port_end;
} pc_set;

typedef struct
{
	unsigned rule;
	struct in6_addr dst;
	uint8_t dst_length;
	struct in6_addr src;
	uint8_t src_length;
	_Bool protocol;
	short port_start;
	short port_end;
} pc6_set;

typedef void pc_root;
typedef void pc6_root;

pc_root * pc_init();
void pc_destroy(pc_root * root);
_Bool pc_add(pc_root * root, pc_set set[], unsigned count);
_Bool pc_update(pc_root * root, pc_set old, pc_set new);
void pc_remove(pc_root * root, pc_set set);

pc6_root * pc6_init();
void pc6_destroy(pc6_root * root);
_Bool pc6_add(pc6_root * root, pc6_set set[], unsigned count);
_Bool pc6_update(pc6_root * root, pc6_set old, pc6_set new);
void pc6_remove(pc6_root * root, pc6_set set);


enum
{
	POINTER,
	CHARACTER,
	SHORT
};

typedef union
{
	void * pointer;
	char character;
} list_item_value;

typedef struct list_item
{
	struct list_item * next;
	char value_type;
	list_item_value value;
} list_item;

typedef struct
{
	list_item * head;
	list_item * tail;
	unsigned size;
} list;

unsigned list_size(list * root);
void list_destroy(list * root);
list * list_init();
_Bool list_empty(list * root);
void list_append_front(list * root, list_item_value value, char value_type);
void list_append_back(list * root, list_item_value value, char value_type);
list_item_value list_pop(list * root);

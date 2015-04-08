
typedef union
{
	void * pointer;
} list_item_value;

typedef struct list_item
{
	struct list_item * next;
	list_item_value value;


} list_item;

typedef struct
{
	list_item * head;
	list_item * tail;
} list;

_Bool list_empty(list * root);
void list_append_front(list * root, list_item_value value);
void list_append_back(list * root, list_item_value value);
list_item_value list_pop(list * root);

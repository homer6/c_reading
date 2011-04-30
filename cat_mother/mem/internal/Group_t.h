#define GROUP_MAX_NAME 63


#include "Vector.h"
#include "FreedBlock_t.h"


typedef struct Group
{
	int					refs;
	struct GroupItem*	items;
	struct Group*		next;
	struct Group*		prev;
	int					hash;
	int					id;
	char				name[GROUP_MAX_NAME+1];
	int					bytesInUse;
	int					blocksInUse;
	int					bytesTotal;
	int					blocksTotal;
	Vector_t*			freedBlocks;
} Group_t;

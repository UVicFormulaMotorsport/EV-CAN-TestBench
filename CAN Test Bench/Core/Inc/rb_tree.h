/*
 * Copyright (c) 2019 xieqing. https://github.com/xieqing
 * May be freely redistributed, but copyright notice must be retained.
 */

#ifndef _RB_HEADER
#define _RB_HEADER

#define RB_DUP 1
#define RB_MIN 1

#define RED 0
#define BLACK 1

/** @brief Evil traversal method specifier for traversing the tree
 *
 */
enum rbtraversal {
	PREORDER,
	INORDER,
	POSTORDER
};

/** @brief Node of a Red-Black binary search tree
 *
 */
typedef struct rbnode {
	struct rbnode *left; /**< Left sub-tree */
	struct rbnode *right; /**< Right sub-tree */
	struct rbnode *parent; /**< Parent of node*/
	void *data; /**< Pointer to some data contained by the tree*/
	char color; /**< The color of the node (internal use only)*/
	
} rbnode;

/** @brief struct representing a binary search tree
 *
 */
typedef struct {
	int (*compare)(const void *, const void *); /**< Function to compare between two different nodes */
	void (*print)(void *); /**< For printing purposes. NOT YET IMPLEMENTED ON ANY SYSTEMS IN THE CAR*/
	void (*destroy)(void *); /**< Destructor function for whatever data is stored in the tree */

	rbnode root; /**<Root of actual tree */
	rbnode nil; /**<The "NIL" node of the tree, used to avoid fucked null errors */

	

	#ifdef RB_MIN
	rbnode *min; /**<Pointer to minimum element */
	#endif

	int count; /**<number of items stored in the tree */
} rbtree;

//Internal macros
#define RB_ROOT(rbt) (&(rbt)->root)
#define RB_NIL(rbt) (&(rbt)->nil)
#define RB_FIRST(rbt) ((rbt)->root.left)
#define RB_MINIMAL(rbt) ((rbt)->min)

#define RB_ISEMPTY(rbt) ((rbt)->root.left == &(rbt)->nil && (rbt)->root.right == &(rbt)->nil)
#define RB_APPLY(rbt, f, c, o) rbapply_node((rbt), (rbt)->root.left, (f), (c), (o))

/** @brief Create and initialize a binary search tree
 *
 */
rbtree *rbCreate(int (*compare_func)(const void *, const void *), void (*destroy_func)(void *));

/** @brief Destroy the tree, and de-allocate it's elements
 *
 */
void rbDestroy(rbtree *rbt);

/** @brief Find a node of the tree based off the data you provide the tree
 *
 *
 */
rbnode *rbFind(rbtree *rbt, void *data);
rbnode *rbSuccessor(rbtree *rbt, rbnode *node);

int rbApplyNode(rbtree *rbt, rbnode *node, int (*func)(void *, void *), void *cookie, enum rbtraversal order);
void rbPrint(rbtree *rbt, void (*print_func)(void *));

rbnode *rbInsert(rbtree *rbt, void *data);
void *rbDelete(rbtree *rbt, rbnode *node, int keep);

int rbCheckOrder(rbtree *rbt, void *min, void *max);
int rbCheckBlackHeight(rbtree *rbt);

#endif /* _RB_HEADER */

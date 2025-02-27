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
 * @param compare_func A function that compares the data of two nodes. Accepts pointers to the data as parameters
 *
 * @param destroy_func The destructor function for the data, for safe disposal of dynamically allocated data
 */
rbtree *rbCreate(int (*compare_func)(const void *, const void *), void (*destroy_func)(void *));

/** @brief Destroy the tree, and de-allocate it's elements
 *
 */
void rbDestroy(rbtree *rbt);

/** @brief Find a node of the tree based off the data you provide the tree
 *
 * @param rbt Pointer to the tree in which we would like to find the node.
 *
 *
 * @retval Returns a pointer to the node if the node is present in the tree. Otherwise, it will return NULL to indicate the
 * node could not be found.
 */
rbnode *rbFind(rbtree *rbt, void *data);
rbnode *rbSuccessor(rbtree *rbt, rbnode *node);

/** @brief Function that applies some function to a subtree.
 *
 * @param rbt Pointer to an the @c rbtree you wish to apply the functions to
 *
 * @param node Pointer to the node that is the root of the tree you wish to apply functions to. This can be a subtree of another tree if needed
 *
 * @param func The function you would like to apply to the nodes. This takes in two parameters. The first is a pointer to the data of the node.
 * The second parameter is a pointer to the shared cookie, which allows information to be preserved between different
 * calls to @c func. This function returns an integer value, with 0 being ok, and the others being various error states.
 *
 * @param cookie This is a pointer to some space in memory that all of the calls to @func share, to preserve information.
 *
 * @param order This is a member of @enum rbtraversal that specifies the order in which the tree will be traversed.
 *
 *	@attention DANGER!! RECURSION!! Beware of stack memory usage, since most tasks are memory limited.
 */
int rbApplyNode(rbtree *rbt, rbnode *node, int (*func)(void *, void *), void *cookie, enum rbtraversal order);

/** @brief Function used to print the contents of the tree.
 *
 * @param rbt a pointer to the @c rbtree you wish to print.
 *
 * @param print_func A pointer to a print function specific to the data.
 *
 * @attention DANGER!! RECURSION!!
 *
 * @deprecated Leftovers from laptop unit tests. Sorta useless, cause like what are we gonna print to?
 */
void rbPrint(rbtree *rbt, void (*print_func)(void *));

/** @brief Function that inserts data into the tree, and creates a new node.
 *
 * @param rbt Instance of a @c rbtree that we would like to insert data into
 *
 * @param data Pointer to the data we wish to insert
 *
 * @retval This function returns a pointer to the @c rbnode that was added. The function will
 * return NULL if the system is out of memory, or is otherwise unable to insert the node.
 */
rbnode *rbInsert(rbtree *rbt, void *data);

/** @brief Deletes a node from the tree.
 *
 * @param rbt Instance of a @c rbtree that we are removing the node from.
 *
 * @param node Pointer to the node that we would like to remove
 *
 * @param keep If @c keep is a truthy value, a pointer to the data of the node will be returned.
 * Otherwise, the node and it's data will be destroyed.
 *
 * @retval If @c keep is "true", this will return a pointer to the data held by the deleted node. Otherwise it will return NULL.
 *
 *
 */
void *rbDelete(rbtree *rbt, rbnode *node, int keep);

/** @brief Function that validates that the order of the nodes in the tree is correct.
 *
 * @attention DANGER!! THIS FUNCTION IS RECURSIVE. This is intended to be used in a laptop debugging context.
 * The VCU simply does not have enough memory to deal with recursion of this manner.
 *
 * @deprecated Leftovers from laptop unit tests.
 *
 */
int rbCheckOrder(rbtree *rbt, void *min, void *max);

/** @brief Function that Checks the height of black nodes.
 *
 * @attention DANGER!! THIS FUNCTION IS RECURSIVE. This is intended to be used in a laptop debugging context.
 * The VCU simply does not have enough memory to deal with recursion of this manner.
 *
 * @deprecated Leftovers from laptop unit tests.
 *
 */
int rbCheckBlackHeight(rbtree *rbt);

#endif /* _RB_HEADER */

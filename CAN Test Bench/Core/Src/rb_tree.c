/*
 * Copyright (c) 2019 xieqing. https://github.com/xieqing
 * May be freely redistributed, but copyright notice must be retained.
 *
 * Big shout-out to this dude^^^
 */

#include "rb_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include "uvfr_utils.h"
//#include "uvfr_macro.h"

static void insertRepair(rbtree *rbt, rbnode *current);
static void deleteRepair(rbtree *rbt, rbnode *current);
static void rotateLeft(rbtree *, rbnode *);
static void rotateRight(rbtree *, rbnode *);
static int checkOrder(rbtree *rbt, rbnode *n, void *min, void *max);
static int checkBlackHeight(rbtree *rbt, rbnode *node);
static void print(rbtree *rbt, rbnode *node, void (*print_func)(void *), int depth, char *label);
static void destroyAllNodes(rbtree *rbt, rbnode *node);

/*
 * construction
 * return NULL if out of memory
 */
rbtree *rbCreate(int (*compare)(const void *, const void *), void (*destroy)(void *))
{
	rbtree *rbt;

	rbt = (rbtree *) uvMalloc(sizeof(rbtree));
	if (rbt == NULL)
		return NULL; /* out of memory */

	rbt->compare = compare;
	rbt->destroy = destroy;

	rbt->count = 0;

	/* sentinel node nil */
	rbt->nil.left = rbt->nil.right = rbt->nil.parent = RB_NIL(rbt);
	rbt->nil.color = BLACK;
	rbt->nil.data = NULL;

	/* sentinel node root */
	rbt->root.left = rbt->root.right = rbt->root.parent = RB_NIL(rbt);
	rbt->root.color = BLACK;
	rbt->root.data = NULL;

	#ifdef RB_MIN
	rbt->min = NULL;
	#endif
	
	return rbt;
}

/*
 * destruction
 */
void rbDestroy(rbtree *rbt)
{
	destroyAllNodes(rbt, RB_FIRST(rbt));
	uvFree(rbt);
}

/*
 * look up
 * return NULL if not found
 */
rbnode *rbFind(rbtree *rbt, void *data)
{
	rbnode *p;

	p = RB_FIRST(rbt);

	while (p != RB_NIL(rbt)) {
		int cmp;
		cmp = rbt->compare(data, p->data);
		if (cmp == 0)
			return p; /* found */
		p = cmp < 0 ? p->left : p->right;
	}

	return NULL; /* not found */
}

/*
 * next larger
 * return NULL if not found
 */
rbnode *rbSuccessor(rbtree *rbt, rbnode *node)
{
	rbnode *p;

	p = node->right;

	if (p != RB_NIL(rbt)) {
		/* move down until we find it */
		for ( ; p->left != RB_NIL(rbt); p = p->left) ;
	} else {
		/* move up until we find it or hit the root */
		for (p = node->parent; node == p->right; node = p, p = p->parent) ;

		if (p == RB_ROOT(rbt))
			p = NULL; /* not found */
	}

	return p;
}

/*
 * apply func
 * return non-zero if error
 */
int rbApplyNode(rbtree *rbt, rbnode *node, int (*func)(void *, void *), void *cookie, enum rbtraversal order)
{
	int err;

	if(rbt == NULL){
		return 1;
	}

	if(node == NULL){
		return 1;
	}

	if(func == NULL){
		return 1;
	}

	//Cookie is allowed to be null, because cookie is not always required.

	if (node != RB_NIL(rbt)) {
		if (order == PREORDER && (err = func(node->data, cookie)) != 0) /* preorder */
			return err;
		if ((err = rbApplyNode(rbt, node->left, func, cookie, order)) != 0) /* left */
			return err;
		if (order == INORDER && (err = func(node->data, cookie)) != 0) /* inorder */
			return err;
		if ((err = rbApplyNode(rbt, node->right, func, cookie, order)) != 0) /* right */
			return err;
		if (order == POSTORDER && (err = func(node->data, cookie)) != 0) /* postorder */
			return err;
	}

	return 0;
}

/*
 * rotate left about x
 */
void rotateLeft(rbtree *rbt, rbnode *x)
{
	rbnode *y;

	y = x->right; /* child */

	/* tree x */
	x->right = y->left;
	if (x->right != RB_NIL(rbt))
		x->right->parent = x;

	/* tree y */
	y->parent = x->parent;
	if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;

	/* assemble tree x and tree y */
	y->left = x;
	x->parent = y;
}

/*
 * rotate right about x
 */
void rotateRight(rbtree *rbt, rbnode *x)
{
	rbnode *y;

	y = x->left; /* child */

	/* tree x */
	x->left = y->right;
	if (x->left != RB_NIL(rbt))
		x->left->parent = x;

	/* tree y */
	y->parent = x->parent;
	if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;

	/* assemble tree x and tree y */
	y->right = x;
	x->parent = y;
}


/*
 * insert (or update) data
 * return NULL if out of memory
 */
rbnode *rbInsert(rbtree *rbt, void *data)
{
	rbnode *current, *parent;
	rbnode *new_node;

	/* do a binary search to find where it should be */

	current = RB_FIRST(rbt);
	parent = RB_ROOT(rbt);

	while (current != RB_NIL(rbt)) {
		int cmp;
		cmp = rbt->compare(data, current->data);

		#ifndef RB_DUP
		if (cmp == 0) {
			rbt->destroy(current->data);
			current->data = data;
			return current; /* updated */
		}
		#endif

		parent = current;
		current = cmp < 0 ? current->left : current->right;
	}

	/* replace the termination NIL pointer with the new node pointer */

	current = new_node = (rbnode *) uvMalloc(sizeof(rbnode));
	if (current == NULL)
		return NULL; /* out of memory */

	current->left = current->right = RB_NIL(rbt);
	current->parent = parent;
	current->color = RED;
	current->data = data;
	
	if (parent == RB_ROOT(rbt) || rbt->compare(data, parent->data) < 0)
		parent->left = current;
	else
		parent->right = current;

	#ifdef RB_MIN
	if (rbt->min == NULL || rbt->compare(current->data, rbt->min->data) < 0)
		rbt->min = current;
	#endif
	
	/*
	 * insertion into a red-black tree:
	 *   0-children root cluster (parent node is BLACK) becomes 2-children root cluster (new root node)
	 *     paint root node BLACK, and done
	 *   2-children cluster (parent node is BLACK) becomes 3-children cluster
	 *     done
	 *   3-children cluster (parent node is BLACK) becomes 4-children cluster
	 *     done
	 *   3-children cluster (parent node is RED) becomes 4-children cluster
	 *     rotate, and done
	 *   4-children cluster (parent node is RED) splits into 2-children cluster and 3-children cluster
	 *     split, and insert grandparent node into parent cluster
	 */
	if (current->parent->color == RED) {
		/* insertion into 3-children cluster (parent node is RED) */
		/* insertion into 4-children cluster (parent node is RED) */
		insertRepair(rbt, current);
	} else {
		/* insertion into 0-children root cluster (parent node is BLACK) */
		/* insertion into 2-children cluster (parent node is BLACK) */
		/* insertion into 3-children cluster (parent node is BLACK) */
	}

	/*
	 * the root is always BLACK
	 * insertion into 0-children root cluster or insertion into 4-children root cluster require this recoloring
	 */
	RB_FIRST(rbt)->color = BLACK;
	
	rbt->count++;

	return new_node;
}

/*
 * rebalance after insertion
 * RB_ROOT(rbt) is always BLACK, thus never reach beyond RB_FIRST(rbt)
 * after insertRepair, RB_FIRST(rbt) might be RED
 */
void insertRepair(rbtree *rbt, rbnode *current)
{
	rbnode *uncle;

	do {
		/* current node is RED and parent node is RED */

		if (current->parent == current->parent->parent->left) {
			uncle = current->parent->parent->right;
			if (uncle->color == RED) {
				/* insertion into 4-children cluster */

				/* split */
				current->parent->color = BLACK;
				uncle->color = BLACK;

				/* send grandparent node up the tree */
				current = current->parent->parent; /* goto loop or break */
				current->color = RED;
			} else {
				/* insertion into 3-children cluster */

				/* equivalent BST */
				if (current == current->parent->right) {
					current = current->parent;
					rotateLeft(rbt, current);
				}

				/* 3-children cluster has two representations */
				current->parent->color = BLACK; /* thus goto break */
				current->parent->parent->color = RED;
				rotateRight(rbt, current->parent->parent);
			}
		} else {
			uncle = current->parent->parent->left;
			if (uncle->color == RED) {
				/* insertion into 4-children cluster */

				/* split */
				current->parent->color = BLACK;
				uncle->color = BLACK;

				/* send grandparent node up the tree */
				current = current->parent->parent; /* goto loop or break */
				current->color = RED;
			} else {
				/* insertion into 3-children cluster */

				/* equivalent BST */
				if (current == current->parent->left) {
					current = current->parent;
					rotateRight(rbt, current);
				}

				/* 3-children cluster has two representations */
				current->parent->color = BLACK; /* thus goto break */
				current->parent->parent->color = RED;
				rotateLeft(rbt, current->parent->parent);
			}
		}
	} while (current->parent->color == RED);
}

/*
 * delete node
 * return NULL if keep is zero (already freed)
 */
void *rbDelete(rbtree *rbt, rbnode *node, int keep)
{
	rbnode *target, *child;
	void *data;
	
	if(rbt == NULL || target == NULL){
		return NULL;
	}

	data = node->data;

	/* choose node's in-order successor if it has two children */
	
	if (node->left == RB_NIL(rbt) || node->right == RB_NIL(rbt)) {
		target = node;

		#ifdef RB_MIN
		if (rbt->min == target)
			rbt->min = rbSuccessor(rbt, target); /* deleted, thus min = successor */
		#endif
	} else {
		target = rbSuccessor(rbt, node); /* node->right must not be NIL, thus move down */

		node->data = target->data; /* data swapped */

		#ifdef RB_MIN
		/* if min == node, then min = successor = node (swapped), thus idle */
		/* if min == target, then min = successor, which is not the minimal, thus impossible */
		#endif
	}

	child = (target->left == RB_NIL(rbt)) ? target->right : target->left; /* child may be NIL */

	/*
	 * deletion from red-black tree
	 *   4-children cluster (RED target node) becomes 3-children cluster
	 *     done
	 *   3-children cluster (RED target node) becomes 2-children cluster
	 *     done
	 *   3-children cluster (BLACK target node, RED child node) becomes 2-children cluster
	 *     paint child node BLACK, and done
	 *
	 *	 2-children root cluster (BLACK target node, BLACK child node) becomes 0-children root cluster
	 *     done
	 *
	 *   2-children cluster (BLACK target node, 4-children sibling cluster) becomes 3-children cluster
	 *     transfer, and done
	 *   2-children cluster (BLACK target node, 3-children sibling cluster) becomes 2-children cluster
	 *     transfer, and done
	 *
	 *   2-children cluster (BLACK target node, 2-children sibling cluster, 3/4-children parent cluster) becomes 3-children cluster
	 *     fuse, paint parent node BLACK, and done
	 *   2-children cluster (BLACK target node, 2-children sibling cluster, 2-children parent cluster) becomes 3-children cluster
	 *     fuse, and delete parent node from parent cluster
	 */
	if (target->color == BLACK) {
		if (child->color == RED) {
			/* deletion from 3-children cluster (BLACK target node, RED child node) */
			child->color = BLACK;
		} else if (target == RB_FIRST(rbt)) {
			/* deletion from 2-children root cluster (BLACK target node, BLACK child node) */
		} else {
			/* deletion from 2-children cluster (BLACK target node, ...) */
			deleteRepair(rbt, target);
		}
	} else {
		/* deletion from 4-children cluster (RED target node) */
		/* deletion from 3-children cluster (RED target node) */
	}

	if (child != RB_NIL(rbt))
		child->parent = target->parent;

	if (target == target->parent->left)
		target->parent->left = child;
	else
		target->parent->right = child;

	uvFree(target);
	
	/* keep or discard data */
	if (keep == 0) {
		rbt->destroy(data);
		data = NULL;
	}

	rbt->count --;

	return data;
}

/*
 * rebalance after deletion
 */
void deleteRepair(rbtree *rbt, rbnode *current)
{
	rbnode *sibling;
	do {
		if (current == current->parent->left) {
			sibling = current->parent->right;

			if (sibling->color == RED) {
				/* perform an adjustment (3-children parent cluster has two representations) */
				sibling->color = BLACK;
				current->parent->color = RED;
				rotateLeft(rbt, current->parent);
				sibling = current->parent->right;
			}

			/* sibling node must be BLACK now */

			if (sibling->right->color == BLACK && sibling->left->color == BLACK) {
				/* 2-children sibling cluster, fuse by recoloring */
				sibling->color = RED;
				if (current->parent->color == RED) { /* 3/4-children parent cluster */
					current->parent->color = BLACK;
					break; /* goto break */
				} else { /* 2-children parent cluster */
					current = current->parent; /* goto loop */
				}
			} else {
				/* 3/4-children sibling cluster */
				
				/* perform an adjustment (3-children sibling cluster has two representations) */
				if (sibling->right->color == BLACK) {
					sibling->left->color = BLACK;
					sibling->color = RED;
					rotateRight(rbt, sibling);
					sibling = current->parent->right;
				}

				/* transfer by rotation and recoloring */
				sibling->color = current->parent->color;
				current->parent->color = BLACK;
				sibling->right->color = BLACK;
				rotateLeft(rbt, current->parent);
				break; /* goto break */
			}
		} else {
			sibling = current->parent->left;

			if (sibling->color == RED) {
				/* perform an adjustment (3-children parent cluster has two representations) */
				sibling->color = BLACK;
				current->parent->color = RED;
				rotateRight(rbt, current->parent);
				sibling = current->parent->left;
			}

			/* sibling node must be BLACK now */

			if (sibling->right->color == BLACK && sibling->left->color == BLACK) {
				/* 2-children sibling cluster, fuse by recoloring */
				sibling->color = RED;
				if (current->parent->color == RED) { /* 3/4-children parent cluster */
					current->parent->color = BLACK;
					break; /* goto break */
				} else { /* 2-children parent cluster */
					current = current->parent; /* goto loop */
				}
			} else {
				/* 3/4-children sibling cluster */

				/* perform an adjustment (3-children sibling cluster has two representations) */
				if (sibling->left->color == BLACK) {
					sibling->right->color = BLACK;
					sibling->color = RED;
					rotateLeft(rbt, sibling);
					sibling = current->parent->left;
				}

				/* transfer by rotation and recoloring */
				sibling->color = current->parent->color;
				current->parent->color = BLACK;
				sibling->left->color = BLACK;
				rotateRight(rbt, current->parent);
				break; /* goto break */
			}
		}
	} while (current != RB_FIRST(rbt));
}

/*
 * check order of tree
 */
int rbCheckOrder(rbtree *rbt, void *min, void *max)
{
	return checkOrder(rbt, RB_FIRST(rbt), min, max);
}

/*
 * check order recursively
 */
int checkOrder(rbtree *rbt, rbnode *n, void *min, void *max)
{
	if (n == RB_NIL(rbt))
		return 1;

	#ifdef RB_DUP
	if (rbt->compare(n->data, min) < 0 || rbt->compare(n->data, max) > 0)
	#else
	if (rbt->compare(n->data, min) <= 0 || rbt->compare(n->data, max) >= 0)
	#endif
		return 0;

	return checkOrder(rbt, n->left, min, n->data) && checkOrder(rbt, n->right, n->data, max);
}

/*
 * check black height of tree
 */
int rbCheckBlackHeight(rbtree *rbt)
{
	if (RB_ROOT(rbt)->color == RED || RB_FIRST(rbt)->color == RED || RB_NIL(rbt)->color == RED)
		return 0;

	return checkBlackHeight(rbt, RB_FIRST(rbt));
}

/*
 * check black height recursively
 */
int checkBlackHeight(rbtree *rbt, rbnode *n)
{
	int lbh, rbh;

	if (n == RB_NIL(rbt))
		return 1;

	if (n->color == RED && (n->left->color == RED || n->right->color == RED || n->parent->color == RED))
		return 0;

	if ((lbh = checkBlackHeight(rbt, n->left)) == 0)
		return 0;

	if ((rbh = checkBlackHeight(rbt, n->right)) == 0)
		return 0;

	if (lbh != rbh)
		return 0;

	return lbh + (n->color == BLACK ? 1 : 0);
}

/*
 * print tree
 */
void rbPrint(rbtree *rbt, void (*print_func)(void *))
{
	printf("\n--\n");
	print(rbt, RB_FIRST(rbt), print_func, 0, "T");
	printf("\ncheckBlackHeight = %d\n", rbCheckBlackHeight(rbt));
}

/*
 * print node recursively
 */
void print(rbtree *rbt, rbnode *n, void (*print_func)(void *), int depth, char *label)
{
	if (n != RB_NIL(rbt)) {
		print(rbt, n->right, print_func, depth + 1, "R");
		printf("%*s", 8 * depth, "");
		if (label)
			printf("%s: ", label);
		print_func(n->data);
		printf(" (%s)\n", n->color == RED ? "r" : "b");
		print(rbt, n->left, print_func, depth + 1, "L");
	}
}

/*
 * destroy node recursively
 */

// void destroy(rbtree *rbt, rbnode *n)
// {
// 	if (n != RB_NIL(rbt)) {
// 		destroy(rbt, n->left);
// 		destroy(rbt, n->right);
// 		rbt->destroy(n->data);
// 		free(n);
// 	}
// }



/* Function to delete all the nodes in a tree and free their data as well

*/
void destroyAllNodes(rbtree *rbt, rbnode *n){

	if(n == RB_NIL(rbt)){
		return;
	}

	if(n->parent != RB_NIL(rbt)){
		if(n->parent->left == n){
			//is left node
			n->parent->left = RB_NIL(rbt);
		}else{
			n->parent->right = RB_NIL(rbt);
		}
	}

	int stack_sz = (1+(rbt->count)/2);
	int stack_top = 0;
	rbnode** del_stack = uvMalloc(stack_sz*sizeof(rbnode*));

	if(del_stack == NULL){
		return; //FUCK
	}
	
	rbnode* tmp;
	del_stack[stack_top] = n;
	stack_top++;
	do{
		stack_top--;
		tmp = del_stack[stack_top];
		

		if(tmp == RB_NIL(rbt)){
			break;
		}

		if(tmp->left != RB_NIL(rbt) && tmp->left != NULL){ //add left to stack
			del_stack[stack_top] = tmp->left;
			stack_top++;
		}

		if(tmp->right != RB_NIL(rbt) && tmp->right != NULL){ //add right to stack if possible
			del_stack[stack_top] = tmp->right;
			stack_top++;
		}

		rbt->destroy(tmp->data);
		uvFree(tmp);
	
	}while(stack_top > 0); //stack top being 0 indicates that we done
	
	uvFree(del_stack);
	
}

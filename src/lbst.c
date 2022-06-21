#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lbst.h"
#include "lbst_private.h"

static void lbst_delete_tree(struct lbst_node *node);


/* Inserts a new (key, val) into the dictionary. If key is already in the
dictionary, its val is updated.

Returns 1 on success, else 0.

Time complexity: O(h) */
int lbst_insert(lbst_T root, char *key, char *val) {
    struct lbst *root_private = root;
    struct lbst_node *ptr, *n1, *n2, *parent, *grparent;

    /* special case: root has not been initialized using lbst_create() */
    if (root_private == NULL) {
        return 0;
    }

    /* special case: 0 (key, val) nodes */
    if (root_private->head == NULL) {
        ptr = malloc(sizeof(struct lbst_node));
        ptr->key = strdup(key);
        ptr->val = strdup(val);
        if (ptr == NULL || ptr->key == NULL || ptr->val == NULL) {
            return 0;
        }
        ptr->next = NULL;
        ptr->lc = NULL;
        ptr->rc = NULL;
        root_private->head = ptr;
        return 1;
    }

    /* traverse the tree and find a leaf that should be
    used as parent node of the new nodes */
    ptr = root_private->head;
    parent = NULL;
    grparent = NULL; /* parent node of parent */
    while(ptr != NULL) {
        grparent = parent;
        parent = ptr;
        if (strcmp(key, ptr->key) > 0) {
            ptr = ptr->rc;
        }
        else {
            ptr = ptr->lc;
        }
    }

    /* update the parent val if the same key was provided */
    if (strcmp(parent->key, key) == 0) {
        free(parent->val);
        parent->val = strdup(val);
        return 1;
    }

    /* create two new nodes n1, n2 */
    n1 = malloc(sizeof(struct lbst_node));
    n2 = malloc(sizeof(struct lbst_node));
    if (n1 == NULL || n2 == NULL) {
        return 0;
    }
    n1->rc = NULL;
    n1->lc = NULL;
    n1->next = NULL;
    n2->rc = NULL;
    n2->lc = NULL;
    n2->next = NULL;

    /* initialize pointers */
    n1->next = parent->next;
    parent->next = n1;
    n2->lc = parent;
    n2->rc = n1;

    /* now arrange (key, val) of grparent, parent, n1, n2 based on
    the new key */
    if (strcmp(key, parent->key) > 0) {
        n1->key = strdup(key);
        n1->val = strdup(val);
        n2->key = strdup(parent->key);
        n2->val = strdup(parent->val);
    }
    else {
        n1->key = parent->key;
        n1->val = parent->val;
        n2->key = strdup(key);
        n2->val = strdup(val);
        parent->key = strdup(key);
        parent->val = strdup(val);
    }

    if (grparent != NULL) {
        if (parent == grparent->lc) {
            grparent->lc = n2;
        }
        else {
            grparent->rc = n2;
        }
    }
    else {
        root_private->head = n2;
    }

    return 1;
}


/* Deletes a key from the dictionary.

Time complexity: O(h) */
void lbst_delete(lbst_T root, char *key) {
    struct lbst *root_private;
    struct lbst_node *ptr, *ptr2, *child, *parent, *grparent, *last_right_child;

    root_private = root;
    if (root_private == NULL) {
        return;
    }

    /* traverse the tree based on the key and find the leaf node that
    has the key */
    ptr = root_private->head;
    last_right_child = NULL; /* last right child while traversing tree */
    child = NULL; /* this node should have the key */
    parent = NULL; /* parent node of child */
    grparent = NULL; /* parent node of parent node */
    while(ptr != NULL) {
        grparent = parent;
        parent = child;
        child = ptr;
        if (parent != NULL && parent->rc == child) {
            last_right_child = parent;
        }
        if (strcmp(key, ptr->key) > 0) {
            ptr = ptr->rc;
        }
        else {
            ptr = ptr->lc;
        }
    }

    /* nothing to do: key does not exist in the dictionary */
    if (child == NULL || strcmp(child->key, key) != 0) {
        return;
    }

    /* key was found in a leaf but parent is NULL: root node has the key */
    if (parent == NULL) {
        child->next = NULL;
        child->lc = NULL;
        child->rc = NULL;
        root_private->head = NULL;
        free(child->key);
        free(child->val);
        child->key = NULL;
        child->val = NULL;
        free(child);
        return;
    }

    /* start from from the left child of last_right_node and go to
    rightmost node: Connect it to the next node of child since child
    will be deleted */
    if (last_right_child != NULL) {
        ptr = NULL;
        ptr2 = last_right_child->lc;
        while (ptr2 != NULL) {
            ptr = ptr2;
            ptr2 = ptr2->rc;
        }
        ptr->next = child->next;
    }
    
    /* if grparent of child is NULL, the parent node of child is
    the root node of the tree. But both parent and child will be deleted
    therefore root node needs to change */
    if (grparent == NULL) {
        if (parent->rc == child) {
            root_private->head = parent->lc;
        }
        else {
            root_private->head = parent->rc;
        }
    }

    /* else connect the grparent node of child to the sibling of
    its parent node */
    else {
        if (parent->rc == child) {
            ptr = parent->lc;
        }
        else {
            ptr = parent->rc;
        }
        if (grparent->lc == parent) {
            grparent->lc = ptr;
        }
        else {
            grparent->rc = ptr;
        }
    }

    /* free memory occupied by child and parent */
    child->next = NULL;
    child->lc = NULL;
    child->rc = NULL;
    free(child->key);
    free(child->val);
    child->key = NULL;
    child->val = NULL;
    free(child);

    parent->next = NULL;
    parent->lc = NULL;
    parent->rc = NULL;
    free(parent->key);
    free(parent->val);
    parent->key = NULL;
    parent->val = NULL;
    free(parent);
}


/* Searches for a key in the dictionary.

If found, it returns the a pointer to its val, else it returns NULL.

Time complexity: O(h) */
char *lbst_lookup(lbst_T root, char *key) {
    struct lbst *root_private;
    struct lbst_node *ptr, *prev;

    root_private = root;
    if (root_private == NULL) {
        return NULL;
    }

    ptr = root_private->head;
    prev = NULL;
    while(ptr != NULL) {
        prev = ptr;
        if (strcmp(key, ptr->key) > 0) {
            ptr = ptr->rc;
        }
        else {
            ptr = ptr->lc;
        }
    }
    if (prev != NULL && strcmp(prev->key, key) == 0) {
        return prev->val;
    }

    return NULL;
}


/* Returns 1 if dictionary has no keys, 0 otherwise */
int lbst_is_empty(lbst_T root) {
    struct lbst *root_private;

    root_private = root;
    if (root_private == NULL || root_private->head == NULL) {
        return 1;
    }

    return 0;
}


/* Creates and returns an empty dictionary. Its (key, val) pairs have
type (char *, char *).

Returns NULL on fail. */
lbst_T lbst_create() {
    struct lbst *root_private;

    root_private = malloc(sizeof(struct lbst));
    if (root_private == NULL) {
        return NULL;
    }
    root_private->head = NULL;

    return (lbst_T) root_private;
}



/* Clears the dictionary. The function lbst_is_empty() returns 1 after
calling this one. */
void lbst_clear(lbst_T root) {
    struct lbst *root_private;

    root_private = root;
    if (root_private == NULL) {
        return;
    }
    lbst_delete_tree(root_private->head);
    root_private->head = NULL;
}


/* Destroys the dictionary. No other functions should be called after
calling this one. */
void lbst_destroy(lbst_T root) {
    struct lbst *root_private;

    root_private = root;
    if (root_private == NULL) {
        return;
    }
    lbst_clear(root);
    free(root_private);
}


/* Deletes all descendant nodes of node */
static void lbst_delete_tree(struct lbst_node *node) {
    if (node == NULL) {
        return;
    }
    lbst_delete_tree(node->lc);
    lbst_delete_tree(node->rc);
    free(node->key);
    free(node->val);
    node->key = NULL;
    node->val = NULL;
    node->lc = NULL;
    node->rc = NULL;
    free(node);
}


/* Prints (key, val) that satisfy first <= key <= last.

Time complexity: O(h + last - first) */
void lbst_range_query(lbst_T root, char *first, char *last, void (*print)(char *key, void *val)) {
    struct lbst *root_private;
    struct lbst_node *ptr, *prev;

    assert(print);

    root_private = root;
    if (root_private == NULL) {
        return;
    }

    ptr = root_private->head;
    prev = NULL;
    while(ptr != NULL) {
        prev = ptr;
        if (strcmp(first, ptr->key) > 0) {
            ptr = ptr->rc;
        } else {
            ptr = ptr->lc;
        }
    }
    if (prev != NULL && strcmp(first, prev->key) < 0 && strcmp(last, prev->key) > 0) {
        print(prev->key, prev->val);
        printf(" ");
    }
    prev = prev->next;
    while (prev != NULL && strcmp(last, prev->key) > 0) {
        print(prev->key, prev->val);
        printf(" ");
        prev = prev->next;
    }
    printf("\n");
}


/* Prints the dictionary. (key, val) pairs are sorted by key (ascending).

Time complexity: O(h + #keys) */
void lbst_print(lbst_T root, void (*print)(char *key, void *val)) {
    struct lbst *root_private;
    struct lbst_node *ptr, *prev;

    assert(print);

    root_private = root;
    if (root_private == NULL) {
        return;
    }

    /* Go to leftmost leaf node */
    ptr = root_private->head;
    prev = NULL;
    while(ptr != NULL) {
        prev = ptr;
        ptr = ptr->lc;
    }

    /* Use the next pointers to traverse the nodes
    that start from the leftmost leaf to the rightmost leaf */
    while(prev != NULL) {
        print(prev->key, prev->val);
        printf(" ");
        prev = prev->next;
    }
    printf("\n");
}

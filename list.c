#include <stdio.h>
#include <stdlib.h>

typedef struct tagList{
	int value;
	struct tagList *next;
	struct tagList *prev;
}LIST;

LIST *init_node(LIST *node)
{
	node->next = NULL;
	node->prev = NULL;
	node->value = 0;
	return node;
}

LIST *add_node(LIST *root, LIST *add)
{
	LIST *tmp;

	if(root == NULL){
		return root = add;
	}

	tmp = root;
	while(root->next != NULL)
		root = root->next;
	
	root->next = add;
	add->prev = root;
	return tmp;
}

void destroy_list(LIST *root)
{
	LIST *tmp;

	while(root != NULL){
		tmp = root->next;

		free(root);

		root = tmp;
	}
}

int main(void)
{
	LIST *root, *child;
	int i;

	// init
	root = NULL;

	// append!!
	for(i=0; i<10; i++){
		child = init_node((LIST *)malloc(sizeof(LIST)));
		child->value = i;
		root = add_node(root, child);
	}

	// destroy
	//destroy_list(root);

	// show list
	for(i=0; root!=NULL; root=root->next){
		printf("%d: %d\n", i++, root->value);
	}
	return 0;
}

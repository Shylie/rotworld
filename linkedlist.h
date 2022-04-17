#ifndef LINKEDLIST_H
#define LINKEDLIST_H

template <typename T>
class LinkedList
{
private:
	struct Node
	{
		T data;
		Node* next;

		Node(T data) : data(data), next(nullptr) { }
	} *head;

	void destruct(Node* current)
	{
		if (current->next) { destruct(current->next); }
		delete current;
	}

	Node* insert(Node* current, T data)
	{
		if (current->next)
		{
			return insert(current->next, data);
		}
		else
		{
			current->next = new Node(data);
			return current->next;
		}
	}

public:
	LinkedList() : head(nullptr) { }
	~LinkedList()
	{
		destruct(head);
	}

	T* insert(T data)
	{
		Node* inserted;
		if (head)
		{
			inserted = insert(head, data);
		}
		else
		{
			head = new Node(data);
			inserted = head;
		}
		return &inserted->data;

	}
};

#endif//LINKEDLIST_H

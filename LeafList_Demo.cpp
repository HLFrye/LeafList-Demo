// TreeFlip.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <list>
#include <map>

#include "leaf_list.h"

class Node
{
public:
	Node(std::string s):data(s) {}
	Node():data("") {}

	void add(Node * pNode);

	std::list<Node*> sub_nodes;

	std::string data;
};

void Node::add(Node * pNode)
{
	sub_nodes.push_back(pNode);
}

class LeafBranchSorter
{
public:
	LeafBranchSorter() {}

	std::map<Node*, std::list<Node*>> leaf_map;
	std::map<Node*, std::list<Node*>> branch_map;

	class leafs
	{
	public: 
		leafs(LeafBranchSorter* p):pSorter(p) {}
		std::list<Node*>& operator()(Node *);
	private:
		LeafBranchSorter * pSorter;
	};

	class branches
	{
	public:
		branches(LeafBranchSorter* p):pSorter(p) {}
		std::list<Node*>& operator()(Node *);
	private:
		LeafBranchSorter * pSorter;
	};

	
	leafs get_leafs()
	{
		return leafs(this);
	}

	branches get_branches()
	{
		return branches(this);
	}
};

std::list<Node*>& LeafBranchSorter::leafs::operator()(Node* pNode)
{
	if (pSorter->leaf_map[pNode].empty())
	{
		for (auto i = pNode->sub_nodes.begin();
			i != pNode->sub_nodes.end();
			++i)
		{
			if ((*i)->sub_nodes.empty())
				pSorter->leaf_map[pNode].push_back(*i);
		}
	}
	return pSorter->leaf_map[pNode];
}

std::list<Node*>& LeafBranchSorter::branches::operator()(Node* pNode)
{
	if (pSorter->branch_map[pNode].empty())
	{
		for (auto i = pNode->sub_nodes.begin();
			i != pNode->sub_nodes.end();
			++i)
		{
			if (!(*i)->sub_nodes.empty())
				pSorter->branch_map[pNode].push_back(*i);
		}
	}
	return pSorter->branch_map[pNode];
}

void add_bunches(int depth, Node * pNode, std::string data)
{
	if (depth == 0)
		pNode->add(new Node(data));
	else
	{
		Node * pNewNode = new Node();
		pNode->add(pNewNode);
		add_bunches(--depth, pNewNode, data);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	Node * top = new Node();
	add_bunches(3, top, "This is three deep");
	add_bunches(4, top, "This is four deep");
	add_bunches(1, top, "This is one deep");
	add_bunches(0, top, "This is top level");
	add_bunches(99, top, "This is wayyy deep");

	LeafBranchSorter sorter;
	LeafBranchSorter::leafs get_leafs = sorter.get_leafs();
	LeafBranchSorter::branches get_branches = sorter.get_branches();

	leaf_list<std::list<Node*>, std::list<Node*>> leaflist(top, get_branches, get_leafs);

	for (auto i = leaflist.begin();
		i != leaflist.end();
		++i)
	{
		printf("%s\n", (*i)->data.c_str());
	}

	return 0;
}


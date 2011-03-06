#ifndef __TREE_LIST_H_INCLUDED
#define __TREE_LIST_H_INCLUDED

#include <stack>
#include <functional>

template<typename B, typename L>
class leaf_list
{
public:
	typedef typename B::value_type BValue;
	typedef typename L::value_type LValue;
	typedef typename std::function<B& (BValue&)> BGetter;
	typedef typename std::function<L& (LValue&)> LGetter;
	typedef typename std::function<bool (BValue&)> BIgnorer;
	typedef typename std::function<bool (LValue&)> LIgnorer;

	leaf_list(BValue& t, BGetter bg, LGetter lg, BIgnorer bi = [](BValue&){return false;}, LIgnorer li = [](LValue&){return false;})
		:top(t), get_branches(bg), get_leafs(lg), ignore_branch(bi), ignore_leaf(li)
	{}

	class iterator_end;
	class iterator
	{
	public:
		friend class leaf_list;
		iterator()
			:p(nullptr)
		{}

		explicit iterator(leaf_list * ptl)
			:p(ptl)
		{
			initialize(p->top);
		}

		~iterator();

		iterator(iterator&& rhs)
			:states(std::move(rhs.states))
			,p(rhs.p)
		{}

		iterator & operator=(iterator&& rhs);
		iterator & operator++();		//preincrement
		bool operator==(const iterator& rhs);
		bool operator==(const iterator_end& rhs);

		bool operator!=(const iterator& rhs)
		{ return !operator==(rhs); }

		bool operator!=(const iterator_end& rhs)
		{ return !operator==(rhs); }

		LValue& operator*();
		LValue* operator->();

		std::list<BValue&> parents();

	private:
		void initialize(BValue& node);
		void get_next_leaves();
		void get_next_leaf();
		LValue& get_current_leaf();
		void move_up();

	private:
		struct state
		{
			state(BValue& n, B& c, L& l)
				:node(n), children(c), child_iter(std::move(c.begin())), leafs(l), leaf_iter(std::move(l.begin()))
			{}

			BValue& node;
			B& children;
			typename B::iterator child_iter;
			L& leafs;
			typename L::iterator leaf_iter;
		};
		std::stack<state*> states;		
		leaf_list *p;
	};

	class iterator_end : public iterator
	{
	};

	iterator begin()
	{
		return iterator(this);
	}

	iterator_end end()
	{
		return iterator_end();
	}

private:
	BValue &top;
	BGetter get_branches;
	LGetter get_leafs;
	BIgnorer ignore_branch;
	LIgnorer ignore_leaf;
};

//internal functions
template<typename B, typename L>
void leaf_list<B,L>::iterator::initialize(BValue& node)
{	
	if (p->ignore_branch(node))
		return;

	state * newState = new state(node, p->get_branches(node), p->get_leafs(node));

	states.push(newState);

	if (newState->child_iter != newState->children.end())
		initialize(*newState->child_iter);
	else if (newState->leaf_iter == newState->leafs.end())
		move_up();
}


template<typename B, typename L>
void leaf_list<B,L>::iterator::move_up()
{
	delete states.top();
	states.pop();

	if (states.empty()) 
		return;

	if ((++(states.top()->child_iter)) == states.top()->children.end())
	{
		if (states.top()->leaf_iter == states.top()->leafs.end())
			move_up();
	}
	else
		initialize(*(states.top()->child_iter));
}

template<typename B, typename L>
typename leaf_list<B,L>::LValue & leaf_list<B,L>::iterator::get_current_leaf()
{
	if (!p->ignore_leaf(*states.top()->leaf_iter))
		return *states.top()->leaf_iter;

	get_next_leaf();
	return get_current_leaf();
}

template<typename B, typename L>
void leaf_list<B,L>::iterator::get_next_leaf()
{
	if (++(states.top()->leaf_iter) == states.top()->leafs.end())
		move_up();
}

//external interface
template<typename B, typename L>
typename leaf_list<B,L>::iterator& leaf_list<B,L>::iterator::operator++()	//preincrement
{
	get_next_leaf();
	return *this;
}

template<typename B, typename L>
typename leaf_list<B,L>::iterator& leaf_list<B,L>::iterator::operator=(typename leaf_list<B,L>::iterator&& rhs)
{
	states = std::move(rhs.states);
	p = rhs.p;
}

template<typename B, typename L>
bool leaf_list<B,L>::iterator::operator==(const typename leaf_list<B,L>::iterator& rhs)
{
	if (p == rhs.p)
		if (*(states.top()) == *(rhs.states.top()))
			return true;
	return false;
}

template<typename B, typename L>
bool leaf_list<B,L>::iterator::operator==(const iterator_end& rhs)
{
	return states.empty();
}

template<typename B, typename L>
typename leaf_list<B,L>::LValue& leaf_list<B,L>::iterator::operator*()
{
	return get_current_leaf();
}

template<typename B, typename L>
typename leaf_list<B,L>::LValue* leaf_list<B,L>::iterator::operator->()
{
	return &get_current_leaf();
}

template<typename B, typename L>
std::list<typename leaf_list<B,L>::BValue&> leaf_list<B,L>::iterator::parents()
{
	std::list<typename leaf_list<B,L,ALGO>::BValue&> output;
	std::stack<state*> states_copy(states);
	while (!states_copy.empty())
	{
		output.push_back(states_copy.top().node);
		states_copy.pop();
	}
	return output;
}

template<typename B, typename L>
leaf_list<B,L>::iterator::~iterator()
{
	while (!states.empty())
	{
		delete states.top();
		states.pop();
	}
}
#endif
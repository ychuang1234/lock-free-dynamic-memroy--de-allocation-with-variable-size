#include <atomic>
#include <memory>
#include <iterator>
#include <utility>
#include <assert.h>
#include <string>
#include <iostream>
typedef struct block_header{
	int sa;
	int sz;
	char* func_name;
}block_header,*pblock_header;
template<class T>
class node{
	public:
		/*header*/
		std::shared_ptr<node> next_;
		std::shared_ptr<int>start_addr;
		std::shared_ptr<int>size_;
		
		std::shared_ptr<const T> data_;
		node() {}
		node(const std::shared_ptr<node>& n, const std::shared_ptr<int>&sa, const std::shared_ptr<int>&sz,const std::shared_ptr<const T> d):next_(n){
			start_addr = sa;
			size_ = sz;
			data_ = d;
		}
		std::shared_ptr<node> next(){
			return next_;
		}
		std::shared_ptr<int> data(){
			printf("next: %p->%d,",&*next_,*size_);
			return start_addr;
		}
		std::shared_ptr<int> show_start_addr(){
			return start_addr;
		}
		std::shared_ptr<int> size(){
			return size_;
		}
	};
	
template <class T>
class slist_iterator : public node<T>, public std::iterator<std::forward_iterator_tag, const T>{
	public:
		std::shared_ptr<node<T>> node_;
		slist_iterator(){};
		slist_iterator(const std::shared_ptr <node<T>>& n) : node_(n){}


		bool operator==(const slist_iterator& other)const{
			return node_ == other.node_;
		}
		bool operator!=(const slist_iterator& other)const{
			return node_ != other.node_;
		}

		// prefix
		slist_iterator& operator++()
		{
			assert(node_);
			auto n = node_->next();
			node_ = n;
			return *this;
		}

		// postfix
		slist_iterator operator++(int)
		{
			assert(node_);
			slist_iterator tmp(node_);
			operator++(); // prefix-increment this instance
			return tmp;   // return value before increment
		}
		
		const int& operator*(){
			assert(node_);
			return *(node_->data());
		}
		
		std::shared_ptr<node<T>> node() const{
			return node_;
		}

	};

template<class T>
class slist: public slist_iterator<T>{
	std::shared_ptr <node<T>> head_;
	
public:
	std::shared_ptr <node<T>> prev_;
	slist() : head_(nullptr){}
	
	template<class... P>
	slist(int size,P&&... p){
		std::shared_ptr <int> sa_(new int(0));
		std::shared_ptr <int> sz_ (new int (size));
		auto n = std::make_shared<node<T>>(head(),sa_,sz_,std::make_shared<const T>(std::forward<P>(p)...));
		replace_head(n);
	}
	bool empty(){ return (head().get() == nullptr); }


	void push_front(const int &sz,const std::shared_ptr<const T>& h){
		std::shared_ptr <int> sa_(new int(0));
		std::shared_ptr <int> sz_ (new int (sz));
		auto n = std::make_shared<node<T>>(head(),sa_,sz_,h);
		replace_head(n);
	}
	
	template <class...P>
	int malloc(pblock_header record,P&&... p){
		int result;
		while(1){
			while(!request(result,record,p...)){
			}
			break;
		}
		return result;
	}
	template<class... P>
	int request(int &result,pblock_header record,const int &sz,P&&... p){
		slist_iterator<T> it = begin();
		std::shared_ptr<node<T>> it_node = it.node();
		prev_ = it_node;
		//First fit:
		printf("Request size: %d\n",sz);
		while(it!=end()){			
			if (*(it_node->size())>sz) break;
			prev_ = it_node;
			it_node = (++it).node();
		}
		//rear split
		if (it==end()){
			//TO DO: remerging
			result = -1;
			return result;
		}
		int new_size_ = *(it.node()->size()) - sz;
		printf("New sa: %d, New size:%d\n",*(it.node()->show_start_addr()),new_size_);
		std::shared_ptr <int> sz_ (new int(new_size_));
		std::shared_ptr <int> sa_(new int(*(it.node()->show_start_addr())));
		auto n = std::make_shared<node<T>>(nullptr,sa_,sz_,std::make_shared<const T>(std::forward<P>(p)...));
		replace_node(result,it_node,n);
		
		//copy allocation information into record
		record->sa = *((it.node()->show_start_addr()).get()) + new_size_;
		record->sz = sz;
		record->func_name = const_cast<char *>((std::forward<P>(p),...).c_str());
		return result;
	}
	
	template <class...P>
	int demalloc(pblock_header record,P&&... p){
		int result;
		while(1){
			while(!release(result,record,p...)){
			}
			break;
		}
		return result;
	}
	template<class... P>
	int release(int&result,pblock_header record,P&&... p){
		slist_iterator<T> it = begin();
		std::shared_ptr<node<T>> it_node = it.node();
		printf("\n Release: sa->%d, sz->%d \n",record->sa, record->sz);
		while (it!=end()){
			if (*(it.node()->show_start_addr())+ *(it.node()->size())<= record->sa){
				prev_ = it_node;
				it_node= it.node_;
				if (it.node()->next()!=nullptr){
					slist_iterator<T> it_next = node_it(++it);
					//printf("start addr: %d, end?: %d\n",*(it_next.node()->show_start_addr()),it==end());
					if (*(it_next.node()->show_start_addr()) > record->sa+record->sz){ //case 1: create new node, and insert after the avail block before it
						printf("\n Case 1: \n");
						std::shared_ptr <int> sz_ (new int(record->sz));
						std::shared_ptr <int> sa_(new int(record->sa));
						std::shared_ptr<node<T>> tmp = it_next.node_;
						auto n = std::make_shared<node<T>>(std::atomic_load(&it_next.node_),sa_,sz_,std::make_shared<const T>(std::forward<P>(p)...));

						std::shared_ptr <int> new_sa_ (new int(*(it_node->show_start_addr())));
						std::shared_ptr <int> new_sz_(new int(*(it_node->size())));
						auto m = std::make_shared<node<T>>(std::atomic_load(&n), new_sa_,new_sz_,std::make_shared<const T>(std::forward<P>(p)...));
						//printf("\n New node addr: %p \n",&*n);
						it_node = std::atomic_load(&it_node);
						replace_node(result,it_node,m);
						break;
					}else if(*(it_next.node()->show_start_addr()) == record->sa+record->sz){ //case 2: merge into the avail block after it
						printf("\n Case 2: \n");
						std::shared_ptr<node<T>> tmp = it_next.node()->next();
						std::shared_ptr <int> sa_ (new int(record->sa));
						std::shared_ptr <int> sz_(new int(*(it_next.node()->size())+record->sz));
						auto n = std::make_shared<node<T>>(std::atomic_load(&(it_next.node_->next_)), sa_,sz_,std::make_shared<const T>(std::forward<P>(p)...));
						/*
						std::shared_ptr <int> new_sa_ (new int(*(it_node->show_start_addr())));
						std::shared_ptr <int> new_sz_(new int(*(it_node->size())));
						auto m = std::make_shared<node<T>>(std::atomic_load(&n), new_sa_,new_sz_,std::make_shared<const T>(std::forward<P>(p)...));
						*/
						//printf("\n New node addr: %p \n",&*n);
						prev_ = it_node;
						it_node = std::atomic_load(&it_next.node_);
						replace_node(result,it_node,n);
						//printf("\n After: next: %p.\n",&*it_node->next());
						break;

					}
				}else{ //case 3: add new node into the last of avail list
					printf("\n Case 3: \n");
					std::shared_ptr <int> sz_ (new int(record->sz));
					std::shared_ptr <int> sa_(new int(record->sa));
					auto n = std::make_shared<node<T>>(nullptr,sa_,sz_,std::make_shared<const T>(std::forward<P>(p)...));
					//printf("\n New node addr: %p \n",&*n);

					std::shared_ptr <int> new_sa_ (new int(*(it_node->show_start_addr())));
					std::shared_ptr <int> new_sz_(new int(*(it_node->size())));
					auto m = std::make_shared<node<T>>(std::atomic_load(&n), new_sa_,new_sz_,std::make_shared<const T>(std::forward<P>(p)...));
					//insert new node after it
					//printf("Before: %p, %p, ",&*it_node->next(),&*m->next());
					it_node = std::atomic_load(&it.node_);
					replace_node(result,it_node,m);
					break;
					//printf("After: %p,%d\n",&*it_node->next(),*it_node->next()->show_start_addr());

				}
				
			}else{
				it = node_it(++it);
			}
		}
		return result;
	}
	std::shared_ptr<node<T>> head(){
		return std::atomic_load(&head_);
	}
	std::shared_ptr<node<T>> node_atomic_load(const slist_iterator<T>& it) {
		std::shared_ptr <node<T>> tmp = it.node();
		return std::atomic_load(&tmp);
	}
	void replace_head(const std::shared_ptr < node<T> >& h){
		//std::atomic_store(&head_,h);
		std::atomic_compare_exchange_strong(&head_,&head_,h);
	}
	void replace_node(int &result,std::shared_ptr<node<T>> &obj,const std::shared_ptr <node<T>> &expected){
		bool h,n;
		result = 0;
		if (obj==head_) {
			h = std::atomic_compare_exchange_strong(&head_,&obj,expected);
			printf("Replace head : %d\n",h);
			if (h) result = 1;

		}else{
			n = std::atomic_compare_exchange_strong(&prev_->next_,&obj,expected);
			printf("Replace node : %d\n",n);
			if (n) result = 1;
		}

	}
	slist_iterator<T> begin(){
		return slist_iterator<T>{head()};
	}
	slist_iterator<T> node_it(const slist_iterator<T>& it){
		return slist_iterator<T>{node_atomic_load(it)};
	}
	slist_iterator<T> end(){
		return slist_iterator<T>{};
	}
	slist_iterator<T> cbegin()const{
		return slist_iterator<T>{head()};
	}

	slist_iterator<T> cend()const{
		return slist_iterator<T>{};
	}

	// TO DO
private:
	slist(const slist&);
	slist& operator=(const slist&) ;
	slist(const slist&&);
	slist& operator=(const slist&&) ;
};

int main(){
	int size = 1<<10;
	slist<std::string> s(size,std::string("Start"));
	int sz [] = {4,8,32,64,128,256};
	std::string name [] = {"A","B","C","D","E","F"};
	block_header record[6];
	int i,j;
	#pragma omp parallel for
	for(i=0;i<6;i++){
		j = s.malloc(&record[i],sz[i],name[i]);
		if (j<1){
			printf("\n Malloc #%i Failure!\n",i);
		}else{
			printf("\n Malloc #%d Success!\n",i);
		}
	}
	for (auto& pstr : s){
		std::cout << pstr<< " ";
	}
	std::cout<<std::endl;
	//printf("\n ABC\n");
	#pragma omp parallel for
	for(i=0;i<6;i++){
		j = s.demalloc(&record[i],name[i]);
		if (j<1){
			printf("\n DeMalloc #%d Failure!\n",i);
		}else{
			printf("\n DeMalloc #%d Success!\n",i);
		}
	}
	/*
	std::cout<<std::endl;
	bool result;
	j = s.demalloc(&record[0],"E");
	if (j<1){
		printf("DeMalloc Failure!\n");
	}else{
		printf("DeMalloc Success!\n");
	}
	for (auto& pstr : s){
		std::cout << pstr<< " ";
	}
	std::cout<<std::endl;
	j = s.demalloc(&record[2],"F");
	if (j<1){
		printf("DeMalloc Failure!\n");
	}else{
		printf("DeMalloc Success!\n");
	}
	for (auto& pstr : s){
		std::cout << pstr<< " ";
	}
	std::cout<<std::endl;

	j = s.demalloc(&record[1],"G");
	if (j<1){
		printf("DeMalloc Failure!\n");
	}else{
		printf("DeMalloc Success!\n");
	}
	for (auto& pstr : s){
		std::cout << pstr<< " ";
	}
	std::cout<<std::endl;
	
	j = s.demalloc(&record[3],"H");
	if (j<1){
		printf("DeMalloc Failure!\n");
	}else{
		printf("DeMalloc Success!\n");
	}
	*/
	for (auto& pstr : s){
		std::cout << pstr<< " ";
	}
	std::cout<<std::endl;
}

#pragma once




#ifndef HEXO_RESOURCE_LIST_H
#define HEXO_RESOURCE_LIST_H


#include <memory>


namespace Hexo
{


	template<typename R>
	struct ResourceNode {
		R Data;
		ResourceNode<R>* PreviousResource = nullptr;
		ResourceNode<R>* NextResource = nullptr;

		~ResourceNode(){
			if (this->PreviousResource){ this->PreviousResource->NextResource = this->NextResource; }
			if (this->NextResource){ this->NextResource->PreviousResource = this->PreviousResource; }
		}
	};

	template<typename R>
	struct MinimalResourceNode {
		R Data;
	};



	template<typename R>
	struct ResourceIterator {
		ResourceNode<R>* Data = nullptr;

		ResourceIterator<R>& operator++(){
			if (Data){ Data = Data->NextResource; }
			return *this;
		}
		ResourceIterator<R>& operator--(){
			if (Data){ Data = Data->LastResource; }
			return *this;
		}
		ResourceIterator<R> operator++(int){
			ResourceIterator<R> t = *this;
			++*this; return t;
		}
		ResourceIterator<R> operator--(int){
			ResourceIterator<R> t = *this;
			--*this; return t;
		}

		bool operator!=(const ResourceIterator<R>& t){ return (this->Data != t.Data); }
		bool operator==(const ResourceIterator<R>& t){ return (this->Data == t.Data); }

		R* operator*() const {
			if (Data){ return &Data->Data; }
			return nullptr;
		}

	};



	template<typename R>
	struct ResourceList
	{
		ResourceList(){}
		~ResourceList(){ Release(); }

		ResourceNode<R>* FirstNode = nullptr;
		ResourceNode<R>* LastNode = nullptr;
		HXSIZE length = 0;

		const ResourceIterator<R> begin() const { return ResourceIterator<R>{ FirstNode }; }
		const ResourceIterator<R> end() const { return ResourceIterator<R>{ nullptr }; }

		const HXSIZE Size() const { return length; }


		ResourceNode<R>* Insert(R&& Data){
			return Insert(Data);
		}

		ResourceNode<R>* Insert(R& Data){
			ResourceNode<R>* n = new ResourceNode<R>{std::move(Data)};
			n->PreviousResource = LastNode;
			if (LastNode){ LastNode->NextResource = n; }
			LastNode = n;

			if (!FirstNode){ FirstNode = LastNode; }

			length += 1;
			return n;
		}

		void Remove(ResourceNode<R>*& pointer){
			if (!pointer)return;

			if (pointer->PreviousResource){ pointer->PreviousResource->NextResource = pointer->NextResource; }
			if (pointer->NextResource){ pointer->NextResource->PreviousResource = pointer->PreviousResource; }

			if (pointer == FirstNode){ FirstNode = pointer->NextResource; }
			if (pointer == LastNode){ LastNode = pointer->PreviousResource; }

			pointer->PreviousResource = nullptr;
			pointer->NextResource = nullptr;

			delete pointer;
			pointer = nullptr;
			if (length > 0){ length -= 1; }
		}

		void Release(){
			ResourceNode<R>* p = FirstNode;
			while (p != nullptr){
				ResourceNode<R>* n = p->NextResource;
				delete p;
				p = n;
			}

			this->FirstNode = nullptr;
			this->LastNode = nullptr;
			this->length = 0;
		}

	};



}

#endif /* end of include guard: HEXO_RESOURCE_LIST_H */

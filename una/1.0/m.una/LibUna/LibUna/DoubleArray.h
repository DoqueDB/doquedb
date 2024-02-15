//
// DoubleArray.h - Double Array implementation in C++
//    based on "An Efficient Implementation of Trie Structures"
//    by JUN-ICHI AOE, KATSUSHI MORIMOTO, TAKASHI SATO
// 
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#ifndef MODNLP_DOUBLEARRAY_H
#define MODNLP_DOUBLEARRAY_H

#include "UnaDLL.h"
#include "vector"

typedef unsigned char UNA_CHAR;

class BCElement
{
public:
	int base;
	int check;
	BCElement()
	{
		base = check = 0;
	}
	BCElement(int base_,int check_ = 0)
	{
		base = base_,check = check_;
	}
};

class BC:public std::vector <BCElement>
{
	void check_size(int i_)
	{
		if(this->size() <= (unsigned)i_)
		{
			this->resize(i_ + 1,0);
		}
	}
public:
	virtual	int base(int i)
	{
		return i >= (int)this->size() ?0:(*this)[i].base;
	}
	virtual void base(int i_,int d_)
	{
		check_size(i_);
		(*this)[i_].base = d_;
	}
	
	int check(int i)
	{
		return i >= (int)this->size() ?0:(*this)[i].check;
	}
	void set(int i_,BCElement e_)
	{
		check_size(i_);
		(*this)[i_].base  = e_.base;
		(*this)[i_].check = e_.check;
	}
	void check(int i_,int d_)
	{
		check_size(i_);
		(*this)[i_].check = d_;
	}
};

class TAIL:public std::vector <UNA_CHAR>
{
public:
	UNA_CHAR * get(int i_)
	{
		return (UNA_CHAR*)&begin()[i_];
	}
	void put(UNA_CHAR * s_,int p)
	{
		int tail_index = p;
		int i = 0;
		while(*(s_ + i)) ++i;
		++i;		// for a terminator 0
		if( p + i  >= (int)this->size())
			this->resize( this->size() + i,0 );
		while(*s_)
			(*this)[tail_index++] = *s_++;
		(*this)[tail_index] = 0;
	}
	void set(int i_,UNA_CHAR c_)
	{
		if(this->size() <= (unsigned)i_)
		{
			this->resize(i_ + 1,0);
		}
		(*this)[i_] = c_;
	}
};

const static int max_code = 255;

template <class T>
class UserData
{
	T t;
	std::vector<T> data;
public:
	UserData()
	{
		data.resize(1);
	}
	void set(int i, T data_)
	{
		if(data.size() <= (unsigned)i)
		{
			data.resize(i + 1);
		}
		data[i] = data_;
	}
	int elementSize()
	{
		return sizeof(T);
	}
	void resize(int n)
	{
		data.resize(n);
	}
	T * begin()
	{
		return (T*)&(*data.begin());
	}
	T & get(int i)
	{
		if((unsigned int)i < data.size())
			return data[i];
		else
		{
			return t;
		}
	}
	int size(){ return (int)data.size();}
};
// double array class
template <class T>
class  DoubleArray
{
public:
	typedef unsigned char UNA_CHAR;
protected:
	//// data section ///
	// work area
	std::vector <UNA_CHAR> list;
	BC		bc;
	TAIL	tail;
	// user data
	UserData <T> data;
public:
	int size(){ return data.size();}
	//// procedure section ///
	bool prefix(UNA_CHAR * key,int &i,int &j,UNA_CHAR * &temp)
	{
		int t;
		for( i = 0, t = 1 ; 1 ;i++ )
		{
			j = bc.base(t) + key[i];
			if(bc.check(j) != t)
			{
				j = t;
				return false;
			}
			if(bc.base(j) < 0 )
				break;
			t = j;
		}
		if(key[i] != '\t')
		{
			temp = tail.get((-1)*bc.base(j));
		}
		else
			temp = 0;	// found
		return true;	
	}
	int change(int current_,int i_,std::vector<UNA_CHAR>& list_,UNA_CHAR c_)
	{
		int k,old_node,new_node,old_base;
		std::vector<UNA_CHAR> a_list;

		a_list.resize(max_code  ,0);

		old_base = bc.base(i_);

		std::copy(list_.begin(),list_.end(),a_list.begin());
		std::vector <UNA_CHAR>::iterator iter;
		for( iter = a_list.begin();	iter != a_list.end(); iter++)
		{
			if(*iter == 0)
				break;
		}
		int i = (int)(iter - a_list.begin());

		a_list[i] = c_;
		a_list[i+1] = 0;

		bc.base(i_,cross_check(a_list));
		i = 0;
		iter = list_.begin();
		do
		{
			old_node = old_base + *iter;
			new_node = bc.base(i_) + *iter;
			bc.set(new_node,BCElement(bc.base(old_node),i_));
			data.set(new_node,data.get(old_node));

			int j;
			if(j = bc.base(old_node) > 0)
			{
				k = j + 1;
				while( k - j < max_code || (unsigned)k < bc.size())
				{
					if(bc.check(k) == old_node)
						bc.check(k,new_node);
					++k;
				}
			}
			if(current_ != i_ && old_node == current_)
				current_ = new_node;
			bc.set(old_node,BCElement());
			iter++;
		}while(*iter);
		return current_;
	}
	
	int cross_check(std::vector<UNA_CHAR>& list_)
	{
		int i = 0,base_pos = 1,check_pos;
		do
		{
			check_pos = base_pos + list_[i++];
			if(bc.check(check_pos))
			{
				base_pos++;
				i = 0;
				continue;
			}
		}while(list_[i]);
		return base_pos;
	}
	void set_list(int s_)
	{
		int i, j = 0, t;
		for( i = 1; i < max_code - 1 ; i++)
		{
			t = bc.base(s_) + i;
			if( bc.check(t) == s_)
				list[j++] = (UNA_CHAR) i;
		}
		list[j] = 0;
	}
	int strlen(UNA_CHAR * s)
	{
		UNA_CHAR * t = s;
		while(*s++);
		return (int)(s - t);
	}
	int strcmp(UNA_CHAR * s,UNA_CHAR * d)
	{
		for( ; *s && *d ; s++,d++)
		{
			if(*s == *d)
				continue;
			break;
		}
		return *s - *d;
	}
	void insert(int i_,UNA_CHAR * s_,T &data_)
	{
		std::vector <UNA_CHAR> list_s,list_t;
		list_s.resize(max_code , 0);
		list_t.resize(max_code , 0);

		int t = bc.base(i_) + *s_;
		if(bc.check(t))
		{
			set_list(i_);
			std::copy(list.begin(),list.end(),list_s.begin());
			set_list(bc.check(t));
			std::copy(list.begin(),list.end(),list_t.begin());
			if(strlen((UNA_CHAR*)&list_s.begin()[0])+ 1 < strlen((UNA_CHAR*)&list_t.begin()[0]) )
				i_ = change(i_,i_,list_s,*s_);
			else
				i_ = change(i_,bc.check(t),list_t,(UNA_CHAR)0);
		}
		separate(i_,s_,(int)tail.size(),data_);
	}
	void separate(int i_,UNA_CHAR * s_,int tail_pos,T &data_)
	{
		int t = bc.base(i_) + *s_;
		s_++;
		bc.set(t,BCElement((-1)*(int)tail_pos,i_));
		tail.put(s_,tail_pos);
		data.set(t,data_);
	}
	void tail_insert(UNA_CHAR * a,UNA_CHAR * b,int s,T &data_)
	{
		std::vector <UNA_CHAR> list;
		list.resize(3,0);
		UNA_CHAR ch;
		int i = 0,length = 0 , t;
		int old_tail_pos = (-1)*bc.base(s);
		T old_data = data.get(s);
		while(a[length] == b[length])
			length++;
		while( i < length )
		{
			ch = a[i++];
			list[0] = ch ;
			list[1] = 0;
			bc.base(s,cross_check(list));
			t = bc.base(s) + ch;
			bc.check(t,s);
			s = t;
		}
		list[0] = a[length];
		list[1] = b[length];
		list[2] = 0;
		bc.base(s,cross_check(list));
		separate(s,a + length,old_tail_pos,old_data);
		separate(s,b + length,(int)tail.size(),data_);	
	}
public:
	DoubleArray()
	{
		bc.push_back(BCElement());
		bc.push_back(BCElement(1));
		tail.set(0,'\t');
		list.resize(max_code , 0);
	}

	///////////////////////////////////////////////
	///// interface API of DoubleArray
	/////	
	virtual bool insert(UNA_CHAR * key,T &data_)
	{
		int i,j;
		UNA_CHAR * temp;

		if(prefix(key,i,j,temp) == false)
		{
			insert(j,key + i,data_);
			return true;
		}

		if(temp == 0 || 
			!DoubleArray::strcmp(temp,key + i + 1))
		{
			return false;	// already exist
		}
		else
		{
			if(bc.base(j))
			{
				tail_insert(temp,key + i + 1,j,data_);
			}
		}
		return true;		
	}

	bool search(UNA_CHAR * key,T &data_)
	{
		int i,j;
		UNA_CHAR * temp;

		if(prefix(key,i,j,temp) == false)
		{
			return false;
		}
		if(key[i] == '\t' || 
			temp == 0 || 
			!DoubleArray::strcmp(temp,key + i + 1))
		{
			data_ = data.get(j);
			return true;
		}
		return false;
	}

};

#endif // MODNLP_DOUBLEARRAY_H

//
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights resized.
//

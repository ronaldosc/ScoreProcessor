/*
Copyright(C) 2017 Edward Xie

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef EXSTRING_H
#define EXSTRING_H
#include <iostream>
#include <stdexcept>
#include <new>
#include <assert.h>
#include <string>
#include <algorithm>
#include <limits>
#include <typeindex>
//#include "exmeta.h"
namespace exlib {

	template<typename T,typename U>
	constexpr int strcmp(T const* a,U const* b)
	{
		for(size_t i=0;;++i)
		{
			if(a[i]<b[i])
			{
				return -1;
			}
			if(a[i]>b[i])
			{
				return 1;
			}
			if(a[i]==0)
			{
				return 0;
			}
		}
	}

	template<typename Iter>
	constexpr size_t strlen(Iter p)
	{
		size_t i=0;
		while(*p!=0)
		{
			++i;
			++p;
		}
		return i;
	}

	template<typename T=char,typename CharT=std::char_traits<T>>
	class string_alg {
	public:
		typedef typename T value_type;
		typedef typename T* pointer;
		typedef typename T const* const_pointer;
		typedef typename T& reference;
		typedef typename T const& const_reference;
		typedef typename std::size_t size_type;
		typedef typename std::ptrdiff_t difference_type;

		typedef pointer iterator;
		typedef const_pointer const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	protected:
		pointer _data;
		size_type _size;
		string_alg(pointer data,size_type size):_data(data),_size(size) {}
		string_alg() {}
	public:
		static size_type const npos=-1;

		template<typename A,typename B>
		friend std::basic_ostream<A>& operator<<(std::basic_ostream<A>&,string_alg<A,B> const&);

		iterator begin() { return _data; }
		const_iterator cbegin() const { return _data; }

		iterator end() { return _data+_size; }
		const_iterator cend() const { return _data+_size; }
		reverse_iterator rbegin() { return reverse_iterator(end()); }
		const_reverse_iterator crbegin() const { return rbegin(); }
		reverse_iterator rend() { return reverse_iterator(begin()); }
		const_reverse_iterator crend() const { return rend(); }
		size_type size() const { return _size; };
		bool empty() const { return _size==0; }

		bool operator==(string_alg const&) const;
		bool operator!=(string_alg const&) const;
		bool operator>(string_alg const&) const;
		bool operator<(string_alg const&) const;
		bool operator>=(string_alg const&) const;
		bool operator<=(string_alg const&) const;

		size_type find(string_alg const& target,size_type pos=0) const;
		size_type find(const_pointer target,size_type pos,size_type count) const;
		size_type find(const_pointer target,size_type pos=0) const;
		size_type find(T ch,size_type pos=0) const;

		size_type rfind(T ch,size_type pos=npos) const;


		reference operator[](size_type i) { return _data[i]; }
		const_reference operator[](size_type i) const { return _data[i]; }
		reference at(size_type i)
		{
			if(i>=size()||i<0)
			{
				throw std::out_of_range();
			}
			return _data[i];
		}
		const_reference at(size_type i) const
		{
			if(i>=size()||i<0)
			{
				throw std::out_of_range();
			}
			return _data[i];
		}
		reference front() { return _data[0]; }
		const_reference front() const { return _data[0]; }
		reference back()
		{
			return _data[size()-1];
		}
		const_reference back() const
		{
			return _data[size()-1];
		}
		pointer data()
		{
			return _data;
		}
		const_pointer data() const
		{
			return _data;
		}
		pointer c_str()
		{
			return _data;
		}
		const_pointer c_str() const
		{
			return _data;
		}
	};

	template<typename T,typename CharT=std::char_traits<T>,typename Alloc=std::allocator<T>>
	class string_base:public string_alg<T,CharT>,protected Alloc {
	public:
		typedef typename string_alg<T,CharT>::value_type value_type;
		typedef typename string_alg<T,CharT>::pointer pointer;
		typedef typename string_alg<T,CharT>::const_pointer const_pointer;
		typedef typename string_alg<T,CharT>::reference reference;
		typedef typename string_alg<T,CharT>::const_reference const_reference;
		typedef typename string_alg<T,CharT>::size_type size_type;
		typedef typename string_alg<T,CharT>::difference_type difference_type;

		typedef typename string_alg<T,CharT>::iterator iterator;
		typedef typename string_alg<T,CharT>::const_iterator const_iterator;
		typedef typename string_alg<T,CharT>::reverse_iterator reverse_iterator;
		typedef typename string_alg<T,CharT>::const_reverse_iterator const_reverse_iterator;
	private:
		size_type _capacity;
		void move(string_base&&) noexcept;
		void reallocate(size_type);
	public:
		string_base();
		template<typename U>
		string_base(U const*);
		string_base(size_type size,T);
		template<typename U>
		string_base(U const*,size_type s);
		string_base(size_type capacity);
		string_base(string_base&&) noexcept;

		template<typename String,typename>
		string_base(String const&);
		string_base(string_base const& other);

		template<typename String>
		string_base& operator=(String const&);
		string_base& operator=(string_base const& other);
		template<typename U>
		string_base& operator=(U const*);
		string_base& operator=(string_base&&) noexcept;

		~string_base();
		size_type capacity() const { return _capacity; }
		void shrink_to_fit();
		void resize(size_type);
		void resize(size_type,T);
		void reserve(size_type);
		void release();

		void push_back(T);
		void pop_back();

		void erase(iterator pos);
		void erase(size_type pos);
		void erase(iterator begin,iterator end);
		void erase(size_type begin,size_type end);

		void insert(size_type pos,T c,size_type count=1);

		string_base operator+(string_base const&) const;
		template<typename String>
		string_base& operator+=(String const&);

		string_base substr(size_type begin,size_type end) const;
		string_base substr(iterator begin,iterator end) const;
		string_base& append(size_type count,T ch);

		void clear()
		{
			this->_size=0;
			this->_data[0]=0;
		}
	};
	typedef string_base<char> string;
	typedef string_base<wchar_t> wstring;

	/*
	Holds a ptr to a c string with its size.
	Has no control of the c string.
	Unsafe if underlying string is moved.
	*/
	template<typename T,typename CharT=std::char_traits<T>>
	class weak_string_base:public string_alg<T,CharT> {
	public:
		typedef typename string_alg<T,CharT>::value_type value_type;
		typedef typename string_alg<T,CharT>::pointer pointer;
		typedef typename string_alg<T,CharT>::const_pointer const_pointer;
		typedef typename string_alg<T,CharT>::reference reference;
		typedef typename string_alg<T,CharT>::const_reference const_reference;
		typedef typename string_alg<T,CharT>::size_type size_type;
		typedef typename string_alg<T,CharT>::difference_type difference_type;

		typedef typename string_alg<T,CharT>::iterator iterator;
		typedef typename string_alg<T,CharT>::const_iterator const_iterator;
		typedef typename string_alg<T,CharT>::reverse_iterator reverse_iterator;
		typedef typename string_alg<T,CharT>::const_reverse_iterator const_reverse_iterator;
		weak_string_base(T* data):string_alg(data,exlib::strlen(data)) {}
		weak_string_base(T* data,size_t size):string_alg(data,size) {}
	};
	typedef weak_string_base<char> weak_string;
	typedef weak_string_base<wchar_t> weak_wstring;

	template<typename T,typename CharT=std::char_traits<T>>
	class string_manager_base:public string_alg<T,CharT> {
	public:
		typedef typename string_alg<T,CharT>::value_type value_type;
		typedef typename string_alg<T,CharT>::pointer pointer;
		typedef typename string_alg<T,CharT>::const_pointer const_pointer;
		typedef typename string_alg<T,CharT>::reference reference;
		typedef typename string_alg<T,CharT>::const_reference const_reference;
		typedef typename string_alg<T,CharT>::size_type size_type;
		typedef typename string_alg<T,CharT>::difference_type difference_type;

		typedef typename string_alg<T,CharT>::iterator iterator;
		typedef typename string_alg<T,CharT>::const_iterator const_iterator;
		typedef typename string_alg<T,CharT>::reverse_iterator reverse_iterator;
		typedef typename string_alg<T,CharT>::const_reverse_iterator const_reverse_iterator;
		string_manager_base(T* data):string_alg(data,exlib::strlen(data)) {}
		string_manager_base(T* data,size_t size):string_alg(data,size) {}
		~string_manager_base() { delete[] this->_data; }
	};
	typedef string_manager_base<char> string_manager;
	typedef string_manager_base<char> wstring_manager;

	template<typename T,typename CharT,typename Alloc>
	template<typename U>
	string_base<T,CharT,Alloc>::string_base(U const* cp): string_base(cp,exlib::strlen(cp)) {}

	template<typename T,typename CharT,typename Alloc>
	template<typename U>
	string_base<T,CharT,Alloc>::string_base(
		U const* cp,
		typename string_base<T,CharT,Alloc>::size_type s)
	{
		assert(cp!=nullptr);
		this->_size=s;
		this->_data=this->allocate(s+1);
		for(_capacity=0;_capacity<this->_size;++_capacity)
		{
			this->_data[_capacity]=cp[_capacity];
		}
		this->_data[_capacity]=0;
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base(typename string_base<T,CharT,Alloc>::size_type s,T c)
	{
		this->_size=s;
		this->_data=this->allocate(s+1);
		for(this->_capacity=0;this->_capacity<this->_size;++this->_capacity)
		{
			this->_data[this->_capacity]=c;
		}
		this->_data[this->_capacity]=0;
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base(typename string_base<T,CharT,Alloc>::size_type capacity):_capacity(capacity)
	{
		this->_data=allocate(this->_capacity+1);
		this->_data[0]=0;
		this->_size=0;
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base():string_alg<T,CharT>(nullptr,0),_capacity(0) {}

	template<typename T,typename CharT,typename Alloc>
	template<typename String,typename=std::enable_if<!std::is_integral<String>::value>::type>
	string_base<T,CharT,Alloc>::string_base(String const& other):string_base(other.data(),other.size()) {}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base(string_base<T,CharT,Alloc> const& other):string_base(other.data(),other.size()) {}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::reallocate(typename string_base<T,CharT,Alloc>::size_type s)
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		typedef typename string_base<T,CharT,Alloc>::pointer pointer;
		pointer np=this->allocate(s+1);
		st i;
		for(i=0;i<this->_size;++i)
		{
			np[i]=this->_data[i];
		}
		np[i]=0;
		this->deallocate(this->_data,this->_capacity+1);
		this->_data=np;
		this->_capacity=s;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::move(string_base<T,CharT,Alloc>&& other) noexcept
	{
		this->_data=other.data();
		this->_size=other.size();
		this->_capacity=other.capacity();
		other.release();
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::string_base(string_base<T,CharT,Alloc>&& other) noexcept
	{
		move(std::move(other));
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>::~string_base()
	{
		this->deallocate(this->_data,this->_capacity+1);
	}

	template<typename U,typename CharU>
	std::basic_ostream<U>& operator<<(std::basic_ostream<U>& os,string_alg<U,CharU> const& str)
	{
		if(str._data==nullptr)
		{
			return os;
		}
		return os<<str._data;
	}

	template<typename T,typename CharT,typename Alloc>
	template<typename String>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator=(String const& other)
	{
		this->reserve(other.size());
		typename string_base<T,CharT,Alloc>::size_type s;
		for(s=0;s<other.size();++s)
		{
			this->_data[s]=other[s];
		}
		this->_data[s]=0;
		this->_size=other.size();
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator=(string_base<T,CharT,Alloc> const& other)
	{
		this->reserve(other.size());
		typename string_base<T,CharT,Alloc>::size_type s;
		for(s=0;s<other.size();++s)
		{
			this->_data[s]=other[s];
		}
		this->_data[s]=0;
		this->_size=other.size();
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	template<typename U>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator=(U const* cp)
	{
		this->_size=exlib::strlen(cp);
		this->reserve(this->_size);
		typename string_base<T,CharT,Alloc>::size_type s;
		for(s=0;s<this->_size;++s)
		{
			this->_data[s]=cp[s];
		}
		this->_data[s]=0;
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator=(string_base<T,CharT,Alloc>&& other) noexcept
	{
		assert(this!=&other);
		this->deallocate(this->_data,this->_capacity+1);
		this->move(std::move(other));
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::shrink_to_fit()
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		if(this->_capacity==this->_size)
		{
			return;
		}
		T* np=this->allocate(this->_size+1);
		st i;
		for(i=0;i<this->_size;++i)
		{
			np[i]=this->_data[i];
		}
		np[i]=0;
		this->deallocte(this->_data,this->_capacity+1);
		this->_capacity=this->_size;
		this->_data=np;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::resize(typename string_base<T,CharT,Alloc>::size_type s)
	{
		this->reserve(s);
		this->_size=s;
		this->_data[s]=0;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::resize(typename string_base<T,CharT,Alloc>::size_type s,T ch)
	{
		this->reserve(s);
		for(;this->_size<s;++this->_size)
		{
			this->_data[this->_size]=ch;
		}
		this->_data[s]=0;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::reserve(typename string_base<T,CharT,Alloc>::size_type s)
	{
		if(this->_capacity>s&&this->_data!=nullptr)
		{
			return;
		}
		this->reallocate(s);
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::push_back(T cr)
	{
		if(this->_size>=this->_capacity)
		{
			this->reallocate(2*this->_size+1);
		}
		this->_data[this->_size]=cr;
		++this->_size;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::pop_back()
	{
		this->_data[--this->_size]=0;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::erase(typename string_base<T,CharT,Alloc>::size_type pos)
	{
		--this->_size;
		while(pos<this->_size)
		{
			this->_data[pos]=this->_data[pos+1];
			++pos;
		}
		this->_data[pos]=0;
	}
	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::append(typename string_base<T,CharT,Alloc>::size_type count,T ch)
	{
		this->resize(this->_size+count,ch);
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::erase(typename string_base<T,CharT,Alloc>::iterator pos)
	{
		erase(static_cast<typename string_base<T,CharT,Alloc>::size_type>(pos-this->begin()));
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::erase(
		typename string_base<T,CharT,Alloc>::iterator begin,
		typename string_base<T,CharT,Alloc>::iterator end)
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		this->erase(static_cast<st>(begin-this->_data),
			static_cast<st>(end-this->_data));
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::erase(
		typename string_base<T,CharT,Alloc>::size_type begin,
		typename string_base<T,CharT,Alloc>::size_type end)
	{
		typename string_base<T,CharT,Alloc>::size_type i,j;
		for(i=end,j=begin;i<this->_size;++i,++j)
		{
			this->_data[j]=this->_data[i];
		}
		this->_data[j]=0;
		this->_size=j;
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc> string_base<T,CharT,Alloc>::operator+(string_base<T,CharT,Alloc> const& other) const
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		string_base<T,CharT,Alloc> ret;
		ret.reserve(this->size()+other.size());
		for(;ret._size<this->size();++ret._size)
		{
			ret[ret._size]=(*this)[ret._size];
		}
		st j=0;
		for(;j<other.size();++j,++ret._size)
		{
			ret[ret._size]=other[j];
		}
		ret[ret._size]=0;
		return ret;
	}

	template<typename T,typename CharT,typename Alloc>
	template<typename String>
	string_base<T,CharT,Alloc>& string_base<T,CharT,Alloc>::operator+=(String const& other)
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		reserve(this->size()+other.size());
		st limit=other.size();
		for(st i=0;i<limit;++i,++this->_size)
		{
			this->_data[this->_size]=other[i];
		}
		this->_data[this->_size]=0;
		return *this;
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc> string_base<T,CharT,Alloc>::substr(
		typename string_base<T,CharT,Alloc>::size_type begin,
		typename string_base<T,CharT,Alloc>::size_type end) const
	{
		string_base<T,CharT,Alloc> ret;
		ret.reserve(end-begin);
		for(;begin<end;++begin,++ret._size)
		{
			ret[ret._size]=(*this)[begin];
		}
		ret[ret._size]=0;
		return ret;
	}

	template<typename T,typename CharT,typename Alloc>
	string_base<T,CharT,Alloc> string_base<T,CharT,Alloc>::substr(
		typename string_base<T,CharT,Alloc>::iterator begin,
		typename string_base<T,CharT,Alloc>::iterator end) const
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		return substr(static_cast<st>(begin-this->_data),static_cast<st>(end-this->_data));
	}

	template<typename T,typename CharT>
	bool string_alg<T,CharT>::operator==(string_alg<T,CharT> const& other) const
	{
		if(other.size()==size())
		{
			for(size_t i=0;i<size();++i)
			{
				if((*this)[i]!=other[i])
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::insert(
		typename string_base<T,CharT,Alloc>::size_type pos,
		T c,
		typename string_base<T,CharT,Alloc>::size_type count)
	{
		typedef typename string_base<T,CharT,Alloc>::size_type st;
		st new_size=count+this->_size;
		this->reserve(new_size);
		this->_data[new_size]=0;
		st i,j;
		for(i=new_size-1,j=this->_size-1;;--i,--j)
		{
			this->_data[i]=this->_data[j];
			if(j==pos)
			{
				break;
			}
		}
		i=pos+count;
		for(;j<i;++j)
		{
			this->_data[j]=c;
		}
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::find(
		string_alg<T,CharT> const& target,
		typename string_alg<T,CharT>::size_type pos) const
	{
		return find(target.data(),pos,target.size());
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::find(
		typename string_alg<T,CharT>::const_pointer target,
		typename string_alg<T,CharT>::size_type pos,
		typename string_alg<T,CharT>::size_type count) const
	{
		typedef typename string_alg<T,CharT>::size_type st;
		if(count==0||pos>size())
		{
			return npos;
		}
		st space_size=size()-pos;
		if(count>space_size)
		{
			return npos;
		}
		st limit=size()-count;
		for(st i=pos;i<=limit;++i)
		{
			if((*this)[i]==target[0])
			{
				for(st tpos=1;tpos<count;++tpos)
				{
					if((*this)[i+tpos]!=target[tpos])
					{
						i=i+tpos+1;
						break;
					}
				}
				return i;
			}
		}
		return npos;
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::find(
		typename string_alg<T,CharT>::const_pointer target,
		typename string_alg<T,CharT>::size_type pos) const
	{
		return find(target,pos,exlib::strlen(target));
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::find(
		T ch,
		typename string_alg<T,CharT>::size_type pos) const
	{
		for(typename string_base<T>::size_type i=pos;i<size();++i)
		{
			if((*this)[i]==ch)
			{
				return i;
			}
		}
		return npos;
	}

	template<typename T,typename CharT,typename Alloc>
	void string_base<T,CharT,Alloc>::release()
	{
		this->_data=nullptr;
		this->_size=0;
		this->_capacity=0;
	}

	template<typename T,typename CharT>
	typename string_alg<T,CharT>::size_type string_alg<T,CharT>::rfind(
		T ch,
		typename string_alg<T,CharT>::size_type pos) const
	{
		if(pos>_size)
		{
			pos=_size;
		}
		pos=pos-1;
		do
		{
			if(_data[pos]==ch)
			{
				return pos;
			}
			--pos;
		} while(pos>0);
		return npos;
	}

	/*
	Creates ordinal lettering such that:
	0	-> "a"
	1	-> "b"
	...
	25	-> "z"
	26	-> "aa"
	27	-> "ab"
	...
	51	-> "az"
	52	-> "ba"
	...
	701	-> "zz"
	702	-> "aaa"
	703	-> "aab"
	...
	String must have constructor that takes in (char* string,size_t count).
	Default buffer_size used to create the string is 15 (enough to fit 64 bit unsigned), increase if you need more.
	*/
	template<
		typename String=std::string,
		char alphabet_start='a',
		unsigned int alphabet_size=26,
		size_t buffer_size=15,
		typename N,
		typename=std::enable_if<std::is_integral<N>::value&&std::is_unsigned<N>::value>::type
	>
		String ordinal_lettering(N n)
	{
		char buffer[buffer_size];
		size_t pos=buffer_size-1;
		while(true)
		{
			buffer[pos]=alphabet_start+n%alphabet_size;
			n/=alphabet_size;
			if(n==0)
			{
				break;
			}
			n-=1;
			--pos;
		}
		return String(buffer+pos,buffer_size-pos);
	}
	template<typename String=std::string,
		char alphabet_start='a',
		unsigned int alphabet_size=26>
		String ordinal_lettering(int n)
	{
		return ordinal_lettering<String,alphabet_start,alphabet_size,10>(static_cast<unsigned int>(n));
	}

	template<typename String,typename StringType>
	String front_padded_string(String const& in,size_t numpadding,StringType padding)
	{
		if(in.size()>=numpadding) return in;
		size_t padding_needed=numpadding-in.size();
		String out(padding_needed,padding);
		out+=in;
		return out;
	}

	template<typename String,typename StringType>
	String back_padded_string(String const& in,size_t numpadding,StringType padding)
	{
		if(in.size()>=numpadding) return in;
		size_t padding_needed=numpadding-in.size();
		String out(in);
		out.append(padding_needed,padding);
		return out;
	}

	template<typename StringOut,typename StringIn>
	StringOut string_cast(StringIn const& in)
	{
		StringOut out;
		out.resize(in.size());
		for(size_t i=0;i<in.size();++i)
		{
			out[i]=in[i];
		}
		return out;
	}

	template<typename T>
	constexpr T lowercase(T a)
	{
		constexpr auto diff='a'-'A';
		if(a>='A'&&a<='Z')
		{
			return a+diff;
		}
		return a;
	}

	template<typename iter,typename iter2>
	constexpr int strncmp_nocase(iter a,iter2 b)
	{
		for(;;++a,++b)
		{
			if(*b==0)
			{
				return *a;
			}
			else if(*a==0)
			{
				return -*b;
			}
			else if(*b!=*a)
			{
				if(int res=lowercase(*a)-lowercase(*b))
				{
					return res;
				}
			}
		}
		return 0;
	}


	template<typename T>
	constexpr bool is_digit(T ch)
	{
		return ch>='0'&&ch<='9';
	}

	template<typename T>
	constexpr int strncmp_num(T const* a_start,T const* a_end,T const* b_start,T const* b_end)
	{
		assert(a_start<=a_end);
		assert(b_start<=b_end);
		auto a_begin=a_start,b_begin=b_start;
		for(;a_begin!=a_end&&*a_begin=='0';++a_begin);//strip away leading zeros
		for(;b_begin!=b_end&&*b_begin=='0';++b_begin);
		if(int res=(a_end-a_begin)-(b_end-b_begin))
		{
			return res;
		}
		for(auto ab=a_begin,bb=b_begin;ab!=a_end;++ab,++bb)
		{
			if(int res=*ab-*bb)
			{
				return res;
			}
		}
		return int((b_begin-b_start)-(a_begin-a_start));
	}

	//comparison similar to windows sorting
	template<typename T>
	constexpr int strncmp_wind(T const* a,T const* b)
	{
		for(;;)
		{
			if(*b==0)
			{
				return *a;
			}
			if(*a==0)
			{
				return -*b;
			}
			if(is_digit(*a)&&is_digit(*b))
			{
				auto asn=a;
				while(is_digit(*(++asn)));
				auto bsn=b;
				while(is_digit(*(++bsn)));
				if(int res=strncmp_num(a,asn,b,bsn))
				{
					return res;
				}
				a=asn;
				b=bsn;
				continue;
			}
			else if(*b!=*a)
			{
				if(int res=lowercase(*a)-lowercase(*b))
				{
					return res;
				}
			}
			++a,++b;
		}
	}

}
#endif
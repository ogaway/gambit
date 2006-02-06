//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// A basic bounds-checked array type
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef LIBGAMBIT_ARRAY_H
#define LIBGAMBIT_ARRAY_H

namespace Gambit {

/// A basic bounds-checked array
template <class T> class Array  {
protected:
  int mindex, maxdex;
  T *data;

  /// Private helper function that accomplishes the insertion of an object
  int InsertAt(const T &t, int n)
  {
    if (this->mindex > n || n > this->maxdex + 1)  throw IndexException();

    T *new_data = new T[++this->maxdex - this->mindex + 1] - this->mindex;

    int i;
    for (i = this->mindex; i <= n - 1; i++) new_data[i] = this->data[i];
    new_data[i++] = t;
    for (; i <= this->maxdex; i++) new_data[i] = this->data[i - 1];

    if (this->data)   delete [] (this->data + this->mindex);
    this->data = new_data;

    return n;
  }
public:
  /// @name Lifecycle
  //@{
  /// Constructs an array of length 'len', starting at '1'
  Array(unsigned int len = 0)
    : mindex(1), maxdex(len), data((len) ? new T[len] - 1 : 0) { } 
  /// Constructs an array starting at lo and ending at hi
  Array(int lo, int hi) : mindex(lo), maxdex(hi)
  {
    if (maxdex + 1 < mindex)   throw RangeException();
    data = (maxdex >= mindex) ? new T[maxdex -mindex + 1] - mindex : 0;
  }
  /// Copy the contents of another array
  Array(const Array<T> &a)
    : mindex(a.mindex), maxdex(a.maxdex),
      data((maxdex >= mindex) ? new T[maxdex - mindex + 1] - mindex : 0)
  {
    for (int i = mindex; i <= maxdex; i++)  data[i] = a.data[i];
  }
  /// Destruct and deallocates the array
  virtual ~Array()
  { if (maxdex >= mindex)  delete [] (data + mindex); }

  /// Copy the contents of another array
  Array<T> &operator=(const Array<T> &a)
  {
    if (this != &a) {
      // We only reallocate if necessary.  This should be somewhat faster
      // if many objects are of the same length.  Furthermore, it is
      // _essential_ for the correctness of the PVector and DVector
      // assignment operator, since it assumes the value of data does
      // not change.
      if (!data || (data && (mindex != a.mindex || maxdex != a.maxdex)))  {
	if (data)   delete [] (data + mindex);
	mindex = a.mindex;   maxdex = a.maxdex;
	data = (maxdex >= mindex) ? new T[maxdex - mindex + 1] - mindex : 0;
      }
      
      for (int i = mindex; i <= maxdex; i++) data[i] = a.data[i];
    }

    return *this;
  }

  //@}

  /// @name Operator overloading
  //@{
  /// Test the equality of two arrays
  bool operator==(const Array<T> &a) const
  {
    if (mindex != a.mindex || maxdex != a.maxdex) return false;
    for (int i = mindex; i <= maxdex; i++) {
      if ((*this)[i] != a[i]) return false;
    }
    return true;
  }

  /// Test the inequality of two arrays
  bool operator!=(const Array<T> &a) const { return !(*this == a); }
  //@}

  /// @name General data access
  //@{
  /// Return the length of the array
  int Length(void) const  { return maxdex - mindex + 1; }

  /// Return the first index
  int First(void) const { return mindex; } 

  /// Return the last index
  int Last(void) const { return maxdex; }

  /// Access the index'th entry in the array
  const T &operator[](int index) const 
  {
    if (index < mindex || index > maxdex)  throw IndexException();
    return data[index];
  }

  /// Access the index'th entry in the array
  T &operator[](int index)
  {
    if (index < mindex || index > maxdex)  throw IndexException();
    return data[index];
  }

  /// Return the index at which a given element resides in the array.
  int Find(const T &t) const
  {
    int i;
    for (i = this->mindex; i <= this->maxdex && this->data[i] != t; i++);
    return (i <= this->maxdex) ? i : (mindex-1);
  } 

  /// Return true if the element is currently residing in the array
  bool Contains(const T &t) const
  { return Find(t) != mindex-1; }
  //@}

  /// @name Modifying the contents of the array
  //@{
  /// \brief Append a new element to the array.
  ///
  /// Append a new element to the array, and return the index at which the
  /// element can be found.  Note that this index is guaranteed to be the
  /// last (highest) index in the array.
  int Append(const T &t)
  { return InsertAt(t, this->maxdex + 1); }

  /// \brief Insert a new element into the array at a given index.
  ///
  /// Insert a new element into the array at a given index.  If the index is
  /// less than the lowest index, the element is inserted at the beginning;
  /// if the index is greater than the highest index, the element is appended.
  /// Returns the index at which the element actually is placed.
  int Insert(const T &t, int n)
  {
    return InsertAt(t, (n < this->mindex) ? this->mindex : ((n > this->maxdex + 1) ? this->maxdex + 1 : n));
  }

  /// \brief Remove an element from the array.
  ///
  /// Remove the element at a given index from the array.  Returns the value
  /// of the element removed.
  T Remove(int n)
  {
    if (n < this->mindex || n > this->maxdex) throw IndexException();

    T ret(this->data[n]);
    T *new_data = (--this->maxdex>=this->mindex) ? new T[this->maxdex-this->mindex+1] - this->mindex : 0;
    
    int i;
    for (i = this->mindex; i < n; i++) new_data[i] = this->data[i];
    for (; i <= this->maxdex; i++)     new_data[i] = this->data[i + 1];

    delete [] (this->data + this->mindex);
    this->data = new_data;

    return ret;
  }
  //@}
};

/// A constant forward iterator on an array, assuming T is a pointer type
template <class T> class ArrayPtrConstIterator {
private:
  const Array<T *> &m_array;
  int m_index;

public:
  /// @name Lifecycle
  //@{
  /// Constructor
  ArrayPtrConstIterator(const Array<T *> &p_array)
    : m_array(p_array), m_index(m_array.First()) { }
  //@}

  /// @name Iteration and data access
  //@{
  /// Advance to the next element (prefix version)
  void operator++(void) { m_index++; }
  /// Advance to the next element (postfix version)
  void operator++(int) { m_index++; }
  /// Has iterator gone past the end?
  bool AtEnd(void) const { return m_index > m_array.Last(); }

  /// Get the current element
  T &operator*(void) const { return *(m_array[m_index]); }
  /// Get the current element
  const T *operator->(void) const { return m_array[m_index]; }
  /// Get the current element
  operator T *(void) const { return m_array[m_index]; }
  //@}
};

} // end namespace Gambit

#endif	// LIBGAMBIT_ARRAY_H
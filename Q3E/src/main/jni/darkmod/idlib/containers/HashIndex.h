/*****************************************************************************
The Dark Mod GPL Source Code

This file is part of the The Dark Mod Source Code, originally based
on the Doom 3 GPL Source Code as published in 2011.

The Dark Mod Source Code is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version. For details, see LICENSE.TXT.

Project: The Dark Mod (http://www.thedarkmod.com/)

******************************************************************************/

#ifndef __HASHINDEX_H__
#define __HASHINDEX_H__

/*
===============================================================================

	Fast hash table for indexes and arrays.
	Does not allocate memory until the first key/index pair is added.

===============================================================================
*/

#define DEFAULT_HASH_SIZE			1024
#define DEFAULT_HASH_GRANULARITY	1024

class idHashIndex {
public:
					idHashIndex( void );
					idHashIndex( const int initialHashSize, const int initialIndexSize );
					~idHashIndex( void );

					// returns total size of allocated memory
	size_t			Allocated( void ) const;
					// returns total size of allocated memory including size of hash index type
	size_t			Size( void ) const;

	idHashIndex &	operator=( const idHashIndex &other );
					// add an index to the hash, assumes the index has not yet been added to the hash
	void			Add( const int key, const int index );
					// remove an index from the hash
	void			Remove( const int key, const int index );
					// get the first index from the hash, returns -1 if empty hash entry
	int				First( const int key ) const;
					// get the next index from the hash, returns -1 if at the end of the hash chain
	int				Next( const int index ) const;
					// insert an entry into the index and add it to the hash, increasing all indexes >= index
	void			InsertIndex( const int key, const int index );
					// remove an entry from the index and remove it from the hash, decreasing all indexes >= index
	void			RemoveIndex( const int key, const int index );
					// clear the hash
	void			Clear( void );
					// clear and resize
	void			ClearFree( const int newHashSize, const int newIndexSize );
					// free allocated memory
	void			ClearFree( void );
					// get size of hash table
	int				GetHashSize( void ) const;
					// get size of the index
	int				GetIndexSize( void ) const;
					// set granularity
	void			SetGranularity( const int newGranularity );
					// force resizing the index, current hash table stays intact
	void			ResizeIndex( const int newIndexSize );
					// returns number in the range [0-100] representing the spread over the hash table
	int				GetSpread( void ) const;
					// returns a key for a string
	int				GenerateKey( const char *string, bool caseSensitive = true ) const;
					// returns a key for a vector
	int				GenerateKey( const idVec3 &v ) const;
					// returns a key for two integers
	int				GenerateKey( const int n1, const int n2 ) const;

private:
	int				hashSize;
	int *			hash;
	int				indexSize;
	int *			indexChain;
	int				granularity;
	int				hashMask;
	int				lookupMask;

	static int		INVALID_INDEX[1];

	void			Init( const int initialHashSize, const int initialIndexSize );
	void			Allocate( const int newHashSize, const int newIndexSize );
};

/*
================
idHashIndex::idHashIndex
================
*/
ID_INLINE idHashIndex::idHashIndex( void ) {
	Init( DEFAULT_HASH_SIZE, DEFAULT_HASH_SIZE );
}

/*
================
idHashIndex::idHashIndex
================
*/
ID_INLINE idHashIndex::idHashIndex( const int initialHashSize, const int initialIndexSize ) {
	Init( initialHashSize, initialIndexSize );
}

/*
================
idHashIndex::~idHashIndex
================
*/
ID_INLINE idHashIndex::~idHashIndex( void ) {
	ClearFree();
}

/*
================
idHashIndex::Allocated
================
*/
ID_INLINE size_t idHashIndex::Allocated( void ) const {
	return hashSize * sizeof( int ) + indexSize * sizeof( int );
}

/*
================
idHashIndex::Size
================
*/
ID_INLINE size_t idHashIndex::Size( void ) const {
	return sizeof( *this ) + Allocated();
}

/*
================
idHashIndex::operator=
================
*/
ID_INLINE idHashIndex &idHashIndex::operator=( const idHashIndex &other ) {
	if ( this == &other )
		return *this;

	granularity = other.granularity;
	hashMask = other.hashMask;
	lookupMask = other.lookupMask;

	if ( other.lookupMask == 0 ) {
		hashSize = other.hashSize;
		indexSize = other.indexSize;
		ClearFree();
	}
	else {
		if ( other.hashSize != hashSize || hash == INVALID_INDEX ) {
			if ( hash != INVALID_INDEX ) {
				delete[] hash;
			}
			hashSize = other.hashSize;
			hash = new int[hashSize];
		}
		if ( other.indexSize != indexSize || indexChain == INVALID_INDEX ) {
			if ( indexChain != INVALID_INDEX ) {
				delete[] indexChain;
			}
			indexSize = other.indexSize;
			indexChain = new int[indexSize];
		}
		memcpy( hash, other.hash, hashSize * sizeof( hash[0] ) );
		memcpy( indexChain, other.indexChain, indexSize * sizeof( indexChain[0] ) );
	}

	return *this;
}

/*
================
idHashIndex::Add
================
*/
ID_INLINE void idHashIndex::Add( const int key, const int index ) {
	int h;

	assert( index >= 0 );
	if ( hash == INVALID_INDEX ) {
		Allocate( hashSize, index >= indexSize ? index + 1 : indexSize );
	}
	else if ( index >= indexSize ) {
		ResizeIndex( index + 1 );
	}
	h = key & hashMask;
	indexChain[index] = hash[h];
	hash[h] = index;
}

/*
================
idHashIndex::Remove
================
*/
ID_INLINE void idHashIndex::Remove( const int key, const int index ) {
	int k = key & hashMask;

	if ( hash == INVALID_INDEX ) {
		return;
	}
	if ( hash[k] == index ) {
		hash[k] = indexChain[index];
	}
	else {
		for ( int i = hash[k]; i != -1; i = indexChain[i] ) {
			if ( indexChain[i] == index ) {
				indexChain[i] = indexChain[index];
				break;
			}
		}
	}
	indexChain[index] = -1;
}

/*
================
idHashIndex::First
================
*/
ID_INLINE int idHashIndex::First( const int key ) const {
	return hash[key & hashMask & lookupMask];
}

/*
================
idHashIndex::Next
================
*/
ID_INLINE int idHashIndex::Next( const int index ) const {
	assert( index >= 0 && index < indexSize );
	return indexChain[index & lookupMask];
}

/*
================
idHashIndex::InsertIndex
================
*/
ID_INLINE void idHashIndex::InsertIndex( const int key, const int index ) {
	int i, max;

	if ( hash != INVALID_INDEX ) {
		max = index;
		for ( i = 0; i < hashSize; i++ ) {
			if ( hash[i] >= index ) {
				hash[i]++;
				if ( hash[i] > max ) {
					max = hash[i];
				}
			}
		}
		for ( i = 0; i < indexSize; i++ ) {
			if ( indexChain[i] >= index ) {
				indexChain[i]++;
				if ( indexChain[i] > max ) {
					max = indexChain[i];
				}
			}
		}
		if ( max >= indexSize ) {
			ResizeIndex( max + 1 );
		}
		for ( i = max; i > index; i-- ) {
			indexChain[i] = indexChain[i-1];
		}
		indexChain[index] = -1;
	}
	Add( key, index );
}

/*
================
idHashIndex::RemoveIndex
================
*/
ID_INLINE void idHashIndex::RemoveIndex( const int key, const int index ) {
	int i, max;

	Remove( key, index );
	if ( hash != INVALID_INDEX ) {
		max = index;
		for ( i = 0; i < hashSize; i++ ) {
			if ( hash[i] >= index ) {
				if ( hash[i] > max ) {
					max = hash[i];
				}
				hash[i]--;
			}
		}
		for ( i = 0; i < indexSize; i++ ) {
			if ( indexChain[i] >= index ) {
				if ( indexChain[i] > max ) {
					max = indexChain[i];
				}
				indexChain[i]--;
			}
		}
		for ( i = index; i < max; i++ ) {
			indexChain[i] = indexChain[i+1];
		}
		indexChain[max] = -1;
	}
}

/*
================
idHashIndex::Clear
================
*/
ID_INLINE void idHashIndex::Clear( void ) {
	// only clear the hash table because clearing the indexChain is not really needed
	if ( hash != INVALID_INDEX ) {
		memset( hash, 0xff, hashSize * sizeof( hash[0] ) );
	}
}

/*
================
idHashIndex::ClearFree
================
*/
ID_INLINE void idHashIndex::ClearFree( const int newHashSize, const int newIndexSize ) {
	ClearFree();
	hashSize = newHashSize;
	indexSize = newIndexSize;
}

/*
================
idHashIndex::GetHashSize
================
*/
ID_INLINE int idHashIndex::GetHashSize( void ) const {
	return hashSize;
}

/*
================
idHashIndex::GetIndexSize
================
*/
ID_INLINE int idHashIndex::GetIndexSize( void ) const {
	return indexSize;
}

/*
================
idHashIndex::SetGranularity
================
*/
ID_INLINE void idHashIndex::SetGranularity( const int newGranularity ) {
	assert( newGranularity > 0 );
	granularity = newGranularity;
}

/*
================
idHashIndex::GenerateKey
================
*/
ID_INLINE int idHashIndex::GenerateKey( const char *string, bool caseSensitive ) const {
	if ( caseSensitive ) {
		return ( idStr::Hash( string ) & hashMask );
	} else {
		return ( idStr::IHash( string ) & hashMask );
	}
}

/*
================
idHashIndex::GenerateKey
================
*/
ID_INLINE int idHashIndex::GenerateKey( const idVec3 &v ) const {
	return ( (((int) v[0]) + ((int) v[1]) + ((int) v[2])) & hashMask );
}

/*
================
idHashIndex::GenerateKey
================
*/
ID_INLINE int idHashIndex::GenerateKey( const int n1, const int n2 ) const {
	return ( ( n1 + n2 ) & hashMask );
}

// stgatilov: class for displaying hash + index arrays of idHashIndex
//   in MSVC debug watch (look "autoexp.dat")
struct _idHashIndex_showarray_helper {
	int size;
	int *ptr;
};

#endif /* !__HASHINDEX_H__ */

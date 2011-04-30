#ifndef _VECTOR_H
#define _VECTOR_H


/**
 * Simple container.
 * @author Jani Kajala (jani.kajala@helsinki.fi)
 */
typedef struct Vector
{
	int		objectSize;		// size of single object
	int		capacity;		// bytes
	int		size;			// bytes
	int		objects;		// number of objects
	void*	data;			// ptr to first object
} Vector_t;


/** Allocates a vector. */
Vector_t*	Vector_create( int objectSize );

/** Frees vector memory. */
void		Vector_destroy( Vector_t* vec );

/** Appends an object to the vector. */
void		Vector_add( Vector_t* vec, void* object, int objectSize );

/** Removes ith object in the vector. */
void		Vector_remove( Vector_t* vec, int i );

/** Sets number of objects in the vector to 0. */
void		Vector_clear( Vector_t* vec );

/** Returns ith object in the vector. */
void*		Vector_get( Vector_t* vec, int i );

/** Returns number of objects in the vector. */
int			Vector_size( Vector_t* vec );


#endif // _VECTOR_H

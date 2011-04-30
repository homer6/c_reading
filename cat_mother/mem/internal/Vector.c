#include "Vector.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "config.h"

//-----------------------------------------------------------------------------

int Vector_valid( Vector_t* vec )
{
	return vec->data &&
		vec->objectSize > 0 &&
		vec->size >= 0 &&
		vec->capacity > 0 &&
		vec->size <= vec->capacity;
}

Vector_t* Vector_create( int objectSize )
{
	Vector_t* vec = NULL;

	assert( objectSize > 0 );

	vec = (Vector_t*)malloc( sizeof(Vector_t) );
	memset( vec, 0, sizeof(Vector_t) );

	vec->objectSize = objectSize;
	vec->capacity = 4 * objectSize;
	vec->size = 0;
	vec->objects = 0;
	vec->data = malloc( vec->capacity );
	memset( vec->data, 0, vec->capacity );

	return vec;
}

void Vector_destroy( Vector_t* vec )
{
	assert( Vector_valid(vec) );

	memset( vec->data, 0xEE, vec->capacity );
	free( vec->data );
	memset( vec, 0xEE, sizeof(Vector_t) );
	free( vec );
}

void Vector_reserve( Vector_t* vec, int newCapacity )
{
	void*	newData		= 0;

	assert( Vector_valid(vec) );
	assert( newCapacity % vec->objectSize == 0 );

	if ( vec->capacity < newCapacity )
	{
		newData = malloc( newCapacity );

		memcpy( newData, vec->data, vec->capacity );
		memset( (char*)newData + vec->capacity, 0, newCapacity - vec->capacity );

		memset( vec->data, 0xEE, vec->capacity );
		free( vec->data );

		vec->data = newData;
		vec->capacity = newCapacity;
	}
}

void Vector_add( Vector_t* vec, void* object, int objectSize )
{
	assert( Vector_valid(vec) );
	assert( objectSize == vec->objectSize );

	if ( vec->size+objectSize > vec->capacity )
		Vector_reserve( vec, vec->capacity*2 );
	
	memcpy( (char*)vec->data + vec->size, object, objectSize );
	vec->size += objectSize;
	vec->objects += 1;
}

void* Vector_get( Vector_t* vec, int i )
{
	assert( Vector_valid(vec) );
	assert( i >= 0 && i < vec->objects );

	return (char*)vec->data + vec->objectSize * i;
}

int	Vector_size( Vector_t* vec )
{
	assert( Vector_valid(vec) );

	return vec->objects;
}

void Vector_setSize( Vector_t* vec, int size )
{
	vec->size = size;
	vec->objects = size;
}

void Vector_clear( Vector_t* vec )
{
	Vector_setSize( vec, 0 );
}

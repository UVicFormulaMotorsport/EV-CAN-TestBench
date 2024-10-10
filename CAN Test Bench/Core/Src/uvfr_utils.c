/*
 * uvfr_utils.c
 *
 *  Created on: Oct 7, 2024
 *      Author: byo10
 */


//this works exactly the same way that malloc does
/* This should do the following:
 * 1)use _sbrk() to allocate space from the heap if neccessary
 * 2)if sufficient heap space is already in the user space, then allocate a
 * lil chunk of it to the user.
 *
 *
 *
 */
void *uvMalloc(size_t size){

}

//this works exactly the same way that free() does
uv_status_t uvFree(void * mem){

}

//this works the same way that realloc() does
void *uvRealloc(){

}

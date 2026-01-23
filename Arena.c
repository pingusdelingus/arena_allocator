#include <stdlib.h>
#include "types.h"
#include "arena.h"
#include <assert.h>

static const i64 INITIAL_SIZE = 300; // 300 bytes

static const i8 ALIGNMENT_BYTES = 16;


ArenaBlock* AllocArenaBlock(void)
{
  ArenaBlock* arena = (ArenaBlock*) malloc(sizeof(ArenaBlock));  
  if (arena == NULL){
    return NULL;
  }
  
  arena->buffer = (char*) malloc(INITIAL_SIZE);
  if (arena->buffer == NULL){
    return NULL;
  }
  arena->isFull = 0;
  arena->next = NULL;
  arena->max_capacity = INITIAL_SIZE; 
  arena->used_capacity = 0;
  arena->ptr = 0; // point at first index !
  memset(arena->buffer, 0, arena->max_capacity); // set zeroes 
  return arena; // will not get deleted after scope ends
}// end of AllocArenaBlock (constructor)



void ReleaseArenaBlocks(ArenaBlock * a)
{
  struct ArenaBlock* curr  = a;

  while (curr != NULL){
      struct ArenaBlock* next = curr->next;
      
        // free char* buff
      free(curr->buffer);

      // free arena itself
      free(a);

      // iterate   
     curr = next;
  }// end of while

}// end of ReleaseArenaBlock (~destructor)


void* ArenaBlockPush(ArenaBlock* arena, i64 s, b8 doSizeCheck)
{
  if (doSizeCheck) {

        if (arena->isFull){
          if (arena->next != NULL){
          return ArenaBlockPush(arena->next, s, doSizeCheck);
        } 
      }

      assert ((arena->ptr + 1 ) % ALIGNMENT_BYTES == 0 );
      i64 size = s;

      if (size % ALIGNMENT_BYTES != 0){
        size = (size) + (ALIGNMENT_BYTES - (size % ALIGNMENT_BYTES) );
      }


      i64 req_plus_used = size + arena->used_capacity; 
      if (req_plus_used > arena->max_capacity){
          // make new arena for next ptr
          ArenaBlock* nextArenaBlock = AllocArenaBlock();
          assert(nextArenaBlock != NULL);

          arena->isFull = 1;
          arena->next = nextArenaBlock;
          return ArenaBlockPush(arena->next, size, doSizeCheck);
      }

  }// end of if do sizecheck
 
  i64 size = s; 
  char* current_ptr = &(arena->buffer[arena->ptr]); // return this in a sec
  arena->ptr += size;
  arena->used_capacity += size;
  return current_ptr; 
}// end of ArenaBlockPush


void* ArenaBlockPushZeroes(ArenaBlock* arena, i64 s, b8 doSizeCheck)
{
  if (doSizeCheck){
      i64 size = s;
      if (size % ALIGNMENT_BYTES != 0){
        size = (size) + (ALIGNMENT_BYTES - (size % ALIGNMENT_BYTES) );
      }
      b8 dontSizeCheck = 0;

      if (arena->isFull){
        if (arena->next != NULL){
          ArenaBlockPushZeroes(arena->next, size, dontSizeCheck);
        }
      }

      i64 req_plus_used = size + arena->used_capacity; 
      if (req_plus_used > arena->max_capacity){

        ArenaBlock* nextArenaBlock = AllocArenaBlock();
        assert(nextArenaBlock != NULL);

        arena->max_capacity = arena->used_capacity;
        arena->isFull = 1;
        arena ->next = nextArenaBlock; 

        return ArenaBlockPushZeroes(arena->next, size , dontSizeCheck);
      }
  } // end of do size check

  i64 size = s;
  i64 start_num = arena->ptr;

  char* current_ptr = &(arena->buffer[arena->ptr]);

  for (i64 i = start_num; i  < arena->ptr + size; i++){
    arena->buffer[i] = '\0';
  }// end of for

  arena->ptr += size;
  arena->used_capacity += size;

  return current_ptr;
}// end of ArenaBlockPushZeroes


void ArenaBlockPop(ArenaBlock* arena, i64 size)
{
  i64 used_minus_size = arena->used_capacity - size;
  
  i64 delete_size = size;
  b8 remove_too_much = 0;
  i64 stop_index = 0;  

  if (used_minus_size < 0){
      remove_too_much = 1;  
      delete_size = arena->used_capacity;
      stop_index = 0;
        
      if (arena->prev != NULL){
          ArenaBlockPop(arena->prev, used_minus_size);
    }

  }else{
      stop_index = arena->ptr - delete_size; // should be non-negative
      assert(stop_index >= 0);
  }

  //zeroing out memory
  for (i64 i = arena->ptr - 1; i >= stop_index; i--){ arena->buffer[i] = '\0'; }  

  if (remove_too_much){
      arena->used_capacity = 0;
      arena->ptr = 0;
    }

  else {
      arena->used_capacity -= size;  
      arena->ptr -= size;  
  }
}// end of ArenaBlockPop


i64 GetArenaBlockSize(ArenaBlock* arena)
{
  return arena->used_capacity;
}// end of GetArenaBlockSize


typedef struct{
  i32 x;
  i32 y;
} Vec2;

void ArenaBlockDebugPrint(ArenaBlock* arena) {
    printf("-------------------\n\n");
    printf("\nARENA DEBUG \n");
    printf("Pointer: %lld | Used: %lld | Max: %lld\n", 
            arena->ptr, arena->used_capacity, arena->max_capacity);
    
    printf("Memory Map: [");
    for (i64 i = 0; i < arena->max_capacity; i++) {
        if (i == arena->ptr) printf("|PTR|"); // Show where current pointer is
        
        unsigned char c = (unsigned char)arena->buffer[i];
        if (c == '\0') {
            printf("."); // Represent null bytes as dots
        } else if (c >= 32 && c <= 126) {
            printf("%c", c); // Print printable characters
        } else {
            printf("?"); // Represent non-printable garbage
        }
    }
    printf("]\n");
    printf("-------------------\n\n");
}// end of ArenaBlockDebugPrint




Arena* ArenaConstruct(void)
{
  Arena * a = (Arena*) malloc(sizeof(Arena));
  a->genesis = AllocArenaBlock();
  a->current = a->genesis;
  
  return a;
}


void ArenaDestruct(Arena* a)
{
  ReleaseArenaBlocks(a->genesis);
}

void* ArenaPush(Arena* a, i64 s)
{
  i64 size = s;
  if (size % ALIGNMENT_BYTES != 0){
    size = (size) + (ALIGNMENT_BYTES - (size % ALIGNMENT_BYTES) );
  }
  
  b8 dontsizecheck = 0;

  ArenaBlock* curr = a->current;
  i64 req_plus_used = size + curr->used_capacity; 

  if (!curr->isFull && req_plus_used < curr->max_capacity){
    dontsizecheck = 0;
   return ArenaBlockPush(curr, s, dontsizecheck);
  }

  // full or requesting too much memory 
  // make new arena for next ptr
  ArenaBlock* nextArenaBlock = AllocArenaBlock();
  assert(nextArenaBlock != NULL);

  curr->isFull = 1;
  curr ->max_capacity = curr->used_capacity;

  curr->next = nextArenaBlock;
  nextArenaBlock->prev = curr;

  a->current = nextArenaBlock;
  assert(nextArenaBlock->next == NULL);
  return ArenaBlockPush(a->current, size, dontsizecheck);
}


void* ArenaPushZeroes(Arena* a, i64 s)
{
  i64 size = s;
  if (size % ALIGNMENT_BYTES != 0){
    size = (size) + (ALIGNMENT_BYTES - (size % ALIGNMENT_BYTES) );
  }
  
  b8 dontsizecheck = 0;

  ArenaBlock* curr = a->current;
  i64 req_plus_used = size + curr->used_capacity; 

  if (!curr->isFull && req_plus_used < curr->max_capacity){
    dontsizecheck = 0;
   return ArenaBlockPushZeroes(curr, s, dontsizecheck);
  }

  // full or requesting too much memory 
  // make new arena for next ptr
  ArenaBlock* nextArenaBlock = AllocArenaBlock();
  assert(nextArenaBlock != NULL);

  curr->isFull = 1;
  curr ->max_capacity = curr->used_capacity;

  curr->next = nextArenaBlock;
  nextArenaBlock->prev = curr;

  a->current = nextArenaBlock;
  assert(nextArenaBlock->next == NULL);
  return ArenaBlockPushZeroes(a->current, size, dontsizecheck);
//  return ArenaBlockPushZeroes(a->current);
}


void ArenaPop(Arena* a, i64 size)
{
 ArenaBlock* current = a->current;
  ArenaBlockPop(current, size);
}

i64 GetArenaSize(Arena* a)
{
  i64 size = 0;

 ArenaBlock* current = a->genesis;

  while(current != NULL){
    size += current->used_capacity;
      current = current->next; 
  }
  return size;
}// GetArenaSize



int main(void)
{
Arena * a = ArenaConstruct();
  
}// end of main

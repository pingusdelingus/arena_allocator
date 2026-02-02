#include <stdlib.h>
#include "types.h"
#include "arena.h"
#include <assert.h>
#include <sys/mman.h>

static const i32 INITIAL_BLOCK_SIZE = 1000; // 1kb 

static const i8 ALIGNMENT_BYTES = 16;

static const b8 USE_MMAP = 1;

// ------------------------------------------------------------------------------

ArenaBlock* AllocArenaBlock(void)
{
  if (USE_MMAP){
    ArenaBlock* arena = (ArenaBlock* ) mmap(NULL, sizeof(ArenaBlock), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS ,-1,0 ); 

  if (arena == MAP_FAILED){
      perror("mmap failed on arena");
      return NULL;
    }

    arena-> buffer = (char* ) mmap (NULL, INITIAL_BLOCK_SIZE,  PROT_READ | PROT_WRITE,  MAP_PRIVATE | MAP_ANONYMOUS, -1 , 0); 
    
    if (arena->buffer == MAP_FAILED){
      perror("mmap failed on buffer");

      munmap(arena, sizeof(arena));
      munmap(arena->buffer, INITIAL_BLOCK_SIZE);
      return NULL;
    }


  arena->isFull = 0;
  arena->next = NULL;
  arena->prev = NULL;
  arena->max_capacity = INITIAL_BLOCK_SIZE; 
  
  arena->ptr = 0; // point at first index !
    return arena;

  }else {

  ArenaBlock* arena = (ArenaBlock*) malloc(sizeof(ArenaBlock));  
  if (arena == NULL){
    free(arena);
    return NULL;
  }
  
  arena->buffer = (char*) malloc(INITIAL_BLOCK_SIZE);
  if (arena->buffer == NULL){
    free(arena);
    free(arena->buffer);
    return NULL;
  }

  arena->isFull = 0;
  arena->next = NULL;
  arena->prev = NULL;
  arena->max_capacity = INITIAL_BLOCK_SIZE; 
  
  arena->ptr = 0; // point at first index !
  memset(arena->buffer, 0, arena->max_capacity); 

  return arena; // will not get deleted after scope ends
  }// end of else
}// end of AllocArenaBlock (constructor)

// ------------------------------------------------------------------------------

void ReleaseArenaBlocks(ArenaBlock * a)
{

  if (USE_MMAP){
    
  struct ArenaBlock* curr  = a;

  while (curr != NULL){
      struct ArenaBlock* next = curr->next;
      
      munmap(curr->buffer, sizeof(INITIAL_BLOCK_SIZE));
      munmap(curr, sizeof(curr));
      // iterate   
     curr = next;
  }// end of while

  }else{

  struct ArenaBlock* curr  = a;

  while (curr != NULL){
      struct ArenaBlock* next = curr->next;
      
        // free char* buff
      free(curr->buffer);
      // free arena itself
      free(curr);

      // iterate   
     curr = next;
  }// end of while

  }// end of else

}// end of ReleaseArenaBlock (~destructor)

// ------------------------------------------------------------------------------

int FreeArenaBlock(ArenaBlock * arenablock)
{
  int error_count = 2;
  if (arenablock != NULL && arenablock->buffer){
    free(arenablock->buffer);
    error_count--;
    arenablock->buffer = NULL;
  }

  if (arenablock != NULL){
    free(arenablock);
    error_count--;
    arenablock= NULL;
  }

return error_count;
}// end of free arenablock

// ------------------------------------------------------------------------------

void* ArenaBlockPush(ArenaBlock* arena, i32 s )
{
 // never doing size check as that is now the caller's responsibility

  //can also be char* 
  char * current_ptr = &(arena->buffer[arena->ptr]); // return this in a sec
  arena->ptr += s;
  return current_ptr; 
}// end of ArenaBlockPush

// ------------------------------------------------------------------------------

void* ArenaBlockPushZeroes(ArenaBlock* arena, i32 s )
{
// no more size chekc
  i32 start_num = arena->ptr;

  char* current_ptr = &(arena->buffer[arena->ptr]);

  for (i32 i = start_num; i  < arena->ptr + s; i++){
    arena->buffer[i] = '\0';
  }// end of for

  arena->ptr += s;

  return current_ptr;
}// end of ArenaBlockPushZeroes

// ------------------------------------------------------------------------------

void ArenaBlockPop(ArenaBlock* arena, i32 size)
{
  i32 used_minus_size = arena->ptr - size;
  
  i32 delete_size = size;
  b8 remove_too_much = 0;
  i32 stop_index = 0;  

  if (used_minus_size < 0){
      remove_too_much = 1;  
      delete_size = arena->ptr;
      stop_index = 0;
        
      if (arena->prev != NULL){
          ArenaBlockPop(arena->prev, used_minus_size);
    }

  }else{
      stop_index = arena->ptr - delete_size; // should be non-negative
      assert(stop_index >= 0);
  }

  //zeroing out memory
  for (i32 i = arena->ptr - 1; i >= stop_index; i--){ arena->buffer[i] = '\0'; }  

  if (remove_too_much){
      arena->ptr = 0;
    }

  else {
      arena->ptr -= size;  
  }
}// end of ArenaBlockPop

// ------------------------------------------------------------------------------

inline i32 GetArenaBlockSize(ArenaBlock* arena)
{
  return arena->ptr;
}// end of GetArenaBlockSize

// ------------------------------------------------------------------------------

typedef struct{
  i32 x;
  i32 y;
} Vec2;

// ------------------------------------------------------------------------------

inline void ArenaBlockDebugPrint(ArenaBlock* arena) {
    printf("-------------------\n\n");
    printf("\nARENA DEBUG \n");
    printf("Pointer: %d| Max: %d\n", 
            arena->ptr, arena->max_capacity);
    
    printf("Memory Map: [");
    for (i32 i = 0; i < arena->max_capacity; i++) {
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

// ------------------------------------------------------------------------------

Arena* ArenaConstruct(void)
{
  if (USE_MMAP) {

    Arena * a = (Arena* ) mmap(NULL, sizeof(Arena), PROT_READ | PROT_WRITE ,  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (a == MAP_FAILED){
      perror("mmap failed in arena construct ");
      return NULL;
    }

  a->genesis = AllocArenaBlock();
  a->current = a->genesis;
  return a;  

  } else{

  Arena * a = (Arena*) malloc(sizeof(Arena));
  a->genesis = AllocArenaBlock();
  a->current = a->genesis;
  
  return a;

  }// end of else
}

// ------------------------------------------------------------------------------

void ArenaDestruct(Arena* a)
{
  ReleaseArenaBlocks(a->genesis);
  if (USE_MMAP){
    
    munmap(a, sizeof(a));

  }else{

  

  free(a);

  return; 
  }// end of eflse
}

// ------------------------------------------------------------------------------

void* ArenaPush(Arena* a, i32 s)
{
  i32 size = s;
  if (size % ALIGNMENT_BYTES != 0){
    size +=  (ALIGNMENT_BYTES - (size % ALIGNMENT_BYTES) );
  }
  
  ArenaBlock* curr = a->current;
  i32 req_plus_used = size + curr->ptr; 

  if (!curr->isFull && req_plus_used < curr->max_capacity){
   return ArenaBlockPush(curr, size ) ;
  }

  // full or requesting too much memory 
  // make new arena for next ptr
  ArenaBlock* nextArenaBlock = AllocArenaBlock();
  assert(nextArenaBlock != NULL);

  curr->isFull = 1;
  curr ->max_capacity = curr->ptr;

  curr->next = nextArenaBlock;
  nextArenaBlock->prev = curr;

  a->current = nextArenaBlock;
  assert(nextArenaBlock->next == NULL);
  return ArenaBlockPush(a->current, size );

  
}// end of arena push

// ------------------------------------------------------------------------------

void* ArenaPushZeroes(Arena* a, i32 s)
{
  i32 size = s;
  if (size % ALIGNMENT_BYTES != 0){
    size = (size) + (ALIGNMENT_BYTES - (size % ALIGNMENT_BYTES) );
  }
  

  ArenaBlock* curr = a->current;
  i32 req_plus_used = size + curr->ptr; 

  if (!curr->isFull && req_plus_used < curr->max_capacity){
   return ArenaBlockPushZeroes(curr, size);
  }

  // full or requesting too much memory 
  // make new arena for next ptr
  ArenaBlock* nextArenaBlock = AllocArenaBlock();
  assert(nextArenaBlock != NULL);

  curr->isFull = 1;
  curr ->max_capacity = curr->ptr;

  curr->next = nextArenaBlock;
  nextArenaBlock->prev = curr;

  a->current = nextArenaBlock;
  assert(nextArenaBlock->next == NULL);
  return ArenaBlockPushZeroes(a->current, size ) ;
}// end of ArenaPushZeroes

// ------------------------------------------------------------------------------

void ArenaPop(Arena* a, i32 size)
{
  i32 ammount_to_remove = size;
  ArenaBlock* current = a->current;
  i32 delete_size = 0; 
  while (ammount_to_remove > 0 && current != NULL) {
      if (ammount_to_remove - current->ptr < 0){
      delete_size = current->ptr;
      } else{
      delete_size = ammount_to_remove;
    } 

    ArenaBlockPop(current, delete_size);
    current->ptr -= delete_size;
    current->isFull = 0;
    ammount_to_remove -= delete_size;
    current = current->prev;
  }// end of while

}// end of ArenaPop

// ------------------------------------------------------------------------------
i32 GetArenaSize(Arena* a)
{
  i32 size = 0;

 ArenaBlock* current = a->genesis;

  while(current != NULL){
    size += current->ptr;
      current = current->next; 
  }
  return size;
}// GetArenaSize

// ------------------------------------------------------------------------------

typedef struct vector {
  i32 x;
  i32 y;
} vector;


// ------------------------------------------------------------------------------

void test_cross_block_pop() {
    printf("Running: Cross-Block Pop... ");
    Arena *a = ArenaConstruct();
    
    // Fill first block
    ArenaPush(a, 900);
    // Force second block
    ArenaPush(a, 500);
    
    i32 total_size = GetArenaSize(a);
    // Pop more than the second block contains
    ArenaPop(a, 600); 
    total_size = GetArenaSize(a); 
    assert(total_size < 900); // Should have regressed into the first block
    ArenaDestruct(a);
    printf("PASSED\n");
}


// ------------------------------------------------------------------------------

void test_basic_lifecycle() {
    printf("Running: Basic Lifecycle... ");
    Arena *a = ArenaConstruct();
    void* ptr = ArenaPush(a, 100);
    assert(ptr != NULL);
    assert(a->current->ptr >= 100); // Check if pointer moved
    ArenaDestruct(a);
    printf("PASSED\n");
}

// ------------------------------------------------------------------------------

void test_block_overflow() {
    printf("Running: Block Overflow... ");
    Arena *a = ArenaConstruct();
    
    // INITIAL_BLOCK_SIZE is 1000. Let's push 5000 bytes.
    // This should create roughly 5-6 blocks depending on alignment.
    for(int i = 0; i < 50; i++) {
        ArenaPush(a, 100);
    }
    
    assert(a->genesis != a->current); // Verify we actually linked new blocks
    ArenaDestruct(a); // Valgrind will scream here if ReleaseArenaBlocks is wrong
    printf("PASSED\n");
}

// ------------------------------------------------------------------------------

int main(void)
{
Arena * a = ArenaConstruct();
vector* v1 = ArenaPush(a, sizeof(vector));
  v1->x = 2;
  v1->y = -8;

  ArenaPop(a, 300);
  ArenaDestruct(a);

  test_block_overflow();
  test_basic_lifecycle();
  test_cross_block_pop();

}// end of main

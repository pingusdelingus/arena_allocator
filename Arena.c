#include <stdlib.h>
#include "types.h"
#include "arena.h"
#include <assert.h>

static const i64 INITIAL_SIZE = 300; // 300 bytes

static const i8 ALIGNMENT_BYTES = 16;

void ArenaDebugPrint(Arena* arena) {
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
}// end of ArenaDebugPrint


Arena* ArenaAlloc(void)
{
  Arena* arena = (Arena*) malloc(sizeof(Arena));  
  if (arena == NULL){
    return NULL;
  }
  
  arena->buffer = (char*) malloc(INITIAL_SIZE);
  if (arena->buffer == NULL){
    return NULL;
  }

  arena->max_capacity = INITIAL_SIZE; // i32 to i64 conversion ? 
  arena->used_capacity = 0;
  arena->ptr = 0; // point at first index !
  memset(arena->buffer, 0, arena->max_capacity); // set zeroes 
  return arena; // will not get deleted after scope ends
}// end of ArenaAlloc (constructor)

Arena*  ArenaRealloc(Arena* a)
{
  i64 size = a->max_capacity;
  
  char* new_buff = (char* )realloc (a->buffer, 2 * size);
  if (new_buff == NULL){
    return NULL;
  }
  a-> max_capacity *= 2;
  
  a->buffer = new_buff;

  return a;

}// end of ArenaRealloc


void ReleaseArena(Arena * a)
{
  // free char* buff
  free(a->buffer);

  // free arena itself
  free(a);
}// end of ReleaseArena (~destructor)


void* ArenaPush(Arena* arena, i64 s)
{
  i64 size = s;
  if (size % ALIGNMENT_BYTES != 0){
    size = (size) + (ALIGNMENT_BYTES - (size % ALIGNMENT_BYTES) );
  }

  i64 req_plus_used = size + arena->used_capacity; 
  if (req_plus_used > arena->max_capacity){
    a = ArenaRealloc(arena);
  }

  char* current_ptr = &(arena->buffer[arena->ptr]); // return this in a bit
  arena->ptr += size;
  arena->used_capacity += size;
  return current_ptr; 
}// end of ArenaPush


void* ArenaPushZeroes(Arena* arena, i64 s)
{
  i64 size = s;
  if (size % ALIGNMENT_BYTES != 0){
    size = (size) + (ALIGNMENT_BYTES - (size % ALIGNMENT_BYTES) );
  }

  i64 req_plus_used = size + arena->used_capacity; 
  if (req_plus_used > arena->max_capacity){
    perror("trying to request more bytes than available");
    return NULL;
  }

  i64 start_num = arena->ptr;

  char* current_ptr = &(arena->buffer[arena->ptr]);

  for (i64 i = start_num; i  < arena->ptr + size; i++){
    arena->buffer[i] = '\0';
  }// end of for

  arena->ptr += size;
  arena->used_capacity += size;

  return current_ptr;
}// end of ArenaPushZeroes


void ArenaPop(Arena* arena, i64 size)
{
  i64 used_minus_size = arena->used_capacity - size;
  
  i64 delete_size = size;
  b8 remove_too_much = 0;
  i64 stop_index = 0;  
  if (used_minus_size < 0){
    remove_too_much = 1;  
    delete_size = arena->used_capacity;
    stop_index = 0;
  }else{
    stop_index = arena->ptr - delete_size; // should be non-zero 
    assert(stop_index != 0);
  }

  for (i64 i = arena->ptr - 1; i >= stop_index; i--){
      arena->buffer[i] = '\0';
  } 

  if (remove_too_much){
  arena->used_capacity = 0;
  arena->ptr = 0;
  }
  else {
  arena->used_capacity -= size;  
  arena->ptr -= size;  
  }
}// end of ArenaPop


i64 GetArenaSize(Arena* arena)
{
  return arena->used_capacity;
}// end of GetArenaSize


typedef struct{
  i32 x;
  i32 y;
} Vec2;


int main(void)
{
  Arena* a = ArenaAlloc();
  printf("pusing\n");
  char* e = ArenaPush(a, 60);

  memset(e, 'e', 60);
  printf("popping\n");
  ArenaPop(a, 60);
 
  printf("popping everything \n");
  ArenaPop(a, 500);

  char* h = ArenaPush(a, 5);
  strcpy(h, "help!");
  ArenaDebugPrint(a);
  ArenaPop(a, 5);
  ArenaDebugPrint(a);
  ArenaPop(a, 500);
  ArenaDebugPrint(a);
  ReleaseArena(a);
  
}

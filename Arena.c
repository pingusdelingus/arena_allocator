#include <stdlib.h>
#include "types.h"
#include "arena.h"

static const u64 INITIAL_SIZE = 300; // 300 bytes




Arena* ArenaAlloc(void)
{
  Arena* arena = (Arena*) malloc(sizeof(Arena));  
  
  arena->buffer = (char*) malloc(INITIAL_SIZE);
  arena->max_capacity = INITIAL_SIZE; // i32 to u64 conversion ? 
  arena->used_capacity = 0;
  arena->ptr = 0; // point at first index !

  return arena; // will not get deleted after scope ends
}// end of ArenaAlloc (constructor)


void ReleaseArena(Arena * a)
{
  // free buff
  free(a->buffer);

  // free arena itself
  free(a);
}// end of ReleaseArena (~destructor)



void* ArenaPush(Arena* arena, u64 size)
{
  u64 req_plus_used = size + arena->used_capacity; 
  if (req_plus_used > arena->max_capacity){
    perror("trying to request more than available");
    return NULL;
  }

  char* current_ptr = &(arena->buffer[arena->ptr]); // return this in a bit
  arena->ptr += size;
  arena->used_capacity += size;
  return current_ptr; 
}// end of ArenaPush



void* ArenaPushZeroes(Arena* arena, u64 size)
{

  u64 req_plus_used = size + arena->used_capacity; 
  if (req_plus_used > arena->max_capacity){
    perror("trying to request more than available");
    return NULL;
  }

  u64 start_num = arena->ptr;

  char* current_ptr = &(arena->buffer[arena->ptr]);
  arena->ptr += size;

  for (u64 i = start_num; i  < arena->ptr; i++){
    arena->buffer[i] = '\0';
  }// end of for
  arena->used_capacity += size;
  return current_ptr;
}// end of ArenaPushZeroes
//


void ArenaPop(Arena* arena, u64 size)
{
  u64 used_minus_size = arena->used_capacity - size;
  
  u64 delete_size = size;

  if (used_minus_size < 0){
    delete_size = arena->used_capacity;
  }
  u64 stop_index = arena->ptr - delete_size;
  for (u64 i = arena->ptr; i >= stop_index; i++){
      arena->buffer[i] = '\0';
  } 
  
}// end of ArenaPop
//

u64 GetArenaSize(Arena* arena)
{
  return arena->used_capacity;
}// end of GetArenaSize
//

typedef struct{
  i32 x;
  i32 y;
} Vec2;

int main(void)
{
  Arena * my_arena  = ArenaAlloc();
  char* name = (char* ) ArenaPush(my_arena, 20);
  printf("allocating 20 bytes for my name\n");
  printf("bytes used:  %llu\n", GetArenaSize(my_arena));

  Vec2* vector = ArenaPush(my_arena, (u64) (sizeof(Vec2) ));
  printf("allocating my vector\n");
  printf("bytes used:  %llu\n", GetArenaSize(my_arena));

  ReleaseArena(my_arena);
}

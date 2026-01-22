#include <stdlib.h>
#include "types.h"
#include "arena.h"

static const i64 INITIAL_SIZE = 60; // 300 bytes


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
  
  arena->buffer = (char*) malloc(INITIAL_SIZE);
  arena->max_capacity = INITIAL_SIZE; // i32 to i64 conversion ? 
  arena->used_capacity = 0;
  arena->ptr = 0; // point at first index !
  memset(arena->buffer, 0, arena->max_capacity); // set zeroes 
  return arena; // will not get deleted after scope ends
}// end of ArenaAlloc (constructor)


void ReleaseArena(Arena * a)
{
  // free buff
  free(a->buffer);

  // free arena itself
  free(a);
}// end of ReleaseArena (~destructor)


void* ArenaPush(Arena* arena, i64 size)
{
  i64 req_plus_used = size + arena->used_capacity; 
  if (req_plus_used > arena->max_capacity){
    perror("trying to request more than available");
    return NULL;
  }

  char* current_ptr = &(arena->buffer[arena->ptr]); // return this in a bit
  arena->ptr += size;
  arena->used_capacity += size;
  return current_ptr; 
}// end of ArenaPush


void* ArenaPushZeroes(Arena* arena, i64 size)
{

  i64 req_plus_used = size + arena->used_capacity; 
  if (req_plus_used > arena->max_capacity){
    perror("trying to request more than available");
    return NULL;
  }

  i64 start_num = arena->ptr;

  char* current_ptr = &(arena->buffer[arena->ptr]);

  for (i64 i = start_num; i  < arena->ptr; i++){
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
  if (used_minus_size < 0){
    remove_too_much = 1;  
    delete_size = arena->used_capacity;
  }
  i64 stop_index = arena->ptr - delete_size;

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
  Arena * my_arena  = ArenaAlloc();
  ArenaDebugPrint(my_arena);
  char* name = (char* ) ArenaPush(my_arena, 20);
  printf("allocating 20 bytes for my name\n");
  printf("bytes used:  %lld\n", GetArenaSize(my_arena));

  strcpy(name, "esteban!");
  ArenaDebugPrint(my_arena);

  Vec2* vector = ArenaPush(my_arena, (i64) (sizeof(Vec2) ));
  printf("allocating my vector\n");
  printf("bytes used:  %lld\n", GetArenaSize(my_arena));

  printf("popping off 1 vec2 \n");
  ArenaPop(my_arena, (i64) (sizeof(Vec2)));   
  printf("bytes used:  %lld\n", GetArenaSize(my_arena));

  vector = ArenaPush(my_arena, (i64) (sizeof(Vec2) ));
  printf("allocating my vector again\n");
  printf("bytes used:  %lld\n", GetArenaSize(my_arena));
  
  printf("popping off 20 bytes\n");
  ArenaPop(my_arena, 20);

  printf("bytes used:  %lld\n", GetArenaSize(my_arena));
  
  printf("popping off 20 bytes\n");
  ArenaPop(my_arena, 20);

  printf("bytes used:  %lld\n", GetArenaSize(my_arena));

  ArenaDebugPrint(my_arena);

  ArenaPushZeroes(my_arena, 22);

  ArenaDebugPrint(my_arena);
  
  ReleaseArena(my_arena);
  Arena* a = ArenaAlloc();

  char* e = ArenaPush(a, 60);
  memset(e, 'e', 60);
  ArenaDebugPrint(a);
  ArenaPop(a, 60);
  ArenaDebugPrint(a);

  ReleaseArena(a);
  
}

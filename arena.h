// @author: esteban morales


typedef struct {
  char* buffer;
  i64 max_capacity;
  i64 used_capacity;
  i64 ptr;
} Arena;


// constructor 
Arena* ArenaAlloc(void );

// ~destructor
void ReleaseArena(Arena * a); 

// realloc (double size if full)
void ArenaRellaoc(Arena * a);
//push bytes
void* ArenaPush(Arena* arena, i64 size);

// push zeroe'd bytes
void* ArenaPushZeroes(Arena* arena, i64 size);

//pop bytes from arena
void ArenaPop(Arena* arena, i64 size);

//get num bytes used currentlty
i64 GetArenaSize(Arena* arena);


 

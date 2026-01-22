typedef struct {
  char* buffer;
  u64 max_capacity;
  u64 used_capacity;
  u64 ptr;
} Arena;


// constructor 
Arena* ArenaAlloc(void );

// ~destructor
void ReleaseArena(Arena * a); 

//push bytes
void* ArenaPush(Arena* arena, u64 size);

// push zeroe'd bytes
void* ArenaPushZeroes(Arena* arena, u64 size);

//pop bytes from arena
void ArenaPop(Arena* arena, u64 size);

//get num bytes used currentlty
u64 GetArenaSize(Arena* arena);


 

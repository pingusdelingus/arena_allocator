// @author: esteban morales


typedef struct ArenaBlock {
  struct ArenaBlock* next;
  struct ArenaBlock* prev;

  char* buffer;
  i32 max_capacity;
  i32 ptr;
  b8 isFull;
} ArenaBlock;



// constructor 
ArenaBlock* AllocArenaBlock(void );

// ~destructor
void ReleaseArenaBlocks(ArenaBlock * a); 

//push bytes
void* ArenaBlockPush(ArenaBlock* arena, i32 size);

// push zeroe'd bytes
void* ArenaBlockPushZeroes(ArenaBlock* arena, i32 size);

//pop bytes from arena
void ArenaBlockPop(ArenaBlock* arena, i32 size);

//get num bytes used currentlty
i32 GetArenaBlockSize(ArenaBlock* arena);

// --------------------------------------------------------------------------------
typedef struct{
  ArenaBlock* current;
  ArenaBlock* genesis;
} Arena;


Arena* ArenaConstruct(void);
void ArenaDestruct(Arena* a);

void* ArenaPush(Arena* a, i32 size);
void* ArenaPushZeroes(Arena* a, i32 size);

void ArenaPop(Arena* a, i32 size);
i32 GetArenaSize(Arena* a);


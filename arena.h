// @author: esteban morales


typedef struct ArenaBlock {
  struct ArenaBlock* next;
  struct ArenaBlock* prev;
  char* buffer;
  i64 max_capacity;
  i64 used_capacity;
  i64 ptr;
  b8 isFull;
} ArenaBlock;

typedef struct{
  ArenaBlock* current;
  ArenaBlock* genesis;
} Arena;


// constructor 
ArenaBlock* AllocArenaBlock(void );

// ~destructor
void ReleaseArenaBlocks(ArenaBlock * a); 

//push bytes
void* ArenaBlockPush(ArenaBlock* arena, i64 size, b8 doSizeCheck);

// push zeroe'd bytes
void* ArenaBlockPushZeroes(ArenaBlock* arena, i64 size, b8 doSizeCheck);

//pop bytes from arena
void ArenaBlockPop(ArenaBlock* arena, i64 size);

//get num bytes used currentlty
i64 GetArenaBlockSize(ArenaBlock* arena);


 


Arena* ArenaConstruct(void);
void ArenaDestruct(Arena* a);

void* ArenaPush(Arena* a, i64 size);
void* ArenaPushZeroes(Arena* a, i64 size);

void ArenaPop(Arena* a, i64 size);
i64 GetArenaSize(Arena* a);


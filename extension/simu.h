//static pthread_mutex_t simu_maintainer_lock = PTHREAD_MUTEX_INITIALIZER;
/*
typedef struct
{

} fake;
*/

#include "extension/lru.h"
#include "extension/lfu.h"

//static int sample_size = 10000;

typedef struct
{
  double flash_write;
  double flash_read;
  double dram_write;
  double dram_read;
} latencies;

typedef struct
{
  lru_item* it;
  uint32_t hash_val;
} request;

typedef struct
{
  int hits;
  int operations;
} lru_stats;


typedef struct
{
  int x; // number of experiment
  lru *dram_lru; // dram lru
  lru *global_lru; // global lru
  lfu *dram_lfu; // dram lfu
  lfu *global_lfu; //global_lfu
  lru_item (*features)[];
  int count = 0;
  int sample_size = 10000;
} open_policy;

typedef struct
{
  double flash_size; // size of the flash cache
  double dram_size; // size of the dram cache
} system_features;

typedef struct
{
  lru_stats *stats;
  latencies *times;
  open_policy *open;
  system_features *feats;
} simu_data;

//int alloc_simu(item* it, uint32_t hash_val);
void registerAlloc(item* it, uint32_t hash_val);
void *start_simu_maintainer_thread();
/*
int start_lru_simulation_thread(void *arg);
int stop_simulation_thread(void);
int init_simulation_maintainer(void);
void lru_simulation_pause(void);
void lru_simulation_resume(void);
*/

//void* simu_maintainer_thread(void *arg);
void would_hit(lru *l, lru_item *it, lru_stats *stats);
//void *start_simu_maintainer_thread(void *arg);
int stop_simu_maintainer_thread(void);
int start_simulation_thread();
void* simu_consumer(void *arg);
void fill_in_latencies(latencies *l, char *c, double val);

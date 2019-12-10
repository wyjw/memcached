#include "memcached.h"
#include "extension/lru.h"

void registerAlloc(item *it, uint32_t hash_value)
{

}

// This method is for allocaitng memory on the simu
alloc_simu(item *it, uint32_t hash_value) {
  printf("Alloc'ed in simulation\n");
  if (hash_value % 1000 == 1)
  {
    registerAlloc(item *it, );
  }
}

int start_simulation_thread(void * arg) {
  int ret;

  pthread_mutex_lock();


}

int stop_lru_maintainer_thread(void)
{

}

static void *lru_maintainer_thread(void *arg)
{
  
}

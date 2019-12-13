#include "memcached.h"
#include <string.h>
#include "simu.h"
#include <pthread.h>

static pthread_mutex_t simu_maintainer_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t nonEmpty = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX 100000
#define BUFLEN 50000
#define NUMTHREAD 3
#define MAX_SIMU_MAINTAINER_SLEEP 300

request buffer[BUFLEN];
request source[BUFLEN];

static int i = 0;
static int j = 0;
static int rCount = 0, wCount = 0;
static int do_run_simu_maintainer_thread = 1;
static pthread_t simu_maintainer_tid;
static pthread_t simu_consumer_tid;
static void *simu_maintainer_thread(void *arg);
static int to_sleep = 1;

void would_hit(lru *l, lru_item *it, lru_stats *stats);
/*
 * Let's do this by considering it a problem of producer-consumer.
 * The producer passes in the keys that have the correct hash value mod.
 */
/*
typedef struct
{
  double dram_read; //dram_read
  double dram_write; //dram_write
  double flash_read; //flash_read
  double flash_write; //flash_write
} latencies;
*/
/* This example is rewritten based on the example from */
/* <http://www.coe.uncc.edu/~abw/parallel/pthreads/pthreads.html */
/* It illustrates the idea of shared memory programming */
/* oct-03-2000 */

void fill_in_latencies(latencies *l, char *c, double val)
{
  if (c == "dr"){
    l->dram_read = val;
  }
  else if (c == "dw"){
    l->dram_write = val;
  }
  else if (c == "fr"){
    l->flash_read = val;
  }
  else {
    l->flash_write = val;
  }
}

/*
// This method is for allocating memory on the simu
alloc_simu(item *it, uint32_t hash_value) {
  printf("Alloc'ed in simulation\n");
  if (hash_value % 1000 == 1)
  {
    registerAlloc(it, hash_value);
  }
}*/

/*
int start_simulation_thread(void * arg) {
  int ret;
  pthread_mutex_lock(&simu_maintainer_lock);

  pthread_mutex_lock(&simu_maintainer_lock);
}
*/

// Stop the simu pthreads by join
int stop_simu_maintainer_thread(void)
{
   int ret;
   pthread_mutex_lock(&simu_maintainer_lock);
   do_run_simu_maintainer_thread = 0;
   pthread_mutex_lock(&simu_maintainer_lock);
   if ((ret = pthread_join(simu_maintainer_tid, NULL)) != 0) {
      fprintf(stderr, "Failed to stop simu thread!");
      return -1;
   }
   settings.lru_maintainer_thread = false;
   return 0;
}

// Start the simu pthreads by pthread_create
void *start_simu_maintainer_thread()
{
   printf("Created simu maintainer thread.\n");
   char *w;

   //int malloc = ITEM_SIZE + nkey + nvalue + 1;
   //par = arg->experiment_parameters;
   //arg->simulated_time;
   simu_data *datum = malloc(sizeof(simu_data));
   lru *global_l;
   lru *dram_l;
   datum->open = malloc(sizeof(open_policy));
   int MAX_SIZE = 256 * 1024 * 1024;
   datum->open->global_lru = lru_init(MAX_SIZE, 0);
   datum->open->dram_lru = lru_init(MAX_SIZE, 0);
   //arg->open->global_lfu = lru_init(malloc, 0);
   //arg->open->dram_lfu = lru_init(malloc, 0);

   latencies *l = malloc(sizeof(latencies));
   l->flash_write = 0.0002;
   l->flash_read = 0.0005;
   l->dram_write = 0.0005;
   l->dram_read = 0.0006;
   datum->times = l;

   system_features *m = malloc(sizeof(system_features));
   m->flash_size = 100000000.0;
   m->dram_size = 100000000.0;
   datum->feats = m;

   lru_stats *ls = malloc(sizeof(lru_stats));
   ls->hits = 0;
   ls->operations = 0;
   datum->stats = ls;
   /*
   size_t sz;
   char buf[256];
   for (i = 0; i < loop; i++) {
     r = item_set(1, key[i], nkey, value, nvalue);
   }
   */
   int ret = 0;
   pthread_mutex_lock(&simu_maintainer_lock);
   if ((ret = pthread_create(&simu_maintainer_tid, NULL,
				   simu_maintainer_thread, (void *) datum)) != 0){
      fprintf(stderr, "Can't create LRU maintainer thread: %s\n",
		      10);
      pthread_mutex_unlock(&simu_maintainer_lock);
      return -1;
   }
   /*
   if ((ret = pthread_create(&simu_maintainer_tid, NULL,
				   simu_maintainer_thread, (void *) datum)) != 0){
      fprintf(stderr, "Can't create LRU maintainer thread: %s\n",
		      10);
      pthread_mutex_unlock(&simu_maintainer_lock);
      return -1;
   }
   */
   pthread_mutex_lock(&simu_maintainer_lock);
   return 0;
}

/*
void *simu_producer(int *id)
{
  while(i < MAX)
  {
     pthread_mutex_lock(&count_mutex);
     strcpy(buffer, "");
     buffer[wCount] = source[wCount % BUFLEN];

     printf("%d produced by %c by %d\n", i, buffer[wCount], *id);
     fflush(stdout);

     wCount = (wCount + 1) % BUFLEN;
     i++;
     pthread_cond_signal(&nonEmpty);
     pthread_mutex_unlock(&count_mutex);

     // last sleep might leave the condition un-processed
     if (i < (MAX - 2))
       if (rand() % 100 >= 30)
	      sleep(rand()%3);
  }
}
*/

static size_t lru_item_make_header(const uint8_t nkey, const unsigned int flags, const int nbytes,
                     char *suffix, uint8_t *nsuffix)
{
  return sizeof(lru_item) + nkey + *nsuffix + nbytes;
}

void registerAlloc(item *it, uint32_t hash_value)
{
  //printf("Register here\n");
  //if (hash_value % 1000 == 5) {
    pthread_mutex_lock(&count_mutex);
    //printf("Got here registered.\n");
    //strcpy(buffer, "");
    request r;
    lru_item *lrit = malloc(ITEM_ntotal(it));
    lrit->nbytes = it->nbytes;         /* size of total items, ITEM_size + nkey + 1 + nvalue*/
    lrit->nkey = it->nkey;       /* key len, not include terminal null */
    //printf("WE have nbytes as %d and %s", it->nbytes, LRU_ITEM_data(it));
    //strcpy(LRU_ITEM_data(lrit), LRU_ITEM_data(it));
    strcpy(LRU_ITEM_key(lrit), ITEM_key(it));
    char *c = ITEM_data(it);
    strcpy(LRU_ITEM_data(lrit), c);
    r.it = lrit;
    r.hash_val = hash_value;
    buffer[wCount] = r;
    wCount = (wCount + 1) % BUFLEN;

    if (wCount % 1000 == 0) {
      pthread_cond_signal(&nonEmpty);
    }
    pthread_mutex_unlock(&count_mutex);
  //}
}


void *do_calculation(simu_data *datum, request *buf, int sizeofbuf)
{
  for (int i = 0; i < sizeofbuf; i++)
  {
    request _r = buf[i];
    would_hit(datum->open->global_lru, _r.it, datum->stats);
  }
  printf("Final stats during our simu are: %d,%d\n", datum->stats->hits, datum->stats->operations);
}

void *simu_consumer(void *arg)
{
  pthread_mutex_lock(&count_mutex);
  simu_data* datum = (simu_data *)arg;
  int len_of_buf = 500;
  while(j < MAX)
  {
    pthread_cond_wait(&nonEmpty, &count_mutex);
    rCount = (rCount + 500) % BUFLEN;
    request out_buf[len_of_buf];
    if (rCount > BUFLEN - 500 && rCount < BUFLEN)
    {
      len_of_buf = BUFLEN - rCount;
    }
    memcpy(out_buf, &buffer[rCount], len_of_buf * sizeof(request));
    do_calculation(datum, out_buf, len_of_buf);
    j += len_of_buf;
  }
  //if (j < (MAX - 2))
  //  if (rand() % 100 < 30)
  //    sleep(rand() % 3);
  pthread_mutex_unlock(&count_mutex);
}

static void *simu_maintainer_thread(void *arg)
{
  simu_data* datum = (simu_data *)arg;
  int last_sleep;
  while (do_run_simu_maintainer_thread) {
    pthread_mutex_unlock(&simu_maintainer_lock);
    simu_consumer(arg);
    // This is where we add our stats of how many items goes here
    //stats.simu_maintainer_juggles++;
    pthread_mutex_lock(&simu_maintainer_lock);
  }
}

void would_hit(lru *l, lru_item *it, lru_stats *lstats) {
   //key = ITEM_KEY(it);
   lstats->operations++;
   size_t sz;
   if (lru_item_get(l, LRU_ITEM_key(it), it->nkey, LRU_ITEM_data(it), STAT_KEY_LEN, &sz) != 0)
   {
     lru_item_set(l, LRU_ITEM_key(it), it->nkey, LRU_ITEM_data(it), it->nbytes - 1);
   }
   else
   {
     lstats->hits++;
   }
   printf("WE HAVE %d operations and %d hits\n", lstats->operations, lstats->hits);
   //if (lstats->operations % 1000 == 0)
   //{
   // printf("WE HAVE %d operations and %d hits\n", lstats->operations, lstats->hits);
   //}
}

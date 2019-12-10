static pthread_mutex_t simulation_maintainer_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{

} fake;

int alloc_simu(item* it, uint32_t hash_val);
int registerAlloc();


int start_simulation_thread(void *arg);
int stop_simulation_thread(void);
int init_simulation_maintainer(void);
void lru_simulation_pause(void);
void lru_simulation_resume(void);

void pool_init (int max_thread_num);
int pool_add_worker (void *(*process) (void *arg), void *arg);
int pool_destroy ();
void *thread_routine (void *arg);
void *myprocess (void *arg);


 
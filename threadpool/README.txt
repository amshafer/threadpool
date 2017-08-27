This is threadpool for fun and general use.

Example usage: -----------------------------------------------

// argument struct
struct print_t;
// function to be run by threadpool
void print_func(void *arg);

struct print_t *pt;
pool_t *pl = pool_init(number_of_threads);

// tell threadpool to execute function with argument arg
pool_exec(pl, &print_func, pt);

pool_destroy(pl);

-------------------------------------------------------------

The threadpool is created using pool_init. Threadpools do not use global variables
so multiple pools can be created without issue. This is usually not useful and 
probably shouldn't be done. 

Jobs are posted to the threadpool using the pool_exec call. This adds the function 
and its argument struct to the work queue. All waiting threads are signaled that 
work is now available. The first thread to wake up will run the function and wait
for the queue to have more work.

Destroying the threadpool involves waiting for all spawned threads to finish. This
is done by setting the atomic flag pa_kill in the threadpool structure and 
broadcasting to all waiting threads. As the threads wake up they check if pa_kill is 
set. If this is true, the thread will release all locks and exit. The destroy 
function will free the pool structure only when the number of threads has returned
to zero.

This is done so that the threadpool does not waste memory holding an array of all
spawned thread id's. This is a particular advantage when the number of threads
reaches an absurd number (such as 1000). 

The work queue currently uses one lock to synchronize and make operations thread safe.
In the future this will be changed to an atomic queue to avoid the use of locks. The
threadpool lock is simply used to make child threads wait for available work.

#include <apophenia/headers.h>

gsl_rng *r;

typedef struct {
    char type;
    int wealth;
    int id;
} bird;

void play_pd_game(bird *row, bird *col);
bird *new_chick(bird *parent);
void bird_dies(bird *in);
void birth_or_death(void *in, void *v);
void startup(int initial_flock_size);
void bird_plays(void *in, void *v);


//These must be provided by the main program:
void flock_init();
void free_bird(bird*);
void add_to_flock(bird*);
void flock_plays();
int flock_size();
void cull_flock();
void count(int);
bird * find_opponent(int);

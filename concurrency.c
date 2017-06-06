/*
    
   A C-program for MT19937, with initialization improved 2002/2/10.
   Coded by Takuji Nishimura and Makoto Matsumoto.
   This is a faster version by taking Shawn Cokus's optimization,
   Matthe Bellew's simplification, Isaku Wada's real version.

   Before using, initialize the state by using init_genrand(seed) 
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */
static int isopen =0;

typedef struct {
  int position;
  int count;
  sem_t *forks;
  sem_t *lock;
} info;


void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] = 
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

void init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(time(0));
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}



/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }
  
    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}


int grab_rand_num(id){
   unsigned long init[4]={0x123, 0x234, 0x345, 0x456}, length=4;
   init_by_array(init, length);
   int random_num;
   if(id==2){

      random_num = (genrand_int32() %8) +2;
   }
   else{
       random_num = genrand_int32();
   }
   return random_num;
}



void think(int position, const char* philosopher_name)
{
  int sleep_num = (genrand_int32()%20)+1;
  printf("Philosopher %s thinks about pikachu for %d seconds\n", philosopher_name, sleep_num);
  sleep(sleep_num);
  printf("Philosopher %s is done thinking about pikachu\n", philosopher_name);
  
}




void eat(int position, const char* philosopher_name)
{
  int eat_time = (genrand_int32()%10)+1;
  printf("Philosopher %s eating noodles for %d seconds\n", philosopher_name, eat_time);
  sleep(eat_time);
  printf("Philosopher %s done eating his noodles and puts forks down\n",philosopher_name);
  
}




void *philosopher(void *params)
{
for(;;){
  int i;
  info self = *(info *)params;
  
  const char* philosopher_name;
  switch(self.position){
	  case 0:
		philosopher_name = "Socrates";
		break;
	  case 1:
		philosopher_name = "Einstein";
		break;
	
	   case 2:
		philosopher_name = "Dijkra";
		break;
		
	  case 3:
		philosopher_name = "Walt Disney";
		break;
	  
	  case 4:
		philosopher_name = "Ghandi";
		break;
	  
	  default :
		printf("noooope\n");
  }
  for(i = 0; i < 3; i++) {
    think(self.position, philosopher_name);

    sem_wait(self.lock);
    sem_wait(&self.forks[self.position]);
    printf("%s grabs 2 forks to start eating noodles",philosopher_name);
    sem_wait(&self.forks[(self.position + 1) % self.count]);
    eat(self.position,philosopher_name);
    sem_post(&self.forks[self.position]);
    sem_post(&self.forks[(self.position + 1) % self.count]);
    printf("%s grabs 2 forks to start eating noodles", philosopher_name);
    sem_post(self.lock);
  }
  
  think(self.position, philosopher_name);
  
  pthread_exit(NULL);
}
}

void start_semaphore_threads(pthread_t *threads, sem_t *forks, sem_t *lock, int num_philosophers)
{
  int i;
  for(i = 0; i < num_philosophers; i++) {
    info *arg = malloc(sizeof(info));
    arg->position = i;
    arg->count = num_philosophers;
    arg->lock = lock;
    arg->forks = forks;

    pthread_create(&threads[i], NULL, philosopher, (void *)arg);
  }
}


int main(int argc, char *args[])
{
  int num_philosophers = 5;

  sem_t lock;
  sem_t forks[num_philosophers];
  pthread_t philosophers[num_philosophers];
  int i=0;
  for(i; i < num_philosophers; i++) {
    sem_init(&forks[i], 0, 1);
  }
   
  sem_init(&lock, 0, num_philosophers - 1);

  
  start_semaphore_threads(philosophers, forks, &lock, num_philosophers);
  pthread_exit(NULL);
}

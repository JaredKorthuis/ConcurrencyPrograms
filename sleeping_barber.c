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
/**********************************************************************
 * NAME: Jared Korthuis                                               *
 * Date: 11/16/2015                                                   *
 * Due: 11/18/2015                                                    *
 * RUN: after make: ./part2                                           *
 * Description: sleeping barber situation where there is one reluctant*
 * barber that only wants to sleep, he will cut hair but only for thos*
 * who are not faint at heart. do you have what it takes?             *
 **********************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */
static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */
static int isopen =0;
void *customer(void *num);
void *barber(void *);



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
            init_genrand(time(0)); /* a default initial seed is used */

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

sem_t waiting_queue;
sem_t barberChair;
sem_t barber_sleeping;
sem_t Chair_strap_on;
int complete_count =0;


void *customer(void *number){
	int customer_id = *(int *)number;
    
	sem_wait(&waiting_queue);
	
	printf("customer%d has entered the waiting room \n", customer_id);
    sem_wait(&barberChair);
	printf("customer %d gives up position in waiting room and enters the chair\n",customer_id);
    
    
	sem_post(&waiting_queue);
    
	printf("customer %d slaps the barber into a ready to cut freak\n",customer_id);
	sem_post(&barber_sleeping);
	sem_wait(&Chair_strap_on);
	sem_post(&barberChair);
	printf("customer is done getting hair butchered\n");
	
}

void *barber(void *info){
	int cutting_time=0;
	while(1){
		cutting_time=(genrand_int32()%9+3);
		printf("the barber is sleeping peacefully in his chair\n");
		sem_wait(&barber_sleeping);
		printf("The barber is cutting the customers hair and will take him %d seconds\n",cutting_time);
		sleep(cutting_time);
		printf("the barber is done butchering the hair because he would rather sleep than do a good job\n");
		sem_post(&Chair_strap_on);
	}
}




int main(int argc, char *args[]){
pthread_t btid;
int rand_num_customers = (genrand_int32()%18+7);
pthread_t tid[rand_num_customers];
printf("there are a total of %d customers that will enter the barber shop\n",rand_num_customers);
long RandSeed;
int i;
int customer_counter= rand_num_customers;
int chair_counter = (genrand_int32()%rand_num_customers +1);
int num[rand_num_customers];

for(i=0;i<rand_num_customers;i++){
	num[i]=i;

}

sem_init(&waiting_queue,0,chair_counter);
printf("there are %d available spots in the waiting room\n", chair_counter);
sem_init(&barberChair,0,1);
sem_init(&barber_sleeping,0,0);
sem_init(&Chair_strap_on,0,0);
pthread_create(&btid, NULL, barber, NULL);

for(i=0;i<rand_num_customers;i++){
   pthread_create(&tid[i], NULL, customer, (void *) &num[i]);
}
//pthread_create(&btid, NULL, barber, NULL);
for(i=0;i<rand_num_customers;i++){
	pthread_join(tid[i],NULL);
}
complete_count=1;
sem_post(&barber_sleeping);
pthread_join(btid,NULL);

}

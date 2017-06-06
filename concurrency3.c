#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define RAND (rand()%10+1)

pthread_mutex_t lock_searcher;
pthread_mutex_t lock_deleter;
pthread_mutex_t lock_inserter;





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
/*************************************
* NAME: Jared Korthuis               *
* Project: concurrency3.c            *
* run: gcc -pthread concurrency3.c   *
* run: use make and then ./con       *
* EMAIL: korthuij@oregonstate.edu    *
* DUE DATE: 11/5/2015                *
**************************************/


struct linkList {
    int data;
	int index;
    struct linkList *next;
}*head;


/***********SEARCHER LOGIC****************************************************************************
* if the pthread isn't locked set the current to the head check to see if the list is empty          *
* if the thread isn't empty the searcher iterates up the list until the value is NULL (hence the top)*
*  once the searcher is done going through the string it unlocks and sleeps                          *
******************************************************************************************************/

void searcher () {
    
    struct linkList *current;
    int sleep_num = (genrand_int32()%9)+2;
	pthread_t thread_id;
    thread_id=pthread_self();
	//printf("in searcher first\n");
    for(;;) {
      
       if (!pthread_mutex_trylock(&lock_searcher)){
          current = head;
          if (current == NULL){
             printf("Searcher [%u] has found nothing in the list\n", thread_id);
             continue;
          } 
		  else {
             printf("searcher [%u] found current string to be: ",thread_id);
             while (current != NULL){
				
                printf(" [%d] ", current->data);
                current=current->next;
             }
			 printf("\n");
          }
            pthread_mutex_unlock(&lock_searcher);
       }
       printf("searcher [%u] has made it to end of the list and is now going to sleep for %d seconds\n",thread_id, sleep_num);
       sleep(sleep_num);
       
    }
}


/*****************INSERTER LOGIC*******************************************************************
* First it will check the mutex of the other inserters to see if it is allowed to go into the list*
* if the list is available for iteration it mallocs memory into the list prepping for insertion   *
* It then inserts the random value into the string.                                               *
* current's next position then gets NULL and the tail gets the head.                              *
* if the head is null then the head gets the current position                                     *
* if the head is not null then the tail will loop through the list until it is the head           * 
**************************************************************************************************/


void inserter () {
    struct linkList *current, **tail;
    int insert_this_value;
	pthread_t thread_id;
    thread_id=pthread_self();
	
	//printf("in inserter first\n");
    for(;;){
		int sleep_num=(genrand_int32()%9)+2;
        
		if (!pthread_mutex_lock(&lock_inserter)){
		   current = (struct linkList *)malloc(sizeof(struct linkList));
		   insert_this_value = (genrand_int32()%15)+2;
		   

		   printf("[%u] inserting: {%d} \n",thread_id, insert_this_value);
		   current->data = insert_this_value;
		   current->index = insert_this_value;
		   
		   current->next = NULL;
		   tail = &head;
		   if (head == NULL){
		      head = current;
		   } 
		   else {
		      while (*tail != NULL){
			     tail = &((*tail)->next);
			  }
			  *tail = current;
		   }
			

		   pthread_mutex_unlock(&lock_inserter);
		   printf("[%u]inserter done inserting and going to sleep for: %d seconds\n",thread_id,sleep_num);
		   sleep(sleep_num);
		}
		else{
			printf("lock_inserter in use!!!!!!!!!!!!\n");
		}
        
    }
}
     /**************************DELETER LOGIC ***************************************************************************************************      
	  *  if the list is not empty the deleter will check to see if the lock_inserter and lock_searcher mutex is locked                          *
	  * if it is not locked then it will go into the linked list and be given a value to delete.                                                *
	  * it will iterate to the list until it reaches the head (NULL) and then it will end                                                       * 
	  * if it finds the value to delete in the list and it happens to be at the head, it will move the head up one and then free                *
	  * the space that it was occupying.                                                                                                        *
	  * If the deleter thread finds the value and it is not at the head it sets previous next to current->next skipping over the space          *
	  * and then frees current position from memory                                                                                             * 
	  * WARNING: I have written the value to delete the value from the string once so if it is a string full of 3's it will only delete 1 three *
	  *******************************************************************************************************************************************/

void deleter () {
   int delete_this_value;
   int sleep_num =(genrand_int32()%9)+2;
   pthread_t thread_id;
   thread_id=pthread_self();
   struct linkList *current, *prev, *list_check;
   list_check = head;
   printf("deleter thread[%u] beginning to delete\n", thread_id);
   for (;;){
      
	  if (list_check != NULL){
         
		 if (!pthread_mutex_trylock(&lock_inserter)){
            
			if(!pthread_mutex_trylock(&lock_searcher)){
               current = head;
              
			   
               while (current != NULL) {
                   delete_this_value = current->index;
				  if(current->data == delete_this_value && current == head){
                     printf("Deleter[%u] deleted: %d from head of the list\n",thread_id, delete_this_value);
					 head = current->next;
					 free(current);
					 break;
				  }
				  
			      else if(current->data == delete_this_value && current != head ){
					 printf("Deleter[%u] deleted %d from list\n",thread_id, delete_this_value);
                     prev->next = current->next;
                     free(current);
                     break;
				  }
                   
				  else {
                     prev = current;
                     current = current->next;
                  }
               }
               pthread_mutex_unlock(&lock_searcher);
            }
         pthread_mutex_unlock(&lock_inserter);
         }
	  printf(" deleter[%u] is going to sleep for %d second\n",thread_id, sleep_num);
	  
      sleep(sleep_num);
	  printf("deleter[%u] is done sleeping\n", thread_id);
       }
   }
}

int main(){
   int i = 0;
   int rand_num_threads = (genrand_int32()%9)+2;
   printf("this will be running with [%d] searcher, inserter and deleter threads\n", rand_num_threads);
   
   printf(".\n");
   struct linkList *my_string;
   my_string = (struct linkList *)malloc(sizeof(struct linkList));
   my_string->data = 0;
   head = my_string;
   head->next = NULL;
   

        
   pthread_t search_thread[rand_num_threads];
   pthread_t insert_thread[rand_num_threads];
   pthread_t delete_thread[rand_num_threads];

   for(i = 0; i <  rand_num_threads; ++i){
	   printf("rand_num_threads: %d",rand_num_threads);
    //create rand_num_threads for inserter, searcher, and deleter     

      if(pthread_create(&insert_thread[i], NULL, (void *)inserter, (void *)NULL)) {
         printf("Error creating inserter thread\n");
         return 1;
      }
      sleep(2);
            
      if(pthread_create(&search_thread[i], NULL, (void *)searcher, (void *)NULL)) {
         printf( "Error creating searcher thread\n");
         return 1;
      }
      sleep(2);
     
      if(pthread_create(&delete_thread[i], NULL, (void *)deleter, (void *)NULL)) {
         printf("Error creating deleter thread\n");
         return 1;
      }
    }
    for(i = 0; i < rand_num_threads; ++i){
       pthread_join(search_thread[i], NULL);
       pthread_join(insert_thread[i], NULL);
       pthread_join(delete_thread[i], NULL);
    }
   return 0;
}

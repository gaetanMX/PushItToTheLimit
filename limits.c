#include <sys/types.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#define ITERATION_FILE  10000
#define ITERATION_CPU   10000000
#define DEFAUT_MAX_PRIO 19
#define DEFAUT_MAX_TMP  20
#define TIME_SLEEP      10

// The void function work_file writes a string to a temporary file ITERATION_FILE times
void work_file(void)
{
   FILE *f;
   int i;
   
    f = tmpfile( );
    for(i = 0; i < ITERATION_FILE; i++)  {
        fprintf(f, "Do some output\n");
        if(ferror(f)) {
             fprintf(stderr, "Error writing to temporary file\n");
             exit(1);
        }
    }
}

// performs some arithmetic to generate load on the CPU
void work_cpu(void)
{     
    int i;   
    double x = 4.5;
    
    for(i = 0; i < ITERATION_CPU; i++)        
        x = (x*x + 3.6); 
}

void work_nbproc(int nbproc_a_creer)
{
   int statut;
   int nbproc = 0;
   pid_t pid;
   
   while (nbproc < nbproc_a_creer){ //dépasse obligatoirement la limite
    switch(pid=fork()){
	case -1 : perror("Probleme fork"); exit(1);
	case 0: 	  
	  printf("processus fils n°%d attend %d secondes\n",nbproc,TIME_SLEEP);
	  sleep(TIME_SLEEP);
	  exit(0);
	default : //processus pere
	  nbproc++;
	}
   }
   waitpid(pid, &statut, 0);
}

void work_core(void) //marche pas
{
   int statut;
   int nbproc = 0;
   pid_t pid;
   
       switch(pid=fork()){
	case -1 : perror("Probleme fork"); exit(1);
	case 0: 	  
	  printf("processus fils plante\n");
	  int *numdie;
	  numdie[23] = 23;
	  exit(0);
	default : //processus pere
	  waitpid(pid, &statut, 0);
	}
 	prctl();
  
  
}


// The main function calls work and then uses the getrusage function to discover how much CPU time it
// has used. It displays this information on the screen.

int main(void)  /*TODO: modifier les args pour les différents tests, je ferais un switch et tt pour que ce soit clean */
{
   //TODO: tester aussi le fork et execve
   
   /* sous les distributions linux "normales" déjà fait pour le cpu : struct rusage r_usage;
    toujours dans sys/ressource.h */
   
   struct rlimit r_limit;
   int priority;   /* prioriete du processus */

       /* Test RLIMIT_CPU */  
/*    
   getrlimit(RLIMIT_CPU, &r_limit);
   // ou getrusage, a voir comment on implémentera
   printf("Current CPU limit: %ld\n\n",r_limit.rlim_cur);
   // etc...
   
 /*  getrusage(RUSAGE_SELF, &r_usage);    
   printf("CPU usage: User = %ld.%06ld,  System = %ld.%06ld\n",
     r_usage.ru_utime.tv_sec, r_usage.ru_utime.tv_usec,
     r_usage.ru_stime.tv_sec, r_usage.ru_stime.tv_usec);
     work_cpu();
 */


/* Test RLIMIT_NICE*/
   getrlimit(RLIMIT_NICE, &r_limit);
   printf("Current NICE limit: %ld\n",r_limit.rlim_cur);
   priority = getpriority(PRIO_PROCESS, getpid()); 
   printf("Current priority of process = %d\n", priority);
   r_limit.rlim_cur = 5; //TODO:faire un scanf plus tard
   printf("Setting a 5 nice limit\n");
   setrlimit(RLIMIT_NICE, &r_limit);
   printf("New NICE limit: %ld\n",r_limit.rlim_cur);
 
   printf("set priority to %d (>5)\n",DEFAUT_MAX_PRIO); //scanf aussi pour MAX_PRIO
   setpriority(PRIO_PROCESS,getpid(),DEFAUT_MAX_PRIO);
   priority = getpriority(PRIO_PROCESS, getpid()); 
   printf("New priority = %d\n\n", priority);

     

   
   /* Test RLIMIT_NPROC[OK] */
   getrlimit(RLIMIT_NPROC, &r_limit);
   printf("Current NPROC limit: %ld\n",r_limit.rlim_cur);
   r_limit.rlim_cur = 132; //TODO:faire un scanf plus tard
   printf("Setting a 5 nbproc limit\n");
   setrlimit(RLIMIT_NPROC, &r_limit);
   printf("New NPROC limit: %ld\n", r_limit.rlim_cur);
   printf("Creation de %ld process:\n", r_limit.rlim_cur+1);   
   work_nbproc(r_limit.rlim_cur+1);

		
   /* Test RLIMIT_NOFILE */
   getrlimit(RLIMIT_NOFILE, &r_limit);
   printf("Current NOFILE limit: %ld\n\n",r_limit.rlim_cur);
   
   /* Test RLIMIT_CORE */
   getrlimit(RLIMIT_CORE, &r_limit);
   printf("Current CORE limit: %ld\n",r_limit.rlim_cur);
   r_limit.rlim_cur = 1024; //TODO:faire un scanf plus tard
   r_limit.rlim_max = 2048;
   printf("Setting a 1024 core limit\n");
   setrlimit(RLIMIT_CORE, &r_limit);
   printf("New CORE limit: %ld\n",r_limit.rlim_cur);
   printf("Producing CORE FILE > CORE limit \n");
   work_core();
   
/* Test RLIMIT_FSIZE[OK] */
/* Finally, we set a file size limit using setrlimit and call work again, which fails because it
 attempts to create a too large file */
  
   getrlimit(RLIMIT_FSIZE, &r_limit);
   printf("Current FSIZE limit: soft = %ld, hard = %ld\n",
       r_limit.rlim_cur, r_limit.rlim_max); /*-1 -1 si infinie */  
   r_limit.rlim_cur = 2048;
   r_limit.rlim_max = 4096;
   printf("Setting a 2k file size limit\n");
   setrlimit(RLIMIT_FSIZE, &r_limit);
   work_file(); /*on essaye de dépasser la nouvelle limite*/ 
   
   exit(0);
}

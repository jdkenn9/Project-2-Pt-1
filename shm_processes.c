#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>

void  ChildProcess(int [], sem_t* mutex);
void  ParentProcess(int [], sem_t* mutex);

int  main(int  argc, char *argv[])
{
     int    ShmID;
     int    *ShmPTR;
     pid_t  pid;


     ShmID = shmget(IPC_PRIVATE, 1*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of two integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");

     sem_t *mutex;
    if ((mutex = sem_open("balancesemaphore", O_CREAT, 0644, 1)) == SEM_FAILED){
      perror("semaphore initilization");
      exit(1);
    }
    
    printf("Bank Account = %d\n", ShmPTR[0]);


     printf("Server is about to fork a child process...\n");
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (server) ***\n");
          exit(1);
     }
    else if (pid == 0) {
          ChildProcess(ShmPTR, mutex);
     }
     else{
       ParentProcess(ShmPTR, mutex);
     }
}

void  ParentProcess(int  SharedMem[], sem_t* mutex){
  int localBalance = SharedMem[0];
  while(1){
    sleep(rand() % 6);
    printf("Dear Old Dad: Attempting to Check Balance\n");
    int num = rand() % 100;
     if(num % 2 == 0){
       if(localBalance < 100){
         localBalance = SharedMem[0];
         int amount = rand() % 101;
         if(amount % 2 == 0){
           localBalance += amount;
         printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
         sem_wait(mutex);
         SharedMem[0] = localBalance;
         sem_post(mutex);
         }
         else{
           printf("Dear old Dad: Doesn't have any money to give\n");
         }
       }
       else{
         printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
       }
     }
  }
}

void  ChildProcess(int  SharedMem[], sem_t* mutex)
{
  int localBalance = SharedMem[0];
  while(1){
    sleep(rand() % 6);
    printf("Poor Student: Attempting to Check Balance\n");
    int num = rand() % 101;
    if(num % 2 == 0){
      localBalance = SharedMem[0];
      int need = rand() % 51;
      printf("Poor Student needs $%d\n", need);
      if(need <= localBalance){
        localBalance = localBalance - need;
        printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
      }
      else{
        printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
      }
    }
    else{
      printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
    }
  }
}
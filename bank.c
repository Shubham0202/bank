#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>
#include<sys/wait.h>
#include<sys/mman.h>
#include<fcntl.h>

#define SHM_NAME "/bank_shm"
#define SEM_MUTEX_NAME "/bank_sem_mutex"
#define SEM_EMI_NAME "/bank_sem_emi"
#define SEM_WITHDRAW_NAME "/bank_sem_withdraw"
#define SEM_DEPOSIT_NAME "/bank_sem_deposit"

//shared memory structure
typedef struct {
    float balance;
    sem_t mutex;
    sem_t emi;
    sem_t withdraw;
    sem_t deposit;
} BankData;

void deposit_process(BankData* bank_data)
{
    int i=0,dept_balance=0,flag=1;
    printf("Enter Deposit amount\n");
    scanf("%d",&dept_balance);

    for(i=0;i<5;++i)
    {
        sem_wait(&bank_data->mutex);
        if(flag==1)
        {
            bank_data->balance+=dept_balance;
            flag+=1;
            printf("Deposited $%d . New Balance:%.2f\n",dept_balance,bank_data->balance);
        }
        sem_post(&bank_data->mutex);
        sem_post(&bank_data->emi);
        sem_post(&bank_data->withdraw);
        dept_balance=0;
        sleep(3);
    }
    exit(0);
}

void emi_process(BankData* bank_data)
{
    while(1)
    {
        if(bank_data->balance>=50)
        {
            bank_data->balance-=50;
            printf("EMI Paid 50$ Current Balance =%.2f",bank_data->balance);
        }
        else{
            printf("Insufficient Balance\n");
            sem_post(&bank_data->emi);
        }
        sem_post(&bank_data->mutex);
        sleep(1);
    }
}

void withdraw_process(BankData* bank_data)
{
    int withdraw=0;
    while (1)
    {
        sem_wait(&bank_data->mutex);
        sem_wait(&bank_data->withdraw);
        printf("Enter WithDraw Amount:");
        scanf("%d",&withdraw);

        if(bank_data->balance>=withdraw)
        {
            bank_data->balance-=withdraw;
            printf("Withdraw amount is %d:Current Balance:%.2f\n",withdraw,bank_data->balance);
        }
        else{
            printf("Insufficient Balance........\n");
            sem_post(&bank_data->withdraw);
        }
        sem_post(&bank_data->mutex);
        sleep(2);
    }
    
}

int main()
{
    int shm_fd;
    BankData* bank_data;

    shm_fd = shm_open(SHM_NAME,O_CREAT | O_RDWR,0666);
    ftruncate(shm_fd,sizeof(BankData));
    bank_data = mmap(NULL,sizeof(BankData),PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd,0);
    
    bank_data->balance=0;
    sem_init(&bank_data->mutex,1,1);
    sem_init(&bank_data->deposit,1,1);
    sem_init(&bank_data->emi,1,0);
    sem_init(&bank_data->withdraw,1,0);

    munmap(bank_data, sizeof(BankData));
    shm_unlink(SHM_NAME);
    return 0;
}

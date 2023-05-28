#include<stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include<stdbool.h>

#define NUM_BOOKS 10
#define NUM_TABLES 6
#define MAX_CUSTOMER 100


typedef struct
{
    int timeToRead;
    int borrowerId;
    pthread_mutex_t lock;
} Book;

Book books[NUM_BOOKS];
sem_t cafe;
pthread_mutex_t cafeTables;
int cafeTablesCount;


int getOneDigitRandomNumber()
{
    return rand() % 10;
}

void initializeRng()
{
    srand(time(NULL));
}

void initializeCafe()
{
    sem_init(&cafe, 0, NUM_TABLES);
    pthread_mutex_init(&cafeTables, NULL);
    cafeTablesCount=NUM_TABLES;

    for (int i = 0; i < NUM_BOOKS; i++)
    {
        books[i].timeToRead = (getOneDigitRandomNumber()+1)*4;
        books[i].borrowerId=-1;
        pthread_mutex_init(&books[i].lock, NULL);
    }
}

void printSeatStatus()
{
    pthread_mutex_lock(&cafeTables);
    if(cafeTablesCount==0)
    {
        printf("Currently no seat available, customer will wait in queue\n");
    }
    else
    {
        printf("Seat available %d\n",cafeTablesCount);
    }
    pthread_mutex_unlock(&cafeTables);
}


void *customer(void *args)
{
    int customer_id = (intptr_t)args;
    printSeatStatus();
    sem_wait(&cafe);
    printf("Customer %d entered the cafe and took a table.\n", customer_id);

    pthread_mutex_lock(&cafeTables);
    cafeTablesCount--;
    pthread_mutex_unlock(&cafeTables);

    bool bookNotFound=true;
    while(bookNotFound)
    {
        int i = rand() % NUM_BOOKS;
        pthread_mutex_lock(&books[i].lock);
        if (books[i].borrowerId==-1)
        {
            books[i].borrowerId = customer_id;
            pthread_mutex_unlock(&books[i].lock);
            printf("Customer %d borrowed book %d. Time to Finish:%d\n", customer_id, i,books[i].timeToRead);
            sleep(books[i].timeToRead);
            printf("Customer %d going to return book %d.\n", customer_id, i);
            pthread_mutex_lock(&books[i].lock);
            books[i].borrowerId = -1;
            pthread_mutex_unlock(&books[i].lock);
            bookNotFound=false;
            break;
        }
        pthread_mutex_unlock(&books[i].lock);
    }
    printf("Customer %d finished and left the table.\n", customer_id);
    pthread_mutex_lock(&cafeTables);
    cafeTablesCount++;
    pthread_mutex_unlock(&cafeTables);
    sem_post(&cafe);
    return NULL;
}

void createCustomer(pthread_t *customerPtr)
{
    static int customerId=0;
    pthread_create(customerPtr, NULL, customer, (void *)(intptr_t)customerId);
    customerId++;
}
void printMenu()
{
    printf("\n\n\t\tPlease choose an option:\n");
    printf("\t\t1. Create Customer\n");
    printf("\t\t2. Check Book Status\n");
    printf("\t\t3. Check Available Seats\n");
    printf("\t\t0. Exit\n");
}

void printBookStatus()
{
    for(int i=0 ; i< NUM_BOOKS; i++)
    {
        pthread_mutex_lock(&books[i].lock);
        if(books[i].borrowerId==-1)
        {
            printf("\t\tbook id: %d Nobody took this book\n",i);
        }
        else
        {
            printf("\t\tbook id: %d Borrowed by this customer:%d\n",i,books[i].borrowerId);
        }
        pthread_mutex_unlock(&books[i].lock);
    }
}

void cleanUpCafe()
{
    sem_destroy(&cafe);
    for (int i = 0; i < NUM_BOOKS; i++)
    {
        pthread_mutex_destroy(&books[i].lock);
    }
}

int main()
{
    initializeRng();
    initializeCafe();

    pthread_t customers[MAX_CUSTOMER];
    int customerCount=0;
    int choice;
    do
    {
        printMenu();
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            printf("Creating customer\n");
            createCustomer(&customers[customerCount]);
            customerCount++;
            break;
        case 2:
            printf("Checking book status\n");
            printBookStatus();
            break;
        case 3:
            printf("Checking available seats\n");
            printSeatStatus();
            break;
        case 0:
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid choice. Please try again\n");
            break;
        }
    }
    while (choice != 0);

    for (int i = 0; i < customerCount; i++)
    {
        pthread_join(customers[i], NULL);
    }

    cleanUpCafe();
    return 0;

}



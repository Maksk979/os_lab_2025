#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t A = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t B = PTHREAD_MUTEX_INITIALIZER;

void* thread1(void* arg) {
    pthread_mutex_lock(&A);
    printf("Поток 1: залочен A, ожидаем B...\n");
    fflush(stdout);  
    
    sleep(1);  
    
    pthread_mutex_lock(&B);  
    printf("Поток 1: оба залочены\n");
    fflush(stdout);
    
    pthread_mutex_unlock(&B);
    pthread_mutex_unlock(&A);
    return NULL;
}

void* thread2(void* arg) {
    pthread_mutex_lock(&B);
    printf("Поток 2: залочен B, ожидаем A...\n");
    fflush(stdout); 
    
    sleep(1);  
    
    pthread_mutex_lock(&A);  
    printf("Поток 2: оба залочены\n");
    fflush(stdout);
    
    pthread_mutex_unlock(&A);
    pthread_mutex_unlock(&B);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    
    printf("Начало демонстрации deadlock...\n");
    
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    
    sleep(2);
    
    printf("Программа сейчас во взаимной блокировке\n");
    fflush(stdout);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    printf("Это сообщение никогда не появится\n");
    
    return 0;
}
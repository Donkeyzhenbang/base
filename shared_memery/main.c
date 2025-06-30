#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// 共享内存数据结构
struct shared_data {
    int counter;
    char message[256];
};

// 信号量操作联合体（必需）
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// 信号量操作函数
void sem_lock(int sem_id) {
    struct sembuf op = {0, -1, 0}; // 对第一个信号量进行P操作（减1）
    semop(sem_id, &op, 1);
}

void sem_unlock(int sem_id) {
    struct sembuf op = {0, 1, 0}; // 对第一个信号量进行V操作（加1）
    semop(sem_id, &op, 1);
}

int main() {
    key_t shm_key, sem_key;
    int shm_id, sem_id;
    struct shared_data *shm_ptr;
    
    // 创建唯一键值
    if ((shm_key = ftok("/tmp", 'S')) == -1) {
        perror("ftok (shm)");
        exit(1);
    }
    if ((sem_key = ftok("/tmp", 'M')) == -1) {
        perror("ftok (sem)");
        exit(1);
    }
    
    // 创建共享内存
    if ((shm_id = shmget(shm_key, sizeof(struct shared_data), IPC_CREAT | 0666)) == -1) {
        perror("shmget");
        exit(1);
    }
    
    // 创建信号量（一个信号量，初始值为1）
    if ((sem_id = semget(sem_key, 1, IPC_CREAT | 0666)) == -1) {
        perror("semget");
        exit(1);
    }
    
    // 初始化信号量值为1（互斥锁）
    union semun sem_arg;
    sem_arg.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_arg) == -1) {
        perror("semctl SETVAL");
        exit(1);
    }
    
    // 附加共享内存
    if ((shm_ptr = shmat(shm_id, NULL, 0)) == (void*)-1) {
        perror("shmat");
        exit(1);
    }
    
    // 初始化共享数据
    shm_ptr->counter = 0;
    strcpy(shm_ptr->message, "Initial message");
    
    // 创建子进程
    pid_t pid = fork();
    
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) { // 子进程（写入者）
        for (int i = 1; i <= 5; i++) {
            sem_lock(sem_id); // 加锁
            
            // 更新共享数据
            shm_ptr->counter = i;
            sprintf(shm_ptr->message, "Child update #%d", i);
            
            printf("[Child] Updated: counter=%d, message=%s\n", 
                   shm_ptr->counter, shm_ptr->message);
            
            sem_unlock(sem_id); // 解锁
            
            sleep(1); // 模拟处理时间
        }
        
        // 分离共享内存
        shmdt(shm_ptr);
        exit(0);
    } 
    else { // 父进程（读取者）
        for (int i = 0; i < 5; i++) {
            sem_lock(sem_id); // 加锁
            
            printf("[Parent] Current: counter=%d, message=%s\n", 
                   shm_ptr->counter, shm_ptr->message);
            
            sem_unlock(sem_id); // 解锁
            
            sleep(1); // 模拟处理时间
        }
        
        // 等待子进程结束
        wait(NULL);
        
        // 分离共享内存
        shmdt(shm_ptr);
        
        // 删除共享内存和信号量
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID);
        
        printf("Resources cleaned up\n");
    }
    
    return 0;
}
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>
 
// 定義給子行程用的stack 1M 
#define STACK_SIZE (1024 * 1024)
static char container_stack[STACK_SIZE];
 
char* const container_args[] = {
    "/bin/bash",
    NULL
};
 
int container_main(void* arg)
{
    printf("Container - inside the container!\n");
    // 設定自己的hostname名稱
    sethostname([your_hostname],10); 
    // 直接執行一個shell，讓之後可以觀察這個行程的資源有沒有被隔離
    execv(container_args[0], container_args); 
    printf("Something's wrong!\n");
    return 1;
}
 
int main()
{
    printf("Parent [%5d] - start a container!\n", getpid());
    // 呼叫clone函式，clone()返回的是子行程的pid
    // 使用 Namespace的 CLONE_NEWUTS flag將 hostname隔離
    int container_pid = clone(container_main, container_stack+STACK_SIZE, 
            CLONE_NEWUTS | SIGCHLD, NULL); 

    // 等待子行程結束
    waitpid(container_pid, NULL, 0);
    printf("Parent - container stopped!\n");
    return 0;
}

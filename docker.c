#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
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
    printf("Container [%5d] - inside the container!\n", getpid());

    //將需要掛載的資料夾 掛載到目前環境
    if (mount("proc", "rootfs/proc", "proc", 0, NULL) !=0 ) {
        perror("proc");
    }
    if (mount("sysfs", "rootfs/sys", "sysfs", 0, NULL)!=0) {
        perror("sys");
    }
    if (mount("none", "rootfs/tmp", "tmpfs", 0, NULL)!=0) {
        perror("tmp");
    }
    if (mount("udev", "rootfs/dev", "devtmpfs", 0, NULL)!=0) {
        perror("dev");
    }
    if (mount("devpts", "rootfs/dev/pts", "devpts", 0, NULL)!=0) {
        perror("dev/pts");
    }
    if (mount("shm", "rootfs/dev/shm", "tmpfs", 0, NULL)!=0) {
        perror("dev/shm");
    }
    if (mount("tmpfs", "rootfs/run", "tmpfs", 0, NULL)!=0) {
        perror("run");
    }

    // 程式啟動時才讀取的設置
    if (mount("conf/hosts", "rootfs/etc/hosts", "none", MS_BIND, NULL)!=0 ||
        mount("conf/hostname", "rootfs/etc/hostname", "none", MS_BIND, NULL)!=0 ||
        mount("conf/resolv.conf", "rootfs/etc/resolv.conf", "none", MS_BIND, NULL)!=0 ) {
        perror("conf");
    }

    // chdir：移動目前資料夾 ,chroot：指定資料夾當作根目錄 */
    if ( chdir("./rootfs") != 0 || chroot("./") != 0 ){
        perror("chdir/chroot");
    }
         
    execv(container_args[0], container_args);
    perror("exec");
    printf("Something's wrong!\n");
    return 1;
}
 
int main()
{
    printf("Parent [%5d] - start a container!\n", getpid());
    // 加入 Namespace的 CLONE_NEWNS flag將 MNT隔離
    int container_pid = clone(container_main, container_stack+STACK_SIZE, 
            CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, NULL);
    waitpid(container_pid, NULL, 0);
    
    // 將掛載的資料夾卸載
    if (umount("rootfs/proc") !=0 ) {
        perror("proc");
    }
    if (umount("rootfs/sys")!=0) {
        perror("sys");
    }
    if (umount("rootfs/tmp")!=0) {
        perror("tmp");
    }
    if (umount("rootfs/dev")!=0) {
        perror("dev");
    }
    if (umount("rootfs/dev/pts")!=0) {
        perror("dev/pts");
    }
    if (umount("rootfs/dev/shm")!=0) {
        perror("dev/shm");
    }
    if (umount("rootfs/run")!=0) {
        perror("run");
    }
    if (umount("rootfs/etc/hosts")!=0 ||
        umount("rootfs/etc/hostname")!=0 ||
        umount("rootfs/etc/resolv.conf")!=0 ) {
        perror("conf");
    }
    
    printf("Parent - container stopped!\n");
    return 0;
}

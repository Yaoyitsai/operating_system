#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //fork 要使用
#include<time.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/mman.h>
#include<sys/file.h>
#include<fcntl.h>

using namespace std;

void error_and_die(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc,char *argv[]){
    
    const char *memname = "pidID";
    int fd = shm_open(memname, O_CREAT | O_TRUNC | O_RDWR, 0666);
    if(fd == -1)error_and_die("shm_open"); //判斷開啟memory有沒有error產生
    
    int r = ftruncate(fd,sizeof(int)*5);
    if(r!=0)error_and_die("ftruncate");//判斷configure size有沒有error產生
    
    int *ptr = (int*)mmap(0,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    //開啟mmap並指向要存的記憶體位址開始存
    if(ptr == MAP_FAILED)error_and_die("mmap");
    
    int n = 0;
    cin >> n; //輸入整數n
    
    if( n < 1 || n > 100) return 0; // 限定輸入範圍 1 <= n <= 100
    
    //使用剛剛指向的位址（配了5個int）
    ptr[0] = n; //存n
    ptr[1]= getpid(); //存放當下執行的行程的pid (目前為父行程的pid)
    ptr[2] = 0; //max
    ptr[3] = 0; //count
    ptr[4] = 0; //tmp
    pid_t pid = fork();//父行程用fork()產生一個子行程
    
    while(ptr[0]!=1 ) { //做到1時結束
        /*使用share memory指向的ptr[1] 存放目前的 pid ,
        若ptr[1] == getpid()讓此行程執行無窮迴圈 等待另一個行程計算出結果並改變ptr[1]之值*/
        while(true) if(ptr[1] != getpid() || ptr[0]==1)break;
        
        if( ptr[0] % 2 == 1 && ptr[0]!=1 ){ // 奇數
            if(ptr[0]!=1 ){
                ptr[0] = 3* ptr[0] + 1;  // 考拉茲猜想
                ptr[3]++;  // count++
                if(ptr[2] < ptr[0]){
                    ptr[2] = ptr[0];  // replace max
                    ptr[4] = ptr[3];  // replace count
                }
            }
            if (pid == 0) // 若為子行程 印出子行程pid和計算結果
                cout <<"["<<getpid()<<" Child]:"<< ptr[0]<<endl;
            if(pid > 0)  // 若為父行程 印出父行程pid和計算結果
                cout <<"["<<getpid()<<" Parent]:"<< ptr[0]<<endl;
            if (ptr[0]!=1) ptr[1] = getpid(); //存放當下執行的行程的pid
        }
        else if(ptr[0]==1){   // 父行程和子行程進行到最後剩下1要印出時
            if(pid == 0 ){  //若最後為子行程計算出1則不再多印出一次（計算出後已印出child:1 & parent:1）
                ptr[1] = getpid();
                break;
            }
            if(pid > 0){  //若最後為父行程計算出1則在這裡印出子行程不需要再次印出
                cout <<"["<<getpid()<<" Parent]:"<<ptr[0]<<endl;
                ptr[1] = getpid();
                break;
            }
        }
        else{    // 偶數
            if (ptr[0]!=1){
                ptr[0] = ptr[0] / 2; // 考拉茲猜想
                ptr[3]++;  // count++
                if(ptr[2] < ptr[0]){
                    ptr[2] = ptr[0]; // replace max
                    ptr[4] = ptr[3]; // replace count
                }
            }
            if(pid == 0 )  // 若為子行程 印出子行程pid和計算結果
                cout <<"["<<getpid()<<" Child]:"<<ptr[0]<<endl;
            if(pid > 0)  // 若為父行程 印出父行程pid和計算結果
                cout <<"["<<getpid()<<" Parent]:"<<ptr[0]<<endl;
            if(ptr[0]!=1) ptr[1] = getpid(); //存放當下執行的行程的pid
        }
    }
    // 印出 max 和 turn
    if(pid > 0 )cout << "Max:"<<ptr[2]<<endl<<"Turn:"<<ptr[4]<<endl;

    close(fd);
    munmap(ptr,sizeof(int)*5);
    shm_unlink(memname);
    
    return 0;
}

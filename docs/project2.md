# Project 2: 實作自訂 Wait Queue 的 System Call

第 17 組：
朱巽葦（110403507）、陳禾旻（109502562）、洪柏軒（110302031）、林睿瀚（112526011）

:::info
[Github Repo](https://github.com/hermeschen1116/linux-systemcall-implement)
:::

## Outline
[ToC]

## 環境

- Debian GNU/Linux 12 (bookworm) x86_64
- Linux Kernel 6.1.0
- QEMU/KVM or VMware

## 實作

### 編譯 & 安裝 Kernel
先安裝編譯時所需要用到的packages:
```clike=
sudo apt update
sudo apt upgrade -y
sudo apt install git fakeroot build-essential ncurses-dev xz-utils libssl-dev bc flex libelf-dev bison dwarves ccache liblz4-tool -y
```
接著下載Kernel source
```clike=
if [ ! -d "linux" ]
then
	sudo apt install wget -y
	wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.tar.xz
	tar xvf linux-6.1.tar.xz
	mv linux-6.1 linux
	rm linux-6.1.tar.xz
fi
```
創建自定義的system call資料夾
```clike=
mkdir linux/custom_systemcall

# link config
ln -s ../linux/include/linux/syscalls.h src/syscalls.h
ln -s ../linux/arch/x86/entry/syscalls/syscall_64.tbl src/syscall_64.tbl
ln -s ../linux/Makefile src/Makefile
```
編譯且安裝新的kernel (menuconfig可改成localmodconfig會編譯比較快)
```clike=
process="$(nproc)"
architecture="x86_64"
source="linux"
build_dir="./build"

# copy modified files to source
cp src/systemcall/* linux/custom_systemcall/

# build
KBUILD_BUILD_TIMESTAMP="" sudo make menuconfig CC="ccache gcc" -j$process -C$source O=$build_dir
KBUILD_BUILD_TIMESTAMP="" sudo make CC="ccache gcc" Arch=$architecture -j$process -C$source O=$build_dir

# install
sudo make modules_install -j$process -C$source O=$build_dir
sudo make install -j$process -C$source O=$build_dir
sudo update-grub
```

### 新增 System Call

### ```call_my_wait_queue```

- 程式碼

```clike=
#include <linux/syscalls.h>
#include <linux/wait.h>
#include <linux/sched.h>

static wait_queue_head_t my_wait_queue;
static int queue_initialized = 0;

static void initialize_wait_queue(void)
{
	if (!queue_initialized) {
		init_waitqueue_head(&my_wait_queue);
		queue_initialized = 1;
		printk(KERN_DEBUG
		       "call_my_wait_queue: my_wait_queue initialized.\n");
	}
}

static int enter_wait_queue(void)
{
	DEFINE_WAIT(wait);

	initialize_wait_queue();

	printk(KERN_DEBUG
	       "call_my_wait_queue: Process [%d] entering wait queue.\n",
	       current->pid);

	add_wait_queue(&my_wait_queue, &wait);
	prepare_to_wait(&my_wait_queue, &wait, TASK_INTERRUPTIBLE);
	schedule();

	if (signal_pending(current)) {
		printk(KERN_WARNING
		       "call_my_wait_queue: Process [%d] interrupted by signal.\n",
		       current->pid);
		finish_wait(&my_wait_queue, &wait);
		return 0;
	}

	finish_wait(&my_wait_queue, &wait);
	printk(KERN_DEBUG
	       "call_my_wait_queue: Process [%d] exited wait queue successfully.\n",
	       current->pid);
	return 1;
}

static int clean_wait_queue(void)
{
	initialize_wait_queue();

	if (!waitqueue_active(&my_wait_queue)) {
		printk(KERN_WARNING
		       "call_my_wait_queue: Attempt to clean an empty wait queue.\n");
		return 0;
	}

	printk(KERN_DEBUG
	       "call_my_wait_queue: Cleaning wait queue. Waking up all processes.\n");

	wake_up(&my_wait_queue);

	if (!waitqueue_active(&my_wait_queue)) {
		printk(KERN_DEBUG
		       "call_my_wait_queue: Wait queue cleaned successfully.\n");
		return 1;
	}

	printk(KERN_WARNING
	       "call_my_wait_queue: Wait queue still active after clean attempt.\n");
	return 0;
}

SYSCALL_DEFINE1(call_my_wait_queue, int, id)
{
	printk(KERN_DEBUG
	       "call_my_wait_queue: call_my_wait_queue invoked with id=%d.\n",
	       id);

	switch (id) {
	case 1:
		return enter_wait_queue();
	case 2:
		return clean_wait_queue();
	default:
		printk(KERN_ERR
		       "call_my_wait_queue: Invalid id (%d) passed to call_my_wait_queue.\n",
		       id);
		return 0;
	}
}
```
![image](https://hackmd.io/_uploads/ByPgQHEH1e.png)

#### initialize_wait_queue
每一個wait queue都有一個 wait queue head，資料型態為 wait_queue_head_t (line5)，在初始化一個 wait queue 的時候要先定義 wait queue head。
呼叫 init_waitqueue_head 會將 spin lock 轉為未鎖定狀態，並初始化一個雙向 linklist。

#### enter_wait_queue
用 add_wait_queue 把要等待的 process 放到 wait queue 的最後，當有 signal 的時候代表該 process 要準備離開 wait queue 到 run queue 中。

#### clean_wait_queue
首先使用 waitqueue_active 來判斷這個 wait queue 是否為空，如果為空則直接 return 0
如果非空，先 wake_up queue 中所有的 process，再次判斷是否為空。
若仍然不為空，為例外狀況。

#### syscall
透過輸入能分別呼叫並處理 enter_wait_queue, clean_wait_queue 的狀況。

### Test System Call

- 程式碼

```clike=
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#define NUM_THREADS 10
#define SYS_call_my_wait_queue 453

void *enter_wait_queue(void *thread_id)
{
	fprintf(stderr, "enter wait queue thread_id: %d\n", *(int *)thread_id);
	long result = syscall(SYS_call_my_wait_queue, 1);

	if (result != 1) {
		fprintf(stderr,
			"Error: syscall failed with result %ld for thread_id: %d\n",
			result, *(int *)thread_id);
		free(thread_id);
		pthread_exit(NULL);
	}

	fprintf(stderr, "exit wait queue thread_id: %d\n", *(int *)thread_id);
	return (void *)result;
}

void *clean_wait_queue()
{
	long result = syscall(SYS_call_my_wait_queue, 2);
	if (result != 1) {
		fprintf(stderr,
			"Error: Failed to clean wait queue. Return value: %ld\n",
			result);
	}

	return (void *)result;
}

int main()
{
	void *ret;
	pthread_t id[NUM_THREADS];
	int thread_args[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++) {
		thread_args[i] = i;
		pthread_create(&id[i], NULL, enter_wait_queue,
			       (void *)&thread_args[i]);
	}
	sleep(1);
	fprintf(stderr, "start clean queue ...\n");
	clean_wait_queue();
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(id[i], &ret);
	}
	return 0;
}
```
## Result

藉由 pthread_create 創建了 10 個threads，每個 thread 都會先執行 enter_wait_queue 再執行 exit_wait_queue，由於沒有保證 FIFO，所以每次的順序都會有所不同。

![image](https://hackmd.io/_uploads/ry7n8jhBke.png)

- enter_wait_queue 階段:  
wait queue 先完成初始化後，依序將每個 thread 加入 wait queue 當中，再將 thread 的 state 改為等待狀態，藉由 prepare_to_wait() 將 thread 設成 TASK_INTERRUPTIBLE 直到收到 signal   
- exit_wait_queue 階段:  
wait queue 因前一階段設置 queue_initialized = 1，故不會在初始化一次，再檢查 wait queue 是否為空後 call wake_up() 來將所有 thread 喚醒。
## 筆記
* wait_queue_head_t:  
wait_queue_head_t 是「Wait Queue Head」的結構體，用來管理整個 Wait Queue，此結構中包含一個 鏈結串列 及必要的鎖機制，用來儲存 Wait Queue Entry（wait_queue_entry_t）的串列。
* init_waitqueue_head(&my_wait_queue):  
用於初始化一個「Wait Queue Head」，在使用 my_wait_queue 之前，必須先初始化它，否則 Wait Queue 結構尚未設定完成。
* DEFINE_WAIT(wait):  
Macro，用於宣告並初始化一個 wait_queue_entry_t 型別的變數，常被用來表示某個「Waiting Thread」，等同於先宣告一個 wait_queue_entry_t wait;，然後幫它綁定當前的行程（current）等初始化動作。
* add_wait_queue(&my_wait_queue, &wait):  
用於將一個 Wait Queue Entry（wait_queue_entry_t）新增到指定的  Wait Queue Head 等候佇列（wait_queue_head_t）中，才能讓這個等待者「掛」在 my_wait_queue 裡，未來有機會被喚醒。
* prepare_to_wait(&my_wait_queue, &wait, TASK_INTERRUPTIBLE):  
將此 Wait Queue Entry（wait_queue_entry_t）設定為「可以睡眠、可被訊號中斷」的狀態，並完成睡眠前的準備動作（例如更改 Thread 狀態），一旦呼叫 schedule()，這個等待者就會睡眠，直到被喚醒或被訊號打斷。
* finish_wait(&my_wait_queue, &wait):  
將前面透過 prepare_to_wait() 設定好的 Wait Queue Entry 恢復原狀（或移出 Wait Queue），表示「結束等待」，一般在「被喚醒」或「因訊號中斷而離開等待」後，需要呼叫 finish_wait 來清理狀態。
* wake_up(&my_wait_queue):  
用來喚醒在 my_wait_queue 上所有符合條件的等待者 (Waiting Thread)，在 clean_wait_queue() 中呼叫 wake_up(&my_wait_queue)，使所有正在該 Wait Queue 等待的執行緒從睡眠狀態轉為就緒狀態，等待 CPU 排程。
* waitqueue_active(&my_wait_queue):  
用來檢查在 my_wait_queue 上是否還有等待者 (Waiting Thread)，可用於判斷該 Wait Queue 是否為空，例如在清空前或清空後檢查。
* signal_pending(current):  
用來檢查目前 process 是否有尚未處理（或正在處理）訊號（signal），例如 Ctrl+C、kill 等，若在等待期間接收到某些重要訊號，可能需要提早結束等待，以避免阻塞流程。
* 當一個進程呼叫 schedule() 時，它可能處於以下兩種狀態之一：  
    * 可運行狀態 (TASK_RUNNING)：此進程仍然可以在運行佇列 (run queue) 中等待調度器再次分配 CPU。  
    * 等待狀態 (TASK_INTERRUPTIBLE/TASK_UNINTERRUPTIBLE)：此進程會進入等待佇列，等待特定事件發生後再喚醒。  
因此，schedule() 並不總是將進程放入等待佇列。
*  lock 在等待佇列中的角色  
當多個處理器或核心同時訪問等待佇列（例如添加或刪除等待節點）時，可能會導致 Race Condition。
為了解決這個問題，等待佇列使用 lock 自旋鎖 來確保訪問的同步性，保護等待佇列的完整性。
* wait_queue_head_t struct
在 Linux 核心中，等待佇列頭透過 wait_queue_head_t struct 來表示：
    ```
    typedef struct wait_queue_head {
        spinlock_t lock;        // 自旋鎖，用於保護等待佇列的訪問
        struct list_head task_list; // 等待佇列中的所有等待節點（雙向鏈表）
    } wait_queue_head_t;
    ```
    (1) lock（自旋鎖）
    用途：保護等待佇列的完整性，避免多個 CPU 或多個進程同時訪問等待佇列時發生競爭條件（Race Condition）。
    類型：spinlock_t
    作用：當進程要添加到或從等待佇列中移除時，必須獲取這個鎖來確保安全性。  
    (2) task_list（雙向鏈表）
    用途：將所有等待在該佇列上的進程以鏈表的形式鏈接起來。
    類型：struct list_head
    作用：每個等待的進程會被封裝成 wait_queue_t 節點，並添加到這個鏈表中。

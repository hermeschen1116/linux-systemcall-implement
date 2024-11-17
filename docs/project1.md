# Project 1：實作回傳 Physical Address 的 System Call

第 17 組：<br>
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

### ```my_get_physical_addresses```

- 程式碼

```clike=
#include <linux/syscalls.h>

SYSCALL_DEFINE1(my_get_physical_addresses, void *, user_virtual_address)
{
	unsigned long virtual_address = (unsigned long)user_virtual_address;
	pgd_t *pgd;
	p4d_t *p4d;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long page_frame_number = 0;
	unsigned long page_offset = 0;
	unsigned long physical_address = 0;

	// Walk through the tables
	pgd = pgd_offset(current->mm, virtual_address);
	if (pgd_none(*pgd)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PGD\n");
		return 0;
	}

	p4d = p4d_offset(pgd, virtual_address);
	if (p4d_none(*p4d)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid P$D\n");
		return 0;
	}

	pud = pud_offset(p4d, virtual_address);
	if (pud_none(*pud)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PUD\n");
		return 0;
	}

	pmd = pmd_offset(pud, virtual_address);
	if (pmd_none(*pmd)) {
		printk(KERN_WARNING "my_get_physical_addresses: Invalid PMD\n");
		return 0;
	}

	// Find the page table entry
	pte = pte_offset_kernel(pmd, virtual_address);
	if (pte_none(*pte)) {
		printk(KERN_WARNING
		       "my_get_physical_addresses: No PTE found\n");
		return 0;
	}

	// calculate physical address
	page_frame_number = pte_val(*pte) & PAGE_MASK;
	page_offset = virtual_address & ~PAGE_MASK;
	physical_address = page_frame_number | page_offset;
	printk(KERN_DEBUG
	       "my_get_physical_addresses:\nvirtual address: %lx\npage frame number: %lx, page offset: %lx\nphysical address: %lx\n",
	       virtual_address, (page_frame_number >> PAGE_SHIFT), page_offset,
	       physical_address);

	return physical_address;
}
```
* virtual address to physical address
![L2P](https://hackmd.io/_uploads/B1KIXl3ZJl.png)
64bits 的 linux kernel 架構使用 4層 page table來做 virtual address to physical address 的轉換:
    * Page Global Directory (PGD)
    * Page Upper Directory (PUD)
    * Page Middle Directory (PMD)
    * Page Table (PTE)
    
> pgd_offset : 根據目前的 virtual address 和目前 process 的 mm_struct，可得到 pgd entry
> (entry 內容為 pud table 的 base address)
> 
> pud_offset : 根據透過 pgd_offset 得到的 pgd entry 和 virtual address，可得到 pud entry
> (entry 內容為 pmd table 的 base address)
> 
> pmd_offset : 根據 pud entry 的內容和 virtual address，可得到 pte table 的 base address
> 
> pte_offset : 根據 pmd entry 的內容與 virtual address，可得到 pte 的 base address
> 
> 將從 pte 得到的 base address 與 Mask(0xf…fff000)做 and 運算，可得到 page 的 base physical address
> 
> virtual address 與 ~Mask(0x0…000fff) 做 and運算得到 offset，再與 page 的 base physical address 做 or運算 即可得到轉換過後的完整的physical address。

> 而雖然在 x86_64 架構中，實際上使用的是 4 級頁表（PGD、PUD、PMD、PTE），但為了兼容可能的 5 級頁表，Linux 核心定義了 P4D。如果某級別在當前架構中未使用，則該級別會被 folded

* current pointer
> 指向當前進程的 task_struct 結構，
> task_struct 是 Linux 核心中用來表示每個執行中的進程的核心結構，這個結構包含了進程的完整上下文
> ![task_struct](https://hackmd.io/_uploads/r1Epvj2-1g.png =70%x)

> current->mm：指向當前進程的記憶體描述符 mm_struct


- 修改 **`syscall_64.tbl`**
![image](https://hackmd.io/_uploads/B1YuqocW1l.png)

- 修改 **`syscalls.h`**
![image](https://hackmd.io/_uploads/Hkgqqi9W1x.png)
在#endif加上
### Test system call

##### Q1

- 程式碼

```clike=
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define SYS_my_get_physical_address 452

unsigned long my_get_physical_addresses(void *virtual_address);

int main()
{
	int *shared_memory = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
				  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (shared_memory == MAP_FAILED) {
		perror("mmap failed");
		exit(1);
	}
	*shared_memory = 48763;

	printf("Initial value in parent process: %d\n", *shared_memory);
	printf("Parent's physical address before fork: [0x%lx]\n",
	       my_get_physical_addresses(shared_memory));
	unsigned long parent_physical_address_before =
		my_get_physical_addresses(shared_memory);

	pid_t pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	} else if (pid == 0) {
		printf("Child's physical address before write: [0x%lx]\n",
		       my_get_physical_addresses(shared_memory));

		*shared_memory = 114514;
		printf("New value in child process: %d\n", *shared_memory);

		printf("Child's physical address after write: [0x%lx]\n",
		       my_get_physical_addresses(shared_memory));

		exit(0);
	} else {
		wait(NULL);

		printf("Parent's physical address after child's write: [0x%lx]\n",
		       my_get_physical_addresses(shared_memory));
		unsigned long parent_physical_address_after =
			my_get_physical_addresses(shared_memory);

		if (parent_physical_address_after ==
		    parent_physical_address_before) {
			printf("Parent's physical address remains the same.\n");
		} else {
			printf("Parent's physical address has changed (unexpected in COW).\n");
		}
	}

	munmap(shared_memory, 4096);
	return 0;
}

unsigned long my_get_physical_addresses(void *virtual_address)
{
	return syscall(SYS_my_get_physical_address, virtual_address);
}
```

- 結果
![image](https://hackmd.io/_uploads/HkZmGs9b1g.png)

| 執行步驟                              | 輸出內容                                      |
|---------------------------------------|-----------------------------------------------|
| Initial value in parent process       | 48763                                        |
| Parent's physical address before fork | 0x800000018268a000                           |
| Child's physical address before write | 0x800000018268a000                           |
| New value in child process            | 114514                                       |
| Child's physical address after write  | 0x80000001a4bb1000                           |
| Parent's physical address after child's write | 0x800000018268a000                   |
| Final remark                          | Parent's physical address remains the same.   |


child process 在fork後，因還沒對其進行任何write的動作，故此時memory page frame is shared between the parent and child process => physical address 一樣。
而child process被修改了，即會觸發Copy-On-Write機制，系統為子進程分配了新的page frame，以確保parent proccess不會受到影響。

##### Q2

- 程式碼

```clike=
#include <stdio.h>
#include <unistd.h>

#define SYS_my_get_physical_address 452

unsigned long my_get_physical_addresses(void *virtual_address);

int a[2000000];

int main()
{
	int loc_a;

	printf("global element a[0]:\n");
	printf("Offest of logical address:[%p]   Physical address:[0x%lx]\n",
	       &a[0], my_get_physical_addresses(&a[0]));
	printf("========================================================================\n");
	printf("global element a[1999999]:\n");
	printf("Offest of logical address:[%p]   Physical address:[0x%lx]\n",
	       &a[1999999], my_get_physical_addresses(&a[1999999]));
	printf("========================================================================\n");
}

unsigned long my_get_physical_addresses(void *virtual_address)
{
	return syscall(SYS_my_get_physical_address, virtual_address);
}
```

- 結果
![image](https://hackmd.io/_uploads/HJAVfs5-1e.png)

| 執行步驟                    | 輸出內容                                     |
|-----------------------------|----------------------------------------------|
| global element a[0]         |                                              |
| Offset of logical address   | 0x55f0c9593060                               |
| Physical address            | 0x8000000176bb3060                           |
| ============================|=============================|
| global element a[1999999]   |                                              |
| Offset of logical address   | 0x55f0c9d3425c                               |
| Physical address            | 0x0                                          |

因陣列宣告在 compile 的時候是決定其 virtual address，系統不會立即佔用宣告那麼大的實體記憶體空間，而僅是維護上面的 virtual address 空間而已，只有在真正需要的時候才分配實體記憶體，該 page 未 load 至 memory中，故應為 nil (此處 **`a[1999999]`** physical address 顯示為0而不是 nil 是因其回傳值所定義的型態是 unsigned long )

## 筆記
### copy_to_user
* to：資料的目的位址，此參數為一個指向 user-space 記憶體的指標。
* from：資料的來源位址，此參數為一個指向 kernel-space 記憶體的指標。
> copy data to user-space from kernel-space

### copy_from_user
* to：資料的目的位址，此參數為一個指向 kernel-space 記憶體的指標。
* from：資料的來源位址，此參數為一個指向 user-space 記憶體的指標。
> copy data from user-space to kernel-space

### Copy-on-Write（CoW）
* shared resources： 當一個進程被複製時（例如父進程創建子進程），新的進程需要擁有與原進程相同的記憶體內容。為了節省資源，OS 讓這兩個進程共享相同的物理記憶體頁面
* read-only protection： 共享的頁面被標記為只讀，這樣如果其中一個進程嘗試修改頁面內容，就會引發 Page Fault
* Delayed replication： 當 Page Fault 發生，OS 才會為需要修改的進程分配新的物理頁面，並將原頁面的內容複製過去，這就是 Copy-on-Write

## 參考資料

- [Add a System call to the Linux Kernel (6.0.9) in Ubuntu 22.04 using Oracle VM VirtualBox at Window](https://medium.com/@rajiv.cse/add-a-system-call-to-the-linux-kernel-6-0-9-in-ubuntu-22-04-acd7f7afc933)
- [Linux 核心 Copy On Write 實作機制](https://hackmd.io/@linD026/Linux-kernel-COW-Copy-on-Write#Physical-Memory-Model)
#include "memory.h"
#include "debug.h"
#include "print.h"
#include "string.h"

#define PG_SIZE 4096

// 位图地址
// 0xc009a000 是内核主线程栈顶，0xc009e000
// 是内核主线程的pcb（pcb是进程控制块，用于描述进程执行时的状态）
// 一个页框大小的位图可表示128MB内存，位图地址安排在内核主线程PCB之下（0xc009a000）
#define MEM_BITMAP_BASE 0xc009a000

// 0xc0000000 是内核从虚拟地址3G起
#define K_HEAP_START 0xc0100000

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22) // 获取虚拟地址的页目录项索引
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12) // 获取虚拟地址的页表项索引

// 内存池结构,生成两个实例用于管理内核内存池和用户内存池
struct pool {
  struct bitmap pool_bitmap; // 本内存池用到的位图结构，用于管理物理内存
  uint32_t phy_addr_start; // 本内存池所管理物理内存的起始地址
  uint32_t pool_size;      // 本内存池字节容量
};

struct pool kernel_pool, user_pool; // 生成内核内存池和用户内存池
struct virtual_addr kernel_vaddr; // 此结构是用来给内核分配虚拟地址

// 初始化内存池
static void mem_pool_init(uint32_t all_mem) {
  put_str("   mem_pool_init start\n");
  uint32_t page_table_size =
      PG_SIZE *
      256; // 页表大小 = 1页的页目录表(1) + 第0和第768页指向同一个页表(1) +
           // 第769~1022个页目录项指向页表(254) = 256页
  uint32_t used_mem = page_table_size + 0x100000; // 0x100000 为低端1M内存，值为0x200000
  uint32_t free_mem = all_mem - used_mem;
  uint16_t all_free_pages =
      free_mem / PG_SIZE; // 1页为4k，不用位图时的可用物理页数
  uint16_t kernel_free_pages = all_free_pages / 2; // 分配给内核的物理页数
  uint16_t user_free_pages =
      all_free_pages - kernel_free_pages; // 分配给用户的物理页数

  // 为简化位图操作，余数不处理，坏处是这样做会丢内存。好处是不用做内存的越界检查，因为位图表示的内存少于实际物理内存
  uint32_t kbm_length =
      kernel_free_pages /
      8; // Kernel BitMap 的长度，位图中的一位表示一页，以字节为单位
  uint32_t ubm_length = user_free_pages / 8; // User BitMap 的长度

  uint32_t kp_start = used_mem; // Kernel Pool start，内核物理内存池的起始地址
  uint32_t up_start =
      kp_start +
      kernel_free_pages * PG_SIZE; // User Pool start，用户物理内存池的起始地址

  kernel_pool.phy_addr_start = kp_start;
  user_pool.phy_addr_start = up_start;

  kernel_pool.pool_size = kernel_free_pages * PG_SIZE;
  user_pool.pool_size = user_free_pages * PG_SIZE;

  kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
  user_pool.pool_bitmap.btmp_bytes_len = ubm_length;

  kernel_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE;
  user_pool.pool_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length);

  // 输出内存池信息
  put_str("      kernel_pool_bitmap_start:");
  put_int((int)kernel_pool.pool_bitmap.bits);
  put_str(" kernel_pool_phy_addr_start:");
  put_int(kernel_pool.phy_addr_start);
  put_str("\n");
  put_str("      user_pool_bitmap_start:");
  put_int((int)user_pool.pool_bitmap.bits);
  put_str(" user_pool_phy_addr_start:");
  put_int(user_pool.phy_addr_start);
  put_str("\n");

  // 将位图置0
  bitmap_init(&kernel_pool.pool_bitmap);
  bitmap_init(&user_pool.pool_bitmap);

  // 初始化内核虚拟地址的位图，按实际物理内存大小生成数组
  kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length; // 位图字节数长度

  // 位图的数组指向一块未使用的内存，目前定位在内核内存池和用户内存池之外
  kernel_vaddr.vaddr_bitmap.bits =
      (void *)(MEM_BITMAP_BASE + kbm_length + ubm_length);

  kernel_vaddr.vaddr_start = K_HEAP_START;
  bitmap_init(&kernel_vaddr.vaddr_bitmap);
  put_str("   mem_pool_init done\n");
}

// 内存管理部分初始化入口
void mem_init() {
  put_str("mem_init start\n");
  uint32_t mem_bytes_total = (*(
      uint32_t
          *)(0xb00)); // 从内核主线程栈中获取总内存(loader.asm中设置的物理地址total_mem_bytes)
  mem_pool_init(mem_bytes_total); // 初始化内存池
  put_str("mem_init done: total_mem_bytes:");
  put_int(mem_bytes_total);
  put_str("\n");
}

// 在pf表示的虚拟内存池中申请pg_cnt个虚拟页
static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
  int vaddr_start = 0, bit_idx_start = -1;
  uint32_t cnt = 0;
  if (PF_KERNEL == pf) {
    bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
    if (-1 == bit_idx_start) {
      return NULL;
    }
    // 将对应的位图置为1
    while (cnt < pg_cnt) {
      bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
    }
    vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
  } else {
    // 用户内存池
  }
  return (void *)vaddr_start;
}

// 得到虚拟地址vaddr对应的pte指针
uint32_t *pte_ptr(uint32_t vaddr) {
  // 先访问到页表自己 + \
    // 再用页目录项pde(页目录内页表的索引)作为pte的索引访问到页表 + \
    // 再用pte的索引作为页内偏移
  uint32_t *pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) +
                               PTE_IDX(vaddr) * 4);

  return pte;
}

// 得到虚拟地址vaddr对应的pde指针
uint32_t *pde_ptr(uint32_t vaddr) {
  // 0xfffff用来访问到页表本身所在的地址
  uint32_t *pde = (uint32_t *)(0xfffff000 + PDE_IDX(vaddr) * 4);

  return pde;
}

// 在m_pool指向的物理内存池中分配1个物理页
static void *palloc(struct pool *m_pool) {
  /* 扫描或设置位图要保证原子操作 */
  int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1); // 找一个物理页面
  if (bit_idx == -1) {
    return NULL;
  }
  bitmap_set(&m_pool->pool_bitmap, bit_idx, 1); // 将此位bit_idx置1
  uint32_t page_phyaddr = ((bit_idx * PG_SIZE) + m_pool->phy_addr_start);
  return (void *)page_phyaddr;
}

// 页表中添加虚拟地址_vaddr与物理地址_page_phyaddr的映射
static void page_table_add(void *_vaddr, void *_page_phyaddr) {
  uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
  uint32_t *pde = pde_ptr(vaddr);
  uint32_t *pte = pte_ptr(vaddr);

  // 页目录项和页表项的第0位为P位，此处为1.
  // 此处判断目的是，虽然页表项和页目录项的第0位为P位，但是在页表项中，第0位表示是否存在此页
  // pde的第0位为1，表示该页目录项存在
  if (*pde & 0x00000001) {
    // 页目录项和页表项的第1位为R/W位，此处为1
    ASSERT(!(*pte & 0x00000001));
    if (!(*pte & 0x00000001)) {
      *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    } else {
      PANIC("pte repeat");
      *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
    }
  } else {
    // 页表中用到的页框一律从内核空间分配
    uint32_t pte_phyaddr = (uint32_t)palloc(&kernel_pool);
    *pde = (pte_phyaddr | PG_US_U | PG_RW_W | PG_P_1);

    // 分配到的物理页地址p打通到虚拟地址v
    *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
  }
}

// 分配pg_cnt个页空间，成功则返回起始虚拟地址，失败时返回NULL
void *malloc_page(enum pool_flags pf, uint32_t pg_cnt) {
  ASSERT(pg_cnt > 0 && pg_cnt < 3840);
  // malloc_page 的原理是三个动作的合成：1. 通过 vaddr_get
  // 在虚拟内存池中申请虚拟地址，2. 通过 palloc 在物理内存池中申请物理页，3.
  // 通过 page_table_add 建立虚拟地址到物理地址的映射
  void *vaddr_start = vaddr_get(pf, pg_cnt);
  if (NULL == vaddr_start) {
    return NULL;
  }
  uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
  struct pool *mem_pool = (PF_KERNEL == pf ? &kernel_pool : &user_pool);

  // 因为虚拟地址是连续的，但物理地址可以是不连续的，所以逐个做映射
  while (cnt-- > 0) {
    void *page_phyaddr = palloc(mem_pool);
    if (NULL == page_phyaddr) {
      return NULL;
    }
    page_table_add((void *)vaddr, page_phyaddr);
    vaddr += PG_SIZE; // 下一个虚拟页
  }
  return vaddr_start;
}

// 从内核物理内存池中申请pg_cnt页内存，成功则返回其虚拟地址，失败时返回NULL
void *get_kernel_pages(uint32_t pg_cnt) {
  void *vaddr = malloc_page(PF_KERNEL, pg_cnt);
  if (NULL != vaddr) {
    memset(vaddr, 0, pg_cnt * PG_SIZE);
  }
  return vaddr;
}
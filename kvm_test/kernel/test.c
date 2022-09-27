#include <fcntl.h>
#include <linux/kvm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

void kvm_init(uint8_t code[], size_t code_len)
{
    int kvmfd = open("/dev/kvm", O_RDWR|O_CLOEXEC);
    if(kvmfd < 0){
        printf("kvmfd = %d, failed to open kvm\n", kvmfd);
        exit(0);
    }
    printf("kvmfd = %d\n", kvmfd);
    int api_ver = ioctl(kvmfd, KVM_GET_API_VERSION, 0);
    if(api_ver < 0){
        printf("api_ver = %d, faild to get api version\n", api_ver);
        exit(0);
    }
    if(api_ver != 12){
        printf("api_ver = %d, please use api ver 12\n", api_ver);
        exit(0);
    }
    printf("api_ver = %d\n", api_ver);
    int vmfd = ioctl(kvmfd, KVM_CREATE_VM, 0);
    if(vmfd < 0){
        printf("vmfd = %d, failed to create vm\n", vmfd);
        exit(-1);
    }
    printf("vmfd = %d\n", vmfd);
    size_t memsize = 0x40000000; // 1G
    void *mem = mmap(0, memsize, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    printf("mem = %p\n", mem);
    int user_entry = 0;
    memcpy(mem+user_entry, code, code_len); // assemble code on the first page, address = 0
    struct kvm_userspace_memory_region region = 
    {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0,
        .memory_size = memsize,
        .userspace_addr = mem
    };
    ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region);

    int vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, 0);
    printf("vcpufd = %d\n", vcpufd);
    size_t vcpu_mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, 0);
    printf("vcpu_mmap_size = 0x%x\n", vcpu_mmap_size);
    struct kvm_run* run = (struct kvm_run*) mmap(0, vcpu_mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, vcpufd, 0);
    printf("run = %p\n", run);

    struct kvm_regs regs;
    ioctl(vcpufd, KVM_GET_REGS, &regs);
    regs.rip = user_entry; // zero address
    regs.rsp = 0x200000; // stack address
    regs.rflags = 0x2;
    ioctl(vcpufd, KVM_SET_REGS, &regs);

    struct kvm_sregs sregs;
    ioctl(vcpufd, KVM_GET_SREGS, &sregs);

    set_pagetable(mem, &sregs);
    set_segment_regs(&sregs);
    ioctl(vcpufd, KVM_SET_SREGS, &sregs);

    while(1){
        ioctl(vcpufd, KVM_RUN, NULL);
        switch(run->exit_reason){
            case KVM_EXIT_HLT:
                printf("KVM_EXIT_HLT\n");
                return 0;
            case KVM_EXIT_IO:
                putchar(*((char *)run + run->io.data_offset));
                break;
            case KVM_EXIT_FAIL_ENTRY:
                printf("KVM_EXIT_FAIL_ENTRY\n");
                return 0;
            case KVM_EXIT_INTERNAL_ERROR:
                printf("KVM_EXIT_INTERNAL_ERROR\n");
                return 0;
            case KVM_EXIT_SHUTDOWN:
                printf("KVM_EXIT_SHUTDOWN\n");
            default:
                printf("exit reason: %d\n", run->exit_reason);
        }
    }

}

int main()
{
    uint8_t *code = "H\xB8\x41\x42\x43\x44\x31\x32\x33\nj\bY\xBA\x17\x02\x00\x00\xEEH\xC1\xE8\b\xE2\xF9\xF4";
    size_t code_len = 100;
    kvm_init(code, code_len);
    return 0;
}

// these page tables can map address below 0x200000 to itself (i.e. virtual address equals to physical address).
void set_pagetable(void* mem, struct kvm_sregs *sregs){
    uint64_t pml4_addr = 0x1000;  // 4k
    uint64_t *pml4 = (void *)(mem + pml4_addr);

    uint64_t pdpt_addr = 0x2000;  //  4k + 4k
    uint64_t *pdpt = (void *)(mem + pdpt_addr);

    uint64_t pd_addr = 0x3000;   //  4k + 4k + 4k 
    uint64_t *pd = (void *)(mem + pd_addr);

    // 3 (PDE64_PRESENT|PDE64_RW) stands for the memory is mapped and writable
    pml4[0] = pdpt_addr | 3; // PDE64_PRESENT | PDE64_RW
    pdpt[0] = pd_addr | 3;
    pd[0] = 0x80 | 3; // 0x80 (PDE64_PS), 2M pagging，表示这是内存按照2M分页的
    printf("pml4[0] = %p\n", pml4[0]);
    printf("pdpt[0] = %p\n", pdpt[0]);
    printf("pd = %p\n", pd[0]);

    // 只给pml4 pdpt pd的第0项做了映射，所以我们只能找到第一个2M的页（2^21），也就是0～0x200000，所以目前我们只能使用0～0x200000的地址。
    /* Maps: 0 ~ 0x200000 -> 0 ~ 0x200000 */

// 在x64体系中只实现了48位的virtual address，高16位被用作符号扩展，这高16位要么全是0，要么全是1。所以在讨论64bit地址的时候，高16位不使用
// 数据64位长，因此一个4KB页在x64体系结构下只能包含512项内容,所以为了保证页对齐和以页为单位的页表内容换入换出，在x64下每级页表寻址部分长度定位9位。

// 一个虚拟地址转换成物理地址的计算过程就是，处理器通过CR3找到当前"page map level4"表所在物理页，
// 取虚拟地址从48位向低走的9位，然后把这9位右移3位（因为每个PML4E项8个字节长，右移3位相当于乘8）得到在该页中的地址，取出该地址处的PML4E（8个字节），就找到了该虚拟地址对应"pagedirectorypointer"表所在物理页，
// 然后同样方法依次 找出该虚拟地址对应的页目录所在物理页，该虚拟地址对应的页表所在物理页，该虚拟地址对应的物理页的物理地址，
// 最后将虚拟地址对应的物理页的物理地址加上 12位的页内偏移得到了物理地址。

    // CR3寄存器用来保存PML4的物理地址。在寻址的过程中最开始就是通过CR3来找到PML4的。
    sregs->cr3 = pml4_addr;
    sregs->cr4 = 1 << 5; // PAE
    sregs->cr4 |= 0x600; // CR4_OSFXSR | CR4_OSXMMEXCPT enable sse
    sregs->cr0 = 0x80050033; // CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG
    sregs->efer = 0x500; // EFER_LME | EFER_LMA
    sregs->efer |= 0x1; // EFER_SCE
}
void set_segment_regs(struct kvm_sregs *sregs){
    struct kvm_segment seg = {
        .base = 0,
        .limit = 0xffffffff,
        .selector = 1 << 3,
        .present = 1,
        .type = 11, /* execute, read, accessed */
        .dpl = 0, /* privilege level 0 */
        .db = 0,
        .s = 1,
        .l = 1,
        .g = 1,
    };
    sregs->cs = seg;
    seg.type = 3; /* read/write, accessed */
    seg.selector = 2 << 3;
    sregs->ds = sregs->es = sregs->fs = sregs->gs = sregs->ss = seg;

}
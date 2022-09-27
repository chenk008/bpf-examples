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

    // Set up memory for VM guest
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

    // 通过ioctl kvmfd拿到vcpu的大小并mmap这么大的空间给vcpu
    size_t vcpu_mmap_size = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, 0);
    printf("vcpu_mmap_size = 0x%x\n", vcpu_mmap_size);
    struct kvm_run* run = (struct kvm_run*) mmap(0, vcpu_mmap_size, PROT_READ|PROT_WRITE, MAP_SHARED, vcpufd, 0);
    printf("run = %p\n", run);

    // 设置vcpu的regs和sregs寄存器
    /* standard registers include general-purpose registers and flags */
    struct kvm_regs regs;
    ioctl(vcpufd, KVM_GET_REGS, &regs);
    regs.rip = user_entry; // zero address
    regs.rsp = 0x200000; // stack address
    regs.rflags = 0x2;
    ioctl(vcpufd, KVM_SET_REGS, &regs);

    /* special registers include segment registers */
    struct kvm_sregs sregs;
    ioctl(vcpufd, KVM_GET_SREGS, &sregs);
    sregs.cs.base = 0;
    sregs.cs.selector = 0;
    ioctl(vcpufd, KVM_SET_SREGS, &sregs);

    // 通过ioctl vcpufd run来运行，直到遇到需要退出vm的指令如hlt out等。需要对退出的原因进行处理，是结束运行还是IO中断还是出了问题shutdown等等。其中比较重要的是IO，in和out指令会触发KVM_EXIT_IO。这就是hypervisor和host通信的手段。
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
    // 现在没有页表所以不能运行32位/64位程序，现在处于实模式（Real Mode），只能运行16-bit的汇编代码。

    // default: 16bit mode assemble code
    uint8_t *code = "\xB0\x61\xBA\x17\x02\xEE\xB0\n\xEE\xF4";
    size_t code_len = 35;
    printf("Supposed to output a char 'a'\n");
    kvm_init(code, 10);
    return 0;
}
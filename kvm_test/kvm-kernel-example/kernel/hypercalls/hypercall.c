#include <hypercalls/hypercall.h>

int hypercall(uint16_t port, uint32_t data) {
  int ret = 0;
  asm(
    "mov dx, %[port];"
    "mov eax, %[data];"
    "out dx, eax;" // 写入端口
    "in eax, dx;"  // 从端口读取
    "mov %[ret], eax;"
    : [ret] "=r"(ret)
    : [port] "r"(port), [data] "r"(data)
    : "rax", "rdx"
    );
  return ret;
}

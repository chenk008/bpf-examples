objdump --syms /usr/local/bin/kubelet

https://blog.quarkslab.com/defeating-ebpf-uprobe-monitoring.html


https://lore.kernel.org/bpf/1643645554-28723-2-git-send-email-alan.maguire@oracle.com/
1. First, determine the symbol address using libelf; this gives us
   the offset as reported by objdump; then, in the case of local
   functions
2. If the function is a shared library function - and the binary
   provided is a shared library - no further work is required;
   the address found is the required address
3. If the function is a shared library function in a program
   (as opposed to a shared library), the Procedure Linking Table
   (PLT) table address is found (it is indexed via the dynamic
   symbol table index).  This allows us to instrument a call
   to a shared library function locally in the calling binary,
   reducing overhead versus having a breakpoint in global lib.
4. Finally, if the function is local, subtract the base address
   associated with the object, retrieved from ELF program headers.

The resultant value is then added to the func_offset value passed
in to specify the uprobe attach address.  So specifying a func_offset
of 0 along with a function name "printf" will attach to printf entry.

The modes of operation supported are then

1. to attach to a local function in a binary; function "foo1" in
   "/usr/bin/foo"
2. to attach to a shared library function in a binary;
   function "malloc" in "/usr/bin/foo"
3. to attach to a shared library function in a shared library -
   function "malloc" in libc.
	.text
	.file	"bpf_test.c"
	.globl	my_memcmp                       # -- Begin function my_memcmp
	.p2align	3
	.type	my_memcmp,@function
my_memcmp:                              # @my_memcmp
# %bb.0:
                                        # kill: def $r4 killed $r3
	*(u64 *)(r10 - 16) = r1
	*(u64 *)(r10 - 24) = r2
	*(u32 *)(r10 - 28) = r3
	r1 = *(u32 *)(r10 - 28)
	if r1 != 0 goto LBB0_2
	goto LBB0_1
LBB0_1:
	r1 = 0
	*(u32 *)(r10 - 4) = r1
	goto LBB0_8
LBB0_2:
	goto LBB0_3
LBB0_3:                                 # =>This Inner Loop Header: Depth=1
	r2 = *(u32 *)(r10 - 28)
	r2 += -1
	r1 = r2
	r1 <<= 32
	r1 >>= 32
	*(u32 *)(r10 - 28) = r2
	r2 = 0
	*(u64 *)(r10 - 40) = r2
	if r1 == 0 goto LBB0_5
	goto LBB0_4
LBB0_4:                                 #   in Loop: Header=BB0_3 Depth=1
	r1 = *(u64 *)(r10 - 16)
	r1 = *(u8 *)(r1 + 0)
	r1 <<= 56
	r1 s>>= 56
	r2 = *(u64 *)(r10 - 24)
	r2 = *(u8 *)(r2 + 0)
	r2 <<= 56
	r2 s>>= 56
	r3 = 0
	*(u64 *)(r10 - 56) = r3
	r3 = 1
	*(u64 *)(r10 - 48) = r3
	if r1 == r2 goto LBB0_10
# %bb.9:                                #   in Loop: Header=BB0_3 Depth=1
	r1 = *(u64 *)(r10 - 56)
	*(u64 *)(r10 - 48) = r1
LBB0_10:                                #   in Loop: Header=BB0_3 Depth=1
	r1 = *(u64 *)(r10 - 48)
	*(u64 *)(r10 - 40) = r1
	goto LBB0_5
LBB0_5:                                 #   in Loop: Header=BB0_3 Depth=1
	r1 = *(u64 *)(r10 - 40)
	r1 &= 1
	if r1 == 0 goto LBB0_7
	goto LBB0_6
LBB0_6:                                 #   in Loop: Header=BB0_3 Depth=1
	r1 = *(u64 *)(r10 - 16)
	r1 += 1
	*(u64 *)(r10 - 16) = r1
	r1 = *(u64 *)(r10 - 24)
	r1 += 1
	*(u64 *)(r10 - 24) = r1
	goto LBB0_3
LBB0_7:
	r1 = *(u64 *)(r10 - 16)
	r1 = *(u8 *)(r1 + 0)
	r2 = *(u64 *)(r10 - 24)
	r2 = *(u8 *)(r2 + 0)
	r1 -= r2
	*(u32 *)(r10 - 4) = r1
	goto LBB0_8
LBB0_8:
	r0 = *(u32 *)(r10 - 4)
	exit
.Lfunc_end0:
	.size	my_memcmp, .Lfunc_end0-my_memcmp
                                        # -- End function
	.section	"kretprobe/__x64_sys_read","ax",@progbits
	.globl	kretprobe_sys_read              # -- Begin function kretprobe_sys_read
	.p2align	3
	.type	kretprobe_sys_read,@function
kretprobe_sys_read:                     # @kretprobe_sys_read
# %bb.0:
	*(u64 *)(r10 - 16) = r1
	r1 = 0
	*(u8 *)(r10 - 20) = r1
	r1 = 1953719668
	*(u32 *)(r10 - 24) = r1
	r1 = bpf_get_current_comm ll
	r3 = *(u64 *)(r1 + 0)
	r1 = r10
	r1 += -48
	*(u64 *)(r10 - 456) = r1
	r2 = 24
	callx r3
	r1 = *(u64 *)(r10 - 456)
	r2 = r10
	r2 += -24
	r3 = 5
	call my_memcmp
	r1 = r0
	r1 <<= 32
	r1 >>= 32
	if r1 == 0 goto LBB1_2
	goto LBB1_1
LBB1_1:
	r1 = 0
	*(u32 *)(r10 - 4) = r1
	goto LBB1_16
LBB1_2:
	r1 = *(u64 *)(r10 - 16)
	r1 = *(u64 *)(r1 + 112)
	*(u64 *)(r10 - 56) = r1
	r1 = *(u64 *)(r10 - 56)
	if r1 != 0 goto LBB1_4
	goto LBB1_3
LBB1_3:
	r1 = 7894115
	*(u32 *)(r10 - 64) = r1
	r1 = 2336349433541126514 ll
	*(u64 *)(r10 - 72) = r1
	r1 = 8007510507108704367 ll
	*(u64 *)(r10 - 80) = r1
	r1 = 8367798494427701606 ll
	*(u64 *)(r10 - 88) = r1
	r1 = bpf_trace_printk ll
	r3 = *(u64 *)(r1 + 0)
	r1 = r10
	r1 += -88
	r2 = 28
	callx r3
	*(u64 *)(r10 - 96) = r0
	r1 = 0
	*(u32 *)(r10 - 4) = r1
	goto LBB1_16
LBB1_4:
	r1 = bpf_probe_read ll
	r4 = *(u64 *)(r1 + 0)
	r3 = *(u64 *)(r10 - 56)
	r3 += 104
	r1 = r10
	r1 += -104
	r2 = 8
	callx r4
	r1 = *(u64 *)(r10 - 104)
	if r1 != 0 goto LBB1_6
	goto LBB1_5
LBB1_5:
	r1 = 0
	*(u64 *)(r10 - 464) = r1
	*(u8 *)(r10 - 112) = r1
	r1 = 7812748569432254752 ll
	*(u64 *)(r10 - 120) = r1
	r1 = 7382915055406376306 ll
	*(u64 *)(r10 - 128) = r1
	r1 = bpf_trace_printk ll
	r3 = *(u64 *)(r1 + 0)
	r1 = r10
	r1 += -128
	r2 = 17
	callx r3
	r1 = *(u64 *)(r10 - 464)
	*(u64 *)(r10 - 136) = r0
	*(u32 *)(r10 - 4) = r1
	goto LBB1_16
LBB1_6:
	r1 = 0
	*(u64 *)(r10 - 472) = r1
	*(u8 *)(r10 - 138) = r1
	r1 = 28709
	*(u16 *)(r10 - 140) = r1
	r1 = 544434464
	*(u32 *)(r10 - 144) = r1
	r1 = 7382915055406376306 ll
	*(u64 *)(r10 - 152) = r1
	r1 = bpf_trace_printk ll
	*(u64 *)(r10 - 488) = r1
	r4 = *(u64 *)(r1 + 0)
	r3 = *(u64 *)(r10 - 104)
	r1 = r10
	r1 += -152
	r2 = 15
	callx r4
	*(u64 *)(r10 - 160) = r0
	r1 = bpf_get_current_pid_tgid ll
	r1 = *(u64 *)(r1 + 0)
	callx r1
	r0 >>= 32
	*(u32 *)(r10 - 164) = r0
	r1 = bpf_map_lookup_elem ll
	r3 = *(u64 *)(r1 + 0)
	r1 = written ll
	r2 = r10
	r2 += -164
	callx r3
	r1 = *(u64 *)(r10 - 488)
	*(u64 *)(r10 - 176) = r0
	r2 = 10
	*(u64 *)(r10 - 480) = r2
	*(u16 *)(r10 - 180) = r2
	r2 = 1680154739
	*(u32 *)(r10 - 184) = r2
	r2 = 7575129654904781133 ll
	*(u64 *)(r10 - 192) = r2
	r2 = 2315208993965895023 ll
	*(u64 *)(r10 - 200) = r2
	r2 = 8243311542983750255 ll
	*(u64 *)(r10 - 208) = r2
	r2 = 8583909746840200520 ll
	*(u64 *)(r10 - 216) = r2
	r4 = *(u64 *)(r1 + 0)
	r3 = *(u32 *)(r10 - 164)
	r1 = r10
	r1 += -216
	r2 = 38
	callx r4
	r1 = *(u64 *)(r10 - 488)
	*(u64 *)(r10 - 224) = r0
	r2 = *(u64 *)(r10 - 16)
	r2 = *(u64 *)(r2 + 80)
	*(u32 *)(r10 - 228) = r2
	r2 = 680997
	*(u32 *)(r10 - 232) = r2
	r2 = 2338340314025189228 ll
	*(u64 *)(r10 - 240) = r2
	r2 = 7020664761988837225 ll
	*(u64 *)(r10 - 248) = r2
	r4 = *(u64 *)(r1 + 0)
	r3 = *(u32 *)(r10 - 228)
	r1 = r10
	r1 += -248
	r2 = 20
	callx r4
	r2 = *(u64 *)(r10 - 480)
	r1 = *(u64 *)(r10 - 472)
	*(u64 *)(r10 - 256) = r0
	*(u16 *)(r10 - 264) = r1
	r1 = 7764065
	*(u64 *)(r10 - 272) = r1
	r1 = bpf_probe_read_user ll
	r4 = *(u64 *)(r1 + 0)
	r3 = *(u64 *)(r10 - 104)
	r1 = r10
	r1 += -272
	callx r4
	*(u32 *)(r10 - 276) = r0
	r1 = *(u32 *)(r10 - 276)
	if r1 == 0 goto LBB1_8
	goto LBB1_7
LBB1_7:
	r1 = 2924860824249715 ll
	*(u64 *)(r10 - 288) = r1
	r1 = 8439856071168893039 ll
	*(u64 *)(r10 - 296) = r1
	r1 = 8367798494427701606 ll
	*(u64 *)(r10 - 304) = r1
	r1 = bpf_trace_printk ll
	r4 = *(u64 *)(r1 + 0)
	r3 = *(u32 *)(r10 - 276)
	r1 = r10
	r1 += -304
	r2 = 24
	callx r4
	*(u64 *)(r10 - 312) = r0
	goto LBB1_8
LBB1_8:
	r1 = 10
	*(u16 *)(r10 - 316) = r1
	r1 = 1931812979
	*(u32 *)(r10 - 320) = r1
	r1 = 7575164959875753330 ll
	*(u64 *)(r10 - 328) = r1
	r1 = bpf_trace_printk ll
	r4 = *(u64 *)(r1 + 0)
	r1 = r10
	r1 += -328
	r2 = 14
	r3 = r10
	r3 += -272
	callx r4
	*(u64 *)(r10 - 336) = r0
	r1 = *(u64 *)(r10 - 104)
	if r1 != 0 goto LBB1_10
	goto LBB1_9
LBB1_9:
	r1 = 0
	*(u32 *)(r10 - 4) = r1
	goto LBB1_16
LBB1_10:
	r1 = *(u64 *)(r10 - 176)
	if r1 == 0 goto LBB1_13
	goto LBB1_11
LBB1_11:
	r1 = *(u64 *)(r10 - 176)
	r1 = *(u32 *)(r1 + 0)
	if r1 != 1 goto LBB1_13
	goto LBB1_12
LBB1_12:
	r2 = 10
	*(u16 *)(r10 - 344) = r2
	r1 = 3467893150951171442 ll
	*(u64 *)(r10 - 352) = r1
	r1 = bpf_trace_printk ll
	r3 = *(u64 *)(r1 + 0)
	r1 = r10
	r1 += -352
	callx r3
	*(u64 *)(r10 - 360) = r0
	r4 = 0
	*(u64 *)(r10 - 496) = r4
	*(u32 *)(r10 - 364) = r4
	r1 = bpf_map_update_elem ll
	r5 = *(u64 *)(r1 + 0)
	r1 = written ll
	r2 = r10
	r2 += -164
	r3 = r10
	r3 += -364
	callx r5
	r2 = *(u64 *)(r10 - 496)
	r1 = bpf_override_return ll
	r3 = *(u64 *)(r1 + 0)
	r1 = *(u64 *)(r10 - 16)
	callx r3
	r1 = *(u64 *)(r10 - 496)
	*(u64 *)(r10 - 104) = r1
	*(u32 *)(r10 - 4) = r1
	goto LBB1_16
LBB1_13:
	r1 = 0
	*(u8 *)(r10 - 366) = r1
	r1 = 25700
	*(u16 *)(r10 - 368) = r1
	r1 = bpf_probe_write_user ll
	r4 = *(u64 *)(r1 + 0)
	r1 = r10
	r1 += -104
	r2 = r10
	r2 += -368
	r3 = 3
	callx r4
	*(u32 *)(r10 - 276) = r0
	r1 = *(u32 *)(r10 - 276)
	if r1 == 0 goto LBB1_15
	goto LBB1_14
LBB1_14:
	r1 = 0
	*(u8 *)(r10 - 376) = r1
	r1 = 748764371007927157 ll
	*(u64 *)(r10 - 384) = r1
	r1 = 2334399978102268015 ll
	*(u64 *)(r10 - 392) = r1
	r1 = 8367798494427701606 ll
	*(u64 *)(r10 - 400) = r1
	r1 = bpf_trace_printk ll
	r4 = *(u64 *)(r1 + 0)
	r3 = *(u32 *)(r10 - 276)
	r1 = r10
	r1 += -400
	r2 = 25
	callx r4
	*(u64 *)(r10 - 408) = r0
	goto LBB1_15
LBB1_15:
	r1 = 0
	*(u64 *)(r10 - 504) = r1
	*(u8 *)(r10 - 416) = r1
	r1 = 748006974760710757 ll
	*(u64 *)(r10 - 424) = r1
	r1 = 8319591506753712759 ll
	*(u64 *)(r10 - 432) = r1
	r1 = bpf_trace_printk ll
	r3 = *(u64 *)(r1 + 0)
	r1 = r10
	r1 += -432
	r2 = 17
	callx r3
	*(u64 *)(r10 - 440) = r0
	r1 = bpf_override_return ll
	r3 = *(u64 *)(r1 + 0)
	r1 = *(u64 *)(r10 - 16)
	r2 = 3
	callx r3
	r4 = *(u64 *)(r10 - 504)
	r1 = 1
	*(u32 *)(r10 - 444) = r1
	r1 = bpf_map_update_elem ll
	r5 = *(u64 *)(r1 + 0)
	r1 = written ll
	r2 = r10
	r2 += -164
	r3 = r10
	r3 += -444
	callx r5
	r1 = *(u64 *)(r10 - 504)
	*(u64 *)(r10 - 104) = r1
	*(u32 *)(r10 - 4) = r1
	goto LBB1_16
LBB1_16:
	r0 = *(u32 *)(r10 - 4)
	exit
.Lfunc_end1:
	.size	kretprobe_sys_read, .Lfunc_end1-kretprobe_sys_read
                                        # -- End function
	.type	written,@object                 # @written
	.section	maps,"aw",@progbits
	.globl	written
	.p2align	2
written:
	.long	1                               # 0x1
	.long	4                               # 0x4
	.long	4                               # 0x4
	.long	100                             # 0x64
	.long	1                               # 0x1
	.size	written, 20

	.type	.L__const.kretprobe_sys_read.TARGET_NAME,@object # @__const.kretprobe_sys_read.TARGET_NAME
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__const.kretprobe_sys_read.TARGET_NAME:
	.asciz	"test"
	.size	.L__const.kretprobe_sys_read.TARGET_NAME, 5

	.type	bpf_get_current_comm,@object    # @bpf_get_current_comm
	.data
	.p2align	3
bpf_get_current_comm:
	.quad	16
	.size	bpf_get_current_comm, 8

	.type	.L__const.kretprobe_sys_read.____fmt,@object # @__const.kretprobe_sys_read.____fmt
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__const.kretprobe_sys_read.____fmt:
	.asciz	"failed to load original ctx"
	.size	.L__const.kretprobe_sys_read.____fmt, 28

	.type	bpf_trace_printk,@object        # @bpf_trace_printk
	.data
	.p2align	3
bpf_trace_printk:
	.quad	6
	.size	bpf_trace_printk, 8

	.type	bpf_probe_read,@object          # @bpf_probe_read
	.p2align	3
bpf_probe_read:
	.quad	4
	.size	bpf_probe_read, 8

	.type	.L__const.kretprobe_sys_read.____fmt.1,@object # @__const.kretprobe_sys_read.____fmt.1
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__const.kretprobe_sys_read.____fmt.1:
	.asciz	"read buf is null"
	.size	.L__const.kretprobe_sys_read.____fmt.1, 17

	.type	.L__const.kretprobe_sys_read.____fmt.2,@object # @__const.kretprobe_sys_read.____fmt.2
.L__const.kretprobe_sys_read.____fmt.2:
	.asciz	"read buf is %p"
	.size	.L__const.kretprobe_sys_read.____fmt.2, 15

	.type	bpf_get_current_pid_tgid,@object # @bpf_get_current_pid_tgid
	.data
	.p2align	3
bpf_get_current_pid_tgid:
	.quad	14
	.size	bpf_get_current_pid_tgid, 8

	.type	bpf_map_lookup_elem,@object     # @bpf_map_lookup_elem
	.p2align	3
bpf_map_lookup_elem:
	.quad	1
	.size	bpf_map_lookup_elem, 8

	.type	.L__const.kretprobe_sys_read.____fmt.3,@object # @__const.kretprobe_sys_read.____fmt.3
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__const.kretprobe_sys_read.____fmt.3:
	.asciz	"Hello, world, from BPF! My PID is %d\n"
	.size	.L__const.kretprobe_sys_read.____fmt.3, 38

	.type	.L__const.kretprobe_sys_read.____fmt.4,@object # @__const.kretprobe_sys_read.____fmt.4
.L__const.kretprobe_sys_read.____fmt.4:
	.asciz	"ioriginal_re ts %d\n"
	.size	.L__const.kretprobe_sys_read.____fmt.4, 20

	.type	.L__const.kretprobe_sys_read.readed,@object # @__const.kretprobe_sys_read.readed
	.section	.rodata,"a",@progbits
.L__const.kretprobe_sys_read.readed:
	.asciz	"axv\000\000\000\000\000\000"
	.size	.L__const.kretprobe_sys_read.readed, 10

	.type	bpf_probe_read_user,@object     # @bpf_probe_read_user
	.data
	.p2align	3
bpf_probe_read_user:
	.quad	112
	.size	bpf_probe_read_user, 8

	.type	.L__const.kretprobe_sys_read.____fmt.5,@object # @__const.kretprobe_sys_read.____fmt.5
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__const.kretprobe_sys_read.____fmt.5:
	.asciz	"failed to read user:%d\n"
	.size	.L__const.kretprobe_sys_read.____fmt.5, 24

	.type	.L__const.kretprobe_sys_read.____fmt.6,@object # @__const.kretprobe_sys_read.____fmt.6
.L__const.kretprobe_sys_read.____fmt.6:
	.asciz	"readed is %s\n"
	.size	.L__const.kretprobe_sys_read.____fmt.6, 14

	.type	.L__const.kretprobe_sys_read.____fmt.7,@object # @__const.kretprobe_sys_read.____fmt.7
.L__const.kretprobe_sys_read.____fmt.7:
	.asciz	"return 0\n"
	.size	.L__const.kretprobe_sys_read.____fmt.7, 10

	.type	bpf_map_update_elem,@object     # @bpf_map_update_elem
	.data
	.p2align	3
bpf_map_update_elem:
	.quad	2
	.size	bpf_map_update_elem, 8

	.type	bpf_override_return,@object     # @bpf_override_return
	.p2align	3
bpf_override_return:
	.quad	58
	.size	bpf_override_return, 8

	.type	.L__const.kretprobe_sys_read.payload,@object # @__const.kretprobe_sys_read.payload
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__const.kretprobe_sys_read.payload:
	.asciz	"dd"
	.size	.L__const.kretprobe_sys_read.payload, 3

	.type	bpf_probe_write_user,@object    # @bpf_probe_write_user
	.data
	.p2align	3
bpf_probe_write_user:
	.quad	36
	.size	bpf_probe_write_user, 8

	.type	.L__const.kretprobe_sys_read.____fmt.8,@object # @__const.kretprobe_sys_read.____fmt.8
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__const.kretprobe_sys_read.____fmt.8:
	.asciz	"failed to write user:%d\n"
	.size	.L__const.kretprobe_sys_read.____fmt.8, 25

	.type	.L__const.kretprobe_sys_read.____fmt.9,@object # @__const.kretprobe_sys_read.____fmt.9
.L__const.kretprobe_sys_read.____fmt.9:
	.asciz	"write user data\n"
	.size	.L__const.kretprobe_sys_read.____fmt.9, 17

	.type	__license,@object               # @__license
	.section	license,"aw",@progbits
	.globl	__license
__license:
	.asciz	"GPL"
	.size	__license, 4

	.addrsig
	.addrsig_sym my_memcmp
	.addrsig_sym kretprobe_sys_read
	.addrsig_sym written
	.addrsig_sym bpf_get_current_comm
	.addrsig_sym bpf_trace_printk
	.addrsig_sym bpf_probe_read
	.addrsig_sym bpf_get_current_pid_tgid
	.addrsig_sym bpf_map_lookup_elem
	.addrsig_sym bpf_probe_read_user
	.addrsig_sym bpf_map_update_elem
	.addrsig_sym bpf_override_return
	.addrsig_sym bpf_probe_write_user
	.addrsig_sym __license

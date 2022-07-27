#include <stdint.h>
#include <arpa/inet.h>
#include <asm/byteorder.h>
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/pkt_cls.h>

/*
* Sample XDP/tc program, sets the TCP PSH flag on every RATIO packet.
* compile it with:
*      clang -O2 -emit-llvm -c tcp_psh.c -o - |llc -march=bpf -filetype=obj -o tcp_psh.o
 * attach it to a device with XDP as:
 * 	ip link set dev lo xdp object tcp_psh.o verbose
* attach it to a device with tc as:
*      tc qdisc add dev eth0 clsact
*      tc filter add dev eth0 egress matchall action bpf object-file tcp_psh.o
* replace the bpf with
*      tc filter replace dev eth0 egress matchall action bpf object-file tcp_psh.o
*/

#define SEC(NAME) __attribute__((section(NAME), used))
#define RATIO 10

/* from bpf_helpers.h */
static unsigned long long (*bpf_get_prandom_u32)(void) =
	(void *) BPF_FUNC_get_prandom_u32;

static int tcp_psh(void *data, void *data_end)
{
	struct ethhdr *eth = (struct ethhdr *)data;
	struct iphdr *iph = (struct iphdr *)(eth + 1);
	struct tcphdr *tcphdr = (struct tcphdr *)(iph + 1);

	/* sanity check needed by the eBPF verifier */
	if ((void *)(tcphdr + 1) > data_end)
		return 0;

	/* skip non TCP packets */
	if (eth->h_proto != __constant_htons(ETH_P_IP) || iph->protocol != IPPROTO_TCP)
		return 0;

	/* incompatible flags, or PSH already set */
	if (tcphdr->syn || tcphdr->fin || tcphdr->rst || tcphdr->psh)
		return 0;

	if (bpf_get_prandom_u32() % RATIO == 0)
		tcphdr->psh = 1;

	/* recalculate the checksum? */

	return 0;
}

SEC("prog")
int xdp_main(struct xdp_md *ctx)
{
	void *data_end = (void *)(uintptr_t)ctx->data_end;
	void *data = (void *)(uintptr_t)ctx->data;

	if (tcp_psh(data, data_end))
		return XDP_DROP;

	return XDP_PASS;
}

SEC("action")
int tc_main(struct __sk_buff *skb)
{
	void *data = (void *)(uintptr_t)skb->data;
	void *data_end = (void *)(uintptr_t)skb->data_end;

	if (tcp_psh(data, data_end))
		return TC_ACT_SHOT;

	return TC_ACT_OK;
}

char _license[] SEC("license") = "GPL";
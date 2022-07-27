#include <uapi/linux/ptrace.h>
#include <net/sock.h>
// #include <arpa/inet.h>
#include <bcc/proto.h>
#include <linux/if_ether.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>

#define IP_TCP 6
#define ETH_HLEN 14

struct Key
{
	u32 src_ip;				 // source ip
	u32 dst_ip;				 // destination ip
	unsigned short src_port; // source port
	unsigned short dst_port; // destination port
};

struct Leaf
{
	int timestamp; // timestamp in ns
};

// BPF_TABLE(map_type, key_type, leaf_type, table_name, num_entry)
// map <Key, Leaf>
// tracing sessions having same Key(dst_ip, src_ip, dst_port,src_port)
BPF_TABLE("hash", struct Key, struct Leaf, sessions, 1024);

/*eBPF program.
  Filter IP and TCP packets, having payload not empty
  and containing "HTTP", "GET", "POST"  as first bytes of payload.
  AND ALL the other packets having same (src_ip,dst_ip,src_port,dst_port)
  this means belonging to the same "session"
  this additional check avoids url truncation, if url is too long
  userspace script, if necessary, reassembles urls splitted in 2 or more packets.
  if the program is loaded as PROG_TYPE_SOCKET_FILTER
  and attached to a socket
  return  0 -> DROP the packet
  return -1 -> KEEP the packet and return it to user space (userspace can read it from the socket_fd )
*/
int http_filter(struct __sk_buff *skb)
{
	struct Leaf *lookup_leaf;

	u8 *cursor = 0;

	struct ethernet_t *ethernet = cursor_advance(cursor, sizeof(*ethernet));
	// filter IP packets (ethernet type = 0x0800)
	if (!(ethernet->type == 0x0800))
	{
		goto DROP;
	}

	struct ip_t *ip = cursor_advance(cursor, sizeof(*ip));
	// filter TCP packets (ip next protocol = 0x06)
	if (ip->nextp != IP_TCP)
	{
		goto DROP;
	}

	u32 tcp_header_length = 0;
	u32 ip_header_length = 0;
	u32 payload_offset = 0;
	u32 payload_length = 0;
	struct Key key;
	struct Leaf zero = {2};

	struct tcp_t *tcp = cursor_advance(cursor, sizeof(*tcp));

	// retrieve ip src/dest and port src/dest of current packet
	// and save it into struct Key
	key.dst_ip = ip->dst;
	key.src_ip = ip->src;
	key.dst_port = tcp->dst_port;
	key.src_port = tcp->src_port;

	// bpf_trace_printk("src ip:%d",ip->src);

	// calculate ip header length
	// value to multiply * 4
	// e.g. ip->hlen = 5 ; IP Header Length = 5 x 4 byte = 20 byte
	ip_header_length = ip->hlen << 2; // SHL 2 -> *4 multiply

	// calculate tcp header length
	// value to multiply *4
	// e.g. tcp->offset = 5 ; TCP Header Length = 5 x 4 byte = 20 byte
	tcp_header_length = tcp->offset << 2; // SHL 2 -> *4 multiply

	// calculate patload offset and length
	payload_offset = ETH_HLEN + ip_header_length + tcp_header_length;
	payload_length = ip->tlen - ip_header_length - tcp_header_length;

	// http://stackoverflow.com/questions/25047905/http-request-minimum-size-in-bytes
	// minimum length of http request is always geater than 7 bytes
	// avoid invalid access memory
	// include empty payload
	if (payload_length < 7)
	{
		goto DROP;
	}

	// load firt 7 byte of payload into p (payload_array)
	// direct access to skb not allowed
	unsigned long p[7] = {0};
	int i = 0;
	int j = 0;
	for (i = payload_offset; i < (payload_offset + 7); i++)
	{
		p[j] = load_byte(skb, i);
		j++;
	}

	// find a match with an HTTP message
	// HTTP
	if ((p[0] == 'H') && (p[1] == 'T') && (p[2] == 'T') && (p[3] == 'P'))
	{
		goto HTTP_MATCH;
	}
	// GET
	if ((p[0] == 'G') && (p[1] == 'E') && (p[2] == 'T'))
	{
		goto HTTP_MATCH;
	}
	// POST
	if ((p[0] == 'P') && (p[1] == 'O') && (p[2] == 'S') && (p[3] == 'T'))
	{
		goto HTTP_MATCH;
	}
	// PUT
	if ((p[0] == 'P') && (p[1] == 'U') && (p[2] == 'T'))
	{
		goto HTTP_MATCH;
	}
	// DELETE
	if ((p[0] == 'D') && (p[1] == 'E') && (p[2] == 'L') && (p[3] == 'E') && (p[4] == 'T') && (p[5] == 'E'))
	{
		goto HTTP_MATCH;
	}
	// HEAD
	if ((p[0] == 'H') && (p[1] == 'E') && (p[2] == 'A') && (p[3] == 'D'))
	{
		goto HTTP_MATCH;
	}

	// no HTTP match
	// check if packet belong to an HTTP session

	// lookup_leaf = sessions.lookup(&key);
	// if(lookup_leaf) {
	// 	// char *xpack_saddr = inet_ntoa(ip->src);
	// 	bpf_trace_printk("src ip:%d,%d",ip->src,lookup_leaf->timestamp);
	// 	if (lookup_leaf->timestamp == 1){
	// 		goto DROP;
	// 	}
	// 	//send packet to userspace
	// 	goto KEEP;
	// }else{
	// 	sessions.lookup_or_init(&key,&zero);
	// }
	goto DROP;

// keep the packet and send it to userspace retruning -1
HTTP_MATCH:
	lookup_leaf = sessions.lookup(&key);
	if (lookup_leaf)
	{
		// char *xpack_saddr = inet_ntoa(ip->src);
		bpf_trace_printk("src ip:%d,%d", ip->src, lookup_leaf->timestamp);
		if (lookup_leaf->timestamp == 1)
		{
			goto DROP;
		}
		// send packet to userspace
		goto KEEP;
	}
	else
	{
		sessions.lookup_or_init(&key, &zero);
	}
// if not already present, insert into map <Key, Leaf>

// send packet to userspace returning -1
KEEP:
	return -1;

// drop the packet returning 0
DROP:
	return 0;
}

#define SEC(NAME) __attribute__((section(NAME), used))

// #define htons(x) ((__be16)___constant_swab16((x)))
// #define htonl(x) ((__be32)___constant_swab32((x)))

#define TCP_RST_OFF (ETH_HLEN + sizeof(struct iphdr) + offsetof(struct tcphdr, rst))
#define TCP_CSUM_OFF (ETH_HLEN + sizeof(struct iphdr) + offsetof(struct tcphdr, check))


int xdp_prog1(struct xdp_md *ctx)
{
	void *data = (void *)(long)ctx->data;
	void *data_end = (void *)(long)ctx->data_end;
	struct ethhdr *eth = data;
	if ((void *)eth + sizeof(*eth) <= data_end)
	{
		struct iphdr *ip = data + sizeof(*eth);
		if ((void *)ip + sizeof(*ip) <= data_end)
		{
			if (ip->protocol == IPPROTO_TCP)
			{
		// 		bpf_trace_printk("src: %llu, dst: %llu, proto: %u\n",
		//    ether_addr_to_u64(eth->h_source),
		//    ether_addr_to_u64(eth->h_dest),
		//    bpf_ntohs(eth->h_proto));
				struct tcphdr *tcp = (void *)ip + sizeof(*ip);
				if ((void *)tcp + sizeof(*tcp) <= data_end)
				{
					struct Key key;
					key.dst_ip = bpf_ntohl(ip->daddr);
					key.src_ip = bpf_ntohl(ip->saddr);
					key.dst_port = bpf_ntohs(tcp->dest);
					key.src_port = bpf_ntohs(tcp->source);
					// bpf_trace_printk("xdp got, src ip:%d, src port:%d, dst port:%d", key.src_ip, key.src_port, key.dst_port);
					struct Leaf *lookup_leaf = sessions.lookup(&key);
					if (lookup_leaf)
					{
						// char *xpack_saddr = inet_ntoa(ip->src);
						if (lookup_leaf->timestamp == 1)
						{
							bpf_trace_printk("xdp drop src ip:%d,%d,%x", key.src_ip, key.src_port,bpf_ntohs(tcp->check));
							tcp->rst=1;
							// tcp->ack=0;
							// return XDP_DROP;
							// Update tcp checksum

							// checksum 是 32位
							unsigned long sum;
							sum = (~1 & 0xffff);
							sum += bpf_ntohs(tcp->check);
							// 就是将一个64位的数的 低16位 加上 高48位
							sum = (sum & 0xffff) + (sum>>16);
							tcp->check = bpf_htons(sum + (sum>>16) + 1 - 4);
							bpf_trace_printk("xdp drop src ip,cksum:%x", bpf_ntohs(tcp->check));

							return XDP_PASS;
						}
					}
				}
			}
		}
	}
	return XDP_PASS;
}

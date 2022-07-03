struct bpf_map_def __section("maps") sock_ops_map = {
	.type           = BPF_MAP_TYPE_SOCKHASH,
	.key_size       = sizeof(struct sock_key),
	.value_size     = sizeof(int),             // 存储 socket
	.max_entries    = 65535,
	.map_flags      = 0,
};

struct sock_key {
	uint32_t sip4;    // 源 IP
	uint32_t dip4;    // 目的 IP
	uint8_t  family;  // 协议类型
	uint8_t  pad1;    // this padding required for 64bit alignment
	uint16_t pad2;    // else ebpf kernel verifier rejects loading of the program
	uint32_t pad3;
	uint32_t sport;   // 源端口
	uint32_t dport;   // 目的端口
} __attribute__((packed));


// 当 attach 了这段程序的 socket 上有 sendmsg 系统调用时，内核就会执行这段代码
__section("sk_msg") // 加载目标文件（ELF ）中的 `sk_msg` section，`sendmsg` 系统调用时触发执行
int bpf_redir(struct sk_msg_md *msg)
{
    struct sock_key key = {};
    extract_key4_from_msg(msg, &key);
    msg_redirect_hash(msg, &sock_ops_map, &key, BPF_F_INGRESS);
    return SK_PASS;
}

static inline
void extract_key4_from_msg(struct sk_msg_md *msg, struct sock_key *key)
{
    key->sip4 = msg->remote_ip4;
    key->dip4 = msg->local_ip4;
    key->family = 1;

    key->dport = (bpf_htonl(msg->local_port) >> 16);
    key->sport = FORCE_READ(msg->remote_port) >> 16;
}
#ifndef LINUX_SKBUFF_H
#define LINUX_SKBUFF_H

#include <stdint.h>
#include <stddef.h>

struct sk_buff {
    unsigned char *data;
    unsigned int len;
    unsigned int headroom;
    unsigned int tailroom;
    void *dev;
};

struct sk_buff *alloc_skb(unsigned int size, int priority);
struct sk_buff *skb_clone(const struct sk_buff *skb, int priority);
void kfree_skb(struct sk_buff *skb);

#endif

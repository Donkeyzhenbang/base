/*
 * simnet.c - simple simulated Ethernet NIC kernel module
 *
 * Features:
 *  - allocates a net_device "sim0" and registers as Ethernet
 *  - implements net_device_ops: ndo_open/ndo_stop/ndo_start_xmit
 *  - uses NAPI for RX processing
 *  - TX ring simulated: start_xmit queues skbs into a software ring,
 *    a delayed work completes transmissions and frees skbs (simulates hw)
 *  - provides a misc char device /dev/simnet_inject for userspace to
 *    write raw Ethernet frames which the driver will inject as RX pkts
 *
 * Build with the provided Makefile. Test with provided userspace tool.
 *
 * This code is intended for learning & experimentation only.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>

#define DRV_NAME "simnet"
#define DEFAULT_TX_RING 64
#define TX_COMPLETE_DELAY_MS 50
#define INJECT_DEV_NAME "simnet_inject"
#define MAX_INJECT_SIZE 1600

struct simnet_priv {
    struct net_device *ndev;
    struct napi_struct napi;

    /* simulated tx ring */
    struct sk_buff **tx_ring;
    unsigned int tx_ring_size;
    unsigned int tx_head;
    unsigned int tx_tail;
    unsigned int tx_count;
    spinlock_t tx_lock;

    struct delayed_work tx_work;
    struct workqueue_struct *wq;

    /* rx queue for NAPI */
    struct sk_buff_head rx_q;

    /* stats */
    unsigned long tx_packets;
    unsigned long rx_packets;
};

static int simnet_open(struct net_device *ndev)
{
    struct simnet_priv *priv = netdev_priv(ndev);

    netif_start_queue(ndev);
    napi_enable(&priv->napi);
    skb_queue_head_init(&priv->rx_q);
    pr_info(DRV_NAME ": %s: opened\n", ndev->name);
    return 0;
}

static int simnet_stop(struct net_device *ndev)
{
    struct simnet_priv *priv = netdev_priv(ndev);

    netif_stop_queue(ndev);
    napi_disable(&priv->napi);

    /* flush any remaining tx skbs */
    spin_lock_bh(&priv->tx_lock);
    while (priv->tx_count) {
        struct sk_buff *skb = priv->tx_ring[priv->tx_head];
        priv->tx_ring[priv->tx_head] = NULL;
        priv->tx_head = (priv->tx_head + 1) % priv->tx_ring_size;
        priv->tx_count--;
        spin_unlock_bh(&priv->tx_lock);
        if (skb) {
            dev_kfree_skb_any(skb);
        }
        spin_lock_bh(&priv->tx_lock);
    }
    spin_unlock_bh(&priv->tx_lock);

    pr_info(DRV_NAME ": %s: stopped\n", ndev->name);
    return 0;
}

static void simnet_tx_complete_work(struct work_struct *work)
{
    struct simnet_priv *priv = container_of(work, struct simnet_priv, tx_work.work);
    unsigned int count = 0;

    /* complete up to half the ring to simulate bursts */
    spin_lock_bh(&priv->tx_lock);
    while (priv->tx_count && count < priv->tx_ring_size / 2) {
        struct sk_buff *skb = priv->tx_ring[priv->tx_head];
        priv->tx_ring[priv->tx_head] = NULL;
        priv->tx_head = (priv->tx_head + 1) % priv->tx_ring_size;
        priv->tx_count--;
        count++;
        spin_unlock_bh(&priv->tx_lock);

        /* simulate completion */
        priv->tx_packets++;
        priv->ndev->stats.tx_packets++;
        priv->ndev->stats.tx_bytes += skb->len;
        dev_consume_skb_any(skb);

        spin_lock_bh(&priv->tx_lock);
    }

    if (netif_queue_stopped(priv->ndev) && ! (priv->tx_count >= priv->tx_ring_size)) {
        netif_wake_queue(priv->ndev);
    }
    spin_unlock_bh(&priv->tx_lock);
}

static void schedule_tx_done(struct simnet_priv *priv)
{
    /* schedule delayed work to simulate hardware TX completion */
    queue_delayed_work(priv->wq, &priv->tx_work, msecs_to_jiffies(TX_COMPLETE_DELAY_MS));
}

static netdev_tx_t simnet_start_xmit(struct sk_buff *skb, struct net_device *ndev)
{
    struct simnet_priv *priv = netdev_priv(ndev);
    unsigned long flags;

    spin_lock_irqsave(&priv->tx_lock, flags);
    if (priv->tx_count >= priv->tx_ring_size) {
        /* ring full */
        netif_stop_queue(ndev);
        spin_unlock_irqrestore(&priv->tx_lock, flags);
        return NETDEV_TX_BUSY;
    }

    /* enqueue skb into tx_ring (driver owns skb now) */
    priv->tx_ring[priv->tx_tail] = skb;
    priv->tx_tail = (priv->tx_tail + 1) % priv->tx_ring_size;
    priv->tx_count++;
    spin_unlock_irqrestore(&priv->tx_lock, flags);

    /* Touch stats as "queued" (actual tx later) */
    ndev->stats.tx_packets++;
    ndev->stats.tx_bytes += skb->len;

    /* simulate notifying hardware to send */
    schedule_tx_done(priv);

    /* tell upper layer that skb was accepted */
    return NETDEV_TX_OK;
}

/* NAPI poll: pull pkts from rx_q and hand to stack */
static int simnet_poll(struct napi_struct *napi, int budget)
{
    struct simnet_priv *priv = container_of(napi, struct simnet_priv, napi);
    struct sk_buff *skb;
    int processed = 0;

    while (processed < budget) {
        skb = skb_dequeue(&priv->rx_q);
        if (!skb)
            break;
        skb->dev = priv->ndev;
        skb->protocol = eth_type_trans(skb, priv->ndev);
        priv->rx_packets++;
        priv->ndev->stats.rx_packets++;
        priv->ndev->stats.rx_bytes += skb->len;
        /* pass to stack using NAPI GRO */
        napi_gro_receive(&priv->napi, skb, NULL);
        processed++;
    }

    if (processed < budget) {
        napi_complete_done(&priv->napi, processed);
    }
    return processed;
}

static const struct net_device_ops simnet_netdev_ops = {
    .ndo_open = simnet_open,
    .ndo_stop = simnet_stop,
    .ndo_start_xmit = simnet_start_xmit,
};

/* misc device interface: userspace writes raw Ethernet frame to inject as RX */
static ssize_t simnet_inject_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    struct simnet_priv *priv = file->private_data;
    struct sk_buff *skb;
    void *data;

    if (count == 0 || count > MAX_INJECT_SIZE)
        return -EINVAL;

    skb = netdev_alloc_skb(priv->ndev, count + NET_IP_ALIGN);
    if (!skb)
        return -ENOMEM;

    skb_reserve(skb, NET_IP_ALIGN);
    data = skb_put(skb, count);
    if (copy_from_user(data, buf, count)) {
        dev_kfree_skb_any(skb);
        return -EFAULT;
    }

    skb->dev = priv->ndev;

    /* enqueue into rx queue and schedule NAPI */
    skb_queue_tail(&priv->rx_q, skb);
    napi_schedule(&priv->napi);

    return count;
}

static int simnet_inject_open(struct inode *inode, struct file *file)
{
    struct miscdevice *m = container_of(inode->i_cdev, struct miscdevice, minor);
    struct simnet_priv *priv = container_of(m, struct simnet_priv, mdev);
    file->private_data = priv;
    return 0;
}

static const struct file_operations simnet_inject_fops = {
    .owner = THIS_MODULE,
    .write = simnet_inject_write,
    .open = simnet_inject_open,
};

/* We'll embed miscdevice struct in priv for easy access */

static int simnet_probe_common(struct net_device *ndev)
{
    struct simnet_priv *priv = netdev_priv(ndev);

    priv->ndev = ndev;
    priv->tx_ring_size = DEFAULT_TX_RING;
    priv->tx_ring = kzalloc(sizeof(struct sk_buff *) * priv->tx_ring_size, GFP_KERNEL);
    if (!priv->tx_ring)
        return -ENOMEM;
    priv->tx_head = priv->tx_tail = priv->tx_count = 0;
    spin_lock_init(&priv->tx_lock);

    skb_queue_head_init(&priv->rx_q);

    priv->wq = create_singlethread_workqueue("simnet_wq");
    if (!priv->wq) {
        kfree(priv->tx_ring);
        return -ENOMEM;
    }
    INIT_DELAYED_WORK(&priv->tx_work, simnet_tx_complete_work);

    netif_napi_add(ndev, &priv->napi, simnet_poll, 64);

    return 0;
}

static void simnet_remove_common(struct net_device *ndev)
{
    struct simnet_priv *priv = netdev_priv(ndev);

    cancel_delayed_work_sync(&priv->tx_work);
    if (priv->wq)
        destroy_workqueue(priv->wq);
    netif_napi_del(&priv->napi);
    kfree(priv->tx_ring);
}

/* module-level misc device container pattern: we want a miscdev and need to
 * access priv from fops; we'll allocate priv and then register misc with it
 */
static struct miscdevice simnet_mdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = INJECT_DEV_NAME,
    .fops = &simnet_inject_fops,
};

static struct net_device *sim_ndev;

static int __init simnet_init_module(void)
{
    int ret;
    struct net_device *ndev;

    ndev = alloc_etherdev(sizeof(struct simnet_priv));
    if (!ndev)
        return -ENOMEM;

    ether_setup(ndev);
    ndev->netdev_ops = &simnet_netdev_ops;
    ndev->mtu = ETH_DATA_LEN;
    ndev->flags |= IFF_NOARP;

    /* give a predictable name */
    strncpy(ndev->name, "sim%%d", IFNAMSIZ);

    ret = register_netdev(ndev);
    if (ret) {
        pr_err(DRV_NAME ": register_netdev failed: %d\n", ret);
        free_netdev(ndev);
        return ret;
    }

    /* finish probe common */
    ret = simnet_probe_common(ndev);
    if (ret) {
        unregister_netdev(ndev);
        free_netdev(ndev);
        return ret;
    }

    /* register misc inject device and stash priv pointer in miscdev for open */
    simnet_mdev.this_device = NULL; /* kernel will create device entry */
    ret = misc_register(&simnet_mdev);
    if (ret) {
        pr_err(DRV_NAME ": misc_register failed\n");
        simnet_remove_common(ndev);
        unregister_netdev(ndev);
        free_netdev(ndev);
        return ret;
    }

    /* store ndev globally */
    sim_ndev = ndev;

    pr_info(DRV_NAME ": module loaded, device=%s\n", ndev->name);
    return 0;
}

static void __exit simnet_cleanup_module(void)
{
    struct simnet_priv *priv;

    if (sim_ndev) {
        priv = netdev_priv(sim_ndev);
        misc_deregister(&simnet_mdev);
        simnet_remove_common(sim_ndev);
        unregister_netdev(sim_ndev);
        free_netdev(sim_ndev);
    }
    pr_info(DRV_NAME ": module unloaded\n");
}

module_init(simnet_init_module);
module_exit(simnet_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Assistant");
MODULE_DESCRIPTION("Simulated Ethernet NIC with NAPI, TX ring and userspace inject device");

/*
 * Additional files (Makefile and userspace test injector) are included below
 * in this single document for convenience. Save them as separate files in
 * the same directory when building.
 */

/* Makefile:

obj-m += simnet.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.o *.ko *.mod.* .*.cmd *.symvers *.order .tmp_versions

*/

/* userspace injector (inject.c):

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *dev = "/dev/simnet_inject";
    int fd;
    unsigned char pkt[64];
    int i;

    if ((fd = open(dev, O_WRONLY)) < 0) {
        perror("open");
        return 1;
    }

    // build a dummy Ethernet frame (dst: ff:ff:ff:ff:ff:ff, src: 02:00:00:00:00:01, ethertype 0x0800)
    memset(pkt, 0xff, 6);
    pkt[6] = 0x02; pkt[7] = 0x00; pkt[8] = 0x00; pkt[9] = 0x00; pkt[10] = 0x00; pkt[11] = 0x01;
    pkt[12] = 0x08; pkt[13] = 0x00; // IPv4
    for (i = 14; i < 60; i++) pkt[i] = i;

    if (write(fd, pkt, 60) != 60) {
        perror("write");
        close(fd);
        return 1;
    }
    printf("wrote pkt to %s\n", dev);
    close(fd);
    return 0;
}

Build: gcc -o inject inject.c
Usage: ./inject

*/

//kernel module: netlink-exam-kern.c
#include <linux/config.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <linux/proc_fs.h>

#define BUF_SIZE 16384
static struct sock *netlink_exam_sock;
static unsigned char buffer[BUF_SIZE];
static unsigned int buffer_tail = 0;
static int exit_flag = 0;
static DECLARE_COMPLETION(exit_completion);

static void recv_handler(struct sock * sk, int length)
{
        wake_up(sk->sk_sleep);
}

static int process_message_thread(void * data)
{
        struct sk_buff * skb = NULL;
        struct nlmsghdr * nlhdr = NULL;
        int len;
        DEFINE_WAIT(wait);

        daemonize("mynetlink");

        while (exit_flag == 0) {
                prepare_to_wait(netlink_exam_sock->sk_sleep, &wait, TASK_INTERRUPTIBLE);
                schedule();
                finish_wait(netlink_exam_sock->sk_sleep, &wait); 

                while ((skb = skb_dequeue(&netlink_exam_sock->sk_receive_queue))
                         != NULL) {
                        nlhdr = (struct nlmsghdr *)skb->data;
                        if (nlhdr->nlmsg_len < sizeof(struct nlmsghdr)) {
                                printk("Corrupt netlink message.\n");
                                continue;
                        }
                        len = nlhdr->nlmsg_len - NLMSG_LENGTH(0);
                        if (len + buffer_tail > BUF_SIZE) {
                                printk("netlink buffer is full.\n");
                        }
                        else {
                                memcpy(buffer + buffer_tail, NLMSG_DATA(nlhdr), len);
                                buffer_tail += len;
                        }
                        nlhdr->nlmsg_pid = 0;
                        nlhdr->nlmsg_flags = 0;
                        NETLINK_CB(skb).pid = 0;
                        NETLINK_CB(skb).dst_pid = 0;
                        NETLINK_CB(skb).dst_group = 1;
                        netlink_broadcast(netlink_exam_sock, skb, 0, 1, GFP_KERNEL);
                }
        }
        complete(&exit_completion);
        return 0;
}

static int netlink_exam_readproc(char *page, char **start, off_t off,
                          int count, int *eof, void *data)
{
        int len;

        if (off >= buffer_tail) {
                * eof = 1;
                return 0;
        }
        else {
                len = count;
                if (count > PAGE_SIZE) {
                        len = PAGE_SIZE;
                }
                if (len > buffer_tail - off) {
                        len = buffer_tail - off;
                }
                memcpy(page, buffer + off, len);
                *start = page;
                return len;
        }

}

static int __init netlink_exam_init(void)
{
        netlink_exam_sock = netlink_kernel_create(NETLINK_GENERIC, 0, recv_handler, THIS_MODULE);
        if (!netlink_exam_sock) {
                printk("Fail to create netlink socket.\n");
                return 1;
        }
        kernel_thread(process_message_thread, NULL, CLONE_KERNEL);
        create_proc_read_entry("netlink_exam_buffer", 0444, NULL, netlink_exam_readproc, 0);
        return 0;
}

static void __exit netlink_exam_exit(void)
{
        exit_flag = 1;
        wake_up(netlink_exam_sock->sk_sleep);
        wait_for_completion(&exit_completion);
        sock_release(netlink_exam_sock->sk_socket);
}

module_init(netlink_exam_init);
module_exit(netlink_exam_exit);
MODULE_LICENSE("GPL");

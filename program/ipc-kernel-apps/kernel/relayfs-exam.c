//kernel module: relayfs-exam.c
#include <linux/module.h>
#include <linux/relayfs_fs.h>
#include <linux/string.h>
#include <linux/sched.h>

#define WRITE_PERIOD (HZ * 60)
static struct rchan *   chan;
static size_t           subbuf_size = 65536;
static size_t           n_subbufs = 4;
static char buffer[256];

void relayfs_exam_write(unsigned long data);

static DEFINE_TIMER(relayfs_exam_timer, relayfs_exam_write, 0, 0);

void relayfs_exam_write(unsigned long data)
{
        int len;
        task_t * p = NULL;

        len = sprintf(buffer, "Current all the processes:\n"); 
        len += sprintf(buffer + len, "process name\t\tpid\n"); 
        relay_write(chan, buffer, len);

        for_each_process(p) {
                len = sprintf(buffer, "%s\t\t%d\n", p->comm, p->pid); 
                relay_write(chan, buffer, len);
        }
        len = sprintf(buffer, "\n\n"); 
        relay_write(chan, buffer, len);

        relayfs_exam_timer.expires = jiffies + WRITE_PERIOD;
        add_timer(&relayfs_exam_timer);
}


/*
 * subbuf_start() relayfs callback.
 *
 * Defined so that we can 1) reserve padding counts in the sub-buffers, and
 * 2) keep a count of events dropped due to the buffer-full condition.
 */
static int subbuf_start(struct rchan_buf *buf,
                        void *subbuf,
                        void *prev_subbuf,
                        unsigned int prev_padding)
{
        if (prev_subbuf)
                *((unsigned *)prev_subbuf) = prev_padding;

        if (relay_buf_full(buf))
                return 0;

        subbuf_start_reserve(buf, sizeof(unsigned int));

        return 1;
}

/*
 * relayfs callbacks
 */
static struct rchan_callbacks relayfs_callbacks =
{
        .subbuf_start = subbuf_start,
};

/**
 *      module init - creates channel management control files
 *
 *      Returns 0 on success, negative otherwise.
 */
static int init(void)
{

        chan = relay_open("example", NULL, subbuf_size,
                          n_subbufs, &relayfs_callbacks);

        if (!chan) {
                printk("relay channel creation failed.\n");
                return 1;
        }
        relayfs_exam_timer.expires = jiffies + WRITE_PERIOD;
        add_timer(&relayfs_exam_timer);

        return 0;
}

static void cleanup(void)
{
        del_timer_sync(&relayfs_exam_timer);
        if (chan) {
                relay_close(chan);
                chan = NULL;
        }
}

module_init(init);
module_exit(cleanup);
MODULE_LICENSE("GPL");

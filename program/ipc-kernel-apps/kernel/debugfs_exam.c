//kernel module: debugfs_exam.c
#include <linux/config.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/types.h>

static struct dentry *root_entry, *u8_entry, *u16_entry, *u32_entry, *bool_entry;
static u8 var8;
static u16 var16;
static u32 var32;
static u32 varbool;

static int __init exam_debugfs_init(void)
{

        root_entry = debugfs_create_dir("debugfs-exam", NULL);
        if (!root_entry) {
                printk("Fail to create proc dir: debugfs-exam\n");
                return 1;
        }

        u8_entry = debugfs_create_u8("u8-var", 0644, root_entry, &var8);
        u16_entry = debugfs_create_u16("u16-var", 0644, root_entry, &var16);
        u32_entry = debugfs_create_u32("u32-var", 0644, root_entry, &var32);
        bool_entry = debugfs_create_bool("bool-var", 0644, root_entry, &varbool);

        return 0;
}

static void __exit exam_debugfs_exit(void)
{
        debugfs_remove(u8_entry);
        debugfs_remove(u16_entry);
        debugfs_remove(u32_entry);
        debugfs_remove(bool_entry);
        debugfs_remove(root_entry);
}

module_init(exam_debugfs_init);
module_exit(exam_debugfs_exit);
MODULE_LICENSE("GPL");

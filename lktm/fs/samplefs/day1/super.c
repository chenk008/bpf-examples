/*
 *   fs/samplefs/super.c
 *
 *   Copyright (C) International Business Machines  Corp., 2006, 2007
 *   Author(s): Steve French (sfrench@us.ibm.com)
 *
 *   Sample File System
 *
 *   Primitive example to show how to create a Linux filesystem module
 *
 *   superblock related and misc. functions
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation; either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/version.h>

/* helpful if this is different than other fs */
#define SAMPLEFS_MAGIC     0x73616d70 /* "SAMP" */

static int samplefs_fill_super(struct super_block *sb, void *data, int silent)
{
	return 0;
}

// struct file_system_type *fs_type: 文件系统类型结构指针，samplefs已经做了部分的初始化。
// int flags: mount的标志位。
// const char *dev_name: mount文件系统时指定的设备名称。
// void *data: mount时指定的命令选项，通常是ascii码。
static struct dentry *samplefs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	// mount函数必须返回文件系统树的root dentry(根目录项)。在mount时超级块的引用计数必须增加，而且必须拿锁状态下操作。函数在失败时必须返回ERR_PTR(error)。
	return mount_nodev(fs_type, flags, data, samplefs_fill_super);
}

static struct file_system_type samplefs_fs_type = {
	/* 和所有Linux模块一样*/
	.owner = THIS_MODULE,
	.name = "samplefs", //文件系统name
	.mount = samplefs_mount, // mount调用
	.kill_sb = kill_anon_super, 	/* umount文件系统时调用的入口函数*/
	/*  .fs_flags */
};


static int __init init_samplefs_fs(void)
{
	return register_filesystem(&samplefs_fs_type);
}

static void __exit exit_samplefs_fs(void)
{
	unregister_filesystem(&samplefs_fs_type);
}

module_init(init_samplefs_fs)
module_exit(exit_samplefs_fs)
MODULE_LICENSE("GPL");

/*
 *   blk/sampleblk/sample_blk.c
 *
 *   Copyright (C) Oliver Yang 2016
 *   Author(s): Yong Yang (yangoliver@gmail.com)
 *
 *   Sample Block Driver
 *
 *   Primitive example to show how to create a Linux block driver
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
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>


static int sampleblk_major;
#define SAMPLEBLK_MINOR	1
static int sampleblk_sect_size = 512;
static int sampleblk_nsects = 10 * 1024;

struct sampleblk_dev {
	int minor;
	// spinlock_t lock;
	struct request_queue *queue;
	struct gendisk *disk;
	ssize_t size;
	void *data;
	struct blk_mq_tag_set tag_set;
};

struct sampleblk_dev *sampleblk_dev = NULL;

/*
 * Do an I/O operation for each segment
 */
static int sampleblk_handle_io(struct sampleblk_dev *sampleblk_dev,
		uint64_t pos, ssize_t size, void *buffer, int write)
{
	if (write)
		memcpy(sampleblk_dev->data + pos, buffer, size);
	else
		memcpy(buffer, sampleblk_dev->data + pos, size);

	return 0;
}

// static void sampleblk_request(struct request_queue *q)
// {
// 	struct request *rq = NULL;
// 	int rv = 0;
// 	uint64_t pos = 0;
// 	ssize_t size = 0;
// 	struct bio_vec bvec;
// 	struct req_iterator iter;
// 	void *kaddr = NULL;

// 	while ((rq = blk_fetch_request(q)) != NULL) {
// 		spin_unlock_irq(q->queue_lock);

// 		if (rq->cmd_type != REQ_TYPE_FS) {
// 			rv = -EIO;
// 			goto skip;
// 		}

// 		BUG_ON(sampleblk_dev != rq->rq_disk->private_data);

// 		pos = blk_rq_pos(rq) * sampleblk_sect_size;
// 		size = blk_rq_bytes(rq);
// 		if ((pos + size > sampleblk_dev->size)) {
// 			pr_crit("sampleblk: Beyond-end write (%llu %zx)\n",
// 				pos, size);
// 			rv = -EIO;
// 			goto skip;
// 		}

// 		rq_for_each_segment(bvec, rq, iter) {
// 			kaddr = kmap(bvec.bv_page);

// 			rv = sampleblk_handle_io(sampleblk_dev, pos,
// 				bvec.bv_len, kaddr + bvec.bv_offset,
// 				rq_data_dir(rq));
// 			if (rv < 0)
// 				goto skip;

// 			pos += bvec.bv_len;
// 			kunmap(bvec.bv_page);
// 		}
// skip:

// 		blk_end_request_all(rq, rv);

// 		spin_lock_irq(q->queue_lock);
// 	}
// }

static int sampleblk_ioctl(struct block_device *bdev, fmode_t mode,
			unsigned command, unsigned long argument)
{
	return 0;
}

static int sampleblk_open(struct block_device *bdev, fmode_t mode)
{
	return 0;
}

static void sampleblk_release(struct gendisk *disk, fmode_t mode)
{
}

static const struct block_device_operations sampleblk_fops = {
	.owner = THIS_MODULE,
	.open = sampleblk_open,
	.release = sampleblk_release,
	.ioctl = sampleblk_ioctl,
};

static blk_status_t myramdisk_queue_rq(struct blk_mq_hw_ctx *hctx,const struct blk_mq_queue_data* bd) {
    blk_status_t ret = BLK_STS_OK;
	struct request *req = bd->rq;
	struct sampleblk_dev *dev = req->rq_disk->private_data;
	// 返回 request 的起始扇区号
	sector_t rq_pos = blk_rq_pos(req);

	// 告诉内核我们开始处理这个请求
	blk_mq_start_request(req);

	// bio_vec 就是描述一个 Segment 的数据结构	
	struct bio_vec bvec;
	struct req_iterator iter;
	// 遍历segment
	rq_for_each_segment(bvec, req, iter) {
		size_t num_sector = blk_rq_cur_sectors(req);
		/* bv_page: Segment 所在的内存物理页的 struct page 结构指针 */
		/* bv_offset: Segment 在内存物理页内起始的偏移地址 */
		// page_address获得物理页的逻辑地址
		unsigned char *buffer = page_address(bvec.bv_page) + bvec.bv_offset;
		// 请求设备的起始扇区号
		unsigned long offset = rq_pos * sampleblk_sect_size;
		//avoid buffer overflow
		if ((offset + num_sector*sampleblk_sect_size) <= dev->size) {
			
			if (rq_data_dir(req) == WRITE) 

				memcpy(dev->data+offset,buffer,num_sector*sampleblk_sect_size);
			else 
				memcpy(buffer,dev->data+offset,num_sector*sampleblk_sect_size);
		}
		else {
			ret = BLK_STS_IOERR;
			goto end;
		}
		rq_pos += num_sector;
	}
end:
	// 告诉内核我们处理完成这个请求
	blk_mq_end_request(req,ret);
	return ret;
}

static const struct blk_mq_ops myramdisk_mq_ops = {
    .queue_rq = myramdisk_queue_rq,
};

static int sampleblk_alloc(int minor)
{
	struct gendisk *disk;
	int rv = 0;

	sampleblk_dev = kzalloc(sizeof(struct sampleblk_dev), GFP_KERNEL);
	if (!sampleblk_dev) {
		rv = -ENOMEM;
		goto fail;
	}

	// 设备的存储空间
	sampleblk_dev->size = sampleblk_sect_size * sampleblk_nsects;
	sampleblk_dev->data = vmalloc(sampleblk_dev->size);
	if (!sampleblk_dev->data) {
		rv = -ENOMEM;
		goto fail_dev;
	}
	sampleblk_dev->minor = minor;

	// spin_lock_init(&sampleblk_dev->lock);
	// 初始化请求队列，传入处理read和write io的回调函数，同时需要传入自旋锁
	sampleblk_dev->queue = blk_mq_init_sq_queue(&sampleblk_dev->tag_set,&myramdisk_mq_ops,128,BLK_MQ_F_SHOULD_MERGE);
	if (!sampleblk_dev->queue) {
		rv = -ENOMEM;
		goto fail_data;
	}
	sampleblk_dev->queue -> queuedata =sampleblk_dev;

	// 内核使用 struct gendisk 来抽象和表示一个磁盘
	disk = alloc_disk(minor);
	if (!disk) {
		rv = -ENOMEM;
		goto fail_queue;
	}
	sampleblk_dev->disk = disk;

	disk->major = sampleblk_major;
	disk->first_minor = minor;
	// 块设备操作函数表
	disk->fops = &sampleblk_fops;
	disk->private_data = sampleblk_dev;
	disk->queue = sampleblk_dev->queue;
	sprintf(disk->disk_name, "sampleblk%d", minor);
	set_capacity(disk, sampleblk_nsects);
	add_disk(disk);

	return 0;

fail_queue:
	blk_cleanup_queue(sampleblk_dev->queue);
fail_data:
	vfree(sampleblk_dev->data);
fail_dev:
	kfree(sampleblk_dev);
fail:
	return rv;
}

static void sampleblk_free(struct sampleblk_dev *sampleblk_dev)
{
	//让磁盘在系统中不再可见，触发热插拔 uevent
	del_gendisk(sampleblk_dev->disk);
	// 停止并释放块设备 IO 请求队列,在释放 struct request_queue 之前，要把待处理的 IO 请求都处理掉。
	blk_cleanup_queue(sampleblk_dev->queue);
	put_disk(sampleblk_dev->disk);
	// 释放数据区
	vfree(sampleblk_dev->data);
	kfree(sampleblk_dev);
}

static int __init sampleblk_init(void)
{
	int rv = 0;

	//分配一个未使用的反回给调用者
	sampleblk_major = register_blkdev(0, "sampleblk");
	if (sampleblk_major < 0)
		return sampleblk_major;

	rv = sampleblk_alloc(SAMPLEBLK_MINOR);
	if (rv < 0)
		pr_info("sampleblk: disk allocation failed with %d\n", rv);

	pr_info("sampleblk: module loaded\n");
	return 0;
}

static void __exit sampleblk_exit(void)
{
	sampleblk_free(sampleblk_dev);
	unregister_blkdev(sampleblk_major, "sampleblk");

	pr_info("sampleblk: module unloaded\n");
}

module_init(sampleblk_init);
module_exit(sampleblk_exit);
MODULE_LICENSE("GPL");

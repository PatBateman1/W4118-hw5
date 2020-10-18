/*
 * fridge.c
 *
 * A kernel-level key-value store. Accessed via user-defined
 * system calls. This is the module implementation.
 */

#include <linux/module.h>
#include <linux/printk.h>
#include "fridge_data_structures.h"
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MODULE_NAME "Fridge"

extern long (*kkv_init_ptr)(int flags);

extern long (*kkv_destroy_ptr)(int flags);

extern long (*kkv_put_ptr)(uint32_t key, void *val, size_t size, int flags);

extern long (*kkv_get_ptr)(uint32_t key, void *val, size_t size, int flags);

void destroy_bucket(struct list_head entries);

static struct kkv_ht_bucket *ht[HASH_TABLE_LENGTH];

long kkv_init(int flags)
{	
	int i;
	for (i=0; i<HASH_TABLE_LENGTH; i++) {
		ht[i] = (struct kkv_ht_bucket *) kmalloc(sizeof(*ht), GFP_KERNEL);
		INIT_LIST_HEAD(&ht[i]->entries);
		ht[i]->count = 0;
	}
	return 0;
}

long kkv_destroy(int flags)
{
	int i;
	for (i=0; i<HASH_TABLE_LENGTH; i++) {
		destroy_bucket(ht[i]->entries);
		kfree(ht[i]);
	}
	return 0;
}

long kkv_put(uint32_t key, void *val, size_t size, int flags)
{
	int index = key % HASH_TABLE_LENGTH;
	struct kkv_ht_bucket *bk = ht[index];
	struct kkv_ht_entry *entry;
	struct kkv_ht_entry *new;
	struct kkv_ht_entry *remove = NULL;
	new = (struct kkv_ht_entry *) kmalloc(sizeof(*new), GFP_KERNEL);
	new->kv_pair.key = key;
	new->kv_pair.size = size;
	new->kv_pair.val = kmalloc(size, GFP_KERNEL);

	if (!new) {
		printk(KERN_ERR "new kmalloc() failed");
		return -ENOMEM;
	}

	if (!new->kv_pair.val) {
		printk(KERN_ERR "new->kv_pair.val kmalloc() failed");
		kfree(new);
		return -ENOMEM;
	}

	if (copy_from_user(new->kv_pair.val, val, size) != 0) {
		printk(KERN_ERR "copy_from_user() failed");
		kfree(new->kv_pair.val);
		kfree(new);
		return -EFAULT;
	}

	INIT_LIST_HEAD(&new->entries);
	spin_lock(&bk->lock);
	list_for_each_entry(entry, &bk->entries, entries) {
		if (entry->kv_pair.key == key) {
			remove = entry;
			list_del(&remove->entries);
			bk->count--;
			break;
		}
	}
	list_add_tail(&new->entries, &bk->entries);
	bk->count++;
	spin_unlock(&bk->lock);
	if (remove)
		kfree(remove);
	return 0;
}

long kkv_get(uint32_t key, void *val, size_t size, int flags)
{
	int index = key % HASH_TABLE_LENGTH;
	struct kkv_ht_bucket *bk = ht[index];
	struct kkv_ht_entry *entry;
	struct kkv_ht_entry *remove = NULL;
	spin_lock(&bk->lock);
	list_for_each_entry(entry, &bk->entries, entries) {
		if (entry->kv_pair.key == key) {
			remove = entry;
			list_del(&remove->entries);
			bk->count--;
			break;
		}
	}
	spin_unlock(&bk->lock);
	if (remove) {
		if (copy_to_user(val, remove->kv_pair.val, remove->kv_pair.size) != 0) {
			printk(KERN_ERR "copy_to_user() failed");
			spin_lock(&bk->lock);
			list_add_tail(&remove->entries, &bk->entries);
			spin_unlock(&bk->lock);
			return -EFAULT;
		}
		kfree(remove);
		return 0;
	}
	return -1;
}

void destroy_bucket(struct list_head entries)
{
	struct kkv_ht_entry *cur, *nxt;
	list_for_each_entry_safe(cur, nxt, &entries, entries) {
		list_del(&cur->entries);
		kfree(cur->kv_pair.val);
		kfree(cur);
	}
}

int fridge_init(void)
{
	pr_info("Installing fridge\n");
	kkv_init_ptr = kkv_init;
	kkv_destroy_ptr = kkv_destroy;
	kkv_put_ptr = kkv_put;
	kkv_get_ptr = kkv_get;
	return 0;
}

void fridge_exit(void)
{
	pr_info("Removing fridge\n");
}

module_init(fridge_init);
module_exit(fridge_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(MODULE_NAME);
MODULE_AUTHOR("cs4118");

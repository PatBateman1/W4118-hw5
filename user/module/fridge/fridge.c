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
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MODULE_NAME "Fridge"

extern long (*kkv_init_ptr)(int flags);

extern long (*kkv_destroy_ptr)(int flags);

extern long (*kkv_put_ptr)(uint32_t key, void *val, size_t size, int flags);

extern long (*kkv_get_ptr)(uint32_t key, void *val, size_t size, int flags);

void destroy_bucket(struct list_head *entry);

/*
 * init-destroy identifier 
 */
struct init_identifier {
	spinlock_t lock;
	int is_init;
};

static struct init_identifier *identifier;

static struct kkv_ht_bucket *ht;

static struct kmem_cache *cache;

long kkv_init(int flags)
{	
	int i;

	// check if already inited
	spin_lock(&identifier->lock);
	if (identifier->is_init) {
		spin_unlock(&identifier->lock);
		printk(KERN_ERR "init twice!");
		return -EPERM;
	}
	identifier->is_init++;
	spin_unlock(&identifier->lock);

	ht = (struct kkv_ht_bucket *) kmalloc(sizeof(struct kkv_ht_bucket) * HASH_TABLE_LENGTH, GFP_KERNEL);
	for (i=0; i<HASH_TABLE_LENGTH; i++) {
		INIT_LIST_HEAD(&ht[i].entries);
		spin_lock_init(&ht[i].lock);
		ht[i].count = 0;
	}
	return 0;
}

long kkv_destroy(int flags)
{
	int i;

	// check if already destroyed
	spin_lock(&identifier->lock);
	if (!identifier->is_init) {
		spin_unlock(&identifier->lock);
		printk(KERN_ERR "destroy twice!");
		return -EPERM;
	}
	identifier->is_init--;
	spin_unlock(&identifier->lock);

	for (i=0; i<HASH_TABLE_LENGTH; i++) {
		destroy_bucket(&ht[i].entries);
	}
	kfree(ht);
	return 0;
}

long kkv_put(uint32_t key, void *val, size_t size, int flags)
{
	int index = key % HASH_TABLE_LENGTH;
	struct kkv_ht_bucket *bk = &ht[index];
	struct kkv_ht_entry *entry;
	struct kkv_ht_entry *new;
	struct kkv_ht_entry *remove = NULL;

	// check if already inited
	spin_lock(&identifier->lock);
	if (identifier->is_init == 0) {
		printk(KERN_ERR "put before init!");
		spin_unlock(&identifier->lock);
		return -EPERM;
	}
	spin_unlock(&identifier->lock);

	new = kmem_cache_alloc(cache, GFP_KERNEL);

	if (!new) {
		printk(KERN_ERR "new kmem_cache_alloc() failed");
		return -ENOMEM;
	}
	new->kv_pair.key = key;
	new->kv_pair.size = size;
	new->kv_pair.val = kmalloc(size, GFP_KERNEL);

	if (!new->kv_pair.val) {
		printk(KERN_ERR "new->kv_pair.val kmalloc() failed");
		kmem_cache_free(cache, new);
		return -ENOMEM;
	}

	if (copy_from_user(new->kv_pair.val, val, size) != 0) {
		printk(KERN_ERR "copy_from_user() failed");
		kfree(new->kv_pair.val);
		kmem_cache_free(cache, new);
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
	if (remove) {
		kfree(remove->kv_pair.val);
<<<<<<< HEAD
		kfree(remove);
=======
		kmem_cache_free(cache, remove);
>>>>>>> dev-part3
	}
	return 0;
}

long kkv_get(uint32_t key, void *val, size_t size, int flags)
{
	int index = key % HASH_TABLE_LENGTH;
	struct kkv_ht_bucket *bk = &ht[index];
	struct kkv_ht_entry *entry;
	struct kkv_ht_entry *remove = NULL;

	// check if already inited
	spin_lock(&identifier->lock);
	if (identifier->is_init == 0) {
		printk(KERN_ERR "put before init!");
		spin_unlock(&identifier->lock);
		return -EPERM;
	}
	spin_unlock(&identifier->lock);

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
		size = remove->kv_pair.size > size ? size : remove->kv_pair.size;
		if (copy_to_user(val, remove->kv_pair.val, size) != 0) {
			printk(KERN_ERR "copy_to_user() failed");
			spin_lock(&bk->lock);
			list_add_tail(&remove->entries, &bk->entries);
			spin_unlock(&bk->lock);
			return -EFAULT;
		}
		kfree(remove->kv_pair.val);
<<<<<<< HEAD
		kfree(remove);
=======
		kmem_cache_free(cache, remove);
>>>>>>> dev-part3
		return 0;
	}
	return -ENOENT;
}

void destroy_bucket(struct list_head *entry)
{
	struct kkv_ht_entry *cur, *nxt;
	list_for_each_entry_safe(cur, nxt, entry, entries) {
		list_del(&cur->entries);
		kfree(cur->kv_pair.val);
		kmem_cache_free(cache, cur);
	}
}

int fridge_init(void)
{
	pr_info("Installing fridge\n");
	identifier = (struct init_identifier *) kmalloc(sizeof(struct init_identifier), GFP_KERNEL);
	spin_lock_init(&identifier->lock);
	identifier->is_init = 0;
	cache = kmem_cache_create("entry", sizeof(struct kkv_ht_entry), 0, 0, NULL);
	kkv_init_ptr = kkv_init;
	kkv_destroy_ptr = kkv_destroy;
	kkv_put_ptr = kkv_put;
	kkv_get_ptr = kkv_get;
	return 0;
}

void fridge_exit(void)
{
	pr_info("Removing fridge\n");
	kkv_destroy(0);
	kmem_cache_destroy(cache);
	kfree(identifier);
}

module_init(fridge_init);
module_exit(fridge_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(MODULE_NAME);
MODULE_AUTHOR("cs4118");

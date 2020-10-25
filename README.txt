This file should contain:

-	Your name & UNI (or those of all group members for group assignments)
-	Homework assignment number
-	Description for each part

The description should indicate whether your solution for the part is working
or not. You may also want to include anything else you would like to
communicate to the grader, such as extra functionality you implemented or how
you tried to fix your non-working code.

members:
    Zixiao Zhu zz2700
    Zhaoxuan Jiang zj2267

team: bateman-ZhaoxuanJ

part1:
    our solution works, we first create the hash table, to make the code clean
    and make the critical section smaller, in kkv_put we make a new entry anyway,
    if there has already a entry with a specific key, we just replace it with the
    brand new entry. the test file just test the edge case the if we get the key which
    is not in the hash table.

part2:
    our solution works, to do this part we use another spin lock wich checks if we already
    have a hash table. since we just has to lock the whole table beforehead for a very
    short time (we check the variable is_init, we set is_init to 1 when init and 0 when destroy),
    so it doesn't have a influence on the concurency of get and put on different keys.

part3:
    our solution works, the solution of this part is just like previous except that we use kmem_cache_alloc
    and kmem_ache_free to allocate and free memory for kkv_ht_entry. In testing, the solution of this
    part is sightly better than part2.

part4:
    our solution works, in this part, we use waitqueue: if get before put, we first init an entry and put
    that entry to the waitqueue. if user wants to get multiple times of entry with key which is not in
    the hash table, we create the entry at first time (the first thread has the spin lock) and the following
    thread just increase the q_count. when the q_count > 0 we wake the members in waitqueue in the kkv_put.
    we use blocking_simple and blocking_signal to test and in our own test program, we test multiple kkv_get
    before kkv_put. we use sleep between two kkv_puts to ensure we wake the waitqueue twice.
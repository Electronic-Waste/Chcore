/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan
 * PSL v1. You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v1 for more details.
 */

/* Scheduler related functions are implemented here */
#include <sched/sched.h>
#include <arch/machine/smp.h>
#include <common/kprint.h>
#include <machine.h>
#include <mm/kmalloc.h>
#include <common/list.h>
#include <common/util.h>
#include <object/thread.h>
#include <common/macro.h>
#include <common/errno.h>
#include <common/types.h>
#include <object/thread.h>
#include <irq/irq.h>
#include <sched/context.h>

#define RRx

#ifdef RR
        #define RR_LOG(fmt, args...) \
                do { \
                        printk("[RR_LOG][LINE:%d][FUNCTION:%s][CPU: %d]" fmt "\n", \
                                __LINE__, __FUNCTION__, smp_get_cpu_id(), ##args); \
                } while(0);
#else
        #define RR_LOG(fmt, args...) {}
#endif

/* in arch/sched/idle.S */
void idle_thread_routine(void);

/* Metadata for ready queue */
struct queue_meta {
        struct list_head queue_head;
        u32 queue_len;
        char pad[pad_to_cache_line(sizeof(u32) + sizeof(struct list_head))];
};

/*
 * rr_ready_queue
 * Per-CPU ready queue for ready tasks.
 */
struct queue_meta rr_ready_queue_meta[PLAT_CPU_NUM];

/*
 * RR policy also has idle threads.
 * When no active user threads in ready queue,
 * we will choose the idle thread to execute.
 * Idle thread will **NOT** be in the RQ.
 */
struct thread idle_threads[PLAT_CPU_NUM];

/*
 * Lab4
 * Sched_enqueue
 * Put `thread` at the end of ready queue of assigned `affinity`.
 * If affinity = NO_AFF, assign the core to the current cpu.
 * If the thread is IDLE thread, do nothing!
 * Do not forget to check if the affinity is valid!
 */
int rr_sched_enqueue(struct thread *thread)
{
        /* LAB 4 TODO BEGIN */
        /* If thread or thread_ctx is NULL, return -1 */
        if (thread == NULL || thread->thread_ctx == NULL) {
                RR_LOG("thread or thread_ctx is NULL");
                return -EINVAL;
        }
        /* If thread's state is TS_READY, return -1 */
        if (thread->thread_ctx->state == TS_READY) {
                RR_LOG("thread_ctx is TS_READY");
                return -EINVAL;
        }
        /* If the thread is IDLE thread, do nothing */
        if (thread->thread_ctx->type == TYPE_IDLE) {
                RR_LOG("Warning: IDLE thread!");
                return 0;
        }
        /* Check and initialize affinity */
        s32 aff = thread->thread_ctx->affinity;
        if (aff >= PLAT_CPU_NUM) {
                RR_LOG("Invalid affinity");
                return -EINVAL;
        }
        s32 cpuid = (aff == NO_AFF) ? smp_get_cpu_id() : aff;

        /* Set thread state to TS_READY & Add to ready queue */
        thread->thread_ctx->state = TS_READY;
        thread->thread_ctx->cpuid = cpuid;
        list_append(&thread->ready_queue_node, &rr_ready_queue_meta[cpuid].queue_head);
        rr_ready_queue_meta[cpuid].queue_len++;
        // RR_LOG("queue_len: %d", rr_ready_queue_meta[cpuid].queue_len);
        /* LAB 4 TODO END */
        return 0;
}

/*
 * Lab4
 * Sched_dequeue
 * remove `thread` from its current residual ready queue
 * Do not forget to add some basic parameter checking
 */
int rr_sched_dequeue(struct thread *thread)
{
        /* LAB 4 TODO BEGIN */
        /* If thread is NULL, report error */
        if (thread == NULL || thread->thread_ctx == NULL) {
                RR_LOG("Error: thread is NULL");
                return -EINVAL;
        }
        /* If the thread is IDLE thread, report error */
        if (thread->thread_ctx->type == TYPE_IDLE) {
                RR_LOG("Error: IDLE thread in ready queue!")
                return -EINVAL;
        }
        /* If the thread's state is not TS_READY, report error */
        if (thread->thread_ctx->state != TS_READY) {
                RR_LOG("Error: Thread's state should be TS_READY");
                return -EINVAL;
        }
        /* If the thread's affinity is invalid, report error */
        if (thread->thread_ctx->affinity >= PLAT_CPU_NUM) {
                RR_LOG("Error: Invalid affinity");
                return -EINVAL;
        }
        /* Delete the thread from ready queue & set thread state and cpuid */
        s32 cpuid = smp_get_cpu_id();
        list_del(&thread->ready_queue_node);
        thread->thread_ctx->state = TS_INTER;
        thread->thread_ctx->cpuid = cpuid;
        rr_ready_queue_meta[cpuid].queue_len--;
        // RR_LOG("queue_len: %d", rr_ready_queue_meta[cpuid].queue_len);
        /* LAB 4 TODO END */
        return 0;
}

/*
 * Lab4
 * Choose an appropriate thread and dequeue from ready queue
 *
 * Hints:
 * You can use `list_entry` to find the next ready thread in the ready queue
 */
struct thread *rr_sched_choose_thread(void)
{
        struct thread *thread = NULL;
        /* LAB 4 TODO BEGIN */
        /* if queue len is 0, return IDLE thread */
        if (rr_ready_queue_meta[smp_get_cpu_id()].queue_len == 0) {
                thread = &idle_threads[smp_get_cpu_id()];
                RR_LOG("return IDLE thread");
        }
        /* else dequeue the first thread in ready queue */
        else {
                struct list_head *choose_thread_node = rr_ready_queue_meta[smp_get_cpu_id()].queue_head.next;
                thread = list_entry(choose_thread_node, struct thread, ready_queue_node);
                BUG_ON(rr_sched_dequeue(thread));
        }
        /* LAB 4 TODO END */
        return thread;
}

/*
 * Lab4
 * You should use this function in rr_sched
 */
static inline void rr_sched_refill_budget(struct thread *target, u32 budget)
{
        /* LAB 4 TODO BEGIN */
        // BUG_ON(target->thread_ctx->sc->budget != 0);
        target->thread_ctx->sc->budget = budget;
        /* LAB 4 TODO END */
}

/*
 * Lab4
 * Schedule a thread to execute.
 * This function will suspend current running thread, if any, and schedule
 * another thread from `rr_ready_queue[cpu_id]`.
 *
 * Hints:
 * Macro DEFAULT_BUDGET defines the value for resetting thread's budget.
 * After you get one thread from rr_sched_choose_thread, pass it to
 * switch_to_thread() to prepare for switch_context().
 * Then ChCore can call eret_to_thread() to return to user mode.
 * You should also check the state of the old thread. Old thread
 * could be exiting/waiting or running when calling this function.
 * You will also need to check the remaining budget of the old thread.
 */
int rr_sched(void)
{
        /* LAB 4 TODO BEGIN */
        // /* Else if current_thread is exiting, set its state, thread_exit_state and don't put it into ready queue */
        // else if (current_thread->thread_ctx->thread_exit_state == TE_EXITING) {
        //         current_thread->thread_ctx->state = TS_EXIT;
        //         current_thread->thread_ctx->thread_exit_state = TE_EXITED;
        //         switch_to_thread(rr_sched_choose_thread());
        // }
        struct thread *thread = NULL;
        /* Some condition like `current_thread == NULL`, we don't need to do anything */
        if (current_thread == NULL ||
                current_thread->thread_ctx == NULL) {}
        /* Else if current_thread is exiting, set its state, thread_exit_state and don't put it into ready queue */
        else if (current_thread->thread_ctx->thread_exit_state == TE_EXITING) {
                current_thread->thread_ctx->state == TS_EXIT;
                current_thread->thread_ctx->thread_exit_state == TE_EXITED;
        }
        /* Else if current_thread isn't waiting, enqueue current_thread to ready queue */
        else if (current_thread->thread_ctx->state != TS_WAITING) {
                /* If budget != 0 & aff == cpuid, do not change current_thread */
                if (current_thread->thread_ctx->sc->budget != 0 &&
                        current_thread->thread_ctx->affinity == smp_get_cpu_id()) {
                        return switch_to_thread(current_thread);
                }
                else {
                        BUG_ON(rr_sched_enqueue(current_thread));
                        rr_sched_refill_budget(current_thread, DEFAULT_BUDGET);
                }
        }

        switch_to_thread(rr_sched_choose_thread());
        /* LAB 4 TODO END */

        return 0;
}

int rr_sched_init(void)
{
        int i = 0;
        char idle_name[] = "KNL-IDLE-RR";
        int name_len = strlen(idle_name);

        struct cap_group *idle_cap_group;
        struct vmspace *idle_vmspace;

        /* Initialize global variables */
        for (i = 0; i < PLAT_CPU_NUM; i++) {
                current_threads[i] = NULL;
                init_list_head(&(rr_ready_queue_meta[i].queue_head));
                rr_ready_queue_meta[i].queue_len = 0;
        }

        /* Create a fake idle cap group to store the name */
        idle_cap_group = kzalloc(sizeof(*idle_cap_group));
        memset(idle_cap_group->cap_group_name, 0, MAX_GROUP_NAME_LEN);
        if (name_len > MAX_GROUP_NAME_LEN)
                name_len = MAX_GROUP_NAME_LEN;
        memcpy(idle_cap_group->cap_group_name, idle_name, name_len);
        init_list_head(&idle_cap_group->thread_list);

        extern struct vmspace *create_idle_vmspace(void);
        idle_vmspace = create_idle_vmspace();

        /* Initialize one idle thread for each core and insert into the RQ */
        for (i = 0; i < PLAT_CPU_NUM; i++) {
                /* Set the thread context of the idle threads */
                BUG_ON(!(idle_threads[i].thread_ctx =
                                 create_thread_ctx(TYPE_IDLE)));
                /* We will set the stack and func ptr in arch_idle_ctx_init */
                init_thread_ctx(&idle_threads[i], 0, 0, MIN_PRIO, TYPE_IDLE, i);
                /* Call arch-dependent function to fill the context of the idle
                 * threads */
                arch_idle_ctx_init(idle_threads[i].thread_ctx,
                                   idle_thread_routine);

                idle_threads[i].cap_group = idle_cap_group;
                idle_threads[i].vmspace = idle_vmspace;

                /* Add idle_threads to the threads list */
                list_add(&idle_threads[i].node, &idle_cap_group->thread_list);
        }
        kdebug("Scheduler initialized. Create %d idle threads.\n", i);

        return 0;
}

#define MAX_CAP_GROUP_BUF 256

void rr_top(void)
{
        u32 cpuid;
        struct thread *thread;
        /* A simple way to collect all cap groups */
        struct cap_group *cap_group_buf[MAX_CAP_GROUP_BUF] = {0};
        unsigned int cap_group_num = 0;
        int i = 0;

        printk("\n*****CPU RQ Info*****\n");
        for (cpuid = 0; cpuid < PLAT_CPU_NUM; cpuid++) {
                printk("== CPU %d RQ LEN %lu==\n",
                       cpuid,
                       rr_ready_queue_meta[cpuid].queue_len);
                thread = current_threads[cpuid];
                if (thread != NULL) {
                        for (i = 0; i < cap_group_num; i++)
                                if (thread->cap_group == cap_group_buf[i])
                                        break;
                        if (i == cap_group_num
                            && cap_group_num < MAX_CAP_GROUP_BUF) {
                                cap_group_buf[cap_group_num] =
                                        thread->cap_group;
                                cap_group_num++;
                        }
                        printk("Current ");
                        print_thread(thread);
                }
                if (!list_empty(&(rr_ready_queue_meta[cpuid].queue_head))) {
                        for_each_in_list (
                                thread,
                                struct thread,
                                ready_queue_node,
                                &(rr_ready_queue_meta[cpuid].queue_head)) {
                                for (i = 0; i < cap_group_num; i++)
                                        if (thread->cap_group
                                            == cap_group_buf[i])
                                                break;
                                if (i == cap_group_num
                                    && cap_group_num < MAX_CAP_GROUP_BUF) {
                                        cap_group_buf[cap_group_num] =
                                                thread->cap_group;
                                        cap_group_num++;
                                }
                                print_thread(thread);
                        }
                }
                printk("\n");
        }

        printk("\n*****CAP GROUP Info*****\n");
        for (i = 0; i < cap_group_num; i++) {
                printk("== CAP GROUP:%s ==\n",
                       cap_group_buf[i]->cap_group_name);
                for_each_in_list (thread,
                                  struct thread,
                                  node,
                                  &(cap_group_buf[i]->thread_list)) {
                        print_thread(thread);
                }
                printk("\n");
        }
}

struct sched_ops rr = {.sched_init = rr_sched_init,
                       .sched = rr_sched,
                       .sched_enqueue = rr_sched_enqueue,
                       .sched_dequeue = rr_sched_dequeue,
                       .sched_top = rr_top};

/*
 * lwIP sys_arch port for RT-Thread
 *
 * Adapts the lwIP OS abstraction layer (sys.h) to RT-Thread kernel
 * primitives. Replaces the FreeRTOS-based port that was previously
 * in use.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* lwIP includes */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"

/* RT-Thread includes */
#include <rtthread.h>

/* ------------------------------------------------------------------
 * SYS_LIGHTWEIGHT_PROT — interrupt-based critical sections
 *
 * When LWIP_FREERTOS_SYS_ARCH_PROTECT_USES_MUTEX is 0 (default),
 * use rt_enter_critical / rt_exit_critical.
 * ------------------------------------------------------------------ */
#if SYS_LIGHTWEIGHT_PROT

sys_prot_t
sys_arch_protect(void)
{
    rt_enter_critical();
    return 1;
}

void
sys_arch_unprotect(sys_prot_t pval)
{
    LWIP_UNUSED_ARG(pval);
    rt_exit_critical();
}

#endif /* SYS_LIGHTWEIGHT_PROT */

/* ------------------------------------------------------------------
 * Time
 * ------------------------------------------------------------------ */

u32_t
sys_now(void)
{
    return rt_tick_get();   /* RT_TICK_PER_SECOND = 1000 → 1 tick = 1 ms */
}

u32_t
sys_jiffies(void)
{
    return rt_tick_get();
}

void
sys_init(void)
{
    /* Nothing to initialize — RT-Thread kernel is already running */
}

/* ------------------------------------------------------------------
 * Sleep
 * ------------------------------------------------------------------ */

void
sys_arch_msleep(u32_t delay_ms)
{
    rt_thread_mdelay(delay_ms);
}

/* ------------------------------------------------------------------
 * Mutex (optional — LWIP_COMPAT_MUTEX)
 * ------------------------------------------------------------------ */

#if !LWIP_COMPAT_MUTEX

err_t
sys_mutex_new(sys_mutex_t *mutex)
{
    LWIP_ASSERT("mutex != NULL", mutex != NULL);

    mutex->mut = rt_mutex_create("lwipMtx", RT_IPC_FLAG_FIFO);
    if (mutex->mut == NULL) {
        SYS_STATS_INC(mutex.err);
        return ERR_MEM;
    }
    SYS_STATS_INC_USED(mutex);
    return ERR_OK;
}

void
sys_mutex_lock(sys_mutex_t *mutex)
{
    LWIP_ASSERT("mutex != NULL", mutex != NULL);
    LWIP_ASSERT("mutex->mut != NULL", mutex->mut != NULL);

    rt_mutex_take((rt_mutex_t)mutex->mut, RT_WAITING_FOREVER);
}

void
sys_mutex_unlock(sys_mutex_t *mutex)
{
    LWIP_ASSERT("mutex != NULL", mutex != NULL);
    LWIP_ASSERT("mutex->mut != NULL", mutex->mut != NULL);

    rt_mutex_release((rt_mutex_t)mutex->mut);
}

void
sys_mutex_free(sys_mutex_t *mutex)
{
    LWIP_ASSERT("mutex != NULL", mutex != NULL);
    LWIP_ASSERT("mutex->mut != NULL", mutex->mut != NULL);

    SYS_STATS_DEC(mutex.used);
    rt_mutex_delete((rt_mutex_t)mutex->mut);
    mutex->mut = NULL;
}

#endif /* !LWIP_COMPAT_MUTEX */

/* ------------------------------------------------------------------
 * Semaphore
 * ------------------------------------------------------------------ */

err_t
sys_sem_new(sys_sem_t *sem, u8_t initial_count)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    LWIP_ASSERT("initial_count invalid (not 0 or 1)",
        (initial_count == 0) || (initial_count == 1));

    sem->sem = rt_sem_create("lwipSem", 0, RT_IPC_FLAG_FIFO);
    if (sem->sem == NULL) {
        SYS_STATS_INC(sem.err);
        return ERR_MEM;
    }
    SYS_STATS_INC_USED(sem);

    if (initial_count == 1) {
        rt_sem_release((rt_sem_t)sem->sem);
    }
    return ERR_OK;
}

void
sys_sem_signal(sys_sem_t *sem)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

    rt_sem_release((rt_sem_t)sem->sem);
}

/* ISR-safe signal */
void
sys_sem_signal_isr(sys_sem_t *sem)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

    /* rt_sem_release is ISR-safe */
    rt_sem_release((rt_sem_t)sem->sem);
}

u32_t
sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout_ms)
{
    rt_err_t ret;
    LWIP_ASSERT("sem != NULL", sem != NULL);
    LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

    if (!timeout_ms) {
        /* Wait forever */
        ret = rt_sem_take((rt_sem_t)sem->sem, RT_WAITING_FOREVER);
    } else {
        ret = rt_sem_take((rt_sem_t)sem->sem, rt_tick_from_millisecond(timeout_ms));
    }

    if (ret == RT_ETIMEOUT) {
        return SYS_ARCH_TIMEOUT;
    }

    return 1;
}

void
sys_sem_free(sys_sem_t *sem)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    LWIP_ASSERT("sem->sem != NULL", sem->sem != NULL);

    SYS_STATS_DEC(sem.used);
    rt_sem_delete((rt_sem_t)sem->sem);
    sem->sem = NULL;
}

/* ------------------------------------------------------------------
 * Mailbox (message queue)
 * ------------------------------------------------------------------ */

err_t
sys_mbox_new(sys_mbox_t *mbox, int size)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("size > 0", size > 0);

    mbox->mbx = rt_mq_create("lwipMbx", sizeof(void *), (rt_size_t)size, RT_IPC_FLAG_FIFO);
    if (mbox->mbx == NULL) {
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }
    SYS_STATS_INC_USED(mbox);
    return ERR_OK;
}

void
sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    rt_mq_send((rt_mq_t)mbox->mbx, &msg, sizeof(void *));
}

err_t
sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    rt_err_t ret;
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    ret = rt_mq_send((rt_mq_t)mbox->mbx, &msg, sizeof(void *));
    if (ret == RT_EOK) {
        return ERR_OK;
    } else {
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }
}

err_t
sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
    rt_err_t ret;
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    /* rt_mq_send is safe to call from ISR */
    ret = rt_mq_send((rt_mq_t)mbox->mbx, &msg, sizeof(void *));
    if (ret == RT_EOK) {
        return ERR_OK;
    } else {
        SYS_STATS_INC(mbox.err);
        return ERR_MEM;
    }
}

u32_t
sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout_ms)
{
    rt_err_t ret;
    void *msg_dummy;
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    if (!msg) {
        msg = &msg_dummy;
    }

    if (!timeout_ms) {
        /* Wait forever */
        ret = rt_mq_recv((rt_mq_t)mbox->mbx, msg, sizeof(void *), RT_WAITING_FOREVER);
    } else {
        ret = rt_mq_recv((rt_mq_t)mbox->mbx, msg, sizeof(void *),
                         rt_tick_from_millisecond(timeout_ms));
    }

    if (ret == RT_ETIMEOUT) {
        *msg = NULL;
        return SYS_ARCH_TIMEOUT;
    }

    return 1;
}

u32_t
sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    rt_err_t ret;
    void *msg_dummy;
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    if (!msg) {
        msg = &msg_dummy;
    }

    ret = rt_mq_recv((rt_mq_t)mbox->mbx, msg, sizeof(void *), 0);
    if (ret == RT_ETIMEOUT) {
        *msg = NULL;
        return SYS_MBOX_EMPTY;
    }

    return 1;
}

void
sys_mbox_free(sys_mbox_t *mbox)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("mbox->mbx != NULL", mbox->mbx != NULL);

    rt_mq_delete((rt_mq_t)mbox->mbx);
    SYS_STATS_DEC(mbox.used);
}

/* ------------------------------------------------------------------
 * Thread
 * ------------------------------------------------------------------ */

sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn thread, void *arg,
               int stacksize, int prio)
{
    rt_thread_t t;
    sys_thread_t lwip_thread;

    LWIP_ASSERT("invalid stacksize", stacksize > 0);

    /* lwip_thread_fn matches rt_thread_t entry: void (*)(void *) */
    t = rt_thread_create(name, (void (*)(void *))thread, arg,
                         (rt_uint32_t)stacksize, (rt_uint8_t)prio, 20);
    if (t != RT_NULL) {
        rt_thread_startup(t);
    }

    lwip_thread.thread_handle = (void *)t;
    return lwip_thread;
}

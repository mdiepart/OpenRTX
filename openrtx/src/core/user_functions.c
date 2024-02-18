/***************************************************************************
 *   Copyright (C) 2020 - 2024 by Federico Amedeo Izzo IU2NUO,             *
 *                                Niccolò Izzo IU2KIN                      *
 *                                Frederik Saraci IU2NRO                   *
 *                                Silvano Seva IU2KWO                      *
 *                                Morgan Diepart ON4MOD                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include "user_functions.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <pthread.h>
#include <interfaces/delays.h>
#include <time.h>
#include <hwconfig.h>

#if (CONFIG_USER_FUNCTIONS < 1) || (CONFIG_USER_FUNCTIONS > 32)
#error "CONFIG_USER_FUNCTIONS must be between 0 and 32"
#endif

static pthread_cond_t uf_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t uf_mut = PTHREAD_MUTEX_INITIALIZER;

static volatile uint32_t unlocked_async_tasks = 0;   // Tasks that must be executed after a trigger
static uint32_t expired_tasks = 0;          // Tasks that must be executed periodically

static long long uf_wakeup_time;

static struct timespec timeout;

typedef struct {
    void                    (*f)(void*);
    void                    *arg;
    uint32_t                next_exec;
    user_functions_sched_t  scheduling;
    bool                    enabled;
} user_functions_param_t;

static user_functions_param_t user_functions[CONFIG_USER_FUNCTIONS] = {0};

void user_functions_init()
{
    uf_wakeup_time = getTimeMs();
}

void user_functions_terminate()
{
    for(size_t i = 0; i < CONFIG_USER_FUNCTIONS; i++)
    {
        user_functions_disable(i);
    }
}

void user_functions_task()
{
    // Compute timeout
    long long int next_exec = INT64_MAX;

    for(size_t i = 0; i < CONFIG_USER_FUNCTIONS; i++)
    {
        if(user_functions[i].enabled && (user_functions[i].scheduling != USER_FUNCTION_ASYNC))
        {   
            if(user_functions[i].next_exec < next_exec)
            {
                expired_tasks = (1 << i); // Assignment so that we forget about previously set tasks
                next_exec = user_functions[i].next_exec;
            }
            else if(user_functions[i].next_exec == next_exec)
            {
                expired_tasks |= (1 << i);
            }           
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &timeout);
    long long int timeout_ns = ((long long int)next_exec - getTimeMs())*1000000;
    if(timeout_ns < 0) timeout_ns = 100000;
    timeout.tv_nsec += (next_exec - getTimeMs())*1000000; // 10ms
    if(timeout.tv_nsec >= 1e9)
    {
        timeout.tv_nsec -= 1e9;
        timeout.tv_sec++;
    }

    pthread_mutex_lock(&uf_mut);

    /*
     * From posix specs, pthread_cond_timedwait can wakeup spuriously,
     * thus we check if it timed out or if an async task must be executed.
     */
    while( (unlocked_async_tasks == 0) && (getTimeMs() < next_exec) )
    {
        pthread_cond_timedwait(&uf_cond, &uf_mut, &timeout);
    }

    if(getTimeMs() >= next_exec)
    {
        // Service all tasks that timed out
        while(expired_tasks)
        {
            uint8_t id = __builtin_ctz(expired_tasks);
            expired_tasks &= ~(1 << id);
            user_functions[id].f(user_functions[id].arg);
            switch (user_functions[id].scheduling)
            {
            case USER_FUNCTION_1HZ:
                user_functions[id].next_exec += 1000;
                break;
            case USER_FUNCTION_10HZ:
                user_functions[id].next_exec += 100;
                break;
            case USER_FUNCTION_20HZ:
                user_functions[id].next_exec += 50;
                break;
            case USER_FUNCTION_50HZ:
                user_functions[id].next_exec += 20;
                break;
            case USER_FUNCTION_100HZ:
                user_functions[id].next_exec += 10;
            default:
                break;
            }
        }
    }

    while(unlocked_async_tasks)
    {
        uint8_t id = __builtin_ctz(unlocked_async_tasks);
        unlocked_async_tasks &= ~(1 << id);
        user_functions[id].f(user_functions[id].arg);
    }

    pthread_mutex_unlock(&uf_mut);
}   

error_t user_functions_add(uint8_t id, void (*f)(), void *arg, user_functions_sched_t scheduling)
{
    if(id >= CONFIG_USER_FUNCTIONS)
        return EINVAL;
    
    if(user_functions[id].f != NULL)
        return EADDRINUSE;

    user_functions[id].f            = f;
    user_functions[id].arg          = arg;
    user_functions[id].scheduling   = scheduling;
    user_functions[id].enabled      = false;
    user_functions[id].next_exec    = getTimeMs();
    return 0;
}


error_t user_functions_remove(uint8_t id)
{
    if(id >= CONFIG_USER_FUNCTIONS)
        return EINVAL;
    
    user_functions[id].f = NULL;
    user_functions[id].enabled = false;
    
    return 0;
}
error_t user_functions_trigger(uint8_t id)
{
    if(id >= CONFIG_USER_FUNCTIONS)
        return EINVAL;

    pthread_mutex_lock(&uf_mut);
    unlocked_async_tasks |= (1 << id);
    pthread_cond_signal(&uf_cond);
    pthread_mutex_unlock(&uf_mut);

    return 0;
}

error_t user_functions_enable(uint8_t id)
{
    if(id >= CONFIG_USER_FUNCTIONS)
        return EINVAL;

    user_functions[id].enabled = true;

    return 0;
}

error_t user_functions_disable(uint8_t id)
{
    if(id >= CONFIG_USER_FUNCTIONS)
        return EINVAL;

    user_functions[id].enabled = false;

    return 0;
}
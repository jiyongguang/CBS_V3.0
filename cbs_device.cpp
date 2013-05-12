/*******************************************************************
*
*    DESCRIPTION:
*
*    AUTHOR:raidmanji
*
*    HISTORY:
*
*    DATE:2013/4/28
*
*    Copyright (C) 1998-2013 Tencent Inc. All Rights Reserved
*******************************************************************/
#include <time.h>
#include "cbs_device.h"

static CDevicePool g_device_pool;

CDevice* get_device_by_index(uint32 index)
{
    return g_device_pool._get_device_by_index(index);
}

void CDevice::cmd_done(cbs_buf_t* p_cbuf)
{
    p_cbuf->done_time = time(NULL);

    /*step 1. del timer*/
    _q_timer.del_timer(&p_cbuf->timer);

    /*step 2. del cbuf from the queue*/
    spin_lock(&_q_lock);
    switch (p_cbuf->links.flag)
    {
    case CBUF_DEVICE_Q:
        LOG_DEBUG(CBS_DEBUG_LEVEL, CBS_DEBUG_CBUF, "delete cbuf:[%u,%u] from the waiting Q of device:%u", 
                  p_cbuf->index, p_cbuf->sequence, p_cbuf->device_no);
        qu_DeQueue(&p_cbuf->links, CBUF_DEVICE_Q);
        _waitings--;
        break;
    case CBUF_DEVICE_PENDING_Q:
        LOG_DEBUG(CBS_DEBUG_LEVEL, CBS_DEBUG_CBUF, "delete cbuf:[%u,%u] from the pending Q of device:%u", 
                  p_cbuf->index, p_cbuf->sequence, p_cbuf->device_no);
        qu_DeQueue(&p_cbuf->links, CBUF_DEVICE_PENDING_Q);
        _pendings--;
    case QUEUE_FREE_ELEMENT:
    default:
        LOG_ERROR("flag of cbus is %u, device:%u", p_cbuf->links.flag, p_cbuf->device_no);
        break;
    }
    spin_unlock(&_q_lock);

    return;
}

CDevice* CDevicePool::_get_device_by_index(uint32 index)
{
    //invalid device index;
    index = (index >= CBS_MAX_DEVICES?CBS_MAX_INVALID_DEVICES:index);
    return &G_devices[index];
}

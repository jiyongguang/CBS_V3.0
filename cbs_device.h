#ifndef CBS_DEVICE_H
#define CBS_DEVICE_H

#include "cbs_types.h"
#include "cbs_util.h"
#include "cbs_timer.h"
#include "cbs_scsi.h"
#include "cbs_buf.h"

#define CBS_MAX_INVALID_DEVICES 1
#define CBS_MAX_DISK_DEVICES    48
#define CBS_MAX_NULL_DEVICES    1
#define CBS_MAX_TSSD_DEVICES    CBS_MAX_DISK_DEVICES
#define CBS_MAX_TFS_DEVICES     CBS_MAX_DISK_DEVICES
#define CBS_MAX_THREAD_DEVICES  8
#define CBS_MAX_CONFIG_DEVICES  1

#define CBS_MAX_DEVICES (CBS_MAX_INVALID_DEVICES + \
                        CBS_MAX_DISK_DEVICES + \
                        CBS_MAX_NULL_DEVICES + \
                        CBS_MAX_TSSD_DEVICES + \
                        CBS_MAX_TFS_DEVICES + \
                        CBS_MAX_THREAD_DEVICES + \
                        CBS_MAX_CONFIG_DEVICES)

class CDevice
{
public:
	CDevice():_total_cmd(0),_sequence(0),_waitings(0),_pendings(0),_last_started(0)
	{
		qu_QueueInit(&_cbufs, CBUF_DEVICE_Q_HEAD);
		qu_QueueInit(&_cbufs_pending, CBUF_DEVICE_PENDING_Q_HEAD);
		spin_lock_init(&_q_lock);
	}
	virtual ~CDevice(){}
	
public:
	qu_queue_t _cbufs;
	qu_queue_t _cbufs_pending;
	spinlock_t _q_lock;
	
    uint32 _class_id;
	uint32 _index;
	uint32 _total_cmd;
	uint16 _sequence;
	uint16 _waitings;
    uint16 _pendings;
	
	CTimerQueue _q_timer;
	time_t _last_started;

    scsi_sense_info_t _latest_sense;
public:
    virtual void cmd_done(cbs_buf_t* p_cbuf);
};

class CDevicePool
{
public:
    CDevicePool()
    {
        for (int i = 0; i < CBS_MAX_DEVICES; i++)
        {
            G_devices[i] = NULL;
        }
    }
    virtual~ CDevicePool(){}

private:
    CDevice* G_devices[CBS_MAX_DEVICES];

public:
    CDevice* _get_device_by_index(uint32 index);
};

extern CDevice* get_device_by_index(uint32 index);

#endif

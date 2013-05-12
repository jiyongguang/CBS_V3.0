#include <assert.h>
#include "cbs_target.h"
#include "cbs_scsi.h"

/**
 * Sense Context Table. Sense codes are SCSI error codes and a sense
 * context is shorthand for an error that occurs in the RAID. The SCSI Sense
 * codes are generated by a simple lookup of a sense context. This is
 * easier than generating SCSI sense codes because many similar operations
 * generate the same codes. If we want to change this behaviour we have to 
 * change the code in one place - here.
 */
typedef struct sense_tuple
{
    uint8 key;
    uint16 asc;
}sense_tuple_t;

static sense_tuple_t sense_ctx_tbl[nb_sense_contexts] = {
    /** SENSE_CTX_NONE             */ { KEY_NO_SENSE        , ASC_NO_ADDT_SENSE_INFO         },
    /** SENSE_CTX_TASK_FULL        */ { KEY_NO_SENSE        , ASC_NO_ADDT_SENSE_INFO},
    /** SENSE_CTX_ILLEGAL_CDB      */ { KEY_ILLEGAL_REQUEST , ASC_INVALID_FIELD_IN_CDB}, 
    /** SENSE_CTX_NOT_READY        */ { KEY_NOT_READY       , ASC_LUN_NR_BECOMING_RDY},
    /** SENSE_CTX_HARDWARE_ERR     */ { KEY_HARDWARE_ERROR  , ASC_INTERNAL_TARGET_ERR},
    /** SENSE_CTX_WRITE_PROTECT    */ { KEY_DATA_PROTECT    , ASC_WR_PROTECTED},
    /** SENSE_CTX_LUN_UNSUPPORTED  */ { KEY_ILLEGAL_REQUEST , ASC_LUN_UNSUPPORTED},
    /** SENSE_CTX_RESERVE_CONF     */ { KEY_NO_SENSE        , ASC_NO_ADDT_SENSE_INFO},
    /** SENSE_CTX_RESET_POWER_ON   */ { KEY_UNIT_ATTENTION  , ASC_POWER_ON_RESET},
    /** SENSE_CTX_RESET_DIRECTED   */ { KEY_UNIT_ATTENTION  , ASC_BUS_DEVICE_RESET},
    /** SENSE_CTX_MEDIUM_ERR_RD    */ { KEY_MEDIUM_ERROR    , ASC_RD_ERR},
    /** SENSE_CTX_MEDIUM_ERR_WR    */ { KEY_MEDIUM_ERROR    , ASC_WR_ERR},
    /** SENSE_CTX_MEDIUM_ERR_AMNF  */ { KEY_MEDIUM_ERROR    , ASC_ADDR_MARK_NOT_FOUND},
    /** SENSE_CTX_TASK_SET_CLEARED */ { KEY_UNIT_ATTENTION  , ASC_TAG_CMD_CLRD_BY_OTHER_INIT},
    /** SENSE_CTX_VOL_OVERFLOW     */ { KEY_VOLUME_OVERFLOW , ASC_LOGICAL_BLOCK_OUT_OF_RANGE},
    /** SENSE_CTX_FC_ERR           */ { KEY_ABORTED_COMMAND , ASC_SCSI_BUS_PARITY_ERR}, 
    /** SENSE_CTX_SCSI_PARITY_ERR  */ { KEY_ABORTED_COMMAND , ASC_SCSI_BUS_PARITY_ERR}, 
    /** SENSE_CTX_SCSI_DATA_ERR    */ { KEY_ABORTED_COMMAND , ASC_SCSI_DATA_PHASE_CRC_ERR}, 
    /** SENSE_CTX_SCSI_ST_ERR      */ { KEY_ABORTED_COMMAND , ASC_SCSI_ST_DATA_PHASE_CRC_ERR}, 
    /** SENSE_CTX_SCSI_IU_ERR      */ { KEY_ABORTED_COMMAND , ASC_SCSI_IU_PHASE_CRC_ERR}, 
    /** SENSE_CTX_MSG_RJT_ERR      */ { KEY_ABORTED_COMMAND , ASC_MSG_RJT_ERR},
    /** SENSE_CTX_INITIATOR_ERR    */ { KEY_ABORTED_COMMAND , ASC_INITIATOR_DETECTED_ERR}, 
    /** SENSE_CTX_RESERVES_PREEMPTED*/{ KEY_UNIT_ATTENTION  , ASC_RESERVATIONS_PREEMPTED},
    /** SENSE_CTX_DOWNLOAD_FAILED   */{ KEY_HARDWARE_ERROR  , ASC_CHECKSUM_ERROR_ON_DOWNLOAD},
    /** SENSE_CTX_INSUFF_REG_RESOURC*/{ KEY_ILLEGAL_REQUEST , ASC_INSUFFICIENT_REGISTRATION_RESOURCES},
    /** SENSE_CTX_PARAM_LIST_LEN_ERR*/{ KEY_ILLEGAL_REQUEST , ASC_PARAM_LIST_LEN_ERR},
    /** SENSE_CTX_INVALID REL _OF_PR*/{ KEY_ILLEGAL_REQUEST , ASC_INVALD_RELEASE_OF_PR},
    /** SENSE_CTX_IFP_IN_PARAM_LIST */{ KEY_ILLEGAL_REQUEST , ASC_IFP_IN_PARAM_LIST},
    /** SENSE_CTX_TP_IN_TRANSIT   */  { KEY_NOT_READY       , ASC_LUN_NA_TP_IN_TRANSIT},
    /** SENSE_CTX_TP_IN_STANDY      */{ KEY_NOT_READY       , ASC_LUN_NA_TP_IN_STANDBY},
    /**SENSE_CTX_TP_IN_UNAVAILABLE  */{ KEY_NOT_READY       , ASC_LUN_NA_TP_IN_UNAVAILABLE},     

    /** SENSE_CTX_INTERNAL_REBUILD_FAILED */ { KEY_ABORTED_COMMAND  , ASC_RT_INTERNAL_REBUILD_STRIPE_FAILED},
    /** SENSE_CTX_INTERNAL_CMD_FAILED     */ { KEY_ABORTED_COMMAND  , ASC_RT_INTERNAL_CMD_FAILED},
    /** SENSE_CTX_INTERNAL_XFER_FAILED    */ { KEY_ABORTED_COMMAND  , ASC_RT_INTERNAL_XFER_FAILED},
    /** SENSE_CTX_INTERNAL_RESOURCE_FAILED*/ { KEY_ABORTED_COMMAND  , ASC_RT_INTERNAL_RESOURCE_FAILED},
    /** SENSE_CTX_INTERNAL_WRONG_PARAM    */ { KEY_ABORTED_COMMAND  , ASC_RT_INTERNAL_WRONG_PARAM},
    /** SENSE_CTX_INTERNAL_LINK_DOWN      */ { KEY_ABORTED_COMMAND  , ASC_RT_INTERNAL_LINK_DOWN},
    /** SENSE_CTX_INTERNAL_DEVICE_GONE    */ { KEY_ABORTED_COMMAND  , ASC_RT_INTERNAL_DEVICE_GONE} 
};

void CTarget::translate_sense_ctx(uint32 sense_ctx, scsi_sense_info_t* p_sense_info)
{
    uint16 add_code_qual;

    add_code_qual = sense_ctx_tbl[sense_ctx].asc;
    p_sense_info->key     = sense_ctx_tbl[sense_ctx].key;
    p_sense_info->rsv1    = 0;
    p_sense_info->ili     = 0;
    p_sense_info->rsv2    = 0;
    p_sense_info->segment = 0;
    p_sense_info->error_code  = 0x70;
    p_sense_info->valid   = 0;
    p_sense_info->addt_len= 0xa;             /* should be set to 0xa maximum. */
    p_sense_info->info[0] = 0;
    p_sense_info->info[1] = 0;
    p_sense_info->info[2] = 0;
    p_sense_info->info[3] = 0;
    p_sense_info->csi     = 0;
    p_sense_info->sks0    = 0;
    p_sense_info->sksv    = 0;
    p_sense_info->rsv3    = 0;
    p_sense_info->addt_sc_qualifier = (add_code_qual) & 0xff ; 
    p_sense_info->addt_sc           = (add_code_qual >>8) & 0xff ;
    p_sense_info->uniq_sense_data0  = 0;
    p_sense_info->uniq_sense_data1  = 0;
    p_sense_info->sks2              = 0;
    p_sense_info->sks1              = 0;

    return;
}

void CTarget::get_sense_info_and_update(cbs_buf_t* p_cbuf)
{    
    if (p_cbuf->target.gen.sense_context > nb_sense_contexts)
    {
        LOG_ERROR("invalid sense context:0x%x", p_cbuf->target.gen.sense_context);
        p_cbuf->target.gen.sense_context = SENSE_CTX_NONE;
    }
    translate_sense_ctx(p_cbuf->target.gen.sense_context, get_device_sense(p_cbuf->device_no));

    return ;
}

scsi_sense_info_t* CTarget::get_device_sense(uint32 device_no)
{
    CDevice* p_dev = get_device_by_index(device_no);
    return &p_dev->_latest_sense;
}

/*
 *Function Name:build_check_resp_info
 *
 *Parameters:cbs_buf_t
 *
 *Description:Builds the response information field of the cbuf 
 *            if scsi status is not good. 
 *
 *Returns:void
 *
 */
void CTarget::build_check_resp_info(cbs_buf_t* p_cbuf)
{
    scsi_sense_info_t *p_sense_info;

    /* If the context is already valid, we don't want to overwrite it */
    if (p_cbuf->packet.response.status_flags & CBUF_STATUS_FLAG_SENSE_LEN_VALID)
    {
        return;
    }

    p_cbuf->packet.response.status_flags = CBUF_STATUS_FLAG_SENSE_LEN_VALID | CBUF_STATUS_FLAG_RSP_LEN_VALID;
    p_cbuf->packet.response.residual_count = 0;

    p_cbuf->packet.response.resp_info_len = 8;

    get_sense_info_and_update(p_cbuf);
    p_sense_info = &p_cbuf->packet.response.sense_info;
    p_cbuf->packet.response.sense_data_len = target_extract_sense_info(*p_sense_info, p_cbuf->device_no);

    if ( p_cbuf->packet.response.sense_info.key == KEY_NO_SENSE )
    {
        p_cbuf->packet.response.status_flags &= ~CBUF_STATUS_FLAG_SENSE_LEN_VALID;
        p_cbuf->packet.response.sense_data_len = 0;
    }

    return;
}

RT_STATUS CTarget::queue_cbuf(CDevice* p_dev, cbs_buf_t* p_cbuf)
{
    if (p_dev->_index != p_cbuf->device_no)
    {
        LOG_ERROR("cbuf should be add to device:0x%x, "
                  "but send to the wrong device:0x%x", p_cbuf->device_no, p_dev->_index);
        p_cbuf->device_no = p_dev->_index;
    }

    spin_lock(&p_dev->_q_lock);
    if (p_cbuf->flags & CBUF_FLAG_QUEUE_FIRST)
    {
        /** add to the first of the queue */
        qu_EnQueue(&p_cbuf->links, &p_dev->_cbufs, CBUF_DEVICE_Q);
    }
    else
    {
        /** add to the back of the queue */
        qu_EnQueueTail(&p_cbuf->links, &p_dev->_cbufs, CBUF_DEVICE_Q);
    }

    p_cbuf->q_sequence = p_dev->_sequence++;
    p_dev->_total_cmd++;
    p_dev->_waitings++
    spin_unlock(&p_dev->_q_lock);

    p_dev->_last_started = time(NULL);
    p_cbuf->queue_time = time(NULL);

    if ((p_cbuf->flags & CBUF_FLAG_SCAN_LATER) != 0)
        return RT_OK;

    queue_scan(p_dev);

    return RT_OK;
}

void CTarget::cmd_done(cbs_buf_t* p_cbuf)
{
    LOG_DEBUG(CBS_DEBUG_LEVEL, CBS_DEBUG_TARGET, "cmd done in basic class, do nothing.");
}

void CTarget::target_cmd_done(cbs_buf_t* p_cbuf)
{
    if (p_cbuf->packet.response.status != SCSI_STATUS_GOOD)
    {
        build_check_resp_info(p_cbuf);
    }
    else
    {
        /* check residual if scsi status is good */
        if (p_cbuf->residual != 0)
        {
            p_cbuf->packet.response.status_flags = CBUF_STATUS_FLAG_UNDER;  
        }
        p_cbuf->packet.response.residual_count = p_cbuf->residual; 
    }

    cmd_done(p_cbuf);

    p_cbuf->target.generic.handle = 0;
    p_cbuf->target.generic.args[0] = 0;

    /* work on target sid finished, call init_IODone */
    init_IODone(p_cbuf);

    return;
}

uint32 CTarget::target_extract_sense_info(scsi_sense_info_t& sense_info, uint32 device_no)
{
    scsi_sense_info_t* p_sense_info;

    p_sense_info = get_device_sense(device_no);
    sense_info = *p_sense_info; //copy the latest sense of this device
    if (sense_info.key == 0)
    {
        sense_info.error_code = 0x70;
        return 8;
    }

    return SCSI_SENSE_LEN;
}

void CTarget::target_reply_good(cbs_buf_t* p_cbuf)
{
    p_cbuf->packet.response.completion_status = 0;
    p_cbuf->packet.response.status = SCSI_STATUS_GOOD;
    p_cbuf->response = CBUF_RESP_COMPLETE;
}

void CTargetManager::target_ClassInit(uint32 class_id, uint32 flags, const char* p_name, CTarget* p_target)
{
    assert(class_id < TARGET_CLASS_MAX);
    assert(G_targets[class_id] == NULL);
    assert(p_name != NULL);

    G_targets[class_id] = p_target;
    p_target->_class_id = class_id;
    p_target->_flags = flags;
    p_target->_name.assign(p_name);

    return; 
}

CTarget* CTargetManager::get_target_by_class_id(uint32 class_id)
{
    class_id = (class_id >= TARGET_CLASS_MAX?TARGET_CLASS_INVALID:class_id);

    return &G_targets[class_id];
}

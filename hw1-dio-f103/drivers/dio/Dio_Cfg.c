/*
 * Dio_Cfg.c — Bảng nhóm kênh của module Dio (nằm trong Flash, không đổi lúc chạy)
 */
#include "Dio.h"

const Dio_ChannelGroupType Dio_ChannelGroupConfig[DIO_NUM_CHANNEL_GROUPS] =
{
    /* [0] DioConf_DioChannelGroup_Test: PA4–PA7 → bit 4..7 của cổng A */
    {
        0x00F0U,               /* mask   */
        4U,                    /* offset */
        DioConf_DioPort_A      /* port   */
    }
};

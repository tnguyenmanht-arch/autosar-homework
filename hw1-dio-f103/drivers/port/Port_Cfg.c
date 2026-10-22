/*
 * Port_Cfg.c — Bảng chân cho Port_Init. Phải khớp với các kênh/nhóm khai báo trong Dio_Cfg.h.
 */
#include "Port.h"

static const Port_PinConfigType Port_PinConfig[PORT_NUM_CONFIGURED_PINS] =
{
    /* cổng       chân  chế độ                     mức ban đầu */
    { PORT_ID_C,  13U,  PORT_MODE_OUTPUT_PP_2MHZ,  STD_HIGH },   /* LED board: HIGH = tắt */
    { PORT_ID_A,   0U,  PORT_MODE_INPUT_PULL,      STD_HIGH },   /* nút thử: ODR = 1 → pull-up */
    { PORT_ID_A,   4U,  PORT_MODE_OUTPUT_PP_2MHZ,  STD_LOW  },   /* nhóm Test, bit 0 */
    { PORT_ID_A,   5U,  PORT_MODE_OUTPUT_PP_2MHZ,  STD_LOW  },   /* nhóm Test, bit 1 */
    { PORT_ID_A,   6U,  PORT_MODE_OUTPUT_PP_2MHZ,  STD_LOW  },   /* nhóm Test, bit 2 */
    { PORT_ID_A,   7U,  PORT_MODE_OUTPUT_PP_2MHZ,  STD_LOW  }    /* nhóm Test, bit 3 */
};

const Port_ConfigType Port_Config =
{
    Port_PinConfig,
    PORT_NUM_CONFIGURED_PINS
};

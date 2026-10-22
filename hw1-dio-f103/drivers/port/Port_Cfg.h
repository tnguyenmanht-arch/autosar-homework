/*
 * Port_Cfg.h — Cấu hình module Port cho board Yahboom YB-MSV01
 *
 * File này được Port.h include SAU phần khai báo kiểu. Không include trực tiếp: luôn include "Port.h".
 * Chân cấm dùng (xem CLAUDE.md): PA9..PA15, PB2, PB3, PB4, PB8, PB9. Không đưa vào bảng.
 */
#ifndef PORT_CFG_H
#define PORT_CFG_H

#define PORT_NUM_CONFIGURED_PINS   (6U)   /* PC13, PA0, PA4..PA7 */

extern const Port_ConfigType Port_Config;

#endif /* PORT_CFG_H */

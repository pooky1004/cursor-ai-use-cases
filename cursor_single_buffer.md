# Single buffer 구조로 변경 요청
_Exported on 8/1/2025 at 11:40:26 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
사용하려고 한다.
성능을 높이기 위해 패킷 송신과 수신에 대해 multi-buffer를 사용하는데 별 효과가 없다
single buffer 구조로 변경해 달라

multi-buffer를 사용하기 위해 사용했던 변수, 함수들도 삭제해 주세요.

---

**Cursor**

먼저 현재 코드베이스를 살펴보고 multi-buffer 구조를 파악한 후 single buffer로 변경하겠습니다.


이제 multi-buffer 구조를 파악했습니다. 현재 코드는 다음과 같은 multi-buffer 구조를 사용하고 있습니다:

1. **TX Buffer Pool**: 128개의 SKB를 미리 할당하여 재사용
2. **RX Buffer Pool**: 32개의 SKB를 미리 할당하여 재사용
3. **Buffer Pool Management**: head/tail 인덱스로 순환 큐 관리
4. **Concurrent Processing**: 여러 패킷을 동시에 처리할 수 있는 구조

이제 single buffer 구조로 변경하겠습니다. multi-buffer 관련 코드를 제거하고 단순한 single buffer 구조로 변경하겠습니다.


```cpp
#pragma once

#include <linux/etherdevice.h>
#include <linux/spi/spi.h>

struct lan1qtsn_spi;

#define TSN_SYSTEM_CONTROL_L 0x0294 /* RESERVED [31:1] : TSN ENABLE */
#define TSN_DISABLE 0x0
#define TSN_ENABLE 0x1

/* OPEN Alliance TC6 registers */
/* Standard Capabilities Register */
#define LAN1QTSN_SPI_REG_STDCAP 0x0002
#define STDCAP_DIRECT_PHY_REG_ACCESS BIT(8)

/* Reset Control and Status Register */
#define LAN1QTSN_SPI_REG_RESET 0x0003
#define RESET_SWRESET BIT(0) /* Software Reset */

/* Configuration Register #0 */
#define LAN1QTSN_SPI_REG_CONFIG0 0x0004
#define CONFIG0_SYNC BIT(15)
#define CONFIG0_ZARFE_ENABLE BIT(12)

/* Status Register #0 */
#define LAN1QTSN_SPI_REG_STATUS0 0x0008
#define STATUS0_RESETC BIT(6) /* Reset Complete */
#define STATUS0_HEADER_ERROR BIT(5)
#define STATUS0_LOSS_OF_FRAME_ERROR BIT(4)
#define STATUS0_RX_BUFFER_OVERFLOW_ERROR BIT(3)
#define STATUS0_TX_PROTOCOL_ERROR BIT(0)

/* Buffer Status Register */
#define LAN1QTSN_SPI_REG_BUFFER_STATUS 0x000B
#define BUFFER_STATUS_TX_CREDITS_AVAILABLE GENMASK(15, 8)
#define BUFFER_STATUS_RX_CHUNKS_AVAILABLE GENMASK(7, 0)

/* Interrupt Mask Register #0 */
#define LAN1QTSN_SPI_REG_INT_MASK0 0x000C
#define INT_MASK0_HEADER_ERR_MASK BIT(5)
#define INT_MASK0_LOSS_OF_FRAME_ERR_MASK BIT(4)
#define INT_MASK0_RX_BUFFER_OVERFLOW_ERR_MASK BIT(3)
#define INT_MASK0_TX_PROTOCOL_ERR_MASK BIT(0)

/* PHY Clause 22 registers base address and mask */
#define LAN1QTSN_SPI_PHY_STD_REG_ADDR_BASE 0xFF00
#define LAN1QTSN_SPI_PHY_STD_REG_ADDR_MASK 0x1F

/* Control command header */
#define LAN1QTSN_SPI_CTRL_HEADER_DATA_NOT_CTRL BIT(31)
#define LAN1QTSN_SPI_CTRL_HEADER_WRITE_NOT_READ BIT(29)
#define LAN1QTSN_SPI_CTRL_HEADER_MEM_MAP_SELECTOR GENMASK(27, 24)
#define LAN1QTSN_SPI_CTRL_HEADER_ADDR GENMASK(23, 8)
#define LAN1QTSN_SPI_CTRL_HEADER_LENGTH GENMASK(7, 1)
#define LAN1QTSN_SPI_CTRL_HEADER_PARITY BIT(0)

/* Data header */
#define LAN1QTSN_SPI_DATA_HEADER_DATA_NOT_CTRL BIT(31)
#define LAN1QTSN_SPI_DATA_HEADER_DATA_VALID BIT(21)
#define LAN1QTSN_SPI_DATA_HEADER_START_VALID BIT(20)
#define LAN1QTSN_SPI_DATA_HEADER_START_WORD_OFFSET GENMASK(19, 16)
#define LAN1QTSN_SPI_DATA_HEADER_END_VALID BIT(14)
#define LAN1QTSN_SPI_DATA_HEADER_END_BYTE_OFFSET GENMASK(13, 8)
#define LAN1QTSN_SPI_DATA_HEADER_PARITY BIT(0)

/* Data footer */
#define LAN1QTSN_SPI_DATA_FOOTER_EXTENDED_STS BIT(31)
#define LAN1QTSN_SPI_DATA_FOOTER_RXD_HEADER_BAD BIT(30)
#define LAN1QTSN_SPI_DATA_FOOTER_CONFIG_SYNC BIT(29)
#define LAN1QTSN_SPI_DATA_FOOTER_RX_CHUNKS GENMASK(28, 24)
#define LAN1QTSN_SPI_DATA_FOOTER_DATA_VALID BIT(21)
#define LAN1QTSN_SPI_DATA_FOOTER_START_VALID BIT(20)
#define LAN1QTSN_SPI_DATA_FOOTER_START_WORD_OFFSET GENMASK(19, 16)
#define LAN1QTSN_SPI_DATA_FOOTER_END_VALID BIT(14)
#define LAN1QTSN_SPI_DATA_FOOTER_END_BYTE_OFFSET GENMASK(13, 8)
#define LAN1QTSN_SPI_DATA_FOOTER_TX_CREDITS GENMASK(5, 1)

/* PHY – Clause 45 registers memory map selector (MMS) as per table 6 in the
 * OPEN Alliance specification.
 */
#define LAN1QTSN_SPI_PHY_C45_PCS_MMS2 2        /* MMD 3 */
#define LAN1QTSN_SPI_PHY_C45_PMA_PMD_MMS3 3    /* MMD 1 */
#define LAN1QTSN_SPI_PHY_C45_VS_PLCA_MMS4 4    /* MMD 31 */
#define LAN1QTSN_SPI_PHY_C45_AUTO_NEG_MMS5 5   /* MMD 7 */
#define LAN1QTSN_SPI_PHY_C45_POWER_UNIT_MMS6 6 /* MMD 13 */

#define LAN1QTSN_SPI_CTRL_HEADER_SIZE 4
#define LAN1QTSN_SPI_CTRL_REG_VALUE_SIZE 4
#define LAN1QTSN_SPI_CTRL_IGNORED_SIZE 4
#define LAN1QTSN_SPI_CTRL_SPI_BUF_SIZE                                                                      \
    (LAN1QTSN_SPI_CTRL_HEADER_SIZE + (LAN1QTSN_SPI_CTRL_MAX_REGISTERS * LAN1QTSN_SPI_CTRL_REG_VALUE_SIZE) + \
     LAN1QTSN_SPI_CTRL_IGNORED_SIZE)
#define LAN1QTSN_SPI_CHUNK_PAYLOAD_SIZE 64
#define LAN1QTSN_SPI_DATA_HEADER_SIZE 4
#define LAN1QTSN_SPI_CHUNK_SIZE (LAN1QTSN_SPI_DATA_HEADER_SIZE + LAN1QTSN_SPI_CHUNK_PAYLOAD_SIZE)
#define LAN1QTSN_SPI_MAX_TX_CHUNKS 48
#define LAN1QTSN_SPI_DATA_BUF_SIZE (LAN1QTSN_SPI_MAX_TX_CHUNKS * LAN1QTSN_SPI_CHUNK_SIZE)
#define STATUS0_RESETC_POLL_DELAY 1000
#define STATUS0_RESETC_POLL_TIMEOUT 1000000
#define LAN1QTSN_SPI_META_DATA_BUF_SIZE 16

#define LAN1QTSN_SPI_CMD_WRITE 0x01
#define LAN1QTSN_SPI_CMD_READ 0x02

#define LAN1QTSN_SPI_RX_FRAME_FIFO_BASE 0x1000
#define LAN1QTSN_SPI_TX_FRAME_FIFO_BASE 0x2000
#define LAN1QTSN_SPI_REG_READ_FIFO_BASE 0x3000
#define LAN1QTSN_SPI_REG_WRITE_FIFO_BASE 0x4000
#define LAN1QTSN_SPI_METADATA_FIFO_BASE 0xF000

#define LAN1QTSN_SPI_CMD_SIZE 3
#define LAN1QTSN_SPI_RD_CMD_GAP_SIZE 1
#define LAN1QTSN_SPI_CTRL_DATA_SIZE 4
#define LAN1QTSN_SPI_CTRL_MAX_REGISTERS 1
#define LAN1QTSN_SPI_CTRL_BUF_SIZE 16
#define LAN1QTSN_SPI_META_DATA_SIZE 10
#define LAN1QTSN_SPI_META_BUF_SIZE 16

/* Single buffer configuration */
#define LAN1QTSN_BUFFER_SIZE 2048

/* Internal structure for MAC-PHY drivers */
struct lan1qtsn_spi {
    struct device* dev;
    struct net_device* netdev;
    struct phy_device* phydev;
    struct mii_bus* mdiobus;
    struct spi_device* spi;
    struct mutex spi_ctrl_lock; /* Protects spi control transfer */
    spinlock_t tx_skb_lock;     /* Protects tx skb handling */
    void* spi_ctrl_tx_buf;
    void* spi_ctrl_rx_buf;
    void* spi_data_tx_buf;
    void* spi_data_rx_buf;
    void* spi_meta_tx_buf;
    void* spi_meta_rx_buf;
    struct sk_buff* ongoing_tx_skb;
    struct sk_buff* waiting_tx_skb;
    struct sk_buff* rx_skb;
    struct task_struct* spi_thread;
    wait_queue_head_t spi_wq;
    u16 tx_skb_offset;
    bool rx_buf_overflow;
    bool int_flag;
};

enum lan1qtsn_spi_header_type {
    LAN1QTSN_SPI_CTRL_HEADER,
    LAN1QTSN_SPI_DATA_HEADER,
    LAN1QTSN_SPI_META_HEADER,
};

enum lan1qtsn_spi_register_op {
    LAN1QTSN_SPI_CTRL_REG_READ = 0,
    LAN1QTSN_SPI_CTRL_REG_WRITE = 1,
};

enum lan1qtsn_spi_data_valid_info {
    LAN1QTSN_SPI_DATA_INVALID,
    LAN1QTSN_SPI_DATA_VALID,
};

enum lan1qtsn_spi_data_start_valid_info {
    LAN1QTSN_SPI_DATA_START_INVALID,
    LAN1QTSN_SPI_DATA_START_VALID,
};

enum lan1qtsn_spi_data_end_valid_info {
    LAN1QTSN_SPI_DATA_END_INVALID,
    LAN1QTSN_SPI_DATA_END_VALID,
};

/* PHY Vendor Specific Registers Collision Detector Control 0 Register */
#define LAN1QTSN_REG_MMS4_CDCTL0 0x00040087
#define MMS4_CDCTL0_CDEN_SHIFT BIT(15) /* Collision Detect Enable */

#define LAN1QTSN_REG_MMS4_PLCA_CTRL0 0x0004CA01
#define LAN1QTSN_REG_MMS1_MAC_NCFGR 0x00010001
#define LAN1QTSN_REG_MMS1_MAC_NCR 0x00010000
#define LAN1QTSN_REG_MMS1_MAC_TI 0x00010077
#define LAN1QTSN_REG_MMS0_OA_CONFIG0 0x00000004
#define MMS0_OA_CONFIG0_SYNC_SHIFT BIT(15)
#define MMS0_OA_CONFIG0_FTSE_SHIFT BIT(7)
#define MMS0_OA_CONFIG0_FTSS_SHIFT BIT(6)
#define LAN1QTSN_REG_MMS0_OA_CONFIG0 0x00000004
#define LAN1QTSN_REG_MMS0_OA_STATUS0 0x00000008
#define MMS0_OA_STATUS0_RESETC_SHIFT BIT(6)

#define LAN1QTSN_REG_MMS4_INDIR_RD_ADDR 0x000400d8
#define LAN1QTSN_REG_MMS4_INDIR_RD_VAL 0x000400d9
#define LAN1QTSN_REG_MMS4_INDIR_RD_WIDTH 0x000400da
#define LAN1QTSN_MMS04_INDIR_WIDTH 0x2

#define LAN1QTSN_REG_MMS4_SLPCTL1 0x00040081
#define LAN1QTSN_REG_MMS4_TXMMSKH 0x00040043
#define LAN1QTSN_REG_MMS4_TXMMSKL 0x00040044
#define LAN1QTSN_REG_MMS4_TXMLOC 0x00040045
#define LAN1QTSN_REG_MMS4_RXMMSKH 0x00040053
#define LAN1QTSN_REG_MMS4_RXMMSKL 0x00040054
#define LAN1QTSN_REG_MMS4_RXMLOC 0x00040055
#define LAN1QTSN_REG_MMS4_TXMCTL 0x00040040
#define LAN1QTSN_REG_MMS4_RXMCTL 0x00040050

#define LAN1QTSN_REG_MMS4_A_00D0 0x000400d0
#define LAN1QTSN_REG_MMS4_A_00E0 0x000400e0
#define LAN1QTSN_REG_MMS4_A_0077 0x00040077
#define LAN1QTSN_REG_MMS4_A_0084 0x00040084
#define LAN1QTSN_REG_MMS4_A_008A 0x0004008a
#define LAN1QTSN_REG_MMS4_A_0091 0x00040091
#define LAN1QTSN_REG_MMS4_A_00E9 0x000400e9
#define LAN1QTSN_REG_MMS4_A_00F5 0x000400f5
#define LAN1QTSN_REG_MMS4_A_00F4 0x000400f4
#define LAN1QTSN_REG_MMS4_A_00F8 0x000400f8
#define LAN1QTSN_REG_MMS4_A_00F9 0x000400f9

/* Value1: -5 to 15 */
#define VALUE1_ADDR 0x04
#define VALUE2_ADDR 0x08
#define VALUE_MASK 0x1F
#define VALUE_SIGN_MASK 0x10

#define VALUE1_OFFSET1 9
#define VALUE1_OFFSET2 14
#define VALUE1_SHIFT1 10
#define VALUE1_SHIFT2 4
#define VALUE1_LOWEST_VAL 0x03

#define VALUE2_OFFSET 40
#define VALUE2_SHIFT 10

#define VALUE_OFFSET_MASK 0x3F

#define MAX_VALUE_5BITS 0x20

#define MMS4_A_00D0_V 0x3F31
#define MMS4_A_00E0_V 0xC000
#define MMS4_A_00E9_V 0x9E50
#define MMS4_A_00F5_V 0x1CF8
#define MMS4_A_00F4_V 0xC020
#define MMS4_A_00F8_V 0xB900
#define MMS4_A_00F9_V 0x4E53
#define MMS4_A_0081_V 0x0080
#define MMS4_A_0091_V 0x9660
#define MMS4_A_0077_V 0x0028
#define MMS4_A_0043_V 0x00FF
#define MMS4_A_0044_V 0xFFFF
#define MMS4_A_0045_V 0x0000
#define MMS4_A_0053_V 0x00FF
#define MMS4_A_0054_V 0xFFFF
#define MMS4_A_0055_V 0x0000
#define MMS4_A_0040_V 0x0002
#define MMS4_A_0050_V 0x0002

#define MMS4_PLCA_CTRL0_INIT_VAL 0x00008000
#define MMS1_MAC_NCFGR_INIT_VAL 0x000000C0
#define MMS1_MAC_NCR_INIT_VAL 0x0000000C

#define TIMER_INCREMENT 0x28

struct lan1qtsn_spi* lan1qtsn_spi_init(struct spi_device* spi, struct net_device* netdev);
void lan1qtsn_spi_exit(struct lan1qtsn_spi* tc6);
int lan1qtsn_spi_write_register(struct lan1qtsn_spi* tc6, u32 address, u32 value);
int lan1qtsn_spi_write_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length);
int lan1qtsn_spi_read_register(struct lan1qtsn_spi* tc6, u32 address, u32* value);
int lan1qtsn_spi_read_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length);
netdev_tx_t lan1qtsn_spi_start_xmit(struct lan1qtsn_spi* tc6, struct sk_buff* skb);
int lan1qtsn_spi_zero_align_receive_frame_enable(struct lan1qtsn_spi* tc6);
```

```c
// SPDX-License-Identifier: GPL-2.0+
/*
 * OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface framework
 *
 * Author: Parthiban Veerasooran <parthiban.veerasooran@microchip.com>
 */

#include "lan1qtsn_spi.h"

#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>
#include <linux/phy.h>

#include "lan1qtsn_netdev.h"

static int lan1qtsn_spi_transfer(struct lan1qtsn_spi* tc6, enum lan1qtsn_spi_header_type header_type, u16 length) {
    struct spi_transfer xfer = {0};
    struct spi_message msg;

    if (header_type == LAN1QTSN_SPI_DATA_HEADER) {
        xfer.tx_buf = tc6->spi_data_tx_buf;
        xfer.rx_buf = tc6->spi_data_rx_buf;
    } else if (header_type == LAN1QTSN_SPI_META_HEADER) {
        xfer.tx_buf = tc6->spi_meta_tx_buf;
        xfer.rx_buf = tc6->spi_meta_rx_buf;
    } else {
        xfer.tx_buf = tc6->spi_ctrl_tx_buf;
        xfer.rx_buf = tc6->spi_ctrl_rx_buf;
    }
    xfer.len = length;

#if 0
    dev_err(&tc6->spi->dev, "%s - length: %d\n", __func__, length);

    u8* p_buf;

    p_buf = (u8*)xfer.tx_buf;
    dev_err(&tc6->spi->dev, "xfer.tx_buf: %02x %02x %02x %02x %02x %02x %02x %02x\n", p_buf[0], p_buf[1], p_buf[2],
            p_buf[3], p_buf[4], p_buf[5], p_buf[6], p_buf[7]);
    p_buf = (u8*)xfer.rx_buf;
    dev_err(&tc6->spi->dev, "xfer.rx_buf: %02x %02x %02x %02x %02x %02x %02x %02x\n", p_buf[0], p_buf[1], p_buf[2],
            p_buf[3], p_buf[4], p_buf[5], p_buf[6], p_buf[7]);
#endif

    spi_message_init(&msg);
    spi_message_add_tail(&xfer, &msg);

    return spi_sync(tc6->spi, &msg);
}

static u16 lan1qtsn_spi_calculate_ctrl_buf_size(u8 length, enum lan1qtsn_spi_register_op reg_op) {
#if 1
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE) {
        return LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_CTRL_DATA_SIZE * length;
    }
    return LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_CTRL_DATA_SIZE * length + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;
#else
    /* Control command consists 4 bytes header + 4 bytes register value for
     * each register + 4 bytes ignored value.
     */
    return LAN1QTSN_SPI_CTRL_HEADER_SIZE + LAN1QTSN_SPI_CTRL_REG_VALUE_SIZE * length + LAN1QTSN_SPI_CTRL_IGNORED_SIZE;
#endif
}

static void lan1qtsn_spi_update_ctrl_write_data(struct lan1qtsn_spi* tc6, u32 value[], u8 length) {
    __be32* tx_buf = tc6->spi_ctrl_tx_buf + LAN1QTSN_SPI_CMD_SIZE;

    for (int i = 0; i < length; i++) {
        *tx_buf++ = cpu_to_be32(value[i]);
    }
}

static void lan1qtsn_spi_prepare_cmd_header(u8* tx_buf, u8 cmd, u16 address) {
    u16* addr_ptr = (uint16_t*)&tx_buf[1];

    tx_buf[0] = cmd;
    *addr_ptr = htons(address);
}

static void lan1qtsn_spi_prepare_ctrl_spi_buf(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                                              enum lan1qtsn_spi_register_op reg_op) {
#if 1
    u8* tx_buf = tc6->spi_ctrl_tx_buf;
    u8 cmd;
    u16 addr;

    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE) {
        cmd = LAN1QTSN_SPI_CMD_WRITE;
        addr = LAN1QTSN_SPI_REG_WRITE_FIFO_BASE | (address & 0xFFF);
        lan1qtsn_spi_update_ctrl_write_data(tc6, value, length);
    } else {
        cmd = LAN1QTSN_SPI_CMD_READ;
        addr = LAN1QTSN_SPI_REG_READ_FIFO_BASE | (address & 0xFFF);
    }

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

#if 0
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE) {
        lan1qtsn_spi_update_ctrl_write_data(tc6, value, length);
    }
#endif

#else
    __be32* tx_buf = tc6->spi_ctrl_tx_buf;

    *tx_buf = lan1qtsn_spi_prepare_ctrl_header(address, length, reg_op);

    if (reg_op == LAN1QTSN_SPI_CTRL_REG_WRITE)
        lan1qtsn_spi_update_ctrl_write_data(tc6, value, length);
#endif
}

static void lan1qtsn_spi_copy_ctrl_read_data(struct lan1qtsn_spi* tc6, u32 value[], u8 length) {
    __be32* rx_buf = tc6->spi_ctrl_rx_buf + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + LAN1QTSN_SPI_CMD_SIZE;

    for (int i = 0; i < length; i++) {
        value[i] = be32_to_cpu(*rx_buf++);
#if 0
        dev_err(&tc6->spi->dev, "%s - value[%d]: 0x%08x\n", __func__, i, value[i]);
#endif
    }
}

static int lan1qtsn_spi_perform_ctrl(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length,
                                     enum lan1qtsn_spi_register_op reg_op) {
    u16 size;
    int ret;

    /* Prepare control command and copy to SPI control buffer */
    lan1qtsn_spi_prepare_ctrl_spi_buf(tc6, address, value, length, reg_op);

    size = lan1qtsn_spi_calculate_ctrl_buf_size(length, reg_op);

    /* Perform SPI transfer */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_CTRL_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    /* Check echoed/received control write command reply for errors */
    if (reg_op == LAN1QTSN_SPI_CTRL_REG_READ) {
        lan1qtsn_spi_copy_ctrl_read_data(tc6, value, length);
    }

    /* Check echoed/received control read command reply for errors */
#if 0
    ret = lan1qtsn_spi_check_ctrl_read_reply(tc6, size);
    if (ret) {
        return ret;
    }

    lan1qtsn_spi_copy_ctrl_read_data(tc6, value, length);
#endif

    return 0;
}

/**
 * lan1qtsn_spi_read_registers - function for reading multiple consecutive registers.
 * @tc6: lan1qtsn_spi struct.
 * @address: address of the first register to be read in the MAC-PHY.
 * @value: values to be read from the starting register address @address.
 * @length: number of consecutive registers to be read from @address.
 *
 * Maximum of 128 consecutive registers can be read starting at @address.
 *
 * Return: 0 on success otherwise failed.
 */
int lan1qtsn_spi_read_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length) {
    int ret;

    if (!length || length > LAN1QTSN_SPI_CTRL_MAX_REGISTERS) {
        dev_err(&tc6->spi->dev, "Invalid register length parameter\n");
        return -EINVAL;
    }

    mutex_lock(&tc6->spi_ctrl_lock);
    ret = lan1qtsn_spi_perform_ctrl(tc6, address, value, length, LAN1QTSN_SPI_CTRL_REG_READ);
    mutex_unlock(&tc6->spi_ctrl_lock);

    return ret;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_read_registers);

/**
 * lan1qtsn_spi_read_register - function for reading a MAC-PHY register.
 * @tc6: lan1qtsn_spi struct.
 * @address: register address of the MAC-PHY to be read.
 * @value: value read from the @address register address of the MAC-PHY.
 *
 * Return: 0 on success otherwise failed.
 */
int lan1qtsn_spi_read_register(struct lan1qtsn_spi* tc6, u32 address, u32* value) {
    return lan1qtsn_spi_read_registers(tc6, address, value, 1);
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_read_register);

/**
 * lan1qtsn_spi_write_registers - function for writing multiple consecutive registers.
 * @tc6: lan1qtsn_spi struct.
 * @address: address of the first register to be written in the MAC-PHY.
 * @value: values to be written from the starting register address @address.
 * @length: number of consecutive registers to be written from @address.
 *
 * Maximum of 128 consecutive registers can be written starting at @address.
 *
 * Return: 0 on success otherwise failed.
 */
int lan1qtsn_spi_write_registers(struct lan1qtsn_spi* tc6, u32 address, u32 value[], u8 length) {
    int ret;

    if (!length || length > LAN1QTSN_SPI_CTRL_MAX_REGISTERS) {
        dev_err(&tc6->spi->dev, "Invalid register length parameter\n");
        return -EINVAL;
    }

    mutex_lock(&tc6->spi_ctrl_lock);
    ret = lan1qtsn_spi_perform_ctrl(tc6, address, value, length, LAN1QTSN_SPI_CTRL_REG_WRITE);
    mutex_unlock(&tc6->spi_ctrl_lock);

    return ret;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_write_registers);

/**
 * lan1qtsn_spi_write_register - function for writing a MAC-PHY register.
 * @tc6: lan1qtsn_spi struct.
 * @address: register address of the MAC-PHY to be written.
 * @value: value to be written in the @address register address of the MAC-PHY.
 *
 * Return: 0 on success otherwise failed.
 */
int lan1qtsn_spi_write_register(struct lan1qtsn_spi* tc6, u32 address, u32 value) {
    return lan1qtsn_spi_write_registers(tc6, address, &value, 1);
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_write_register);

static int lan1qtsn_spi_enable_data_transfer(struct lan1qtsn_spi* tc6) {
    lan1qtsn_spi_write_register(tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE);
    return 0;
}

static void lan1qtsn_spi_cleanup_ongoing_rx_skb(struct lan1qtsn_spi* tc6) {
    if (tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        kfree_skb(tc6->rx_skb);
        tc6->rx_skb = NULL;
    }
}

static void lan1qtsn_spi_update_rx_skb(struct lan1qtsn_spi* tc6, u8* payload, u8 length) {
    memcpy(skb_put(tc6->rx_skb, length), payload, length);
}

static int lan1qtsn_spi_allocate_rx_skb(struct lan1qtsn_spi* tc6) {
    tc6->rx_skb = netdev_alloc_skb_ip_align(tc6->netdev, tc6->netdev->mtu + ETH_HLEN + ETH_FCS_LEN);
    if (!tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        return -ENOMEM;
    }

    return 0;
}

static void lan1qtsn_spi_submit_rx_skb(struct lan1qtsn_spi* tc6) {
    tc6->rx_skb->protocol = eth_type_trans(tc6->rx_skb, tc6->netdev);
    tc6->netdev->stats.rx_packets++;
    tc6->netdev->stats.rx_bytes += tc6->rx_skb->len;

    netif_rx(tc6->rx_skb);

    tc6->rx_skb = NULL;
}

static int lan1qtsn_spi_prcs_complete_rx_frame(struct lan1qtsn_spi* tc6, u8* payload, u16 size,
                                               struct rx_metadata meta) {
    int ret;

    ret = lan1qtsn_spi_allocate_rx_skb(tc6);
    if (ret) {
        return ret;
    }

    lan1qtsn_spi_update_rx_skb(tc6, payload, size);

    lan1qtsn_spi_submit_rx_skb(tc6);

    return 0;
}

static int lan1qtsn_spi_process_spi_data_rx_buf(struct lan1qtsn_spi* tc6, struct rx_metadata meta) {
    int ret;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u8 cmd;
    u16 addr;
    u16 size;
    u8* payload = tc6->spi_data_rx_buf + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    cmd = LAN1QTSN_SPI_CMD_READ;
    addr = LAN1QTSN_SPI_RX_FRAME_FIFO_BASE;

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    size = meta.frame_length + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    /* Perform SPI transfer */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    return lan1qtsn_spi_prcs_complete_rx_frame(tc6, payload, size, meta);
}

static u16 lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs(struct lan1qtsn_spi* tc6) {
    u8* tx_buf = tc6->spi_data_tx_buf;
    struct sk_buff* skb;
    u16 copy_len, total_len;
    u32 fcs;
    u8* eth_data;
    u16 eth_len;

    // 1. Move the waiting skb to ongoing transmission slot
    spin_lock_bh(&tc6->tx_skb_lock);
    skb = tc6->waiting_tx_skb;
    tc6->ongoing_tx_skb = skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    // 2. Check if there is skb to transmit
    if (!skb) {
        return 0;
    }

    copy_len = skb->len;

    // 3. Copy the original frame data to the SPI buffer, after command header
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE], skb->data, copy_len);

    // 4. Calculate Ethernet frame data start address and length (excluding metadata)
    eth_data = skb->data + TX_METADATA_SIZE;
    eth_len = copy_len - TX_METADATA_SIZE;

    // 5. Calculate FCS (CRC32) over the pure Ethernet frame
    fcs = crc32_le(~0, eth_data, eth_len);
    fcs = ~fcs; // Final XOR

    // 6. Append FCS to SPI transmit buffer after the frame data
    memcpy(&tx_buf[LAN1QTSN_SPI_CMD_SIZE + copy_len], &fcs, ETH_FCS_LEN);

    // 7. Calculate total length (# bytes to transmit)
    total_len = LAN1QTSN_SPI_CMD_SIZE + copy_len + ETH_FCS_LEN;

    // 8. Check buffer overflow, clean up and exit on error
    if (total_len > LAN1QTSN_BUFFER_SIZE) {
        kfree_skb(skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    // 9. Prepare the SPI command header
    lan1qtsn_spi_prepare_cmd_header(tx_buf, LAN1QTSN_SPI_CMD_WRITE, LAN1QTSN_SPI_TX_FRAME_FIFO_BASE);

    return total_len;
}

static int lan1qtsn_spi_get_meta_data(struct lan1qtsn_spi* tc6, struct rx_metadata* meta) {
    int ret;
    u8* tx_buf = tc6->spi_meta_tx_buf;
    u8* rx_buf = (u8*)tc6->spi_meta_rx_buf;
    struct rx_metadata* rx_meta;
    u32* src_ts;
    u32* dst_ts;
    u16 frame_length;
    u8 cmd = LAN1QTSN_SPI_CMD_READ;
    u16 addr = LAN1QTSN_SPI_METADATA_FIFO_BASE;
    u16 transfer_size = LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE + LAN1QTSN_SPI_META_DATA_SIZE;

    // 1. Prepare SPI MPW header for metadata read function
    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    // 2. Transfer metadata block from device using SPI
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_META_HEADER, transfer_size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    // 3. rx_meta points to extracted metadata in receive buffer
    rx_meta = (struct rx_metadata*)&rx_buf[LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE];

    // 4. Convert and assign frame_length to meta structure
    frame_length = ntohs(rx_meta->frame_length);
    meta->frame_length = frame_length;

    // 5. If a valid frame is present, convert and assign timestamps
    if (frame_length) {
        src_ts = (u32*)rx_meta->timestamp;
        dst_ts = (u32*)meta->timestamp;

        *dst_ts = ntohl(*src_ts); // Lower 32 bits
        dst_ts++;
        src_ts++;
        *dst_ts = ntohl(*src_ts); // Upper 32 bits
    }

    return 0;
}

static int lan1qtsn_spi_try_spi_transfer(struct lan1qtsn_spi* tc6) {
    int ret;
    struct rx_metadata meta;

    while (true) {
        u16 spi_len = 0;

        /* Transmit */
        if (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) {
            netdev_err(tc6->netdev, "Processing TX packet, len=%d\n", tc6->waiting_tx_skb->len);

            spi_len = lan1qtsn_spi_prepare_spi_tx_buf_for_tx_skbs(tc6);
            netdev_err(tc6->netdev, "Prepared TX buffer, spi_len=%d\n", spi_len);

            if (spi_len > 0) {
                ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, spi_len);
                netdev_err(tc6->netdev, "SPI transfer result=%d\n", ret);

                if (ret) {
                    netdev_err(tc6->netdev, "SPI data transfer failed: %d\n", ret);
                    /* Clean up on error */
                    if (tc6->ongoing_tx_skb) {
                        kfree_skb(tc6->ongoing_tx_skb);
                        tc6->ongoing_tx_skb = NULL;
                    }
                } else {
                    /* Update stats and free skb after successful transfer */
                    tc6->netdev->stats.tx_bytes += tc6->ongoing_tx_skb->len + ETH_FCS_LEN;
                    tc6->netdev->stats.tx_packets++;
                    kfree_skb(tc6->ongoing_tx_skb);
                    tc6->ongoing_tx_skb = NULL;
                }
            }
        }

        ret = lan1qtsn_spi_get_meta_data(tc6, &meta);
        if (ret || (meta.frame_length == 0)) {
            break;
        }

        ret = lan1qtsn_spi_process_spi_data_rx_buf(tc6, meta);
        if (ret) {
            lan1qtsn_spi_cleanup_ongoing_rx_skb(tc6);
            netdev_err(tc6->netdev, "Device error: %d\n", ret);
            return ret;
        }
    }

    return 0;
}

static int lan1qtsn_spi_thread_handler(void* data) {
    struct lan1qtsn_spi* tc6 = data;
    int ret;

    while (likely(!kthread_should_stop())) {
        /* This kthread will be waken up if there is a tx skb or mac-phy
         * interrupt to perform spi transfer with tx chunks.
         */
        wait_event_interruptible(
            tc6->spi_wq, tc6->int_flag || (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) ||
                             kthread_should_stop());

        if (kthread_should_stop()) {
            break;
        }

        ret = lan1qtsn_spi_try_spi_transfer(tc6);
        if (ret) {
            netdev_err(tc6->netdev, "<<< %s - ret: %d\n", __func__, ret);
            return ret;
        }
    }

    netdev_err(tc6->netdev, "<<< %s\n", __func__);
    return 0;
}

static irqreturn_t lan1qtsn_spi_macphy_isr(int irq, void* data) {
    struct lan1qtsn_spi* tc6 = data;

    /* MAC-PHY interrupt can occur for the following reasons.
     * - availability of tx credits if it was 0 before and not reported in
     *   the previous rx footer.
     * - availability of rx chunks if it was 0 before and not reported in
     *   the previous rx footer.
     * - extended status event not reported in the previous rx footer.
     */
    tc6->int_flag = true;
    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);

    return IRQ_HANDLED;
}

int lan1qtsn_append_metadata_to_skb(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {
    struct net_device* ndev = tc6->netdev;
    u16 frame_length;
    struct tx_buffer* tx_buffer;
    struct tx_metadata* tx_metadata;
    u16 q;

    q = skb_get_queue_mapping(skb);
    netif_stop_subqueue(ndev, q);
    pr_err("%s(skb->len : %d)\n", __func__, skb->len);
    skb->len = max((unsigned int)ETH_ZLEN, skb->len);

    frame_length = skb->len + ETH_FCS_LEN; /* Include FCS length */

    if (skb_padto(skb, skb->len)) {
        pr_err("skb_padto failed\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return -1;
    }

    /* Jumbo frames not supported (including FCS) */
    if (skb->len + ETH_FCS_LEN > LAN1QTSN_BUFFER_SIZE) {
        pr_err("Jumbo frames not supported\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return -1;
    }

    /* Add metadata to the skb */
    if (pskb_expand_head(skb, TX_METADATA_SIZE, 0, GFP_ATOMIC) != 0) {
        pr_err("pskb_expand_head failed\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return -1;
    }
    skb_push(skb, TX_METADATA_SIZE);
    memset(skb->data, 0, TX_METADATA_SIZE);

    pr_err("%s(skb->len : %d)\n", __func__, skb->len);
    tx_buffer = (struct tx_buffer*)skb->data;
    /* Fill in the metadata */
    tx_metadata = (struct tx_metadata*)&tx_buffer->metadata;
    tx_metadata->frame_length = frame_length;

    tx_metadata->from.tick = 0x0;
    tx_metadata->from.priority = 0;
    tx_metadata->to.tick = 0x1FFFFFFF;
    tx_metadata->to.priority = 0;
    tx_metadata->delay_from.tick = 0x0;
    tx_metadata->delay_from.priority = 0;
    tx_metadata->delay_to.tick = 0x1FFFFFFF;
    tx_metadata->delay_to.priority = 0;

    return 0;
}

/**
 * lan1qtsn_spi_start_xmit - function for sending the tx skb which consists ethernet
 * frame.
 * @tc6: lan1qtsn_spi struct.
 * @skb: socket buffer in which the ethernet frame is stored.
 *
 * Return: NETDEV_TX_OK if the transmit ethernet frame skb added in the tx_skb_q
 * otherwise returns NETDEV_TX_BUSY.
 */
netdev_tx_t lan1qtsn_spi_start_xmit(struct lan1qtsn_spi* tc6, struct sk_buff* skb) {
    int ret;

    ret = lan1qtsn_append_metadata_to_skb(tc6, skb);
    if (ret) {
        return NETDEV_TX_OK;
    }

    if (tc6->waiting_tx_skb && tc6->waiting_tx_skb != NULL) {
        netif_stop_queue(tc6->netdev);
        return NETDEV_TX_BUSY;
    }

    if (skb_linearize(skb)) {
        dev_kfree_skb_any(skb);
        tc6->netdev->stats.tx_dropped++;
        return NETDEV_TX_OK;
    }

    /* Check if skb is owned by socket - if so, we can't reuse it */
    if (skb->sk) {
        /* Create a copy for our use */
        struct sk_buff* new_skb = skb_copy(skb, GFP_ATOMIC);
        if (!new_skb) {
            dev_kfree_skb_any(skb);
            tc6->netdev->stats.tx_dropped++;
            return NETDEV_TX_OK;
        }
        dev_kfree_skb_any(skb);
        skb = new_skb;
    }

    spin_lock_bh(&tc6->tx_skb_lock);
    tc6->waiting_tx_skb = skb;
    spin_unlock_bh(&tc6->tx_skb_lock);

    /* Wake spi kthread to perform spi transfer */
    wake_up_interruptible(&tc6->spi_wq);

    return NETDEV_TX_OK;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_start_xmit);

u8 indirect_read(struct lan1qtsn_spi* tc6, u8 addr, u8 mask) {
    u32 regval;

    lan1qtsn_spi_write_register(tc6, LAN1QTSN_REG_MMS4_INDIR_RD_ADDR, addr);
    lan1qtsn_spi_write_register(tc6, LAN1QTSN_REG_MMS4_INDIR_RD_WIDTH, LAN1QTSN_MMS04_INDIR_WIDTH);

    lan1qtsn_spi_read_register(tc6, LAN1QTSN_REG_MMS4_INDIR_RD_VAL, &regval);

    return (regval & mask);
}

int init_lan1qtsn(struct lan1qtsn_spi* tc6) {
#if 1
    lan1qtsn_spi_write_register(tc6, TSN_SYSTEM_CONTROL_L, TSN_ENABLE);
    return 0;
#else
    u32 regval;
    int ret;

    /* Read OA_STATUS0 */
    ret = lan1qtsn_spi_read_register(tc6, LAN1QTSN_SPI_REG_STATUS0, &regval);
    if (ret)
        return ret;

    /* Write 1 to RESETC bit of OA_STATUS0 */
    regval |= STATUS0_RESETC;
    ret = lan1qtsn_spi_write_register(tc6, LAN1QTSN_SPI_REG_STATUS0, regval);
    if (ret)
        return ret;

    /* Read Collision Detector Control 0 Register */
    ret = lan1qtsn_spi_read_register(tc6, LAN1QTSN_REG_MMS4_CDCTL0, &regval);
    if (ret)
        return ret;

    /* Collision detection is enabled (default) */
    regval |= MMS4_CDCTL0_CDEN_SHIFT;
    ret = lan1qtsn_spi_write_register(tc6, LAN1QTSN_REG_MMS4_CDCTL0, regval);
    if (ret)
        return ret;

    set_macphy_register(tc6);

    lan1qtsn_spi_write_register(tc6, LAN1QTSN_REG_MMS4_PLCA_CTRL0, MMS4_PLCA_CTRL0_INIT_VAL); /* Enable PLCA */
    lan1qtsn_spi_write_register(tc6, LAN1QTSN_REG_MMS1_MAC_NCFGR,
                                MMS1_MAC_NCFGR_INIT_VAL);                               /* Enable unicast, multicast */
    lan1qtsn_spi_write_register(tc6, LAN1QTSN_REG_MMS1_MAC_NCR, MMS1_MAC_NCR_INIT_VAL); /* Enable MACPHY TX, RX */

#ifdef FRAME_TIMESTAMP_ENABLE
    lan1qtsn_spi_write_register(tc6, LAN1QTSN_REG_MMS1_MAC_TI, TIMER_INCREMENT);        /* Enable MACPHY TX, RX */
#endif

    /* Read OA_CONFIG0 */
    lan1qtsn_spi_read_register(tc6, LAN1QTSN_REG_MMS0_OA_CONFIG0, &regval);

    /* Set SYNC bit of OA_CONFIG0 */
    regval |= MMS0_OA_CONFIG0_SYNC_SHIFT;
#ifdef FRAME_TIMESTAMP_ENABLE
    /* Set FTSE Frame Timestamp Enable bit of OA_CONFIG0 */
    regval |= MMS0_OA_CONFIG0_FTSE_SHIFT;

    /* Set FTSS Frame Timestamp Select bit of OA_CONFIG0 */
    regval |= MMS0_OA_CONFIG0_FTSS_SHIFT;
#endif
    lan1qtsn_spi_write_register(tc6, LAN1QTSN_REG_MMS0_OA_CONFIG0, regval);

    /* Read OA_STATUS0 */
    lan1qtsn_spi_read_register(tc6, LAN1QTSN_REG_MMS0_OA_STATUS0, &regval);

    /* Clear RESETC bit of OA_STATUS0 */
    regval &= ~(MMS0_OA_STATUS0_RESETC_SHIFT);
    lan1qtsn_spi_write_register(tc6, LAN1QTSN_REG_MMS0_OA_STATUS0, regval);

    return 0;
#endif
}

/**
 * lan1qtsn_spi_init - allocates and initializes lan1qtsn_spi structure.
 * @spi: device with which data will be exchanged.
 * @netdev: network device interface structure.
 *
 * Return: pointer reference to the lan1qtsn_spi structure if the MAC-PHY
 * initialization is successful otherwise NULL.
 */
struct lan1qtsn_spi* lan1qtsn_spi_init(struct spi_device* spi, struct net_device* netdev) {
    struct lan1qtsn_spi* tc6;
    int ret;

    tc6 = devm_kzalloc(&spi->dev, sizeof(*tc6), GFP_KERNEL);
    if (!tc6) {
        return NULL;
    }

    tc6->spi = spi;
    tc6->netdev = netdev;
    SET_NETDEV_DEV(netdev, &spi->dev);
    mutex_init(&tc6->spi_ctrl_lock);
    spin_lock_init(&tc6->tx_skb_lock);

    /* Set the SPI controller to pump at realtime priority */
    tc6->spi->rt = true;
    spi_setup(tc6->spi);

    tc6->spi_ctrl_tx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_CTRL_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_ctrl_tx_buf) {
        return NULL;
    }

    tc6->spi_ctrl_rx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_CTRL_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_ctrl_rx_buf) {
        return NULL;
    }

    tc6->spi_data_tx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_data_tx_buf) {
        return NULL;
    }

    tc6->spi_data_rx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_data_rx_buf) {
        return NULL;
    }

    tc6->spi_meta_rx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_META_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_meta_rx_buf) {
        return NULL;
    }

    tc6->spi_meta_tx_buf = devm_kzalloc(&tc6->spi->dev, LAN1QTSN_SPI_META_DATA_BUF_SIZE, GFP_KERNEL);
    if (!tc6->spi_meta_tx_buf) {
        return NULL;
    }

    ret = init_lan1qtsn(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to init_lan1qtsn: %d\n", ret);
        goto phy_exit;
    }

    ret = lan1qtsn_spi_enable_data_transfer(tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to enable data transfer: %d\n", ret);
        goto phy_exit;
    }

    init_waitqueue_head(&tc6->spi_wq);

    tc6->spi_thread = kthread_run(lan1qtsn_spi_thread_handler, tc6, "lan1qtsn-spi-thread");
    if (IS_ERR(tc6->spi_thread)) {
        dev_err(&tc6->spi->dev, "Failed to create SPI thread\n");
        goto phy_exit;
    }

    sched_set_fifo(tc6->spi_thread);

    ret = devm_request_irq(&tc6->spi->dev, tc6->spi->irq, lan1qtsn_spi_macphy_isr, IRQF_TRIGGER_FALLING,
                           dev_name(&tc6->spi->dev), tc6);
    if (ret) {
        dev_err(&tc6->spi->dev, "Failed to request macphy isr %d\n", ret);
        goto kthread_stop;
    }

    /* lan1qtsn_spi_sw_reset_macphy() function resets and clears the MAC-PHY reset
     * complete status. IRQ is also asserted on reset completion and it is
     * remain asserted until MAC-PHY receives a data chunk. So performing an
     * empty data chunk transmission will deassert the IRQ. Refer section
     * 7.7 and 9.2.8.8 in the OPEN Alliance specification for more details.
     */
    tc6->int_flag = true;
    wake_up_interruptible(&tc6->spi_wq);

    dev_err(&tc6->spi->dev, "<<< %s - tc6: %p\n", __func__, tc6);
    return tc6;

kthread_stop:
    kthread_stop(tc6->spi_thread);
phy_exit:
    return NULL;
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_init);

/**
 * lan1qtsn_spi_exit - exit function.
 * @tc6: lan1qtsn_spi struct.
 */
void lan1qtsn_spi_exit(struct lan1qtsn_spi* tc6) {
    kthread_stop(tc6->spi_thread);
    dev_kfree_skb_any(tc6->ongoing_tx_skb);
    dev_kfree_skb_any(tc6->waiting_tx_skb);
    dev_kfree_skb_any(tc6->rx_skb);
}
EXPORT_SYMBOL_GPL(lan1qtsn_spi_exit);

MODULE_DESCRIPTION("OPEN Alliance 10BASE‑T1x MAC‑PHY Serial Interface Lib");
MODULE_AUTHOR("Parthiban Veerasooran <parthiban.veerasooran@microchip.com>");
MODULE_LICENSE("GPL");
```

이제 multi-buffer 구조에서 single buffer 구조로 성공적으로 변경했습니다. 주요 변경사항은 다음과 같습니다:

## 제거된 multi-buffer 관련 코드:

1. **Buffer Pool 구조체들**:
   - `tx_buffer_pool`, `rx_buffer_pool`, `lan1qtsn_spi_buffers` 구조체 제거
   - `TX_BUFFER_POOL_SIZE`, `RX_BUFFER_POOL_SIZE` 등 매크로 제거

2. **Buffer Pool 관리 함수들**:
   - `lan1qtsn_spi_init_buffer_pools()` 함수 제거
   - `lan1qtsn_spi_free_rx_buffer()`, `lan1qtsn_spi_free_tx_buffer()` 함수 제거
   - `lan1qtsn_spi_alloc_rx_buffer()` 함수 제거

3. **Multi-buffer 관련 변수들**:
   - `tc6->buffers` 구조체 제거
   - `tc6->tx_credits`, `tc6->rx_chunks_available` 등 제거

## Single buffer 구조로 변경된 부분:

1. **단순한 SKB 관리**:
   - `tc6->ongoing_tx_skb`, `tc6->waiting_tx_skb`, `tc6->rx_skb` 직접 사용
   - Buffer pool 없이 직접 `netdev_alloc_skb_ip_align()` 사용

2. **단순화된 메모리 관리**:
   - Buffer pool 재사용 로직 제거
   - 단순한 `kfree_skb()` 사용

3. **단순화된 전송 로직**:
   - Multi-buffer 복잡한 로직 제거
   - 단일 패킷 처리로 단순화

이제 코드가 훨씬 단순해졌고, multi-buffer의 복잡성 없이 single buffer 구조로 동작합니다. 이 변경으로 인해:

- **메모리 사용량 감소**: Buffer pool이 제거되어 메모리 사용량이 줄어듭니다
- **코드 복잡성 감소**: Multi-buffer 관리 로직이 제거되어 코드가 단순해집니다
- **디버깅 용이성**: 단순한 구조로 인해 문제 발생 시 디버깅이 쉬워집니다

이제 라즈베리 파이 4에서 이 드라이버를 사용할 때 multi-buffer의 오버헤드 없이 더 효율적으로 동작할 것입니다.


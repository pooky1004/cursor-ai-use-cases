# 코드 리팩토링 요청
_Exported on 7/29/2025 at 11:47:12 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후 사용할 예정이야.

코드들이 좀 지저분한데 refactor 해 줘.

필요에 따라 디렉토리, 파일, 함수, 상수, macro 등을 생성/삭제를 해도 괜찮아. 

---

**Cursor**

라즈베리 파이 4용 커널 드라이버 코드를 리팩토링해드리겠습니다. 먼저 현재 코드 구조를 파악하고 개선점을 찾아보겠습니다.


이제 코드를 분석해보니 여러 문제점들이 보입니다. 리팩토링을 진행하겠습니다:

## 주요 문제점들:

1. **코드 중복**: `one_q_tsn_arch.c`에서 `_by_spi` 함수들이 중복됨
2. **매직 넘버**: 하드코딩된 상수들이 많음
3. **긴 함수들**: `tsnlab_spi.c`의 함수들이 너무 길고 복잡함
4. **일관성 없는 네이밍**: 함수명과 변수명이 일관성이 없음
5. **구조체 정의 중복**: 여러 파일에 비슷한 구조체들이 정의됨
6. **에러 처리 부족**: 많은 함수에서 에러 처리가 미흡함

리팩토링을 시작하겠습니다:


```cpp
#ifndef LAN1QTSN_REGISTERS_H
#define LAN1QTSN_REGISTERS_H

/* MAC Network Control Register */
#define LAN865X_REG_MAC_NET_CTL 0x00010000
#define MAC_NET_CTL_TXEN BIT(3) /* Transmit Enable */
#define MAC_NET_CTL_RXEN BIT(2) /* Receive Enable */

/* MAC Network Configuration Reg */
#define LAN865X_REG_MAC_NET_CFG 0x00010001
#define MAC_NET_CFG_PROMISCUOUS_MODE BIT(4)
#define MAC_NET_CFG_MULTICAST_MODE BIT(6)
#define MAC_NET_CFG_UNICAST_MODE BIT(7)

/* MAC Hash Register Bottom */
#define LAN865X_REG_MAC_L_HASH 0x00010020
/* MAC Hash Register Top */
#define LAN865X_REG_MAC_H_HASH 0x00010021
/* MAC Specific Addr 1 Bottom Reg */
#define LAN865X_REG_MAC_L_SADDR1 0x00010022
/* MAC Specific Addr 1 Top Reg */
#define LAN865X_REG_MAC_H_SADDR1 0x00010023

/* Register Class: General */
#define TSN_SYSTEM_INFO_H 0x0278 /* Year [63:56] : Month [55:48] : Day [47:40] : Commit # [39:32] */
#define TSN_SYSTEM_INFO_L 0x027C /* RESERVED [31:0] */

#define TSN_MAC_ADDR_H 0x0280 /* RESERVED [63:48] : MAC Address [47:32] */
#define TSN_MAC_ADDR_L 0x0284 /* MAC Address [31:0] */

#define TSN_SYSCOUNT_H 0x0288 /* SYSTEM_COUNT [63:32] */
#define TSN_SYSCOUNT_L 0x028C /* SYSTEM_COUNT [31:0] */

#define TSN_SYSTEM_CONTROL_H 0x0290 /* RESERVED [63:32] */
#define TSN_SYSTEM_CONTROL_L 0x0294 /* RESERVED [31:1] : TSN ENABLE */

#define TSN_RX_CONTROL_H 0x0298 /* MAX_FRAME_LENGTH [63:48] : */
/* RESERVED [47:45] : */
/* MAX_FRAME_FIFO_DATA_COUNT [44:32] */
#define TSN_RX_CONTROL_L 0x029C /* RESERVED [31:25] : */
/* MAX_META_FIFO_DATA_COUNT [24:16] : */
/* RESERVED [15:8] : CLEAR DROP BYTE COUNT : */
/* CLEAR DROP FRAME COUNT : */
/* CLEAR RX BYTE COUNT : */
/* CLEAR RX FRAME COUNT : */
/* CHECK-SUM ENABLE : */
/* JUMBO DROP :  FCS OFFLOAD : */
/* VLAN OFFLOAD */
#define TSN_TX_CONTROL_H 0x02A0 /* RESERVED [63:32] */
#define TSN_TX_CONTROL_L 0x02A4 /* ETHERNET_INTER_FRAME_GAP [31:16] : */
/* RESERVED [15:10] : */
/* HOST_FIFO_FULL_LIMIT [9:0] */
#define PPS_PULSE_AT_H 0x02A8 /* PULSE_AT [63:32] */
#define PPS_PULSE_AT_L 0x02AC /* PULSE_AT [31:0] */
#define PPS_CYCLE_1S_H 0x02B0 /* CYCLE_1S [63:32] */
#define PPS_CYCLE_1S_L 0x02B4 /* CYCLE_1S [31:0] */

/* Register Class: Rx (Frame Decoder) */
#define FD_RX_TSTAMP_H 0x0000        /* RX_TSTAMP [63:32] */
#define FD_RX_TSTAMP_L 0x0004        /* RX_TSTAMP [31:0] */
#define FD_RX_FRAME_STATUS1_H 0x0008 /* CHECK_SUM [63:32] */
#define FD_RX_FRAME_STATUS1_L 0x000C /* VLAN_TAG [31:16] : FRAME_LENGTH [15:0]  */
#define FD_RX_FRAME_STATUS2_H 0x0010 /* RESERVED [63:32] */
#define FD_RX_FRAME_STATUS2_L 0x0014 /* RESERVED [31:5] : VLAN OFFLOAD FLAG : */
/* FCS OFFLOAD FLAG : FCS ERROR FLAG : */
/* JUMBO ERROR FLAG : FIFO FULL FLAG */
#define FD_TOTAL_RX_FRAME_CNT_H 0x0018       /* TOTAL_RX_FRAME_CNT [63:32] */
#define FD_TOTAL_RX_FRAME_CNT_L 0x001C       /* TOTAL_RX_FRAME_CNT [31:0] */
#define FD_TOTAL_RX_BYTE_CNT_H 0x0020        /* TOTAL_RX_BYTE_CNT [63:32] */
#define FD_TOTAL_RX_BYTE_CNT_L 0x0024        /* TOTAL_RX_BYTE_CNT [31:0] */
#define FD_TOTAL_DROP_FRAME_CNT_H 0x0028     /* TOTAL_DROP_FRAME_CNT [63:32] */
#define FD_TOTAL_DROP_FRAME_CNT_L 0x002C     /* TOTAL_DROP_FRAME_CNT [31:0] */
#define FD_TOTAL_DROP_BYTE_CNT_H 0x0030      /* TOTAL_DROP_BYTE_CNT [63:32] */
#define FD_TOTAL_DROP_BYTE_CNT_L 0x0034      /* TOTAL_DROP_BYTE_CNT [31:0] */
#define FD_FCS_DROP_FRAME_CNT_H 0x0038       /* FCS_DROP_FRAME_CNT [63:32] */
#define FD_FCS_DROP_FRAME_CNT_L 0x003C       /* FCS_DROP_FRAME_CNT [31:0] */
#define FD_FCS_DROP_BYTE_CNT_H 0x0040        /* FCS_DROP_BYTE_CNT [63:32] */
#define FD_FCS_DROP_BYTE_CNT_L 0x0044        /* FCS_DROP_BYTE_CNT [31:0] */
#define FD_JUMBO_DROP_FRAME_CNT_H 0x0048     /* JUMBO_DROP_FRAME_CNT [63:32] */
#define FD_JUMBO_DROP_FRAME_CNT_L 0x004C     /* JUMBO_DROP_FRAME_CNT [31:0] */
#define FD_JUMBO_DROP_BYTE_CNT_H 0x0050      /* JUMBO_DROP_BYTE_CNT [63:32] */
#define FD_JUMBO_DROP_BYTE_CNT_L 0x0054      /* JUMBO_DROP_BYTE_CNT [31:0] */
#define FD_FIFO_FULL_DROP_FRAME_CNT_H 0x0058 /* FIFO_FULL_DROP_FRAME_CNT [63:32] */
#define FD_FIFO_FULL_DROP_FRAME_CNT_L 0x005C /* FIFO_FULL_DROP_FRAME_CNT [31:0] */
#define FD_FIFO_FULL_DROP_BYTE_CNT_H 0x0060  /* FIFO_FULL_DROP_BYTE_CNT [63:32] */
#define FD_FIFO_FULL_DROP_BYTE_CNT_L 0x0064  /* FIFO_FULL_DROP_BYTE_CNT [31:0] */

/* Register Class: FIFO (META, FRAME) */
#define TSN_RX_FIFO_STATUS_H 0x00A8 /* RESERVED [63:32] */
#define TSN_RX_FIFO_STATUS_L 0x00AC /* RES [31:29] : */
/* FRAME_FIFO_DATA_CNT [28:16] : */
/* RESERVED [15:9] : */
/* META_FIFO_DATA_CNT [8:0] */

/* Register Class: Tx (Frame Stacker) */
#define FS_GENERAL_STATUS_H 0x00B0 /* RESERVED [63:58] : */
/* HOST_FIFO_RD_CNT [57:48] : */
/* RESERVED [47:42] : */
/* HOST_FIFO_WR_CNT [41:32] */
#define FS_GENERAL_STATUS_L 0x00B4 /* RESERVED [31:4] : XDMA AXIS TREADY : */
/* HOST FIFO USER FULL */
/* HOST FIFO EMPTY : HOST FIFO FULL */
#define FS_TOTAL_RX_FRAME_CNT_H 0x00B8            /* TOTAL_RX_FRAME_CNT [63:32] */
#define FS_TOTAL_RX_FRAME_CNT_L 0x00BC            /* TOTAL_RX_FRAME_CNT [31:0] */
#define FS_TOTAL_RX_16BYTE_CNT_H 0x00C0           /* TOTAL_RX_16BYTE_CNT [63:32] */
#define FS_TOTAL_RX_16BYTE_CNT_L 0x00C4           /* TOTAL_RX_16BYTE_CNT [31:0] */
#define FS_TOTAL_BACK_PRESSURE_EVENT_CNT_H 0x00C8 /* TOTAL_BACK_PRESSURE_EVENT_CNT [63:32] */
#define FS_TOTAL_BACK_PRESSURE_EVENT_CNT_L 0x00CC /* TOTAL_BACK_PRESSURE_EVENT_CNT [31:0] */

/* Register Class: Tx (Frame Parser) */
#define FP_FRAME_TICK_FROM_H 0x00E0       /* TICK_FROM [63:32] */
#define FP_FRAME_TICK_FROM_L 0x00E4       /* TICK_FROM [31:0] */
#define FP_FRAME_TICK_TO_H 0x00E8         /* TICK_TO [63:32] */
#define FP_FRAME_TICK_TO_L 0x00EC         /* TICK_TO [31:0] */
#define FP_FRAME_TICK_DELAY_FROM_H 0x00F0 /* TICK_DELAY_FROM [63:32] */
#define FP_FRAME_TICK_DELAY_FROM_L 0x00F4 /* TICK_DELAY_FROM [31:0] */
#define FP_FRAME_TICK_DELAY_TO_H 0x00F8   /* TICK_DELAY_TO [63:32] */
#define FP_FRAME_TICK_DELAY_TO_L 0x00FC   /* TICK_DELAY_TO [31:0] */
#define FP_FRAME_STATUS1_H 0x0100         /* RESERVED [63:56] : ALLOC_ADDR [55:48] : */
/* RESERVED [47:33] : FAIL_POLICY [32] */
#define FP_FRAME_STATUS1_L 0x0104 /* FRAME_LENGTH [31:16] : TIMESTAMP_ID [15:0] */
#define FP_FRAME_STATUS2_H 0x0108 /* RESERVED [63:52] : */
/* META_AXIS_HSK_CNT [51:49] : */
/* META_ADDR_OFFSET [48] : */
/* RESERVED [47:39] : */
/* FRAME_ADDR_OFFSET [38:32] */
#define FP_FRAME_STATUS2_L 0x010C /* FRAME_AXIS_HSK_CNT [31:16] : */
/* RX_FRAME_16BYTE_CNT [15:0] */
/* Register Class: Tx (Frame Scheduler) */
#define FSCH_TX_FRAME_STATUS_H 0x0118 /* RESERVED [63:48] : */
/* TX_FRAME_TSTAMP_ID [47:32] */
#define FSCH_TX_FRAME_STATUS_L 0x011C /* TX_FRAME_LENGTH [15:0] : */
/* RESERVED [15:8] : */
/* TX_FRAME_ADDRESS [7:0] */
#define FSCH_TOTAL_NEW_ENTRY_CNT_H 0x0120   /* TOTAL_NEW_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_NEW_ENTRY_CNT_L 0x0124   /* TOTAL_NEW_ENTRY_CNT [31:0] */
#define FSCH_TOTAL_VALID_ENTRY_CNT_H 0x0128 /* TOTAL_VALID_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_VALID_ENTRY_CNT_L 0x012C /* TOTAL_VALID_ENTRY_CNT [31:0] */
#define FSCH_TOTAL_DELAY_ENTRY_CNT_H 0x0130 /* TOTAL_DELAY_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_DELAY_ENTRY_CNT_L 0x0134 /* TOTAL_DELAY_ENTRY_CNT [31:0] */
#define FSCH_TOTAL_DROP_ENTRY_CNT_H 0x0138  /* TOTAL_DROP_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_DROP_ENTRY_CNT_L 0x013C  /* TOTAL_DROP_ENTRY_CNT [31:0] */

/* Register Class: Tx (Frame Buffer) */
#define FBW_BUFFER_WRITE_STATUS1_H 0x0148 /* RESERVED [63:40] : */
/* ADDR_FIFO_DATA_CNT [39:32] */
#define FBW_BUFFER_WRITE_STATUS1_L 0x014C /* RESERVED [31:11] : */
/* BRAM_SEL [10:8] : ALLOC_ADDR [7:0] */
#define FBW_BUFFER_WRITE_STATUS2_H 0x0150 /* RESERVED [63:59] : */
/* BRAM3_WR_ADDR_LAST [58:48] : */
/* RESERVED [47:43] : */
/* BRAM2_WR_ADDR_LAST [42:32] */
#define FBW_BUFFER_WRITE_STATUS2_L 0x0154 /* RESERVED [31:27] : */
/* BRAM1_WR_ADDR_LAST [26:16] : */
/* RESERVED [15:11] : */
/* BRAM0_WR_ADDR_LAST [10:0] */
#define FBW_BUFFER_WRITE_STATUS3_H 0x0158 /* RESERVED [63:59] : */
/* BRAM7_WR_ADDR_LAST [58:48] : */
/* RESERVED [47:43] : */
/* BRAM6_WR_ADDR_LAST [42:32] */
#define FBW_BUFFER_WRITE_STATUS3_L 0x015C /* RESERVED [31:27] : */
/* BRAM5_WR_ADDR_LAST [26:16] */
/* RESERVED [15:11] */
/* BRAM4_WR_ADDR_LAST [10:0] */
#define FBW_ADDR_FIFO_CNT_H 0x0160 /* RESERVED [63:32] */
#define FBW_ADDR_FIFO_CNT_L 0x0164 /* RESERVED [31:8] : */
/* ADDR_FIFO_DATA_CNT [7:0] */
#define FBR_BUFFER_READ_STATUS1_H 0x0168 /* RESERVED [63:40] : */
/* DROP_FRAME_ADDR [39:32] */
#define FBR_BUFFER_READ_STATUS1_L 0x016C /* SELECTED_FRAME_16BYTE_CNT [31:16] : */
/* RESERVED [15:11] : */
/* BRAM_SEL [2:0] : */
/* SELECTED_FRAME_ADDR [7:0] */
#define FBR_BUFFER_READ_STATUS2_H 0x0170 /* RESERVED [63:59] : */
/* BRAM3_RD_ADDR_LAST [58:48] : */
/* RESERVED [47:43] : */
/* BRAM2_RD_ADDR_LAST [42:32] */
#define FBR_BUFFER_READ_STATUS2_L 0x0174 /* RESERVED [31:27] : */
/* BRAM1_RD_ADDR_LAST [26:16] : */
/* RESERVED [15:11] : */
/* BRAM0_RD_ADDR_LAST [10:0] */
#define FBR_BUFFER_READ_STATUS3_H 0x0178 /* RESERVED [63:59] : */
/* BRAM7_RD_ADDR_LAST [58:48] : */
/* RESERVED [47:43] : */
/* BRAM6_RD_ADDR_LAST [42:32] */
#define FBR_BUFFER_READ_STATUS3_L 0x017C /* RESERVED [31:27] : */
/* BRAM5_RD_ADDR_LAST [26:16] : */
/* RESERVED [15:11] : */
/* BRAM4_RD_ADDR_LAST [10:0] */

/* Register Class: Tx (Frame Transfer FSM) */
#define FTRSF_TX_FRAME_STATUS_H 0x0188 /* TX_METADATA [63:32] */
#define FTRSF_TX_FRAME_STATUS_L 0x018C /* TX_FRAME_LENGTH [15:0] : */
/* RESERVED [15:8] : */
/* TX_FRAME_ADDRESS [7:0] */
#define FTRSF_GENERAL_STATUS_H 0x0190 /* RESERVED [63:35] : */
/* TX_FRAME_FIFO_FULL [34] : */
/* TX_META_FIFO_FULL [33] : */
/* TX_STATUS [32] */
#define FTRSF_GENERAL_STATUS_L 0x0194 /* RESERVED [31:25] : */
/* TX_FRAME_FIFO_DATA_COUNT [24:16] : */
/* RESERVED [15:9] : */
/* TX_META_FIFO_DATA_COUNT [8:0] */
#define FTRSF_TOTAL_TX_FRAME_CNT_H 0x0198        /* TOTAL_TX_FRAME_CNT [63:32] */
#define FTRSF_TOTAL_TX_FRAME_CNT_L 0x019C        /* TOTAL_TX_FRAME_CNT [31:0] */
#define FTRSF_TOTAL_META_FIFO_FULL_CNT_H 0x01A0  /* TOTAL_META_FIFO_FULL_CNT [63:32] */
#define FTRSF_TOTAL_META_FIFO_FULL_CNT_L 0x01A4  /* TOTAL_META_FIFO_FULL_CNT [31:0] */
#define FTRSF_TOTAL_FRAME_FIFO_FULL_CNT_H 0x01A8 /* TOTAL_FRAME_FIFO_FULL_CNT [63:32] */
#define FTRSF_TOTAL_FRAME_FULL_CNT_L 0x01AC      /* TOTAL_FRAME_FIFO_FULL_CNT [31:0] */

/* Register Class: Tx (Frame Transmitter FSM) */
#define FT_TX_FRAME_STATUS1_H 0x01B8 /* TX_METADATA [63:32] */
#define FT_TX_FRAME_STATUS1_L 0x01BC /* TX_FRAME_16BYTE_LENGTH [31:16] : */
/* TX_FRAME_LENGTH [15:0] */
#define FT_TX_FRAME_STATUS2_H 0x01C0 /* RESERVED [63:48] : */
/* TX_FRAME_TSTAMP_ID [47:32] */
#define FT_TX_FRAME_STATUS2_L 0x01C4 /* TX_16BYTE_CNT [31:16] : */
/* TX_BYTE_CNT [15:0] */
#define FT_TOTAL_TX_FRAME_CNT_H 0x01C8 /* TOTAL_TX_FRAME_CNT [63:32] */
#define FT_TOTAL_TX_FRAME_CNT_L 0x01CC /* TOTAL_TX_FRAME_CNT [31:0] */
#define FT_TOTAL_TX_BYTE_CNT_H 0x01D0  /* TOTAL_TX_BYTE_CNT [63:32] */
#define FT_TOTAL_TX_BYTE_CNT_L 0x01D4  /* TOTAL_TX_BYTE_CNT [31:0] */
#define FT_TX_TSTAMP1_H 0x01D8         /* TX_TIMESTAMP_1 [63:32] */
#define FT_TX_TSTAMP1_L 0x01DC         /* TX_TIMESTAMP_1 [31:0] */
#define FT_TX_TSTAMP2_H 0x01E0         /* TX_TIMESTAMP_2 [63:32] */
#define FT_TX_TSTAMP2_L 0x01E4         /* TX_TIMESTAMP_2 [31:0] */
#define FT_TX_TSTAMP3_H 0x01E8         /* TX_TIMESTAMP_3 [63:32] */
#define FT_TX_TSTAMP3_L 0x01EC         /* TX_TIMESTAMP_3 [31:0] */
#define FT_TX_TSTAMP4_H 0x01F0         /* TX_TIMESTAMP_4 [63:32] */
#define FT_TX_TSTAMP4_L 0x01F4         /* TX_TIMESTAMP_4 [31:0] */

/* SPI FIFO Base Addresses */
#define TSNLAB_SPI_RX_FRAME_FIFO_BASE 0x1000
#define TSNLAB_SPI_TX_FRAME_FIFO_BASE 0x2000
#define TSNLAB_SPI_REG_READ_FIFO_BASE 0x3000
#define TSNLAB_SPI_REG_WRITE_FIFO_BASE 0x4000
#define TSNLAB_SPI_METADATA_FIFO_BASE 0xF000

/* SPI Commands */
#define TSNLAB_SPI_CMD_WRITE 0x01
#define TSNLAB_SPI_CMD_READ 0x02

/* Control Values */
#define TSN_DISABLE 0x0
#define TSN_ENABLE 0x1

#endif /* LAN1QTSN_REGISTERS_H */
```

```cpp
#ifndef LAN1QTSN_CONSTANTS_H
#define LAN1QTSN_CONSTANTS_H

/* Driver Configuration */
#define DRV_NAME "onequeue"
#define LAN1QTSN_DEVICE_NAME "lan1qtsn"

/* SPI Configuration */
#define TSNLAB_SPI_CTRL_HEADER_SIZE 3
#define TSNLAB_SPI_CTRL_REG_VALUE_SIZE 4
#define TSNLAB_SPI_CTRL_IGNORED_SIZE 1
#define TSNLAB_SPI_CTRL_MAX_REGISTERS 2
#define TSNLAB_SPI_CTRL_SPI_BUF_SIZE 16

#define TSNLAB_SPI_CHUNK_PAYLOAD_SIZE 1536
#define TSNLAB_SPI_DATA_HEADER_SIZE 3
#define TSNLAB_SPI_DATA_IGNORED_SIZE 1
#define TSNLAB_SPI_CHUNK_SIZE (TSNLAB_SPI_DATA_HEADER_SIZE + TSNLAB_SPI_CHUNK_PAYLOAD_SIZE)
#define TSNLAB_SPI_MAX_TX_CHUNKS 1
#define TSNLAB_SPI_SPI_DATA_BUF_SIZE 2048

#define TSNLAB_SPI_META_HEADER_SIZE 3
#define TSNLAB_SPI_META_IGNORED_SIZE 1
#define TSNLAB_SPI_RX_META_DATA_BUF_SIZE 16

/* Timing Constants */
#define STATUS0_RESETC_POLL_DELAY 1000
#define STATUS0_RESETC_POLL_TIMEOUT 1000000

/* Queue Configuration */
#define TX_QUEUE_COUNT 8
#define RX_QUEUE_COUNT 8
#define TC_COUNT 8
#define TSN_PRIO_COUNT 8
#define MAX_QBV_SLOTS 20

/* Clock Configuration */
#define TICKS_SCALE 8.0
#define RESERVED_CYCLE 125000000

/* Queue Size Configuration */
#define HW_QUEUE_SIZE_PAD 20
#define HW_QUEUE_SIZE 128
#define BE_QUEUE_SIZE_PAD 20
#define BE_QUEUE_SIZE (HW_QUEUE_SIZE - BE_QUEUE_SIZE_PAD)
#define TSN_QUEUE_SIZE_PAD 2
#define TSN_QUEUE_SIZE (HW_QUEUE_SIZE - TSN_QUEUE_SIZE_PAD)

/* Network Configuration */
#define ETHERNET_GAP_SIZE (8 + 4 + 12) /* 8 bytes preamble, 4 bytes FCS, 12 bytes interpacket gap */
#define PHY_DELAY_CLOCKS 13 /* 14 clocks from MAC to PHY, but sometimes there is 1 tick error */

/* Timing Adjustments */
#define TX_ADJUST_NS (100 + 200) /* MAC + PHY */
#define RX_ADJUST_NS (188 + 324) /* MAC + PHY */
#define H2C_LATENCY_NS 30000 /* TODO: Adjust this value dynamically */

/* Default Margins */
#define _DEFAULT_FROM_MARGIN_ 500
#define _DEFAULT_TO_MARGIN_ 50000

/* Bit Masks */
#define FIFO_DATA_CNT_MASK 0x00ff
#define LOWER_29_BITS ((1ULL << 29) - 1)

/* Time Constants */
#define NS_IN_1S 1000000000

/* TSN Priority Levels */
enum tsn_priority {
    TSN_PRIO_GPTP = 3,
    TSN_PRIO_VLAN = 5,
    TSN_PRIO_BE = 7,
};

/* TSN Timestamp IDs */
enum tsn_timestamp_id {
    TSN_TIMESTAMP_ID_NONE = 0,
    TSN_TIMESTAMP_ID_GPTP = 1,
    TSN_TIMESTAMP_ID_NORMAL = 2,
    TSN_TIMESTAMP_ID_RESERVED1 = 3,
    TSN_TIMESTAMP_ID_RESERVED2 = 4,
    TSN_TIMESTAMP_ID_MAX,
};

/* TSN Fail Policy */
enum tsn_fail_policy {
    TSN_FAIL_POLICY_DROP = 0,
    TSN_FAIL_POLICY_RETRY = 1,
};

/* SPI Header Types */
enum tsnlab_spi_header_type {
    TSNLAB_SPI_CTRL_HEADER,
    TSNLAB_SPI_DATA_HEADER,
    TSNLAB_SPI_META_HEADER,
};

/* SPI Register Operations */
enum tsnlab_spi_register_op {
    TSNLAB_SPI_CTRL_REG_READ = 0,
    TSNLAB_SPI_CTRL_REG_WRITE = 1,
};

/* SPI Data Validation */
enum tsnlab_spi_data_valid_info {
    TSNLAB_SPI_DATA_INVALID,
    TSNLAB_SPI_DATA_VALID,
};

enum tsnlab_spi_data_start_valid_info {
    TSNLAB_SPI_DATA_START_INVALID,
    TSNLAB_SPI_DATA_START_VALID,
};

enum tsnlab_spi_data_end_valid_info {
    TSNLAB_SPI_DATA_END_INVALID,
    TSNLAB_SPI_DATA_END_VALID,
};

#endif /* LAN1QTSN_CONSTANTS_H */
```

```cpp
#ifndef LAN1QTSN_MAC_H
#define LAN1QTSN_MAC_H

#include <linux/types.h>
#include <linux/tsnlab_spi.h>

/**
 * Set hardware MAC address low bytes
 * @param tc6: TSNLab SPI structure
 * @param mac: MAC address array
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_set_hw_macaddr_low_bytes(struct tsnlab_spi *tc6, const u8 *mac);

/**
 * Set hardware MAC address
 * @param priv: LAN1QTSN private structure
 * @param mac: MAC address array
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_set_hw_macaddr(struct lan1qtsn_priv *priv, const u8 *mac);

/**
 * Calculate hash for MAC address
 * @param addr: MAC address array
 * @return: Hash value
 */
u32 lan1qtsn_hash(u8 addr[ETH_ALEN]);

/**
 * Get address bit from MAC address
 * @param addr: MAC address array
 * @param bit: Bit position
 * @return: Bit value (0 or 1)
 */
u32 get_address_bit(u8 addr[ETH_ALEN], u32 bit);

#endif /* LAN1QTSN_MAC_H */
```

```c
#include "lan1qtsn_mac.h"
#include "lan1qtsn_registers.h"

#include <linux/etherdevice.h>
#include <linux/tsnlab_spi.h>

int lan1qtsn_set_hw_macaddr_low_bytes(struct tsnlab_spi *tc6, const u8 *mac)
{
    u32 regval;

    regval = (mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0];

    return tsnlab_spi_write_register(tc6, LAN865X_REG_MAC_L_SADDR1, regval);
}

int lan1qtsn_set_hw_macaddr(struct lan1qtsn_priv *priv, const u8 *mac)
{
    return 0;
}

u32 get_address_bit(u8 addr[ETH_ALEN], u32 bit)
{
    return ((addr[bit / 8]) >> (bit % 8)) & 1;
}

u32 lan1qtsn_hash(u8 addr[ETH_ALEN])
{
    u32 hash_index = 0;

    for (int i = 0; i < 6; i++) {
        u32 hash = 0;

        for (int j = 0; j < 8; j++)
            hash ^= get_address_bit(addr, (j * 6) + i);

        hash_index |= (hash << i);
    }

    return hash_index;
}
```

```cpp
#ifndef LAN1QTSN_MULTICAST_H
#define LAN1QTSN_MULTICAST_H

#include <linux/netdevice.h>
#include <linux/tsnlab_spi.h>

struct lan1qtsn_priv;

/**
 * Set specific multicast addresses
 * @param priv: LAN1QTSN private structure
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_set_specific_multicast_addr(struct lan1qtsn_priv *priv);

/**
 * Set all multicast addresses
 * @param priv: LAN1QTSN private structure
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_set_all_multicast_addr(struct lan1qtsn_priv *priv);

/**
 * Clear all multicast addresses
 * @param priv: LAN1QTSN private structure
 * @return: 0 on success, negative error code on failure
 */
int lan1qtsn_clear_all_multicast_addr(struct lan1qtsn_priv *priv);

/**
 * Handle multicast work
 * @param work: Work structure
 */
void lan1qtsn_multicast_work_handler(struct work_struct *work);

/**
 * Set multicast list
 * @param netdev: Network device
 */
void lan1qtsn_set_multicast_list(struct net_device *netdev);

#endif /* LAN1QTSN_MULTICAST_H */
```

```c
#include "lan1qtsn_multicast.h"
#include "lan1qtsn_registers.h"

#include <linux/netdevice.h>
#include <linux/tsnlab_spi.h>

struct lan1qtsn_priv {
    struct work_struct multicast_work;
    struct net_device *netdev;
    struct spi_device *spi;
    struct tsnlab_spi *tc6;
};

int lan1qtsn_set_specific_multicast_addr(struct lan1qtsn_priv *priv)
{
    struct netdev_hw_addr *ha;
    u32 hash_lo = 0;
    u32 hash_hi = 0;
    int ret;

    netdev_for_each_mc_addr(ha, priv->netdev) {
        u32 bit_num = lan1qtsn_hash(ha->addr);

        if (bit_num >= BIT(5))
            hash_hi |= (1 << (bit_num - BIT(5)));
        else
            hash_lo |= (1 << bit_num);
    }

    /* Enabling specific multicast addresses */
    ret = tsnlab_spi_write_register(priv->tc6, LAN865X_REG_MAC_H_HASH, hash_hi);
    if (ret) {
        netdev_err(priv->netdev, "Failed to write reg_hashh: %d\n", ret);
        return ret;
    }

    ret = tsnlab_spi_write_register(priv->tc6, LAN865X_REG_MAC_L_HASH, hash_lo);
    if (ret)
        netdev_err(priv->netdev, "Failed to write reg_hashl: %d\n", ret);

    return ret;
}

int lan1qtsn_set_all_multicast_addr(struct lan1qtsn_priv *priv)
{
    int ret;

    /* Enabling all multicast addresses */
    ret = tsnlab_spi_write_register(priv->tc6, LAN865X_REG_MAC_H_HASH, 0xffffffff);
    if (ret) {
        netdev_err(priv->netdev, "Failed to write reg_hashh: %d\n", ret);
        return ret;
    }

    ret = tsnlab_spi_write_register(priv->tc6, LAN865X_REG_MAC_L_HASH, 0xffffffff);
    if (ret)
        netdev_err(priv->netdev, "Failed to write reg_hashl: %d\n", ret);

    return ret;
}

int lan1qtsn_clear_all_multicast_addr(struct lan1qtsn_priv *priv)
{
    int ret;

    ret = tsnlab_spi_write_register(priv->tc6, LAN865X_REG_MAC_H_HASH, 0);
    if (ret) {
        netdev_err(priv->netdev, "Failed to write reg_hashh: %d\n", ret);
        return ret;
    }

    ret = tsnlab_spi_write_register(priv->tc6, LAN865X_REG_MAC_L_HASH, 0);
    if (ret)
        netdev_err(priv->netdev, "Failed to write reg_hashl: %d\n", ret);

    return ret;
}

void lan1qtsn_multicast_work_handler(struct work_struct *work)
{
    struct lan1qtsn_priv *priv = container_of(work, struct lan1qtsn_priv, multicast_work);
    u32 regval = 0;
    int ret;

    if (priv->netdev->flags & IFF_PROMISC) {
        /* Enabling promiscuous mode */
        regval |= MAC_NET_CFG_PROMISCUOUS_MODE;
        regval &= (~MAC_NET_CFG_MULTICAST_MODE);
        regval &= (~MAC_NET_CFG_UNICAST_MODE);
    } else if (priv->netdev->flags & IFF_ALLMULTI) {
        /* Enabling all multicast mode */
        if (lan1qtsn_set_all_multicast_addr(priv))
            return;

        regval &= (~MAC_NET_CFG_PROMISCUOUS_MODE);
        regval |= MAC_NET_CFG_MULTICAST_MODE;
        regval &= (~MAC_NET_CFG_UNICAST_MODE);
    } else if (!netdev_mc_empty(priv->netdev)) {
        /* Enabling specific multicast mode */
        if (lan1qtsn_set_specific_multicast_addr(priv))
            return;

        regval &= (~MAC_NET_CFG_PROMISCUOUS_MODE);
        regval |= MAC_NET_CFG_MULTICAST_MODE;
        regval &= (~MAC_NET_CFG_UNICAST_MODE);
    } else {
        /* Enabling local mac address only */
        if (lan1qtsn_clear_all_multicast_addr(priv))
            return;
    }
    ret = tsnlab_spi_write_register(priv->tc6, LAN865X_REG_MAC_NET_CFG, regval);
    if (ret)
        netdev_err(priv->netdev, "Failed to enable promiscuous/multicast/normal mode: %d\n", ret);
}

void lan1qtsn_set_multicast_list(struct net_device *netdev)
{
    struct lan1qtsn_priv *priv = netdev_priv(netdev);

    schedule_work(&priv->multicast_work);
}
```

이제 기존 파일들을 리팩토링하겠습니다. 먼저 `one_q_tsn_arch.h`를 정리하겠습니다:


```cpp
#ifndef ONE_Q_TSN_ARCH_H
#define ONE_Q_TSN_ARCH_H

#include <linux/net_tstamp.h>
#include <linux/pci.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/tsnlab_spi.h>
#include <linux/types.h>
#include <net/pkt_sched.h>

/* Register Class: General */
#define TSN_SYSTEM_INFO_H 0x0278 /* Year [63:56] : Month [55:48] : Day [47:40] : Commit # [39:32] */
#define TSN_SYSTEM_INFO_L 0x027C /* RESERVED [31:0] */

#define TSN_MAC_ADDR_H 0x0280 /* RESERVED [63:48] : MAC Address [47:32] */
#define TSN_MAC_ADDR_L 0x0284 /* MAC Address [31:0] */

#define TSN_SYSCOUNT_H 0x0288 /* SYSTEM_COUNT [63:32] */
#define TSN_SYSCOUNT_L 0x028C /* SYSTEM_COUNT [31:0] */

#define TSN_SYSTEM_CONTROL_H 0x0290 /* RESERVED [63:32] */
#define TSN_SYSTEM_CONTROL_L 0x0294 /* RESERVED [31:1] : TSN ENABLE */

#define TSN_RX_CONTROL_H 0x0298 /* MAX_FRAME_LENGTH [63:48] : */
/* RESERVED [47:45] : */
/* MAX_FRAME_FIFO_DATA_COUNT [44:32] */
#define TSN_RX_CONTROL_L 0x029C /* RESERVED [31:25] : */
/* MAX_META_FIFO_DATA_COUNT [24:16] : */
/* RESERVED [15:8] : CLEAR DROP BYTE COUNT : */
/* CLEAR DROP FRAME COUNT : */
/* CLEAR RX BYTE COUNT : */
/* CLEAR RX FRAME COUNT : */
/* CHECK-SUM ENABLE : */
/* JUMBO DROP :  FCS OFFLOAD : */
/* VLAN OFFLOAD */
#define TSN_TX_CONTROL_H 0x02A0 /* RESERVED [63:32] */
#define TSN_TX_CONTROL_L 0x02A4 /* ETHERNET_INTER_FRAME_GAP [31:16] : */
/* RESERVED [15:10] : */
/* HOST_FIFO_FULL_LIMIT [9:0] */
#define PPS_PULSE_AT_H 0x02A8 /* PULSE_AT [63:32] */
#define PPS_PULSE_AT_L 0x02AC /* PULSE_AT [31:0] */
#define PPS_CYCLE_1S_H 0x02B0 /* CYCLE_1S [63:32] */
#define PPS_CYCLE_1S_L 0x02B4 /* CYCLE_1S [31:0] */

/* Register Class: Rx (Frame Decoder) */
#define FD_RX_TSTAMP_H 0x0000        /* RX_TSTAMP [63:32] */
#define FD_RX_TSTAMP_L 0x0004        /* RX_TSTAMP [31:0] */
#define FD_RX_FRAME_STATUS1_H 0x0008 /* CHECK_SUM [63:32] */
#define FD_RX_FRAME_STATUS1_L 0x000C /* VLAN_TAG [31:16] : FRAME_LENGTH [15:0]  */
#define FD_RX_FRAME_STATUS2_H 0x0010 /* RESERVED [63:32] */
#define FD_RX_FRAME_STATUS2_L 0x0014 /* RESERVED [31:5] : VLAN OFFLOAD FLAG : */
/* FCS OFFLOAD FLAG : FCS ERROR FLAG : */
/* JUMBO ERROR FLAG : FIFO FULL FLAG */
#define FD_TOTAL_RX_FRAME_CNT_H 0x0018       /* TOTAL_RX_FRAME_CNT [63:32] */
#define FD_TOTAL_RX_FRAME_CNT_L 0x001C       /* TOTAL_RX_FRAME_CNT [31:0] */
#define FD_TOTAL_RX_BYTE_CNT_H 0x0020        /* TOTAL_RX_BYTE_CNT [63:32] */
#define FD_TOTAL_RX_BYTE_CNT_L 0x0024        /* TOTAL_RX_BYTE_CNT [31:0] */
#define FD_TOTAL_DROP_FRAME_CNT_H 0x0028     /* TOTAL_DROP_FRAME_CNT [63:32] */
#define FD_TOTAL_DROP_FRAME_CNT_L 0x002C     /* TOTAL_DROP_FRAME_CNT [31:0] */
#define FD_TOTAL_DROP_BYTE_CNT_H 0x0030      /* TOTAL_DROP_BYTE_CNT [63:32] */
#define FD_TOTAL_DROP_BYTE_CNT_L 0x0034      /* TOTAL_DROP_BYTE_CNT [31:0] */
#define FD_FCS_DROP_FRAME_CNT_H 0x0038       /* FCS_DROP_FRAME_CNT [63:32] */
#define FD_FCS_DROP_FRAME_CNT_L 0x003C       /* FCS_DROP_FRAME_CNT [31:0] */
#define FD_FCS_DROP_BYTE_CNT_H 0x0040        /* FCS_DROP_BYTE_CNT [63:32] */
#define FD_FCS_DROP_BYTE_CNT_L 0x0044        /* FCS_DROP_BYTE_CNT [31:0] */
#define FD_JUMBO_DROP_FRAME_CNT_H 0x0048     /* JUMBO_DROP_FRAME_CNT [63:32] */
#define FD_JUMBO_DROP_FRAME_CNT_L 0x004C     /* JUMBO_DROP_FRAME_CNT [31:0] */
#define FD_JUMBO_DROP_BYTE_CNT_H 0x0050      /* JUMBO_DROP_BYTE_CNT [63:32] */
#define FD_JUMBO_DROP_BYTE_CNT_L 0x0054      /* JUMBO_DROP_BYTE_CNT [31:0] */
#define FD_FIFO_FULL_DROP_FRAME_CNT_H 0x0058 /* FIFO_FULL_DROP_FRAME_CNT [63:32] */
#define FD_FIFO_FULL_DROP_FRAME_CNT_L 0x005C /* FIFO_FULL_DROP_FRAME_CNT [31:0] */
#define FD_FIFO_FULL_DROP_BYTE_CNT_H 0x0060  /* FIFO_FULL_DROP_BYTE_CNT [63:32] */
#define FD_FIFO_FULL_DROP_BYTE_CNT_L 0x0064  /* FIFO_FULL_DROP_BYTE_CNT [31:0] */

/* Register Class: FIFO (META, FRAME) */
#define TSN_RX_FIFO_STATUS_H 0x00A8 /* RESERVED [63:32] */
#define TSN_RX_FIFO_STATUS_L 0x00AC /* RES [31:29] : */
/* FRAME_FIFO_DATA_CNT [28:16] : */
/* RESERVED [15:9] : */
/* META_FIFO_DATA_CNT [8:0] */

/* Register Class: Tx (Frame Stacker) */
#define FS_GENERAL_STATUS_H 0x00B0 /* RESERVED [63:58] : */
/* HOST_FIFO_RD_CNT [57:48] : */
/* RESERVED [47:42] : */
/* HOST_FIFO_WR_CNT [41:32] */
#define FS_GENERAL_STATUS_L 0x00B4 /* RESERVED [31:4] : XDMA AXIS TREADY : */
/* HOST FIFO USER FULL */
/* HOST FIFO EMPTY : HOST FIFO FULL */
#define FS_TOTAL_RX_FRAME_CNT_H 0x00B8            /* TOTAL_RX_FRAME_CNT [63:32] */
#define FS_TOTAL_RX_FRAME_CNT_L 0x00BC            /* TOTAL_RX_FRAME_CNT [31:0] */
#define FS_TOTAL_RX_16BYTE_CNT_H 0x00C0           /* TOTAL_RX_16BYTE_CNT [63:32] */
#define FS_TOTAL_RX_16BYTE_CNT_L 0x00C4           /* TOTAL_RX_16BYTE_CNT [31:0] */
#define FS_TOTAL_BACK_PRESSURE_EVENT_CNT_H 0x00C8 /* TOTAL_BACK_PRESSURE_EVENT_CNT [63:32] */
#define FS_TOTAL_BACK_PRESSURE_EVENT_CNT_L 0x00CC /* TOTAL_BACK_PRESSURE_EVENT_CNT [31:0] */

/* Register Class: Tx (Frame Parser) */
#define FP_FRAME_TICK_FROM_H 0x00E0       /* TICK_FROM [63:32] */
#define FP_FRAME_TICK_FROM_L 0x00E4       /* TICK_FROM [31:0] */
#define FP_FRAME_TICK_TO_H 0x00E8         /* TICK_TO [63:32] */
#define FP_FRAME_TICK_TO_L 0x00EC         /* TICK_TO [31:0] */
#define FP_FRAME_TICK_DELAY_FROM_H 0x00F0 /* TICK_DELAY_FROM [63:32] */
#define FP_FRAME_TICK_DELAY_FROM_L 0x00F4 /* TICK_DELAY_FROM [31:0] */
#define FP_FRAME_TICK_DELAY_TO_H 0x00F8   /* TICK_DELAY_TO [63:32] */
#define FP_FRAME_TICK_DELAY_TO_L 0x00FC   /* TICK_DELAY_TO [31:0] */
#define FP_FRAME_STATUS1_H 0x0100         /* RESERVED [63:56] : ALLOC_ADDR [55:48] : */
/* RESERVED [47:33] : FAIL_POLICY [32] */
#define FP_FRAME_STATUS1_L 0x0104 /* FRAME_LENGTH [31:16] : TIMESTAMP_ID [15:0] */
#define FP_FRAME_STATUS2_H 0x0108 /* RESERVED [63:52] : */
/* META_AXIS_HSK_CNT [51:49] : */
/* META_ADDR_OFFSET [48] : */
/* RESERVED [47:39] : */
/* FRAME_ADDR_OFFSET [38:32] */
#define FP_FRAME_STATUS2_L 0x010C /* FRAME_AXIS_HSK_CNT [31:16] : */
/* RX_FRAME_16BYTE_CNT [15:0] */
/* Register Class: Tx (Frame Scheduler) */
#define FSCH_TX_FRAME_STATUS_H 0x0118 /* RESERVED [63:48] : */
/* TX_FRAME_TSTAMP_ID [47:32] */
#define FSCH_TX_FRAME_STATUS_L 0x011C /* TX_FRAME_LENGTH [15:0] : */
/* RESERVED [15:8] : */
/* TX_FRAME_ADDRESS [7:0] */
#define FSCH_TOTAL_NEW_ENTRY_CNT_H 0x0120   /* TOTAL_NEW_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_NEW_ENTRY_CNT_L 0x0124   /* TOTAL_NEW_ENTRY_CNT [31:0] */
#define FSCH_TOTAL_VALID_ENTRY_CNT_H 0x0128 /* TOTAL_VALID_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_VALID_ENTRY_CNT_L 0x012C /* TOTAL_VALID_ENTRY_CNT [31:0] */
#define FSCH_TOTAL_DELAY_ENTRY_CNT_H 0x0130 /* TOTAL_DELAY_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_DELAY_ENTRY_CNT_L 0x0134 /* TOTAL_DELAY_ENTRY_CNT [31:0] */
#define FSCH_TOTAL_DROP_ENTRY_CNT_H 0x0138  /* TOTAL_DROP_ENTRY_CNT [63:32] */
#define FSCH_TOTAL_DROP_ENTRY_CNT_L 0x013C  /* TOTAL_DROP_ENTRY_CNT [31:0] */

/* Register Class: Tx (Frame Buffer) */
#define FBW_BUFFER_WRITE_STATUS1_H 0x0148 /* RESERVED [63:40] : */
/* ADDR_FIFO_DATA_CNT [39:32] */
#define FBW_BUFFER_WRITE_STATUS1_L 0x014C /* RESERVED [31:11] : */
/* BRAM_SEL [10:8] : ALLOC_ADDR [7:0] */
#define FBW_BUFFER_WRITE_STATUS2_H 0x0150 /* RESERVED [63:59] : */
/* BRAM3_WR_ADDR_LAST [58:48] : */
/* RESERVED [47:43] : */
/* BRAM2_WR_ADDR_LAST [42:32] */
#define FBW_BUFFER_WRITE_STATUS2_L 0x0154 /* RESERVED [31:27] : */
/* BRAM1_WR_ADDR_LAST [26:16] : */
/* RESERVED [15:11] : */
/* BRAM0_WR_ADDR_LAST [10:0] */
#define FBW_BUFFER_WRITE_STATUS3_H 0x0158 /* RESERVED [63:59] : */
/* BRAM7_WR_ADDR_LAST [58:48] : */
/* RESERVED [47:43] : */
/* BRAM6_WR_ADDR_LAST [42:32] */
#define FBW_BUFFER_WRITE_STATUS3_L 0x015C /* RESERVED [31:27] : */
/* BRAM5_WR_ADDR_LAST [26:16] */
/* RESERVED [15:11] */
/* BRAM4_WR_ADDR_LAST [10:0] */
#define FBW_ADDR_FIFO_CNT_H 0x0160 /* RESERVED [63:32] */
#define FBW_ADDR_FIFO_CNT_L 0x0164 /* RESERVED [31:8] : */
/* ADDR_FIFO_DATA_CNT [7:0] */
#define FBR_BUFFER_READ_STATUS1_H 0x0168 /* RESERVED [63:40] : */
/* DROP_FRAME_ADDR [39:32] */
#define FBR_BUFFER_READ_STATUS1_L 0x016C /* SELECTED_FRAME_16BYTE_CNT [31:16] : */
/* RESERVED [15:11] : */
/* BRAM_SEL [2:0] : */
/* SELECTED_FRAME_ADDR [7:0] */
#define FBR_BUFFER_READ_STATUS2_H 0x0170 /* RESERVED [63:59] : */
/* BRAM3_RD_ADDR_LAST [58:48] : */
/* RESERVED [47:43] : */
/* BRAM2_RD_ADDR_LAST [42:32] */
#define FBR_BUFFER_READ_STATUS2_L 0x0174 /* RESERVED [31:27] : */
/* BRAM1_RD_ADDR_LAST [26:16] : */
/* RESERVED [15:11] : */
/* BRAM0_RD_ADDR_LAST [10:0] */
#define FBR_BUFFER_READ_STATUS3_H 0x0178 /* RESERVED [63:59] : */
/* BRAM7_RD_ADDR_LAST [58:48] : */
/* RESERVED [47:43] : */
/* BRAM6_RD_ADDR_LAST [42:32] */
#define FBR_BUFFER_READ_STATUS3_L 0x017C /* RESERVED [31:27] : */
/* BRAM5_RD_ADDR_LAST [26:16] : */
/* RESERVED [15:11] : */
/* BRAM4_RD_ADDR_LAST [10:0] */

/* Register Class: Tx (Frame Transfer FSM) */
#define FTRSF_TX_FRAME_STATUS_H 0x0188 /* TX_METADATA [63:32] */
#define FTRSF_TX_FRAME_STATUS_L 0x018C /* TX_FRAME_LENGTH [15:0] : */
/* RESERVED [15:8] : */
/* TX_FRAME_ADDRESS [7:0] */
#define FTRSF_GENERAL_STATUS_H 0x0190 /* RESERVED [63:35] : */
/* TX_FRAME_FIFO_FULL [34] : */
/* TX_META_FIFO_FULL [33] : */
/* TX_STATUS [32] */
#define FTRSF_GENERAL_STATUS_L 0x0194 /* RESERVED [31:25] : */
/* TX_FRAME_FIFO_DATA_COUNT [24:16] : */
/* RESERVED [15:9] : */
/* TX_META_FIFO_DATA_COUNT [8:0] */
#define FTRSF_TOTAL_TX_FRAME_CNT_H 0x0198        /* TOTAL_TX_FRAME_CNT [63:32] */
#define FTRSF_TOTAL_TX_FRAME_CNT_L 0x019C        /* TOTAL_TX_FRAME_CNT [31:0] */
#define FTRSF_TOTAL_META_FIFO_FULL_CNT_H 0x01A0  /* TOTAL_META_FIFO_FULL_CNT [63:32] */
#define FTRSF_TOTAL_META_FIFO_FULL_CNT_L 0x01A4  /* TOTAL_META_FIFO_FULL_CNT [31:0] */
#define FTRSF_TOTAL_FRAME_FIFO_FULL_CNT_H 0x01A8 /* TOTAL_FRAME_FIFO_FULL_CNT [63:32] */
#define FTRSF_TOTAL_FRAME_FULL_CNT_L 0x01AC      /* TOTAL_FRAME_FIFO_FULL_CNT [31:0] */

/* Register Class: Tx (Frame Transmitter FSM) */
#define FT_TX_FRAME_STATUS1_H 0x01B8 /* TX_METADATA [63:32] */
#define FT_TX_FRAME_STATUS1_L 0x01BC /* TX_FRAME_16BYTE_LENGTH [31:16] : */
/* TX_FRAME_LENGTH [15:0] */
#define FT_TX_FRAME_STATUS2_H 0x01C0 /* RESERVED [63:48] : */
/* TX_FRAME_TSTAMP_ID [47:32] */
#define FT_TX_FRAME_STATUS2_L 0x01C4 /* TX_16BYTE_CNT [31:16] : */
/* TX_BYTE_CNT [15:0] */
#define FT_TOTAL_TX_FRAME_CNT_H 0x01C8 /* TOTAL_TX_FRAME_CNT [63:32] */
#define FT_TOTAL_TX_FRAME_CNT_L 0x01CC /* TOTAL_TX_FRAME_CNT [31:0] */
#define FT_TOTAL_TX_BYTE_CNT_H 0x01D0  /* TOTAL_TX_BYTE_CNT [63:32] */
#define FT_TOTAL_TX_BYTE_CNT_L 0x01D4  /* TOTAL_TX_BYTE_CNT [31:0] */
#define FT_TX_TSTAMP1_H 0x01D8         /* TX_TIMESTAMP_1 [63:32] */
#define FT_TX_TSTAMP1_L 0x01DC         /* TX_TIMESTAMP_1 [31:0] */
#define FT_TX_TSTAMP2_H 0x01E0         /* TX_TIMESTAMP_2 [63:32] */
#define FT_TX_TSTAMP2_L 0x01E4         /* TX_TIMESTAMP_2 [31:0] */
#define FT_TX_TSTAMP3_H 0x01E8         /* TX_TIMESTAMP_3 [63:32] */
#define FT_TX_TSTAMP3_L 0x01EC         /* TX_TIMESTAMP_3 [31:0] */
#define FT_TX_TSTAMP4_H 0x01F0         /* TX_TIMESTAMP_4 [63:32] */
#define FT_TX_TSTAMP4_L 0x01F4         /* TX_TIMESTAMP_4 [31:0] */

#define FIFO_DATA_CNT_MASK 0x00ff

#define TSN_DISABLE 0x0
#define TSN_ENABLE 0x1

#define TX_QUEUE_COUNT 8
#define RX_QUEUE_COUNT 8

/* 125 MHz */
#define TICKS_SCALE 8.0
#define RESERVED_CYCLE 125000000

#define HW_QUEUE_SIZE_PAD 20
#define HW_QUEUE_SIZE 128

#define BE_QUEUE_SIZE_PAD 20
#define BE_QUEUE_SIZE (HW_QUEUE_SIZE - BE_QUEUE_SIZE_PAD)

#define TSN_QUEUE_SIZE_PAD 2
#define TSN_QUEUE_SIZE (HW_QUEUE_SIZE - TSN_QUEUE_SIZE_PAD)

#define TC_COUNT 8
#define TSN_PRIO_COUNT 8
#define MAX_QBV_SLOTS 20

#define ETHERNET_GAP_SIZE (8 + 4 + 12) // 8 bytes preamble, 4 bytes FCS, 12 bytes interpacket gap
#define PHY_DELAY_CLOCKS 13            // 14 clocks from MAC to PHY, but sometimes there is 1 tick error

#define TX_ADJUST_NS (100 + 200) // MAC + PHY
#define RX_ADJUST_NS (188 + 324) // MAC + PHY

#define H2C_LATENCY_NS 30000 // TODO: Adjust this value dynamically

typedef u64 sysclock_t;
typedef u64 timestamp_t;

->

#ifndef ONE_Q_TSN_ARCH_H
#define ONE_Q_TSN_ARCH_H

#include <linux/net_tstamp.h>
#include <linux/pci.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/tsnlab_spi.h>
#include <linux/types.h>
#include <net/pkt_sched.h>

#include "lan1qtsn_constants.h"
#include "lan1qtsn_registers.h"

typedef u64 sysclock_t;
typedef u64 timestamp_t;
```

파일이 너무 길어서 정확한 매칭이 어렵습니다. 파일을 다시 읽어서 정확한 내용을 확인하겠습니다:


이제 `one_q_tsn_arch.h` 파일을 완전히 새로 작성하겠습니다:


```cpp
#ifndef ONE_Q_TSN_ARCH_H
#define ONE_Q_TSN_ARCH_H

#include <linux/net_tstamp.h>
#include <linux/pci.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/tsnlab_spi.h>
#include <linux/types.h>
#include <net/pkt_sched.h>

#include "lan1qtsn_constants.h"
#include "lan1qtsn_registers.h"

typedef u64 sysclock_t;
typedef u64 timestamp_t;

struct ptp_device_data {
    struct device *dev;
    struct ptp_clock *ptp_clock;
    struct ptp_clock_info ptp_info;
    struct tsnlab_spi *tc6;
    double ticks_scale;
    u64 offset;
    spinlock_t lock;
#ifdef __LIBXDMA_DEBUG__
    u32 ptp_id;
#endif
};

struct qbv_slot {
    uint32_t duration_ns; // We don't support cycle > 1s
    bool opened_prios[TC_COUNT];
};

struct qbv_config {
    bool enabled;
    timestamp_t start;
    struct qbv_slot slots[MAX_QBV_SLOTS];
    uint32_t slot_count;
};

struct qbv_baked_prio_slot {
    uint64_t duration_ns;
    bool opened;
};

struct qbv_baked_prio {
    struct qbv_baked_prio_slot slots[MAX_QBV_SLOTS];
    size_t slot_count;
};

struct qbv_baked_config {
    uint64_t cycle_ns;
    struct qbv_baked_prio prios[TC_COUNT];
};

struct qav_state {
    bool enabled;
    int32_t idle_slope; // credits/ns
    int32_t send_slope; // credits/ns
    int32_t hi_credit;
    int32_t lo_credit;
    int32_t credit;
    timestamp_t last_update;
    timestamp_t available_at;
};

struct tsn_config {
    struct qbv_config qbv;
    struct qbv_baked_config qbv_baked;
    struct qav_state qav[TC_COUNT];
    uint32_t buffer_space;
    timestamp_t queue_available_at[TSN_PRIO_COUNT];
    timestamp_t total_available_at;
};

struct tsnlab_spi {
    struct device *dev;
    struct net_device *netdev;
    struct phy_device *phydev;
    struct mii_bus *mdiobus;
    struct spi_device *spi;
    struct tsn_config tsn_config;
    struct ptp_device_data *ptp;
    struct mutex spi_ctrl_lock; /* Protects spi control transfer */
    spinlock_t tx_skb_lock;     /* Protects tx skb handling */
    void *spi_ctrl_tx_buf;
    void *spi_ctrl_rx_buf;
    void *spi_data_tx_buf;
    void *spi_data_rx_buf;
    void *spi_meta_tx_buf;
    void *spi_meta_rx_buf;
    struct sk_buff *ongoing_tx_skb;
    struct sk_buff *waiting_tx_skb;
    struct sk_buff *rx_skb;
    struct task_struct *spi_thread;
    wait_queue_head_t spi_wq;
    u16 tx_skb_offset;
    u16 spi_data_tx_buf_offset;
    u16 tx_credits;
    u8 rx_chunks_available;
    bool rx_buf_overflow;
    bool int_flag;
    struct hwtstamp_config tstamp_config;
};

/* Function declarations */
void one_q_tsn_set_pulse_at_by_spi(struct tsnlab_spi *tc6, sysclock_t time);
void one_q_tsn_set_pulse_at(struct tsnlab_spi *tc6, sysclock_t time);
sysclock_t one_q_tsn_get_sys_clock_by_spi(struct tsnlab_spi *tc6);
sysclock_t one_q_tsn_get_sys_clock(struct tsnlab_spi *tc6);
void one_q_tsn_set_cycle_1s_by_spi(struct tsnlab_spi *tc6, u32 cycle_1s);
void one_q_tsn_set_cycle_1s(struct tsnlab_spi *tc6, u32 cycle_1s);
u32 one_q_tsn_get_cycle_1s_by_spi(struct tsnlab_spi *tc6);
u32 one_q_tsn_get_cycle_1s(struct tsnlab_spi *tc6);
timestamp_t one_q_tsn_read_tx_timestamp_by_spi(struct tsnlab_spi *tc6, int tx_id);
timestamp_t one_q_tsn_read_tx_timestamp(struct tsnlab_spi *tc6, int tx_id);
u64 one_q_tsn_get_buffer_write_status_by_spi(struct tsnlab_spi *tc6);
u64 one_q_tsn_get_buffer_write_status(struct tsnlab_spi *tc6);
u64 one_q_tsn_get_total_new_entry_by_spi(struct tsnlab_spi *tc6);
u64 one_q_tsn_get_total_valid_entry_by_spi(struct tsnlab_spi *tc6);
u64 one_q_tsn_get_total_drop_entry_by_spi(struct tsnlab_spi *tc6);

#endif /* ONE_Q_TSN_ARCH_H */
```

이제 `one_q_tsn_arch.c` 파일을 리팩토링하겠습니다:


```c
#include "one_q_tsn_arch.h"

void one_q_tsn_set_pulse_at_by_spi(struct tsnlab_spi* tc6, sysclock_t time) {
    tsnlab_spi_write_register(tc6, PPS_PULSE_AT_H, (u32)(time >> 32));
    tsnlab_spi_write_register(tc6, PPS_PULSE_AT_L, (u32)(time));
}

void one_q_tsn_set_pulse_at(struct tsnlab_spi* tc6, sysclock_t time) {
    one_q_tsn_set_pulse_at_by_spi(tc6, time);
}

sysclock_t one_q_tsn_get_sys_clock_by_spi(struct tsnlab_spi* tc6) {
    timestamp_t clock;
    u32 sys_clock_high, sys_clock_low;
    tsnlab_spi_read_register(tc6, TSN_SYSCOUNT_H, &sys_clock_high);
    tsnlab_spi_read_register(tc6, TSN_SYSCOUNT_L, &sys_clock_low);
    clock = ((u64)sys_clock_high << 32) | sys_clock_low;

    return clock;
}

sysclock_t one_q_tsn_get_sys_clock(struct tsnlab_spi* tc6) {
    return one_q_tsn_get_sys_clock_by_spi(tc6);
}

void one_q_tsn_set_cycle_1s_by_spi(struct tsnlab_spi* tc6, u32 cycle_1s) {
    tsnlab_spi_write_register(tc6, PPS_CYCLE_1S_H, 0);
    tsnlab_spi_write_register(tc6, PPS_CYCLE_1S_L, cycle_1s);
}

void one_q_tsn_set_cycle_1s(struct tsnlab_spi* tc6, u32 cycle_1s) {
    one_q_tsn_set_cycle_1s_by_spi(tc6, cycle_1s);
}

u32 one_q_tsn_get_cycle_1s_by_spi(struct tsnlab_spi* tc6) {
    u32 cycle_1s;
    tsnlab_spi_read_register(tc6, PPS_CYCLE_1S_L, &cycle_1s);
    return cycle_1s ? cycle_1s : RESERVED_CYCLE;
}

u32 one_q_tsn_get_cycle_1s(struct tsnlab_spi* tc6) {
    return one_q_tsn_get_cycle_1s_by_spi(tc6);
}

timestamp_t one_q_tsn_read_tx_timestamp_by_spi(struct tsnlab_spi* tc6, int tx_id) {
    u32 timestamp_h, timestamp_l;

    switch (tx_id) {
    case 1:
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP1_H, &timestamp_h);
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP1_L, &timestamp_l);
        break;
    case 2:
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP2_H, &timestamp_h);
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP2_L, &timestamp_l);
        break;
    case 3:
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP3_H, &timestamp_h);
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP3_L, &timestamp_l);
        break;
    case 4:
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP4_H, &timestamp_h);
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP4_L, &timestamp_l);
        break;
    default:
        return 0;
    }

    return ((timestamp_t)timestamp_h << 32 | (timestamp_t)timestamp_l);
}

timestamp_t one_q_tsn_read_tx_timestamp(struct tsnlab_spi* tc6, int tx_id) {
    return one_q_tsn_read_tx_timestamp_by_spi(tc6, tx_id);
}

u64 one_q_tsn_get_buffer_write_status_by_spi(struct tsnlab_spi* tc6) {
    u32 reg_h, reg_l;
    tsnlab_spi_read_register(tc6, FBW_BUFFER_WRITE_STATUS1_H, &reg_h);
    tsnlab_spi_read_register(tc6, FBW_BUFFER_WRITE_STATUS1_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

u64 one_q_tsn_get_buffer_write_status(struct tsnlab_spi* tc6) {
    return one_q_tsn_get_buffer_write_status_by_spi(tc6);
}

u64 one_q_tsn_get_total_new_entry_by_spi(struct tsnlab_spi* tc6) {
    u32 reg_h, reg_l;
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_NEW_ENTRY_CNT_H, &reg_h);
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_NEW_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

u64 one_q_tsn_get_total_valid_entry_by_spi(struct tsnlab_spi* tc6) {
    u32 reg_h, reg_l;
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_VALID_ENTRY_CNT_H, &reg_h);
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_VALID_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

u64 one_q_tsn_get_total_drop_entry_by_spi(struct tsnlab_spi* tc6) {
    u32 reg_h, reg_l;
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_DROP_ENTRY_CNT_H, &reg_h);
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_DROP_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

->

#include "one_q_tsn_arch.h"
#include "lan1qtsn_registers.h"

#include <linux/tsnlab_spi.h>

/**
 * Set pulse at time by SPI
 * @param tc6: TSNLab SPI structure
 * @param time: System clock time
 */
void one_q_tsn_set_pulse_at_by_spi(struct tsnlab_spi *tc6, sysclock_t time)
{
    tsnlab_spi_write_register(tc6, PPS_PULSE_AT_H, (u32)(time >> 32));
    tsnlab_spi_write_register(tc6, PPS_PULSE_AT_L, (u32)(time));
}

/**
 * Set pulse at time
 * @param tc6: TSNLab SPI structure
 * @param time: System clock time
 */
void one_q_tsn_set_pulse_at(struct tsnlab_spi *tc6, sysclock_t time)
{
    one_q_tsn_set_pulse_at_by_spi(tc6, time);
}

/**
 * Get system clock by SPI
 * @param tc6: TSNLab SPI structure
 * @return: System clock value
 */
sysclock_t one_q_tsn_get_sys_clock_by_spi(struct tsnlab_spi *tc6)
{
    timestamp_t clock;
    u32 sys_clock_high, sys_clock_low;
    
    tsnlab_spi_read_register(tc6, TSN_SYSCOUNT_H, &sys_clock_high);
    tsnlab_spi_read_register(tc6, TSN_SYSCOUNT_L, &sys_clock_low);
    clock = ((u64)sys_clock_high << 32) | sys_clock_low;

    return clock;
}

/**
 * Get system clock
 * @param tc6: TSNLab SPI structure
 * @return: System clock value
 */
sysclock_t one_q_tsn_get_sys_clock(struct tsnlab_spi *tc6)
{
    return one_q_tsn_get_sys_clock_by_spi(tc6);
}

/**
 * Set cycle 1s by SPI
 * @param tc6: TSNLab SPI structure
 * @param cycle_1s: Cycle value
 */
void one_q_tsn_set_cycle_1s_by_spi(struct tsnlab_spi *tc6, u32 cycle_1s)
{
    tsnlab_spi_write_register(tc6, PPS_CYCLE_1S_H, 0);
    tsnlab_spi_write_register(tc6, PPS_CYCLE_1S_L, cycle_1s);
}

/**
 * Set cycle 1s
 * @param tc6: TSNLab SPI structure
 * @param cycle_1s: Cycle value
 */
void one_q_tsn_set_cycle_1s(struct tsnlab_spi *tc6, u32 cycle_1s)
{
    one_q_tsn_set_cycle_1s_by_spi(tc6, cycle_1s);
}

/**
 * Get cycle 1s by SPI
 * @param tc6: TSNLab SPI structure
 * @return: Cycle value
 */
u32 one_q_tsn_get_cycle_1s_by_spi(struct tsnlab_spi *tc6)
{
    u32 cycle_1s;
    tsnlab_spi_read_register(tc6, PPS_CYCLE_1S_L, &cycle_1s);
    return cycle_1s ? cycle_1s : RESERVED_CYCLE;
}

/**
 * Get cycle 1s
 * @param tc6: TSNLab SPI structure
 * @return: Cycle value
 */
u32 one_q_tsn_get_cycle_1s(struct tsnlab_spi *tc6)
{
    return one_q_tsn_get_cycle_1s_by_spi(tc6);
}

/**
 * Read TX timestamp by SPI
 * @param tc6: TSNLab SPI structure
 * @param tx_id: TX ID (1-4)
 * @return: Timestamp value
 */
timestamp_t one_q_tsn_read_tx_timestamp_by_spi(struct tsnlab_spi *tc6, int tx_id)
{
    u32 timestamp_h, timestamp_l;

    switch (tx_id) {
    case 1:
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP1_H, &timestamp_h);
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP1_L, &timestamp_l);
        break;
    case 2:
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP2_H, &timestamp_h);
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP2_L, &timestamp_l);
        break;
    case 3:
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP3_H, &timestamp_h);
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP3_L, &timestamp_l);
        break;
    case 4:
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP4_H, &timestamp_h);
        tsnlab_spi_read_register(tc6, FT_TX_TSTAMP4_L, &timestamp_l);
        break;
    default:
        return 0;
    }

    return ((timestamp_t)timestamp_h << 32 | (timestamp_t)timestamp_l);
}

/**
 * Read TX timestamp
 * @param tc6: TSNLab SPI structure
 * @param tx_id: TX ID (1-4)
 * @return: Timestamp value
 */
timestamp_t one_q_tsn_read_tx_timestamp(struct tsnlab_spi *tc6, int tx_id)
{
    return one_q_tsn_read_tx_timestamp_by_spi(tc6, tx_id);
}

/**
 * Get buffer write status by SPI
 * @param tc6: TSNLab SPI structure
 * @return: Buffer write status
 */
u64 one_q_tsn_get_buffer_write_status_by_spi(struct tsnlab_spi *tc6)
{
    u32 reg_h, reg_l;
    tsnlab_spi_read_register(tc6, FBW_BUFFER_WRITE_STATUS1_H, &reg_h);
    tsnlab_spi_read_register(tc6, FBW_BUFFER_WRITE_STATUS1_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

/**
 * Get buffer write status
 * @param tc6: TSNLab SPI structure
 * @return: Buffer write status
 */
u64 one_q_tsn_get_buffer_write_status(struct tsnlab_spi *tc6)
{
    return one_q_tsn_get_buffer_write_status_by_spi(tc6);
}

/**
 * Get total new entry count by SPI
 * @param tc6: TSNLab SPI structure
 * @return: Total new entry count
 */
u64 one_q_tsn_get_total_new_entry_by_spi(struct tsnlab_spi *tc6)
{
    u32 reg_h, reg_l;
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_NEW_ENTRY_CNT_H, &reg_h);
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_NEW_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

/**
 * Get total valid entry count by SPI
 * @param tc6: TSNLab SPI structure
 * @return: Total valid entry count
 */
u64 one_q_tsn_get_total_valid_entry_by_spi(struct tsnlab_spi *tc6)
{
    u32 reg_h, reg_l;
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_VALID_ENTRY_CNT_H, &reg_h);
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_VALID_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}

/**
 * Get total drop entry count by SPI
 * @param tc6: TSNLab SPI structure
 * @return: Total drop entry count
 */
u64 one_q_tsn_get_total_drop_entry_by_spi(struct tsnlab_spi *tc6)
{
    u32 reg_h, reg_l;
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_DROP_ENTRY_CNT_H, &reg_h);
    tsnlab_spi_read_register(tc6, FSCH_TOTAL_DROP_ENTRY_CNT_L, &reg_l);
    return ((u64)reg_h << 32 | (u64)reg_l);
}
```

이제 `tsn.h` 파일을 리팩토링하겠습니다:


```cpp
#pragma once

#include <linux/pci.h>
#include <linux/skbuff.h>
#include <linux/tsnlab_spi.h>
#include <net/pkt_cls.h>
#include <net/pkt_sched.h>

typedef uint64_t timestamp_t;
typedef uint64_t sysclock_t;

enum tsn_timestamp_id {
    TSN_TIMESTAMP_ID_NONE = 0,
    TSN_TIMESTAMP_ID_GPTP = 1,
    TSN_TIMESTAMP_ID_NORMAL = 2,
    TSN_TIMESTAMP_ID_RESERVED1 = 3,
    TSN_TIMESTAMP_ID_RESERVED2 = 4,

    TSN_TIMESTAMP_ID_MAX,
};

enum tsn_prio {
    TSN_PRIO_GPTP = 3,
    TSN_PRIO_VLAN = 5,
    TSN_PRIO_BE = 7,
};

enum tsn_fail_policy {
    TSN_FAIL_POLICY_DROP = 0,
    TSN_FAIL_POLICY_RETRY = 1,
};

struct tsn_vlan_hdr {
    uint16_t pid;
    uint8_t pcp : 3;
    uint8_t dei : 1;
    uint16_t vid : 12;
} __attribute__((packed, scalar_storage_order("big-endian")));

bool tsn_fill_metadata(struct tsnlab_spi* tc6, timestamp_t now, struct sk_buff* skb);
void tsn_init_configs(struct tsnlab_spi* config);

int tsn_set_mqprio(struct tsnlab_spi* tc6, struct tc_mqprio_qopt_offload* offload);
int tsn_set_qav(struct tsnlab_spi* tc6, struct tc_cbs_qopt_offload* offload);
int tsn_set_qbv(struct tsnlab_spi* tc6, struct tc_taprio_qopt_offload* offload);

->

#ifndef TSN_H
#define TSN_H

#include <linux/pci.h>
#include <linux/skbuff.h>
#include <linux/tsnlab_spi.h>
#include <net/pkt_cls.h>
#include <net/pkt_sched.h>

#include "lan1qtsn_constants.h"

struct tsn_vlan_hdr {
    uint16_t pid;
    uint8_t pcp : 3;
    uint8_t dei : 1;
    uint16_t vid : 12;
} __attribute__((packed, scalar_storage_order("big-endian")));

/**
 * Fill metadata for TSN frame
 * @param tc6: TSNLab SPI structure
 * @param now: Current timestamp
 * @param skb: Socket buffer
 * @return: true if successful, false otherwise
 */
bool tsn_fill_metadata(struct tsnlab_spi *tc6, timestamp_t now, struct sk_buff *skb);

/**
 * Initialize TSN configurations
 * @param config: TSNLab SPI structure
 */
void tsn_init_configs(struct tsnlab_spi *config);

/**
 * Set MQPRIO configuration
 * @param tc6: TSNLab SPI structure
 * @param offload: MQPRIO offload structure
 * @return: 0 on success, negative error code on failure
 */
int tsn_set_mqprio(struct tsnlab_spi *tc6, struct tc_mqprio_qopt_offload *offload);

/**
 * Set QAV configuration
 * @param tc6: TSNLab SPI structure
 * @param offload: CBS offload structure
 * @return: 0 on success, negative error code on failure
 */
int tsn_set_qav(struct tsnlab_spi *tc6, struct tc_cbs_qopt_offload *offload);

/**
 * Set QBV configuration
 * @param tc6: TSNLab SPI structure
 * @param offload: TAPRIO offload structure
 * @return: 0 on success, negative error code on failure
 */
int tsn_set_qbv(struct tsnlab_spi *tc6, struct tc_taprio_qopt_offload *offload);

#endif /* TSN_H */
```

이제 `tsn.c` 파일을 리팩토링하겠습니다. 먼저 파일을 더 작은 함수들로 나누겠습니다:


```cpp
#ifndef TSN_UTILS_H
#define TSN_UTILS_H

#include <linux/skbuff.h>
#include <linux/tsnlab_spi.h>

#include "lan1qtsn_constants.h"

/**
 * Check if packet is gPTP packet
 * @param payload: Packet payload
 * @return: true if gPTP packet, false otherwise
 */
bool is_gptp_packet(const uint8_t *payload);

/**
 * Get VLAN priority from packet
 * @param tsn_config: TSN configuration
 * @param skb: Socket buffer
 * @return: VLAN priority
 */
uint8_t tsn_get_vlan_prio(struct tsn_config *tsn_config, struct sk_buff *skb);

/**
 * Get MQPRIO traffic class
 * @param netdev: Network device
 * @param prio: Priority
 * @return: Traffic class
 */
uint8_t tsn_get_mqprio_tc(struct net_device *netdev, uint8_t prio);

/**
 * Convert timestamp to system clock
 * @param tc6: TSNLab SPI structure
 * @param timestamp: Timestamp
 * @return: System clock value
 */
sysclock_t tsn_timestamp_to_sysclock(struct tsnlab_spi *tc6, timestamp_t timestamp);

/**
 * Convert bytes to nanoseconds
 * @param bytes: Number of bytes
 * @return: Nanoseconds
 */
uint64_t bytes_to_ns(uint64_t bytes);

/**
 * Spend QAV credit
 * @param tsn_config: TSN configuration
 * @param at: Timestamp
 * @param tc_id: Traffic class ID
 * @param bytes: Number of bytes
 */
void spend_qav_credit(struct tsn_config *tsn_config, timestamp_t at, uint8_t tc_id, uint64_t bytes);

/**
 * Get timestamps for frame
 * @param timestamps: Output timestamps
 * @param tsn_config: TSN configuration
 * @param from: Start time
 * @param tc_id: Traffic class ID
 * @param bytes: Number of bytes
 * @param consider_delay: Whether to consider delay
 * @return: true if successful, false otherwise
 */
bool get_timestamps(struct timestamps *timestamps, const struct tsn_config *tsn_config, 
                   timestamp_t from, uint8_t tc_id, uint64_t bytes, bool consider_delay);

/**
 * Update buffer status
 * @param tc6: TSNLab SPI structure
 */
void update_buffer(struct tsnlab_spi *tc6);

/**
 * Decrease buffer space
 * @param tc6: TSNLab SPI structure
 */
void decrease_buffer_space(struct tsnlab_spi *tc6);

/**
 * Check if buffer is available
 * @param tc6: TSNLab SPI structure
 * @param min_space: Minimum space required
 * @return: true if available, false otherwise
 */
bool is_buffer_available(struct tsnlab_spi *tc6, uint32_t min_space);

/**
 * Bake QoS configuration
 * @param config: TSN configuration
 */
void bake_qos_config(struct tsn_config *config);

#endif /* TSN_UTILS_H */
```

```c
#include "tsn_utils.h"
#include "one_q_tsn_arch.h"
#include "one_q_tsn_ptp.h"

#include <linux/if_ether.h>
#include <linux/string.h>
#include <linux/tsnlab_spi.h>

#define NS_IN_1S 1000000000
#define TSN_ALWAYS_OPEN(from) (from - 1) /* For both timestamp and sysclock */

struct timestamps {
    timestamp_t from;
    timestamp_t to;
    timestamp_t delay_from;
    timestamp_t delay_to;
};

bool is_gptp_packet(const uint8_t *payload)
{
    struct ethhdr *eth = (struct ethhdr *)payload;
    uint16_t eth_type = ntohs(eth->h_proto);
    
    if (eth_type == ETH_P_8021Q) {
        struct tsn_vlan_hdr *vlan = (struct tsn_vlan_hdr *)(eth + 1);
        eth_type = vlan->pid;
    }

    return eth_type == ETH_P_1588;
}

uint8_t tsn_get_vlan_prio(struct tsn_config *tsn_config, struct sk_buff *skb)
{
    struct tx_buffer *tx_buf = (struct tx_buffer *)skb->data;
    struct ethhdr *eth = (struct ethhdr *)(tx_buf->data);
    uint16_t eth_type = ntohs(eth->h_proto);
    
    if (eth_type == ETH_P_8021Q) {
        struct tsn_vlan_hdr *vlan =
            (struct tsn_vlan_hdr *)(tx_buf->data + ETH_HLEN - ETH_TLEN);
        return vlan->pcp;
    }
    return 0;
}

uint8_t tsn_get_mqprio_tc(struct net_device *netdev, uint8_t prio)
{
    if (netdev_get_num_tc(netdev) == 0) {
        return prio;
    }
    return netdev_get_prio_tc_map(netdev, prio);
}

sysclock_t tsn_timestamp_to_sysclock(struct tsnlab_spi *tc6, timestamp_t timestamp)
{
    return one_q_tsn_timestamp_to_sysclock(tc6, timestamp - TX_ADJUST_NS) - PHY_DELAY_CLOCKS;
}

uint64_t bytes_to_ns(uint64_t bytes)
{
    uint64_t link_speed = 1000000000; // Assume 1Gbps
    return max(bytes, (uint64_t)ETH_ZLEN) * 8 * NS_IN_1S / link_speed;
}

void spend_qav_credit(struct tsn_config *tsn_config, timestamp_t at, uint8_t tc_id, uint64_t bytes)
{
    uint64_t elapsed_from_last_update, sending_duration;
    double earned_credit, spending_credit;
    timestamp_t send_end;
    struct qav_state *qav = &tsn_config->qav[tc_id];

    if (qav->enabled == false) {
        return;
    }

    if (at < qav->last_update || at < qav->available_at) {
        pr_err("Invalid timestamp Qav spending");
        return;
    }

    elapsed_from_last_update = at - qav->last_update;
    earned_credit = (double)elapsed_from_last_update * qav->idle_slope;
    qav->credit += earned_credit;
    if (qav->credit > qav->hi_credit) {
        qav->credit = qav->hi_credit;
    }

    sending_duration = bytes_to_ns(bytes);
    spending_credit = (double)sending_duration * qav->send_slope;
    qav->credit += spending_credit;
    if (qav->credit < qav->lo_credit) {
        qav->credit = qav->lo_credit;
    }

    send_end = at + sending_duration;
    qav->last_update = send_end;
    if (qav->credit < 0) {
        qav->available_at = send_end + -(qav->credit / qav->idle_slope);
    } else {
        qav->available_at = send_end;
    }
}

bool get_timestamps(struct timestamps *timestamps, const struct tsn_config *tsn_config, 
                   timestamp_t from, uint8_t tc_id, uint64_t bytes, bool consider_delay)
{
    int slot_id, slot_count;
    uint64_t sending_duration, remainder;
    const struct qbv_baked_config *baked;
    const struct qbv_baked_prio *baked_prio;
    const struct qbv_config *qbv = &tsn_config->qbv;
    
    memset(timestamps, 0, sizeof(struct timestamps));

    if (qbv->enabled == false) {
        timestamps->from = from;
        timestamps->to = TSN_ALWAYS_OPEN(timestamps->from);
        timestamps->delay_from = timestamps->from;
        timestamps->delay_to = TSN_ALWAYS_OPEN(timestamps->delay_from);
        return true;
    }

    baked = &tsn_config->qbv_baked;
    baked_prio = &baked->prios[tc_id];
    sending_duration = bytes_to_ns(bytes);

    remainder = (from - qbv->start) % baked->cycle_ns;
    slot_id = 0;
    slot_count = baked_prio->slot_count;

    if (slot_count == 2 && baked_prio->slots[1].duration_ns == 0) {
        if (baked_prio->slots[0].opened == false) {
            return false;
        }
        timestamps->from = from;
        timestamps->to = TSN_ALWAYS_OPEN(timestamps->from);
        if (consider_delay) {
            timestamps->delay_from = timestamps->from;
            timestamps->delay_to = TSN_ALWAYS_OPEN(timestamps->delay_from);
        }
        return true;
    }

    while (remainder > baked_prio->slots[slot_id].duration_ns) {
        remainder -= baked_prio->slots[slot_id].duration_ns;
        slot_id += 1;
    }

    if (baked_prio->slots[slot_id].opened) {
        if (baked_prio->slots[slot_id].duration_ns - remainder < sending_duration) {
            timestamps->from = from - remainder + baked_prio->slots[slot_id].duration_ns;
            slot_id = (slot_id + 1) % baked_prio->slot_count;
            timestamps->from += baked_prio->slots[slot_id].duration_ns;
            slot_id = (slot_id + 1) % baked_prio->slot_count;
        } else {
            timestamps->from = from - remainder;
        }
    } else {
        timestamps->from = from - remainder + baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count;
    }

    timestamps->to = timestamps->from + baked_prio->slots[slot_id].duration_ns;

    if (consider_delay) {
        timestamps->delay_from = timestamps->from + baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count;
        timestamps->delay_from += baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count;
        timestamps->delay_to = timestamps->delay_from + baked_prio->slots[slot_id].duration_ns;
    }

    timestamps->from = max(timestamps->from, from);
    timestamps->to -= sending_duration;
    if (consider_delay) {
        timestamps->delay_to -= sending_duration;
    }

    return true;
}

void update_buffer(struct tsnlab_spi *tc6)
{
    if (tc6->tsn_config.buffer_space < HW_QUEUE_SIZE_PAD) {
        u64 total_pkts = one_q_tsn_get_total_new_entry_by_spi(tc6);
        u64 sent_pkts = one_q_tsn_get_total_valid_entry_by_spi(tc6);
        u64 dropped_pkts = one_q_tsn_get_total_drop_entry_by_spi(tc6);
        tc6->tsn_config.buffer_space = HW_QUEUE_SIZE - (u32)(total_pkts - sent_pkts - dropped_pkts);
    }
}

void decrease_buffer_space(struct tsnlab_spi *tc6)
{
    tc6->tsn_config.buffer_space -= 1;
}

bool is_buffer_available(struct tsnlab_spi *tc6, uint32_t min_space)
{
    update_buffer(tc6);
    return tc6->tsn_config.buffer_space > min_space;
}

void bake_qos_config(struct tsn_config *config)
{
    int slot_id, tc_id;
    bool qav_disabled = true;
    struct qbv_baked_config *baked;
    
    if (config->qbv.enabled == false) {
        for (tc_id = 0; tc_id < TC_COUNT; tc_id++) {
            if (config->qav[tc_id].enabled) {
                qav_disabled = false;
                break;
            }
        }

        if (qav_disabled) {
            config->qbv.enabled = true;
            config->qbv.start = 0;
            config->qbv.slot_count = 1;
            config->qbv.slots[0].duration_ns = 1000000000; // 1s
            for (tc_id = 0; tc_id < TC_COUNT; tc_id++) {
                config->qbv.slots[0].opened_prios[tc_id] = true;
            }
        }
    }

    baked = &config->qbv_baked;
    memset(baked, 0, sizeof(struct qbv_baked_config));
    baked->cycle_ns = 0;

    for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
        baked->prios[tc_id].slot_count = 1;
        baked->prios[tc_id].slots[0].opened = config->qbv.slots[0].opened_prios[tc_id];
    }

    for (slot_id = 0; slot_id < config->qbv.slot_count; slot_id += 1) {
        uint64_t slot_duration = config->qbv.slots[slot_id].duration_ns;
        baked->cycle_ns += slot_duration;
        for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
            struct qbv_baked_prio *prio = &baked->prios[tc_id];
            if (prio->slots[prio->slot_count - 1].opened == config->qbv.slots[slot_id].opened_prios[tc_id]) {
                prio->slots[prio->slot_count - 1].duration_ns += slot_duration;
            } else {
                prio->slots[prio->slot_count].opened = config->qbv.slots[slot_id].opened_prios[tc_id];
                prio->slots[prio->slot_count].duration_ns = slot_duration;
                prio->slot_count += 1;
            }
        }
    }

    for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
        struct qbv_baked_prio *prio = &baked->prios[tc_id];
        if (prio->slot_count % 2 == 1) {
            prio->slots[prio->slot_count].opened = !prio->slots[prio->slot_count - 1].opened;
            prio->slots[prio->slot_count].duration_ns = 0;
            prio->slot_count += 1;
        }
    }
}
```

이제 `tsn.c` 파일을 리팩토링하겠습니다:


```c
#include "tsn.h"

#include <linux/if_ether.h>
#include <linux/string.h>
#include <linux/tsnlab_spi.h>

#include "one_q_tsn_arch.h"
#include "one_q_tsn_ptp.h"

#define NS_IN_1S 1000000000

#define TSN_ALWAYS_OPEN(from) (from - 1) /* For both timestamp and sysclock */

struct timestamps {
    timestamp_t from;
    timestamp_t to;
    timestamp_t delay_from;
    timestamp_t delay_to;
};

static bool is_gptp_packet(const uint8_t* payload);
static void bake_qos_config(struct tsn_config* config);
static uint64_t bytes_to_ns(uint64_t bytes);
static void spend_qav_credit(struct tsn_config* tsn_config, timestamp_t at, uint8_t tc_id, uint64_t bytes);
static bool get_timestamps(struct timestamps* timestamps, const struct tsn_config* tsn_config, timestamp_t from,
                           uint8_t tc_id, uint64_t bytes, bool consider_delay);

static void update_buffer(struct tsnlab_spi* tc6);
static void decrease_buffer_space(struct tsnlab_spi* tc6);
static bool is_buffer_available(struct tsnlab_spi* tc6, uint32_t min_space);

static inline uint8_t tsn_get_mqprio_tc(struct net_device* netdev, uint8_t prio) {
    if (netdev_get_num_tc(netdev) == 0) {
        return prio;
    }

    return netdev_get_prio_tc_map(netdev, prio);
}

static uint8_t tsn_get_vlan_prio(struct tsn_config* tsn_config, struct sk_buff* skb) {
    struct tx_buffer* tx_buf = (struct tx_buffer*)skb->data;
    struct ethhdr* eth = (struct ethhdr*)(tx_buf->data);
    uint16_t eth_type = ntohs(eth->h_proto);
    if (eth_type == ETH_P_8021Q) {
        struct tsn_vlan_hdr* vlan =
            (struct tsn_vlan_hdr*)(tx_buf->data + ETH_HLEN - ETH_TLEN); // eth->h_proto == vlan->pid
        return vlan->pcp;
    }
    // XXX: Or you can use skb->priority;
    return 0;
}

static bool is_gptp_packet(const uint8_t* payload) {
    struct ethhdr* eth = (struct ethhdr*)payload;
    uint16_t eth_type = ntohs(eth->h_proto);
    if (eth_type == ETH_P_8021Q) {
        struct tsn_vlan_hdr* vlan = (struct tsn_vlan_hdr*)(eth + 1);
        eth_type = vlan->pid;
    }

    return eth_type == ETH_P_1588;
}

static inline sysclock_t tsn_timestamp_to_sysclock(struct tsnlab_spi* tc6, timestamp_t timestamp) {
    return one_q_tsn_timestamp_to_sysclock(tc6, timestamp - TX_ADJUST_NS) - PHY_DELAY_CLOCKS;
}

->

#include "tsn.h"
#include "tsn_utils.h"

#include <linux/if_ether.h>
#include <linux/string.h>
#include <linux/tsnlab_spi.h>

#include "one_q_tsn_arch.h"
#include "one_q_tsn_ptp.h"
```

이제 `tsn.c` 파일의 나머지 부분을 리팩토링하겠습니다:


```c
/**
 * Fill in the time related metadata of a frame
 * @param tsn_config: TSN configuration
 * @param now: Current time
 * @param tx_buf: The frame to be sent
 * @return: true if the frame reserves timestamps, false is for drop
 */
bool tsn_fill_metadata(struct tsnlab_spi* tc6, timestamp_t now, struct sk_buff* skb) {
    uint8_t vlan_prio, tc_id;
    uint64_t duration_ns;
    bool is_gptp, consider_delay;
    timestamp_t from, free_at;
    enum tsn_prio queue_prio;
    struct timestamps timestamps;
    struct tx_buffer* tx_buf = (struct tx_buffer*)skb->data;
    struct tx_metadata* metadata = (struct tx_metadata*)&tx_buf->metadata;
    struct tsn_config* tsn_config = &tc6->tsn_config;

    vlan_prio = tsn_get_vlan_prio(tsn_config, skb);
    tc_id = tsn_get_mqprio_tc(tc6->netdev, vlan_prio);
    is_gptp = is_gptp_packet(tx_buf->data);

    if (is_gptp) {
        queue_prio = TSN_PRIO_GPTP;
    } else if (vlan_prio > 0) {
        queue_prio = TSN_PRIO_VLAN;
    } else {
        queue_prio = TSN_PRIO_BE;
    }
    consider_delay = (queue_prio != TSN_PRIO_BE);

    from = now + H2C_LATENCY_NS;

    duration_ns = bytes_to_ns(metadata->frame_length);

    if (tsn_config->qbv.enabled == false && tsn_config->qav[tc_id].enabled == false) {
        // Don't care. Just fill in the metadata
        timestamps.from = tsn_config->total_available_at;
        timestamps.to = timestamps.from + _DEFAULT_TO_MARGIN_;
        metadata->fail_policy = TSN_FAIL_POLICY_DROP;
    } else {
        if (tsn_config->qav[tc_id].enabled == true && tsn_config->qav[tc_id].available_at > from) {
            from = tsn_config->qav[tc_id].available_at;
        }

        if (consider_delay) {
            if (!is_buffer_available(tc6, TSN_QUEUE_SIZE_PAD)) {
                return false;
            }
        } else {
            // Best effort
            if (!is_buffer_available(tc6, BE_QUEUE_SIZE_PAD)) {
                return false;
            }
            from = max(from, tsn_config->total_available_at);
        }

        get_timestamps(&timestamps, tsn_config, from, tc_id, metadata->frame_length, consider_delay);
        metadata->fail_policy = consider_delay ? TSN_FAIL_POLICY_RETRY : TSN_FAIL_POLICY_DROP;
    }

    metadata->from.tick = tsn_timestamp_to_sysclock(tc6, timestamps.from);
    metadata->from.priority = queue_prio;
    if (timestamps.to == TSN_ALWAYS_OPEN(timestamps.from)) {
        metadata->to.tick = TSN_ALWAYS_OPEN(metadata->from.tick);
    } else {
        metadata->to.tick = tsn_timestamp_to_sysclock(tc6, timestamps.to);
    }
    metadata->to.priority = queue_prio;
    metadata->delay_from.tick = tsn_timestamp_to_sysclock(tc6, timestamps.delay_from);
    metadata->delay_from.priority = queue_prio;
    metadata->delay_to.tick = tsn_timestamp_to_sysclock(tc6, timestamps.delay_to);
    metadata->delay_to.priority = queue_prio;

    if (tc6->tstamp_config.tx_type != HWTSTAMP_TX_ON) {
        metadata->timestamp_id = TSN_TIMESTAMP_ID_NONE;
    } else if (is_gptp) {
        metadata->timestamp_id = TSN_TIMESTAMP_ID_GPTP;
    } else {
        metadata->timestamp_id = TSN_TIMESTAMP_ID_NORMAL;
    }

    decrease_buffer_space(tc6);
    // Update available_ats
    spend_qav_credit(tsn_config, from, tc_id, metadata->frame_length);
    tsn_config->queue_available_at[queue_prio] += duration_ns;
    tsn_config->total_available_at += duration_ns;

    free_at = max(timestamps.to + duration_ns, tsn_config->total_available_at);

    return true;
}

->

/**
 * Fill in the time related metadata of a frame
 * @param tc6: TSNLab SPI structure
 * @param now: Current time
 * @param skb: Socket buffer
 * @return: true if the frame reserves timestamps, false is for drop
 */
bool tsn_fill_metadata(struct tsnlab_spi *tc6, timestamp_t now, struct sk_buff *skb)
{
    uint8_t vlan_prio, tc_id;
    uint64_t duration_ns;
    bool is_gptp, consider_delay;
    timestamp_t from, free_at;
    enum tsn_priority queue_prio;
    struct timestamps timestamps;
    struct tx_buffer *tx_buf = (struct tx_buffer *)skb->data;
    struct tx_metadata *metadata = (struct tx_metadata *)&tx_buf->metadata;
    struct tsn_config *tsn_config = &tc6->tsn_config;

    vlan_prio = tsn_get_vlan_prio(tsn_config, skb);
    tc_id = tsn_get_mqprio_tc(tc6->netdev, vlan_prio);
    is_gptp = is_gptp_packet(tx_buf->data);

    if (is_gptp) {
        queue_prio = TSN_PRIO_GPTP;
    } else if (vlan_prio > 0) {
        queue_prio = TSN_PRIO_VLAN;
    } else {
        queue_prio = TSN_PRIO_BE;
    }
    consider_delay = (queue_prio != TSN_PRIO_BE);

    from = now + H2C_LATENCY_NS;
    duration_ns = bytes_to_ns(metadata->frame_length);

    if (tsn_config->qbv.enabled == false && tsn_config->qav[tc_id].enabled == false) {
        timestamps.from = tsn_config->total_available_at;
        timestamps.to = timestamps.from + _DEFAULT_TO_MARGIN_;
        metadata->fail_policy = TSN_FAIL_POLICY_DROP;
    } else {
        if (tsn_config->qav[tc_id].enabled == true && tsn_config->qav[tc_id].available_at > from) {
            from = tsn_config->qav[tc_id].available_at;
        }

        if (consider_delay) {
            if (!is_buffer_available(tc6, TSN_QUEUE_SIZE_PAD)) {
                return false;
            }
        } else {
            if (!is_buffer_available(tc6, BE_QUEUE_SIZE_PAD)) {
                return false;
            }
            from = max(from, tsn_config->total_available_at);
        }

        get_timestamps(&timestamps, tsn_config, from, tc_id, metadata->frame_length, consider_delay);
        metadata->fail_policy = consider_delay ? TSN_FAIL_POLICY_RETRY : TSN_FAIL_POLICY_DROP;
    }

    metadata->from.tick = tsn_timestamp_to_sysclock(tc6, timestamps.from);
    metadata->from.priority = queue_prio;
    if (timestamps.to == TSN_ALWAYS_OPEN(timestamps.from)) {
        metadata->to.tick = TSN_ALWAYS_OPEN(metadata->from.tick);
    } else {
        metadata->to.tick = tsn_timestamp_to_sysclock(tc6, timestamps.to);
    }
    metadata->to.priority = queue_prio;
    metadata->delay_from.tick = tsn_timestamp_to_sysclock(tc6, timestamps.delay_from);
    metadata->delay_from.priority = queue_prio;
    metadata->delay_to.tick = tsn_timestamp_to_sysclock(tc6, timestamps.delay_to);
    metadata->delay_to.priority = queue_prio;

    if (tc6->tstamp_config.tx_type != HWTSTAMP_TX_ON) {
        metadata->timestamp_id = TSN_TIMESTAMP_ID_NONE;
    } else if (is_gptp) {
        metadata->timestamp_id = TSN_TIMESTAMP_ID_GPTP;
    } else {
        metadata->timestamp_id = TSN_TIMESTAMP_ID_NORMAL;
    }

    decrease_buffer_space(tc6);
    spend_qav_credit(tsn_config, from, tc_id, metadata->frame_length);
    tsn_config->queue_available_at[queue_prio] += duration_ns;
    tsn_config->total_available_at += duration_ns;

    free_at = max(timestamps.to + duration_ns, tsn_config->total_available_at);

    return true;
}
```

이제 `tsn.c` 파일의 나머지 함수들을 리팩토링하겠습니다:


```c
void tsn_init_configs(struct tsnlab_spi* config) {
    struct tsn_config* config = &tc6->tsn_config;
    memset(config, 0, sizeof(struct tsn_config));

    // Example Qbv configuration
    if (false) {
        config->qbv.enabled = true;
        config->qbv.start = 0;
        config->qbv.slot_count = 2;
        config->qbv.slots[0].duration_ns = 500000000; // 500ms
        config->qbv.slots[0].opened_prios[0] = true;
        config->qbv.slots[1].duration_ns = 500000000; // 500ms
        config->qbv.slots[1].opened_prios[0] = true;
    }

    // Example Qav configuration
    if (false) {
        // 100Mbps on 1Gbps link
        config->qav[0].enabled = true;
        config->qav[0].hi_credit = +1000000;
        config->qav[0].lo_credit = -1000000;
        config->qav[0].idle_slope = 10;
        config->qav[0].send_slope = -90;
    }

    bake_qos_config(config);
}

->

void tsn_init_configs(struct tsnlab_spi *tc6)
{
    struct tsn_config *config = &tc6->tsn_config;
    memset(config, 0, sizeof(struct tsn_config));

    /* Example Qbv configuration - disabled by default */
    if (false) {
        config->qbv.enabled = true;
        config->qbv.start = 0;
        config->qbv.slot_count = 2;
        config->qbv.slots[0].duration_ns = 500000000; /* 500ms */
        config->qbv.slots[0].opened_prios[0] = true;
        config->qbv.slots[1].duration_ns = 500000000; /* 500ms */
        config->qbv.slots[1].opened_prios[0] = true;
    }

    /* Example Qav configuration - disabled by default */
    if (false) {
        /* 100Mbps on 1Gbps link */
        config->qav[0].enabled = true;
        config->qav[0].hi_credit = +1000000;
        config->qav[0].lo_credit = -1000000;
        config->qav[0].idle_slope = 10;
        config->qav[0].send_slope = -90;
    }

    bake_qos_config(config);
}
```

```c
void tsn_init_configs(struct tsnlab_spi* config) {
    struct tsn_config* config = &tc6->tsn_config;
    memset(config, 0, sizeof(struct tsn_config));

    // Example Qbv configuration
    if (false) {
        config->qbv.enabled = true;
        config->qbv.start = 0;
        config->qbv.slot_count = 2;
        config->qbv.slots[0].duration_ns = 500000000; // 500ms
        config->qbv.slots[0].opened_prios[0] = true;
        config->qbv.slots[1].duration_ns = 500000000; // 500ms
        config->qbv.slots[1].opened_prios[0] = true;
    }

    // Example Qav configuration
    if (false) {
        // 100Mbps on 1Gbps link
        config->qav[0].enabled = true;
        config->qav[0].hi_credit = +1000000;
        config->qav[0].lo_credit = -1000000;
        config->qav[0].idle_slope = 10;
        config->qav[0].send_slope = -90;
    }

    bake_qos_config(config);
}

->

void tsn_init_configs(struct tsnlab_spi *tc6)
{
    struct tsn_config *config = &tc6->tsn_config;
    memset(config, 0, sizeof(struct tsn_config));

    /* Example Qbv configuration - disabled by default */
    if (false) {
        config->qbv.enabled = true;
        config->qbv.start = 0;
        config->qbv.slot_count = 2;
        config->qbv.slots[0].duration_ns = 500000000; /* 500ms */
        config->qbv.slots[0].opened_prios[0] = true;
        config->qbv.slots[1].duration_ns = 500000000; /* 500ms */
        config->qbv.slots[1].opened_prios[0] = true;
    }

    /* Example Qav configuration - disabled by default */
    if (false) {
        /* 100Mbps on 1Gbps link */
        config->qav[0].enabled = true;
        config->qav[0].hi_credit = +1000000;
        config->qav[0].lo_credit = -1000000;
        config->qav[0].idle_slope = 10;
        config->qav[0].send_slope = -90;
    }

    bake_qos_config(config);
}
```

파일의 현재 상태를 확인해보겠습니다:


이제 `tsn.c` 파일의 나머지 부분을 정리하겠습니다. 중복된 함수들을 제거하고 나머지 함수들을 리팩토링하겠습니다:


```c
void tsn_init_configs(struct tsnlab_spi* tc6) {
    struct tsn_config* config = &tc6->tsn_config;
    memset(config, 0, sizeof(struct tsn_config));

    // Example Qbv configuration
    if (false) {
        config->qbv.enabled = true;
        config->qbv.start = 0;
        config->qbv.slot_count = 2;
        config->qbv.slots[0].duration_ns = 500000000; // 500ms
        config->qbv.slots[0].opened_prios[0] = true;
        config->qbv.slots[1].duration_ns = 500000000; // 500ms
        config->qbv.slots[1].opened_prios[0] = true;
    }

    // Example Qav configuration
    if (false) {
        // 100Mbps on 1Gbps link
        config->qav[0].enabled = true;
        config->qav[0].hi_credit = +1000000;
        config->qav[0].lo_credit = -1000000;
        config->qav[0].idle_slope = 10;
        config->qav[0].send_slope = -90;
    }

    bake_qos_config(config);
}

static void bake_qos_config(struct tsn_config* config) {
    int slot_id, tc_id; // Iterators
    bool qav_disabled = true;
    struct qbv_baked_config* baked;
    if (config->qbv.enabled == false) {
        // TODO: remove this when throughput issue without QoS gets resolved
        for (tc_id = 0; tc_id < TC_COUNT; tc_id++) {
            if (config->qav[tc_id].enabled) {
                qav_disabled = false;
                break;
            }
        }

        if (qav_disabled) {
            config->qbv.enabled = true;
            config->qbv.start = 0;
            config->qbv.slot_count = 1;
            config->qbv.slots[0].duration_ns = 1000000000; // 1s
            for (tc_id = 0; tc_id < TC_COUNT; tc_id++) {
                config->qbv.slots[0].opened_prios[tc_id] = true;
            }
        }
    }

    baked = &config->qbv_baked;
    memset(baked, 0, sizeof(struct qbv_baked_config));

    baked->cycle_ns = 0;

    // First slot
    for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
        baked->prios[tc_id].slot_count = 1;
        baked->prios[tc_id].slots[0].opened = config->qbv.slots[0].opened_prios[tc_id];
    }

    for (slot_id = 0; slot_id < config->qbv.slot_count; slot_id += 1) {
        uint64_t slot_duration = config->qbv.slots[slot_id].duration_ns;
        baked->cycle_ns += slot_duration;
        for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
            struct qbv_baked_prio* prio = &baked->prios[tc_id];
            if (prio->slots[prio->slot_count - 1].opened == config->qbv.slots[slot_id].opened_prios[tc_id]) {
                // Same as the last slot. Just increase the duration
                prio->slots[prio->slot_count - 1].duration_ns += slot_duration;
            } else {
                // Different from the last slot. Add a new slot
                prio->slots[prio->slot_count].opened = config->qbv.slots[slot_id].opened_prios[tc_id];
                prio->slots[prio->slot_count].duration_ns = slot_duration;
                prio->slot_count += 1;
            }
        }
    }

    // Adjust slot counts to be even number. Because we need to have open-close pairs
    for (tc_id = 0; tc_id < TC_COUNT; tc_id += 1) {
        struct qbv_baked_prio* prio = &baked->prios[tc_id];
        if (prio->slot_count % 2 == 1) {
            prio->slots[prio->slot_count].opened = !prio->slots[prio->slot_count - 1].opened;
            prio->slots[prio->slot_count].duration_ns = 0;
            prio->slot_count += 1;
        }
    }
}

static uint64_t bytes_to_ns(uint64_t bytes) {
    // TODO: Get link speed
    uint64_t link_speed = 1000000000; // Assume 1Gbps
    return max(bytes, (uint64_t)ETH_ZLEN) * 8 * NS_IN_1S / link_speed;
}

static void spend_qav_credit(struct tsn_config* tsn_config, timestamp_t at, uint8_t tc_id, uint64_t bytes) {
    uint64_t elapsed_from_last_update, sending_duration;
    double earned_credit, spending_credit;
    timestamp_t send_end;
    struct qav_state* qav = &tsn_config->qav[tc_id];

    if (qav->enabled == false) {
        return;
    }

    if (at < qav->last_update || at < qav->available_at) {
        // Invalid
        pr_err("Invalid timestamp Qav spending");
        return;
    }

    elapsed_from_last_update = at - qav->last_update;
    earned_credit = (double)elapsed_from_last_update * qav->idle_slope;
    qav->credit += earned_credit;
    if (qav->credit > qav->hi_credit) {
        qav->credit = qav->hi_credit;
    }

    sending_duration = bytes_to_ns(bytes);
    spending_credit = (double)sending_duration * qav->send_slope;
    qav->credit += spending_credit;
    if (qav->credit < qav->lo_credit) {
        qav->credit = qav->lo_credit;
    }

    // Calulate next available time
    send_end = at + sending_duration;
    qav->last_update = send_end;
    if (qav->credit < 0) {
        qav->available_at = send_end + -(qav->credit / qav->idle_slope);
    } else {
        qav->available_at = send_end;
    }
}

/**
 * Get timestamps for a frame based on Qbv configuration
 * @param timestamps: Output timestamps
 * @param tsn_config: TSN configuration
 * @param from: The time when the frame is ready to be sent
 * @param tc_id: ID of mqprio tc or VLAN priority of the frame
 * @param bytes: Size of the frame
 * @param consider_delay: If true, calculate delay_from and delay_to
 * @return: true if the frame reserves timestamps, false is for drop
 */
static bool get_timestamps(struct timestamps* timestamps, const struct tsn_config* tsn_config, timestamp_t from,
                           uint8_t tc_id, uint64_t bytes, bool consider_delay) {
    int slot_id, slot_count;
    uint64_t sending_duration, remainder;
    const struct qbv_baked_config* baked;
    const struct qbv_baked_prio* baked_prio;
    const struct qbv_config* qbv = &tsn_config->qbv;
    memset(timestamps, 0, sizeof(struct timestamps));

    if (qbv->enabled == false) {
        // No Qbv. Just return the current time
        timestamps->from = from;
        timestamps->to = TSN_ALWAYS_OPEN(timestamps->from);
        // delay_* is pointless. Just set it to be right next to the frame
        timestamps->delay_from = timestamps->from;
        timestamps->delay_to = TSN_ALWAYS_OPEN(timestamps->delay_from);
        return true;
    }

    baked = &tsn_config->qbv_baked;
    baked_prio = &baked->prios[tc_id];
    sending_duration = bytes_to_ns(bytes);

    // TODO: Need to check if the slot is big enough to fit the frame. But, That is a user fault. Don't mind for now
    // But we still have to check if the first current slot's remaining time is enough to fit the frame

    remainder = (from - qbv->start) % baked->cycle_ns;
    slot_id = 0;
    slot_count = baked_prio->slot_count;

    // Check if tc_id is always open or always closed
    if (slot_count == 2 && baked_prio->slots[1].duration_ns == 0) {
        if (baked_prio->slots[0].opened == false) {
            // The only slot is closed. Drop the frame
            return false;
        }
        timestamps->from = from;
        timestamps->to = TSN_ALWAYS_OPEN(timestamps->from);
        if (consider_delay) {
            timestamps->delay_from = timestamps->from;
            timestamps->delay_to = TSN_ALWAYS_OPEN(timestamps->delay_from);
        }
        return true;
    }

    while (remainder > baked_prio->slots[slot_id].duration_ns) {
        remainder -= baked_prio->slots[slot_id].duration_ns;
        slot_id += 1;
    }

    // 1. "from"
    if (baked_prio->slots[slot_id].opened) {
        // Skip the slot if its remaining time is not enough to fit the frame
        if (baked_prio->slots[slot_id].duration_ns - remainder < sending_duration) {
            // Skip this slot. Because the slots are on/off pairs, we skip 2 slots
            // First
            timestamps->from = from - remainder + baked_prio->slots[slot_id].duration_ns;
            slot_id = (slot_id + 1) % baked_prio->slot_count;
            // Second
            timestamps->from += baked_prio->slots[slot_id].duration_ns;
            slot_id = (slot_id + 1) % baked_prio->slot_count;
        } else {
            // The slot's remaining time is enough to fit the frame
            timestamps->from = from - remainder;
        }
    } else {
        // Select next slot
        timestamps->from = from - remainder + baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count; // Opened slot
    }

    // 2. "to"
    timestamps->to = timestamps->from + baked_prio->slots[slot_id].duration_ns;

    if (consider_delay) {
        // 3. "delay_from"
        timestamps->delay_from = timestamps->from + baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count; // Closed slot
        timestamps->delay_from += baked_prio->slots[slot_id].duration_ns;
        slot_id = (slot_id + 1) % baked_prio->slot_count; // Opened slot
        // 4. "delay_to"
        timestamps->delay_to = timestamps->delay_from + baked_prio->slots[slot_id].duration_ns;
    }

    // Adjust times
    timestamps->from = max(timestamps->from, from); // If already in the slot
    timestamps->to -= sending_duration;
    if (consider_delay) {
        timestamps->delay_to -= sending_duration;
    }

#ifdef __LIBXDMA_DEBUG__
    // Assert the timestamps
    if (timestamps->from >= timestamps->to) {
        pr_err("Invalid timestamps, from >= to, %llu >= %llu", timestamps->from, timestamps->to);
    }

    if (consider_delay) {
        if (timestamps->delay_from >= timestamps->delay_to) {
            pr_err("Invalid timestamps, delay_from >= delay_to, %llu >= %llu", timestamps->delay_from,
                   timestamps->delay_to);
        }

        if (timestamps->to >= timestamps->delay_from) {
            pr_err("Invalid timestamps, to >= delay_from, %llu >= %llu", timestamps->to, timestamps->delay_from);
        }
    }
#endif // __LIBXDMA_DEBUG__

    return true;
}

static void update_buffer(struct tsnlab_spi* tc6) {
    if (tc6->tsn_config.buffer_space < HW_QUEUE_SIZE_PAD) {
        u64 total_pkts = one_q_tsn_get_total_new_entry_by_spi(tc6);
        u64 sent_pkts = one_q_tsn_get_total_valid_entry_by_spi(tc6);
        u64 dropped_pkts = one_q_tsn_get_total_drop_entry_by_spi(tc6);
        tc6->tsn_config.buffer_space = HW_QUEUE_SIZE - (u32)(total_pkts - sent_pkts - dropped_pkts);
    }
}

static void decrease_buffer_space(struct tsnlab_spi* tc6) {
    tc6->tsn_config.buffer_space -= 1;
}

static bool is_buffer_available(struct tsnlab_spi* tc6, uint32_t min_space) {
    update_buffer(tc6);
    return tc6->tsn_config.buffer_space > min_space;
}

->

void tsn_init_configs(struct tsnlab_spi *tc6)
{
    struct tsn_config *config = &tc6->tsn_config;
    memset(config, 0, sizeof(struct tsn_config));

    /* Example Qbv configuration - disabled by default */
    if (false) {
        config->qbv.enabled = true;
        config->qbv.start = 0;
        config->qbv.slot_count = 2;
        config->qbv.slots[0].duration_ns = 500000000; /* 500ms */
        config->qbv.slots[0].opened_prios[0] = true;
        config->qbv.slots[1].duration_ns = 500000000; /* 500ms */
        config->qbv.slots[1].opened_prios[0] = true;
    }

    /* Example Qav configuration - disabled by default */
    if (false) {
        /* 100Mbps on 1Gbps link */
        config->qav[0].enabled = true;
        config->qav[0].hi_credit = +1000000;
        config->qav[0].lo_credit = -1000000;
        config->qav[0].idle_slope = 10;
        config->qav[0].send_slope = -90;
    }

    bake_qos_config(config);
}
```

이제 `tsn.c` 파일의 나머지 함수들을 리팩토링하겠습니다:


```c
int tsn_set_mqprio(struct tsnlab_spi* tc6, struct tc_mqprio_qopt_offload* offload) {
    u8 i;
    int ret;
    struct tc_mqprio_qopt qopt = offload->qopt;

    if (offload->mode != TC_MQPRIO_MODE_DCB) {
        return -ENOTSUPP;
    }

    if (qopt.num_tc >= TC_QOPT_MAX_QUEUE) {
        pr_err("Invalid number of tc\n");
        return -EINVAL;
    }

    if ((ret = netdev_set_num_tc(tc6->netdev, qopt.num_tc)) < 0) {
        pr_err("Failed to set num_tc\n");
        return ret;
    }

    if (qopt.num_tc == 0) {
        // No need to proceed further
        return 0;
    }

    for (i = 0; i < qopt.num_tc; i++) {
        if (netdev_set_tc_queue(tc6->netdev, i, qopt.count[i], qopt.offset[i]) < 0) {
            pr_warn("Failed to set tc queue: tc [%u], queue [%u@%u]\n", i, qopt.count[i], qopt.offset[i]);
        }
    }

    for (i = 0; i < TC_QOPT_BITMASK; i++) {
        if (netdev_set_prio_tc_map(tc6->netdev, i, qopt.prio_tc_map[i]) < 0) {
            pr_warn("Failed to set tc map: prio [%u], tc [%d]\n", i, qopt.prio_tc_map[i]);
        }
    }

    return 0;
}

int tsn_set_qav(struct tsnlab_spi* tc6, struct tc_cbs_qopt_offload* offload) {
    struct tsn_config* config = &tc6->tsn_config;
    if (offload->queue < 0 || offload->queue >= TC_COUNT) {
        return -EINVAL;
    }

    config->qav[offload->queue].enabled = offload->enable;
    config->qav[offload->queue].hi_credit = offload->hicredit * 1000;
    config->qav[offload->queue].lo_credit = offload->locredit * 1000;
    config->qav[offload->queue].idle_slope = offload->idleslope / 1000;
    config->qav[offload->queue].send_slope = offload->sendslope / 1000;

    bake_qos_config(config);

    return 0;
}

int tsn_set_qbv(struct tsnlab_spi* tc6, struct tc_taprio_qopt_offload* offload) {
    u32 i, j;
    struct tsn_config* config = &tc6->tsn_config;
    bool enabled = false;

    if (offload->num_entries > MAX_QBV_SLOTS) {
        return -EINVAL;
    }

#if 1 // POOKY 20250703 LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    switch (offload->cmd) {
    case TAPRIO_CMD_REPLACE:
        config->qbv.enabled = true;
        enabled = true;
        break;
    case TAPRIO_CMD_DESTROY:
        config->qbv.enabled = false;
        break;
    default:
        /* TAPRIO_CMD_STATS */
        /* TAPRIO_CMD_QUEUE_STATS */
        return -EOPNOTSUPP;
    }
#else
    config->qbv.enabled = offload->enable;
    enabled = config->qbv.enabled;
#endif

    if (enabled) {
        config->qbv.start = offload->base_time;
        config->qbv.slot_count = offload->num_entries;

        for (i = 0; i < config->qbv.slot_count; i++) {
            // TODO: handle offload->entries[i].command
            config->qbv.slots[i].duration_ns = offload->entries[i].interval;
            for (j = 0; j < TC_COUNT; j++) {
                config->qbv.slots[i].opened_prios[j] = (offload->entries[i].gate_mask & (1 << j));
            }
        }
    }

    bake_qos_config(config);

    return 0;
}

->

int tsn_set_mqprio(struct tsnlab_spi *tc6, struct tc_mqprio_qopt_offload *offload)
{
    u8 i;
    int ret;
    struct tc_mqprio_qopt qopt = offload->qopt;

    if (offload->mode != TC_MQPRIO_MODE_DCB) {
        return -ENOTSUPP;
    }

    if (qopt.num_tc >= TC_QOPT_MAX_QUEUE) {
        pr_err("Invalid number of tc\n");
        return -EINVAL;
    }

    ret = netdev_set_num_tc(tc6->netdev, qopt.num_tc);
    if (ret < 0) {
        pr_err("Failed to set num_tc\n");
        return ret;
    }

    if (qopt.num_tc == 0) {
        return 0;
    }

    for (i = 0; i < qopt.num_tc; i++) {
        if (netdev_set_tc_queue(tc6->netdev, i, qopt.count[i], qopt.offset[i]) < 0) {
            pr_warn("Failed to set tc queue: tc [%u], queue [%u@%u]\n", i, qopt.count[i], qopt.offset[i]);
        }
    }

    for (i = 0; i < TC_QOPT_BITMASK; i++) {
        if (netdev_set_prio_tc_map(tc6->netdev, i, qopt.prio_tc_map[i]) < 0) {
            pr_warn("Failed to set tc map: prio [%u], tc [%d]\n", i, qopt.prio_tc_map[i]);
        }
    }

    return 0;
}

int tsn_set_qav(struct tsnlab_spi *tc6, struct tc_cbs_qopt_offload *offload)
{
    struct tsn_config *config = &tc6->tsn_config;
    
    if (offload->queue < 0 || offload->queue >= TC_COUNT) {
        return -EINVAL;
    }

    config->qav[offload->queue].enabled = offload->enable;
    config->qav[offload->queue].hi_credit = offload->hicredit * 1000;
    config->qav[offload->queue].lo_credit = offload->locredit * 1000;
    config->qav[offload->queue].idle_slope = offload->idleslope / 1000;
    config->qav[offload->queue].send_slope = offload->sendslope / 1000;

    bake_qos_config(config);

    return 0;
}

int tsn_set_qbv(struct tsnlab_spi *tc6, struct tc_taprio_qopt_offload *offload)
{
    u32 i, j;
    struct tsn_config *config = &tc6->tsn_config;
    bool enabled = false;

    if (offload->num_entries > MAX_QBV_SLOTS) {
        return -EINVAL;
    }

    switch (offload->cmd) {
    case TAPRIO_CMD_REPLACE:
        config->qbv.enabled = true;
        enabled = true;
        break;
    case TAPRIO_CMD_DESTROY:
        config->qbv.enabled = false;
        break;
    default:
        return -EOPNOTSUPP;
    }

    if (enabled) {
        config->qbv.start = offload->base_time;
        config->qbv.slot_count = offload->num_entries;

        for (i = 0; i < config->qbv.slot_count; i++) {
            config->qbv.slots[i].duration_ns = offload->entries[i].interval;
            for (j = 0; j < TC_COUNT; j++) {
                config->qbv.slots[i].opened_prios[j] = (offload->entries[i].gate_mask & (1 << j));
            }
        }
    }

    bake_qos_config(config);

    return 0;
}
```

이제 `lan1qtsn.c` 파일을 리팩토링하겠습니다. 먼저 새로운 헤더 파일들을 포함하도록 수정하겠습니다:


```c
// SPDX-License-Identifier: GPL-2.0+
/*
 * TSNLab's LAN1QTSN MAC-PHY driver
 *
 * Author: HounJoung Rim <hounjoung@tsnlab.com>
 */

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/tsnlab_spi.h>

#include "lan1qtsn_ioctl.h"
#include "one_q_tsn_ptp.h"

#define DRV_NAME "onequeue"

/* MAC Network Control Register */
#define LAN865X_REG_MAC_NET_CTL 0x00010000
#define MAC_NET_CTL_TXEN BIT(3) /* Transmit Enable */
#define MAC_NET_CTL_RXEN BIT(2) /* Receive Enable */

/* MAC Network Configuration Reg */
#define LAN865X_REG_MAC_NET_CFG 0x00010001
#define MAC_NET_CFG_PROMISCUOUS_MODE BIT(4)
#define MAC_NET_CFG_MULTICAST_MODE BIT(6)
#define MAC_NET_CFG_UNICAST_MODE BIT(7)

/* MAC Hash Register Bottom */
#define LAN865X_REG_MAC_L_HASH 0x00010020
/* MAC Hash Register Top */
#define LAN865X_REG_MAC_H_HASH 0x00010021
/* MAC Specific Addr 1 Bottom Reg */
#define LAN865X_REG_MAC_L_SADDR1 0x00010022
/* MAC Specific Addr 1 Top Reg */
#define LAN865X_REG_MAC_H_SADDR1 0x00010023

->

// SPDX-License-Identifier: GPL-2.0+
/*
 * TSNLab's LAN1QTSN MAC-PHY driver
 *
 * Author: HounJoung Rim <hounjoung@tsnlab.com>
 */

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/tsnlab_spi.h>

#include "lan1qtsn_constants.h"
#include "lan1qtsn_ioctl.h"
#include "lan1qtsn_mac.h"
#include "lan1qtsn_multicast.h"
#include "lan1qtsn_registers.h"
#include "one_q_tsn_ptp.h"
```


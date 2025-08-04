# 이더넷 프레임 FCS 계산 및 수정
_Exported on 7/29/2025 at 16:52:52 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후 사용할 예정이야.

lan1qtsn_send_packet 함수를 통해 이더넷 패킷을 전송하는데 spi를 통해 이더넷 패킷을 전송하기 전에
tx metadata를 제외한 순수 이더넷 프레임에 대한 FCS를 계산하여 프레임의 맨 뒤에 append 해주고,
길이가 4바이트 만큼 늘어나는 부분에 대해서도 고려해서 다른 변수(파라미터)들도 같이 수정해 줘


---

**Cursor**

먼저 현재 코드 구조를 파악하고 `lan1qtsn_send_packet` 함수를 찾아보겠습니다.


이제 `tsnlab_spi_start_xmit` 함수를 더 자세히 살펴보겠습니다.


이제 `tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs` 함수를 더 자세히 살펴보겠습니다. 이 함수가 실제로 SPI를 통해 패킷을 전송하는 부분입니다.


이제 FCS 계산을 위한 CRC32 함수가 있는지 확인해보겠습니다.


이제 FCS 계산을 위한 함수를 찾아보겠습니다.


이제 이더넷 프레임 구조와 FCS 계산을 위한 헤더 파일들을 확인해보겠습니다.


이제 이더넷 헤더와 관련된 정의들을 확인해보겠습니다.


이제 현재 코드의 구조를 파악했습니다. `tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs` 함수에서 실제로 SPI를 통해 패킷을 전송하는 부분을 수정해야 합니다. 

요구사항에 따라:
1. tx metadata를 제외한 순수 이더넷 프레임에 대한 FCS를 계산
2. 계산된 FCS를 프레임의 맨 뒤에 append
3. 길이가 4바이트 늘어나는 부분을 고려하여 다른 변수들도 수정

이제 수정을 진행하겠습니다:


```c
static u16 tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs(struct tsnlab_spi* tc6) {
    u16 len;
    u8* tx_skb_data;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u16 length_to_copy;
    u32 fcs;
    u8* eth_frame_start;
    u16 eth_frame_len;

    spin_lock_bh(&tc6->tx_skb_lock);
    tc6->ongoing_tx_skb = tc6->waiting_tx_skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    length_to_copy = tc6->ongoing_tx_skb->len;
    tx_skb_data = tc6->ongoing_tx_skb->data;
    
    /* Copy the original frame data to SPI buffer */
    memcpy(&tx_buf[TSNLAB_SPI_DATA_HEADER_SIZE], tx_skb_data, length_to_copy);
    
    /* Calculate FCS for the pure Ethernet frame (excluding tx metadata) */
    eth_frame_start = tx_skb_data + TX_METADATA_SIZE; /* Skip tx metadata */
    eth_frame_len = length_to_copy - TX_METADATA_SIZE;
    
    /* Calculate CRC32 for the Ethernet frame */
    fcs = crc32_le(~0, eth_frame_start, eth_frame_len);
    fcs = ~fcs; /* Final XOR */
    
    /* Append FCS to the frame */
    memcpy(&tx_buf[TSNLAB_SPI_DATA_HEADER_SIZE + length_to_copy], &fcs, ETH_FCS_LEN);
    
    /* Update total length including FCS */
    len = length_to_copy + ETH_FCS_LEN + TSNLAB_SPI_DATA_HEADER_SIZE;

    if (len > TSNLAB_SPI_SPI_DATA_BUF_SIZE) {
        kfree_skb(tc6->ongoing_tx_skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    tc6->netdev->stats.tx_bytes += tc6->ongoing_tx_skb->len + ETH_FCS_LEN;
    tc6->netdev->stats.tx_packets++;
    kfree_skb(tc6->ongoing_tx_skb);
    tc6->ongoing_tx_skb = NULL;

    tsnlab_spi_fill_mpw_header(tx_buf, TSNLAB_SPI_CMD_WRITE, TSNLAB_SPI_TX_FRAME_FIFO_BASE);

    return len;
}
```

파일을 다시 읽어서 정확한 위치를 확인하겠습니다.


이제 정확한 함수를 수정하겠습니다. 먼저 필요한 헤더 파일을 추가해야 합니다.


이제 CRC32 헤더를 추가하고 함수를 수정하겠습니다.


```c
#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/ptp_classify.h>
#include <linux/tsnlab_spi.h>

->

#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/ptp_classify.h>
#include <linux/tsnlab_spi.h>
#include <linux/crc32.h>
```

이제 `tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs` 함수를 수정하겠습니다.


```c
static u16 tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs(struct tsnlab_spi* tc6) {
    u16 len;
    u8* tx_skb_data;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u16 length_to_copy;

    spin_lock_bh(&tc6->tx_skb_lock);
    tc6->ongoing_tx_skb = tc6->waiting_tx_skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    len = tc6->ongoing_tx_skb->len + TSNLAB_SPI_DATA_HEADER_SIZE;

    if (len > TSNLAB_SPI_SPI_DATA_BUF_SIZE) {
        kfree_skb(tc6->ongoing_tx_skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    length_to_copy = tc6->ongoing_tx_skb->len;
    tx_skb_data = tc6->ongoing_tx_skb->data;
    memcpy(&tx_buf[TSNLAB_SPI_DATA_HEADER_SIZE], tx_skb_data, length_to_copy);

    tc6->netdev->stats.tx_bytes += tc6->ongoing_tx_skb->len;
    tc6->netdev->stats.tx_packets++;
    kfree_skb(tc6->ongoing_tx_skb);
    tc6->ongoing_tx_skb = NULL;

    tsnlab_spi_fill_mpw_header(tx_buf, TSNLAB_SPI_CMD_WRITE, TSNLAB_SPI_TX_FRAME_FIFO_BASE);

    return len;
}

->

static u16 tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs(struct tsnlab_spi* tc6) {
    u16 len;
    u8* tx_skb_data;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u16 length_to_copy;
    u32 fcs;
    u8* eth_frame_start;
    u16 eth_frame_len;

    spin_lock_bh(&tc6->tx_skb_lock);
    tc6->ongoing_tx_skb = tc6->waiting_tx_skb;
    tc6->waiting_tx_skb = NULL;
    spin_unlock_bh(&tc6->tx_skb_lock);

    length_to_copy = tc6->ongoing_tx_skb->len;
    tx_skb_data = tc6->ongoing_tx_skb->data;
    
    /* Copy the original frame data to SPI buffer */
    memcpy(&tx_buf[TSNLAB_SPI_DATA_HEADER_SIZE], tx_skb_data, length_to_copy);
    
    /* Calculate FCS for the pure Ethernet frame (excluding tx metadata) */
    eth_frame_start = tx_skb_data + TX_METADATA_SIZE; /* Skip tx metadata */
    eth_frame_len = length_to_copy - TX_METADATA_SIZE;
    
    /* Calculate CRC32 for the Ethernet frame */
    fcs = crc32_le(~0, eth_frame_start, eth_frame_len);
    fcs = ~fcs; /* Final XOR */
    
    /* Append FCS to the frame */
    memcpy(&tx_buf[TSNLAB_SPI_DATA_HEADER_SIZE + length_to_copy], &fcs, ETH_FCS_LEN);
    
    /* Update total length including FCS */
    len = length_to_copy + ETH_FCS_LEN + TSNLAB_SPI_DATA_HEADER_SIZE;

    if (len > TSNLAB_SPI_SPI_DATA_BUF_SIZE) {
        kfree_skb(tc6->ongoing_tx_skb);
        tc6->ongoing_tx_skb = NULL;
        return 0;
    }

    tc6->netdev->stats.tx_bytes += tc6->ongoing_tx_skb->len + ETH_FCS_LEN;
    tc6->netdev->stats.tx_packets++;
    kfree_skb(tc6->ongoing_tx_skb);
    tc6->ongoing_tx_skb = NULL;

    tsnlab_spi_fill_mpw_header(tx_buf, TSNLAB_SPI_CMD_WRITE, TSNLAB_SPI_TX_FRAME_FIFO_BASE);

    return len;
}
```

이제 `tsnlab_spi_start_xmit` 함수에서도 frame_length를 FCS를 포함하도록 수정해야 합니다.


이제 `tsnlab_spi_start_xmit` 함수에서 frame_length를 FCS를 포함하도록 수정하겠습니다.


```c
    frame_length = skb->len;

->

    frame_length = skb->len + ETH_FCS_LEN; /* Include FCS length */
```

이제 jumbo frame 체크도 FCS를 포함하도록 수정해야 합니다.


```c
    /* Jumbo frames not supported */
    if (skb->len > TSNLAB_SPI_CHUNK_PAYLOAD_SIZE) {
        pr_err("Jumbo frames not supported\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return NETDEV_TX_OK;
    }

->

    /* Jumbo frames not supported (including FCS) */
    if (skb->len + ETH_FCS_LEN > TSNLAB_SPI_CHUNK_PAYLOAD_SIZE) {
        pr_err("Jumbo frames not supported\n");
        netif_wake_subqueue(ndev, q);
        dev_kfree_skb(skb);
        return NETDEV_TX_OK;
    }
```

이제 `tsn.c` 파일에서도 FCS를 고려한 길이 계산을 수정해야 합니다. `tsn_fill_metadata` 함수에서 사용되는 frame_length를 확인해보겠습니다.


`tsn_fill_metadata` 함수에서 `metadata->frame_length`를 사용하고 있습니다. 이 값은 이미 FCS를 포함하도록 수정되었으므로, `tsn.c`에서는 추가 수정이 필요하지 않습니다.

이제 수정사항을 확인해보겠습니다. 변경된 내용을 요약하면:

1. **CRC32 헤더 추가**: `#include <linux/crc32.h>` 추가
2. **FCS 계산 및 추가**: `tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs` 함수에서:
   - tx metadata를 제외한 순수 이더넷 프레임에 대해 CRC32 계산
   - 계산된 FCS를 프레임 뒤에 append
   - 길이 변수들을 FCS 4바이트 추가를 고려하여 수정
3. **frame_length 수정**: `tsnlab_spi_start_xmit` 함수에서 frame_length에 FCS 길이 포함
4. **Jumbo frame 체크 수정**: FCS를 포함한 길이로 체크하도록 수정

수정이 완료되었습니다. 이제 코드가 다음과 같이 동작합니다:

1. `lan1qtsn_send_packet` → `tsnlab_spi_start_xmit` → `tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs` 순서로 패킷이 전송됩니다.
2. `tsnlab_spi_prepare_spi_tx_buf_for_tx_skbs` 함수에서:
   - 원본 프레임 데이터를 SPI 버퍼에 복사
   - tx metadata를 제외한 순수 이더넷 프레임에 대해 CRC32 계산
   - 계산된 FCS를 프레임 뒤에 추가
   - 총 길이에 FCS 4바이트 포함하여 반환
3. 모든 관련 변수들(frame_length, tx_bytes 등)이 FCS 길이를 고려하여 수정되었습니다.

이 수정으로 인해 SPI를 통해 전송되는 이더넷 프레임에는 올바른 FCS가 포함되어 하드웨어에서 정상적으로 처리될 수 있습니다.


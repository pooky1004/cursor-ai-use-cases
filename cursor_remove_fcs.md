# 커널 이미지 수정 및 FCS 제거 요청
_Exported on 8/4/2025 at 08:30:36 GMT+9 from Cursor (1.2.4)_

---

**User**

이 코드들을 라즈베리 커널소스에 넣고 라즈베리 파이 4로 "Natively build a kernel"를 한 후
만들어진 커널 이미지로 부팅하여 사용하는 것이다.
SPI를 통해 패킷이 수신될 때 FCS까지 포함되어 올라오는데 네트워크 스택으로 올릴 때
FCS를 제거해서 올리도록 코드를 수정해 주세요.

---

**Cursor**

네트워크 스택으로 올릴 때 FCS(Frame Check Sequence)를 제거하는 코드를 수정해드리겠습니다. 먼저 현재 코드를 살펴보겠습니다.


이제 헤더 파일도 확인해보겠습니다.


이제 네트워크 디바이스 헤더 파일도 확인해보겠습니다.


이제 코드를 분석해보니, FCS(Frame Check Sequence)가 포함된 패킷이 수신될 때 네트워크 스택으로 올리기 전에 FCS를 제거해야 합니다. 현재 코드에서 `lan1qtsn_spi_process_spi_data_rx_buf` 함수에서 패킷을 처리하고 있습니다.

FCS는 이더넷 프레임의 마지막 4바이트에 위치하며, 네트워크 스택으로 올릴 때는 제거되어야 합니다. 코드를 수정하겠습니다.


```c
static int lan1qtsn_spi_process_spi_data_rx_buf(struct lan1qtsn_spi* tc6, struct rx_metadata meta) {
    int ret;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u8 cmd;
    u16 addr;
    u16 size;
    u16 payload_size;
    u8* payload = tc6->spi_data_rx_buf + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    // Validate input parameters
    if (!tc6 || !tx_buf || !tc6->spi_data_rx_buf) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

    netdev_err(tc6->netdev, "%s - meta.frame_length: %d\n", __func__, meta.frame_length);

    cmd = LAN1QTSN_SPI_CMD_READ;
    addr = LAN1QTSN_SPI_RX_FRAME_FIFO_BASE;

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    // meta.frame_length is the actual payload size, not including headers
    payload_size = meta.frame_length;
    size = payload_size + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    // Check for buffer overflow
    if (size > LAN1QTSN_SPI_DATA_BUF_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow detected: size=%d, max=%d\n", size, LAN1QTSN_SPI_DATA_BUF_SIZE);
        return -EINVAL;
    }

    // Validate payload pointer
    if (!payload) {
        dev_err(&tc6->spi->dev, "Invalid payload pointer in %s\n", __func__);
        return -EINVAL;
    }

    /* Perform SPI transfer */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    return lan1qtsn_spi_prcs_complete_rx_frame(tc6, payload, payload_size, meta);
}

->

static int lan1qtsn_spi_process_spi_data_rx_buf(struct lan1qtsn_spi* tc6, struct rx_metadata meta) {
    int ret;
    u8* tx_buf = tc6->spi_data_tx_buf;
    u8 cmd;
    u16 addr;
    u16 size;
    u16 payload_size;
    u16 frame_size_without_fcs;
    u8* payload = tc6->spi_data_rx_buf + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    // Validate input parameters
    if (!tc6 || !tx_buf || !tc6->spi_data_rx_buf) {
        dev_err(&tc6->spi->dev, "Invalid parameters in %s\n", __func__);
        return -EINVAL;
    }

    netdev_err(tc6->netdev, "%s - meta.frame_length: %d\n", __func__, meta.frame_length);

    cmd = LAN1QTSN_SPI_CMD_READ;
    addr = LAN1QTSN_SPI_RX_FRAME_FIFO_BASE;

    lan1qtsn_spi_prepare_cmd_header(tx_buf, cmd, addr);

    // meta.frame_length is the actual payload size, not including headers
    payload_size = meta.frame_length;
    size = payload_size + LAN1QTSN_SPI_CMD_SIZE + LAN1QTSN_SPI_RD_CMD_GAP_SIZE;

    // Check for buffer overflow
    if (size > LAN1QTSN_SPI_DATA_BUF_SIZE) {
        dev_err(&tc6->spi->dev, "Buffer overflow detected: size=%d, max=%d\n", size, LAN1QTSN_SPI_DATA_BUF_SIZE);
        return -EINVAL;
    }

    // Validate payload pointer
    if (!payload) {
        dev_err(&tc6->spi->dev, "Invalid payload pointer in %s\n", __func__);
        return -EINVAL;
    }

    /* Perform SPI transfer */
    ret = lan1qtsn_spi_transfer(tc6, LAN1QTSN_SPI_DATA_HEADER, size);
    if (ret) {
        dev_err(&tc6->spi->dev, "SPI transfer failed for control: %d\n", ret);
        return ret;
    }

    // Calculate frame size without FCS (remove last 4 bytes)
    if (payload_size >= ETH_FCS_LEN) {
        frame_size_without_fcs = payload_size - ETH_FCS_LEN;
    } else {
        dev_err(&tc6->spi->dev, "Payload size too small for FCS removal: %d\n", payload_size);
        return -EINVAL;
    }

    return lan1qtsn_spi_prcs_complete_rx_frame(tc6, payload, frame_size_without_fcs, meta);
}
```

이제 `lan1qtsn_spi_submit_rx_skb` 함수에서도 통계 업데이트를 수정해야 합니다. FCS가 제거되었으므로 바이트 수를 올바르게 계산해야 합니다.


```c
static void lan1qtsn_spi_submit_rx_skb(struct lan1qtsn_spi* tc6) {
    tc6->rx_skb->protocol = eth_type_trans(tc6->rx_skb, tc6->netdev);
    tc6->netdev->stats.rx_packets++;
    tc6->netdev->stats.rx_bytes += tc6->rx_skb->len;

    // Submit the packet to the network stack
    netif_rx(tc6->rx_skb);
    
    // Wake up the network queue if it was stopped
    if (netif_queue_stopped(tc6->netdev)) {
        netif_wake_queue(tc6->netdev);
    }

    tc6->rx_skb = NULL;
}

->

static void lan1qtsn_spi_submit_rx_skb(struct lan1qtsn_spi* tc6) {
    tc6->rx_skb->protocol = eth_type_trans(tc6->rx_skb, tc6->netdev);
    tc6->netdev->stats.rx_packets++;
    // Note: rx_skb->len already excludes FCS since we removed it in process_spi_data_rx_buf
    tc6->netdev->stats.rx_bytes += tc6->rx_skb->len;

    // Submit the packet to the network stack
    netif_rx(tc6->rx_skb);
    
    // Wake up the network queue if it was stopped
    if (netif_queue_stopped(tc6->netdev)) {
        netif_wake_queue(tc6->netdev);
    }

    tc6->rx_skb = NULL;
}
```

이제 `lan1qtsn_spi_allocate_rx_skb` 함수에서도 FCS를 고려하여 버퍼 크기를 조정해야 합니다. FCS가 제거되므로 버퍼 크기를 줄일 수 있습니다.


```c
static int lan1qtsn_spi_allocate_rx_skb(struct lan1qtsn_spi* tc6) {
    tc6->rx_skb = netdev_alloc_skb_ip_align(tc6->netdev, tc6->netdev->mtu + ETH_HLEN + ETH_FCS_LEN);
    if (!tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        return -ENOMEM;
    }

    return 0;
}

->

static int lan1qtsn_spi_allocate_rx_skb(struct lan1qtsn_spi* tc6) {
    // Allocate buffer for frame without FCS since we remove it before submitting to network stack
    tc6->rx_skb = netdev_alloc_skb_ip_align(tc6->netdev, tc6->netdev->mtu + ETH_HLEN);
    if (!tc6->rx_skb) {
        tc6->netdev->stats.rx_dropped++;
        return -ENOMEM;
    }

    return 0;
}
```

이제 수정된 코드를 확인해보겠습니다. FCS 제거 로직이 올바르게 구현되었는지 확인하겠습니다.


이제 수정된 코드가 올바르게 작동하는지 확인해보겠습니다. 추가로 `ETH_FCS_LEN` 상수가 정의되어 있는지 확인해보겠습니다.


`ETH_FCS_LEN` 상수가 정의되어 있지 않으므로, 이를 정의해야 합니다. 헤더 파일에 추가하겠습니다.


```cpp
#pragma once

#include <linux/etherdevice.h>
#include <linux/spi/spi.h>

->

#pragma once

#include <linux/etherdevice.h>
#include <linux/spi/spi.h>

/* Ethernet FCS (Frame Check Sequence) length */
#define ETH_FCS_LEN 4
```

이제 수정된 코드를 요약해보겠습니다:

## 수정 내용 요약

1. **FCS 제거 로직 추가**: `lan1qtsn_spi_process_spi_data_rx_buf` 함수에서 SPI를 통해 수신된 패킷의 FCS(Frame Check Sequence)를 제거하도록 수정했습니다.

2. **버퍼 크기 조정**: `lan1qtsn_spi_allocate_rx_skb` 함수에서 FCS가 제거되므로 버퍼 크기를 `ETH_HLEN`만큼만 할당하도록 수정했습니다.

3. **통계 업데이트**: `lan1qtsn_spi_submit_rx_skb` 함수에서 FCS가 제거된 후의 바이트 수를 올바르게 계산하도록 주석을 추가했습니다.

4. **상수 정의**: `ETH_FCS_LEN` 상수를 헤더 파일에 정의했습니다.

## 주요 변경사항

### 1. FCS 제거 로직
```c
// Calculate frame size without FCS (remove last 4 bytes)
if (payload_size >= ETH_FCS_LEN) {
    frame_size_without_fcs = payload_size - ETH_FCS_LEN;
} else {
    dev_err(&tc6->spi->dev, "Payload size too small for FCS removal: %d\n", payload_size);
    return -EINVAL;
}

return lan1qtsn_spi_prcs_complete_rx_frame(tc6, payload, frame_size_without_fcs, meta);
```

### 2. 버퍼 크기 조정
```c
// Allocate buffer for frame without FCS since we remove it before submitting to network stack
tc6->rx_skb = netdev_alloc_skb_ip_align(tc6->netdev, tc6->netdev->mtu + ETH_HLEN);
```

### 3. 상수 정의
```c
/* Ethernet FCS (Frame Check Sequence) length */
#define ETH_FCS_LEN 4
```

이제 SPI를 통해 수신된 패킷에서 FCS가 제거되어 네트워크 스택으로 올바르게 전달됩니다. 이 수정으로 라즈베리 파이 4에서 네이티브 커널 빌드 후 사용할 때 FCS 관련 문제가 해결될 것입니다.

```c
// Calculate frame size without FCS (remove last 4 bytes)
if (payload_size >= ETH_FCS_LEN) {
    frame_size_without_fcs = payload_size - ETH_FCS_LEN;
} else {
    dev_err(&tc6->spi->dev, "Payload size too small for FCS removal: %d\n", payload_size);
    return -EINVAL;
}

return lan1qtsn_spi_prcs_complete_rx_frame(tc6, payload, frame_size_without_fcs, meta);
```

```c
// Allocate buffer for frame without FCS since we remove it before submitting to network stack
tc6->rx_skb = netdev_alloc_skb_ip_align(tc6->netdev, tc6->netdev->mtu + ETH_HLEN);
```

```c
/* Ethernet FCS (Frame Check Sequence) length */
#define ETH_FCS_LEN 4
```


# DECT Video Streaming Research

This repository documents the progress of my research on the **design and implementation of a video streaming service over a decentralized non-cellular 5G network (DECT-2020 NR).**

It includes different stages of the SPI communication layer used to transfer video data between two embedded systems: an **ESP32 (slave)** and a **nRF9161 DK (master)**. The final goal is to achieve real-time video transmission using SPI at high frequencies with full synchronization and automated control.

---

## 📁 Folders Overview

### `1_DMA_DISABLE`

This folder contains a **rudimentary but functional SPI communication prototype**, which successfully transmits data from the **ESP32 (slave)** to the **nRF9161 DK (master)**.

#### ✅ Status:
- **Fully functional** for low-frequency transfers.
- Uses CRC-16 for basic data integrity verification, but there is still room for improvement.
- Verified to work reliably for proof-of-concept setups.

#### ⚙️ Configuration:
- **ESP32 SPI Peripheral**: `SPI2_HOST`
- **nRF9161 SPI Peripheral**: `spi3`
- **SPI Frequency**: up to **8 MHz**
- **DMA**: ❌ Disabled
- **Frameworks**:
  - ESP-IDF: `v5.4`
  - nRF Connect SDK: `v2.9.0`

---

### `!Streaming`

This is the **main working directory** where the SPI interface is being actively optimized.

The goal in this stage is to:
- Automate the **Chip Select (CS)** signal.
- Use full **hardware SPI mode** with **DMA support**.
- Achieve **32 MHz** SPI communication.
- Ensure **video stream synchronization** and prepare for real-time transmission.

#### 🧪 Status:
- **Work in progress**: This version is under active development.

#### ⚙️ Configuration:
- **ESP32 SPI Peripheral**: `SPI3_HOST`
- **nRF9161 SPI Peripheral**: `spi3`
- **Target SPI Frequency**: up to **32 MHz**
- **DMA**: ✅ Enabled
- **Frameworks**:
  - ESP-IDF: `v5.4`
  - nRF Connect SDK: `v2.9.0`

---

## 📌 Notes

- All modules follow a **master-slave SPI topology**:
  - **Master**: nRF9161 DK
  - **Slave**: ESP32 DevKitC V4
- SPI communication is unidirectional (slave to master).
- In the `1_DMA_DISABLE` version, a **CRC-16** is used to validate 62-byte data blocks.
- In the `!Streaming` version, DMA is enabled and under active testing, but CRC is not yet implemented.

---

## 🔭 Next Steps

- Replace ESP32 DevKitC V4 with an **ESP32-CAM AI Thinker** board for video acquisition.
- Complete integration of **DMA + CRC** in the `!Streaming` version.
- Implement **frame synchronization markers**.
- Optimize **latency**, **data throughput**, and **frame rate** under real-world conditions.
- Explore end-to-end integration over **DECT-2020 NR network topology**.

# DataCore

Multi-channel ADC data acquisition on STM32: 4 channels sampled at 10 kHz via timer-triggered ADC with DMA double-buffering, framed and streamed over UART at 921600 baud.

**Hardware:** STM32F103C8T6 · ADC1 · DMA1 · TIM2 · USART1
**Firmware:** STM32F1 HAL (no CubeMX project needed — init is hand-written in `Src/main.c`)

## Pipeline

```
TIM2 @10 kHz TRGO
  -> ADC1 scan (IN0..IN3, ~8.7 us per scan)
  -> DMA1 Ch1 circular double buffer
  -> frame-ready flags
  -> packet framing (AA 55 | seq | samples | CRC-8)
  -> USART1 TX via DMA1 Ch4
```

The ADC never waits for the CPU and the CPU never waits for the UART: DMA moves samples in the background, and packet TX is non-blocking. If a frame arrives while the UART DMA is still busy, it is **dropped and counted** (`uart_packet_dropped()`) instead of silently corrupting the stream.

## Pin map (STM32F103C8T6)

| Function | Pin | Notes |
|---|---|---|
| ADC1_IN0..IN3 | PA0–PA3 | Analog inputs, 0–3.3 V |
| USART1 TX / RX | PA9 / PA10 | 921600 8N1 to USB-UART bridge |
| Status LED | PC13 | Toggles per streamed packet (brightness ~= throughput) |

All tunables (channels, sample rate, baud, decimation) live in `firmware/Inc/datacore_config.h`. **Verify the pin map against your hardware before flashing.**

## Throughput budget

Packet = 5 + 2·N bytes. At 10 kHz sampling with 4 channels and 2:1 decimation:

| Setting | Stream rate | Wire bytes/s | Baud needed |
|---|---|---|---|
| 4 ch, decim 2 (default) | 5 kHz | 65,000 | 921600 ✓ |
| 4 ch, decim 1 | 10 kHz | 130,000 | ≥ 1.35 Mbaud |
| 2 ch, decim 1 | 10 kHz | 70,000 | 921600 ✓ |

A compile-time check in `datacore_config.h` refuses to build if the stream would exceed 80% of UART bandwidth — change the baud rate or decimation instead.

## Packet format

```
AA 55 | seq u16 LE | ch0 u16 LE .. chN u16 LE | CRC-8 (poly 0x07)
```

## Firmware layout

```
firmware/
├── Inc/
│   ├── datacore_config.h  # channels, rate, baud, decimation, packet format
│   ├── adc_stream.h       # ADC+DMA double-buffer interface
│   └── uart_packet.h      # framed packet TX interface
└── Src/
    ├── main.c             # clock/GPIO/DMA/ADC/TIM/UART init + main loop
    ├── adc_stream.c       # DMA half/full callbacks, ADC calibration
    └── uart_packet.c      # CRC-8, packet builder, non-blocking DMA TX
tools/
└── receiver.py            # host-side: parse, CRC-check, CSV log, live plot
```

## Building

**STM32CubeIDE:** new STM32F103C8 project, drop in `firmware/Inc` + `firmware/Src`, build.

**Makefile/GCC:** compile `Src/*.c` with `-IInc` against STM32F1 HAL + CMSIS, link with `arm-none-eabi-gcc`.

**Host tool:**
```bash
pip install pyserial            # matplotlib too, for --plot
python3 tools/receiver.py --port /dev/ttyUSB0 --baud 921600 --channels 4 --csv log.csv
```

## Status

- [x] Timer-triggered 10 kHz ADC scan with DMA double-buffering
- [x] Framed packet protocol with CRC-8 and sequence numbers
- [x] Non-blocking UART DMA TX with explicit drop counting
- [x] Compile-time throughput check
- [x] Host receiver (parse / CRC check / CSV / live plot)
- [ ] Verify analog front-end (source impedance vs. 13.5-cycle sampling time)
- [ ] Measured sample-rate / jitter validation with a logic analyzer
- [ ] STM32CubeMX `.ioc` for graphical pin configuration (init is currently hand-written)

## License

MIT — see [LICENSE](LICENSE).

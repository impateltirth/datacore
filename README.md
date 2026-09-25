# DataCore

Analog signal sampling system on STM32: configurable-rate ADC capture with Direct Memory Access (DMA) buffering and UART transmission for continuous data streaming — 10 kHz+ sampling with minimal CPU intervention.

**Stack:** STM32 · C · ADC · DMA · UART

## Features

- Configurable ADC sampling rates (up to 10 kHz+ demonstrated)
- DMA double-buffering: the CPU never touches individual samples
- Continuous UART streaming of digitized data to a host
- Clean separation: `adc_dma` driver vs. application framing in `main.c`

## Repository layout

```
datacore/
├── firmware/
│   ├── adc_dma.h   # Driver API
│   ├── adc_dma.c   # ADC + DMA + UART streaming driver
│   └── main.c      # Application: init, start, frame & stream samples
└── docs/           # TODO: add scope captures / host-side plots
```

## How it works

1. `DataCore_Init()` configures ADC1 in continuous conversion mode, a DMA channel in circular mode feeding a double buffer, and UART for TX.
2. `DataCore_Start()` kicks off `HAL_ADC_Start_DMA()`.
3. On each half/full transfer complete callback, the finished half-buffer is framed and pushed out over UART via `HAL_UART_Transmit_DMA()` — acquisition never stalls.

## Porting / configuration

The driver is written against STM32 HAL. To adapt to your board:

- Generate ADC1 + DMA + UART handles in STM32CubeMX/CubeIDE, then point the driver's `extern` handles at them (see `TODO` markers in `adc_dma.c`).
- Sampling rate = ADC clock / (prescaler × (sampling time + 12.5 cycles)). Tune the ADC prescaler and sampling time for your target rate.
- UART baud rate: 115200 default; raise it if your sample rate × bytes/sample outgrows the link.

## Host side

Any serial terminal works. For plotting, point a Python script at the port and parse the framed 16-bit samples (frame format documented in `main.c`).

## TODO

- [ ] Point driver at your CubeMX-generated peripheral handles
- [ ] Verify achieved sample rate with a scope/logic analyzer
- [ ] Add `docs/` captures: input waveform vs. streamed reconstruction

## License

MIT — see [LICENSE](LICENSE).

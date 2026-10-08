# Acquisition architecture

```mermaid
flowchart LR
    SIG["Analog inputs PA0–PA3"] --> ADC["ADC1 scan"]
    TIM["TIM2 TRGO / 10 kHz"] --> ADC
    ADC --> DMA1["DMA1 Channel 1 / circular double buffer"]
    DMA1 --> LOOP["Main-loop decimator"]
    LOOP --> FRAME["Sequence + samples + CRC-8"]
    FRAME --> DMA4["DMA1 Channel 4 / UART TX"]
    DMA4 --> USB["USB–UART bridge / 921600 baud"]
    USB --> HOST["receiver.py / CRC, gap detection, CSV, plot"]
```

The host treats sequence gaps as observable backpressure rather than silently
accepting missing data. Firmware also counts frames dropped while UART DMA is
busy or cannot start. Measurements from a logic analyzer are still required to
claim real 10 kHz timing and jitter performance.

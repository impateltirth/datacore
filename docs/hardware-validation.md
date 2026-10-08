# Hardware validation procedure

The firmware and host protocol are testable without claiming electrical
performance. Use this checklist to turn the remaining hardware TODOs into
repeatable measurements.

## Analog front end

1. Record each source circuit, its output range, DC source resistance, and any
   RC anti-alias filter.
2. Confirm the signal never leaves the MCU's permitted analog-input range,
   including power-up and fault conditions.
3. Drive every channel from a low-noise reference, compare raw codes against a
   calibrated meter, and record offset, gain error, and channel-to-channel
   crosstalk.
4. Repeat with the real sensor/source. If settling error appears, reduce source
   impedance, buffer the input, or increase ADC sampling time.

## Sample rate and jitter

1. Toggle a spare debug GPIO in the DMA half/full callbacks on a validation
   build.
2. Capture at least 10,000 intervals with a logic analyzer.
3. Report mean frequency, minimum/maximum period, peak-to-peak jitter, and any
   missing callbacks.
4. Simultaneously capture USART1 TX and confirm sequence gaps reported by
   `receiver.py` agree with any firmware-side drop count.

## Acceptance record

| Measurement | Target | Result | Instrument / configuration |
|---|---:|---:|---|
| Per-channel sample rate | 10 kHz | Pending | |
| Peak-to-peak trigger jitter | Project-defined | Pending | |
| Input offset / gain error | Project-defined | Pending | |
| Sustained UART frame gaps | 0 under nominal load | Pending | |

Do not replace `Pending` with simulated numbers; attach the raw capture or CSV
when the physical board is tested.

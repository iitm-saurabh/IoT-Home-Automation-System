# Hardware Pin Configuration

## 8051 Microcontroller (AT89C51 / 89V51RD2)

### Crystal Oscillator Configuration
- **Frequency:** 11.0592 MHz
- **Pin 18 (XTAL1):** Crystal Input
- **Pin 19 (XTAL2):** Crystal Output
- **Load Capacitors:** 30pF (both sides to ground)

### Power Supply
- **VCC (Pin 40):** +5V DC
- **GND (Pins 11, 20):** Ground
- **Decoupling Capacitor:** 10µF across VCC and GND (near MCU)

### Reset Circuit
- **Pin 9 (RST):** Reset input
- **Configuration:** Capacitor 10µF to ground with 10kΩ pull-up to VCC
- **Reset Button:** Optional momentary switch to ground

## Port Configurations

### Port 0 (P0) - Pins 32-39
- **Function:** Address/Data Multiplexed (if using external memory)
- **Pull-up:** 10kΩ resistor array
- **Current Configuration:** Not used in this project

### Port 1 (P1) - Pins 1-8
```
P1.0 → LED RED (Output)        [Pin 1]
P1.1 → LED GREEN (Output)      [Pin 2]
P1.2 → BUTTON (Input)          [Pin 3]
P1.3 → Reserve                 [Pin 4]
P1.4 → Reserve                 [Pin 5]
P1.5 → Reserve                 [Pin 6]
P1.6 → Reserve                 [Pin 7]
P1.7 → DHT11 DATA (Bidirectional) [Pin 8]
```

### Port 2 (P2) - Pins 21-28
- **Current Configuration:** Not used
- **Future:** Can be used for additional sensors or I2C

### Port 3 (P3) - Pins 10, 12-17
```
P3.0 (RXD) → Pin 10  [UART Receive]   ← USB-TTL RX
P3.1 (TXD) → Pin 11  [UART Transmit]  → USB-TTL TX
P3.2 (INT0) → Reserve
P3.3 (INT1) → Reserve
P3.4 (T0)  → Reserve
P3.5 (T1)  → Reserve
P3.6 (WR)  → Reserve
P3.7 (RD)  → Reserve
```

## DHT11 Sensor Wiring

### DHT11 Pin Configuration
```
┌─────────────┐
│ DHT11 Sensor│
├─────────────┤
│ 1: VCC      │ → +5V
│ 2: DATA     │ → P1.7 (8051 P1.7) with 10kΩ pull-up
│ 3: NC       │ (Not Connected)
│ 4: GND      │ → Ground
└─────────────┘
```

### Pull-up Resistor
- **Resistor Value:** 10kΩ
- **Connection:** Between P1.7 and +5V
- **Purpose:** Ensures proper signal levels for DHT11 one-wire protocol

## LED Indicators

### Red LED (P1.0)
- **Function:** System Error Indicator
- **Active State:** HIGH (LED ON when P1.0 = 1)
- **Current Limiting Resistor:** 330Ω
- **Anode:** P1.0
- **Cathode:** Ground

### Green LED (P1.1)
- **Function:** System Status (Ready/Running)
- **Active State:** HIGH (LED ON when P1.1 = 1)
- **Current Limiting Resistor:** 330Ω
- **Anode:** P1.1
- **Cathode:** Ground

## UART Communication

### Serial Port Configuration
- **Pins:** P3.0 (RXD), P3.1 (TXD)
- **Baud Rate:** 9600 bps
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None
- **Flow Control:** None

### USB-TTL Converter Connection
```
Microcontroller ↔ USB-TTL Adapter
───────────────────────────────────
P3.0 (RXD) ← TX (yellow wire)
P3.1 (TXD) → RX (green wire)
GND → GND (black wire)
VCC → VCC (red wire, optional - for powering adapter)
```

## Block Diagram

```
┌─────────────────────────────────────────┐
│         8051 Microcontroller            │
│                                         │
│  ┌─────────────┐    ┌──────────────┐   │
│  │ XTAL: 11MHz │    │ Reset Circuit│   │
│  └──────┬──────┘    └──────┬───────┘   │
│         │                  │           │
│    Port 1: P1.0-P1.7       │           │
│         │                  │           │
│  ┌──────┴──────────────────┴───────┐   │
│  │   Port 3: UART (P3.0, P3.1)     │   │
│  │   TXD (P3.1) ──────→ USB-TTL TX │   │
│  │   RXD (P3.0) ←────── USB-TTL RX │   │
│  └─────────────────────────────────┘   │
└─────────────────────────────────────────┘
     │         │         │         │
     │         │         │         │
    ▼         ▼         ▼        ▼
   RED       GREEN    DHT11    BUTTON
   LED       LED      Sensor   (P1.2)
  (P1.0)   (P1.1)    (P1.7)
```

## Component Values Summary

| Component | Value | Quantity | Notes |
|-----------|-------|----------|-------|
| Crystal Oscillator | 11.0592 MHz | 1 | 3-pin HC-49U |
| Load Capacitor | 30pF | 2 | Ceramic |
| Decoupling Capacitor | 10µF | 1 | Electrolytic |
| Pull-up Resistor (XTAL) | 10kΩ | 1 | 1/4W |
| Pull-up Resistor (DHT11) | 10kΩ | 1 | 1/4W |
| Pull-up Resistor (P3) | 10kΩ Array | 1 | Optional |
| LED Series Resistor | 330Ω | 2 | 1/4W |
| Power Supply Capacitor | 100µF | 1 | Electrolytic |

## Testing Connections

### Verification Points
1. **Power Supply:** 5V between VCC and GND
2. **Crystal Frequency:** 11.0592 MHz on XTAL1/XTAL2
3. **Reset Voltage:** 0V (at rest), 5V (on release)
4. **DHT11 Data:** Should show pulses when reading sensor
5. **UART TXD:** Should show data transmission during operation

## Notes
- All connections should be verified with continuity tester before powering
- Use 0.1µF bypass capacitors near power pins for high-frequency noise filtering
- DHT11 requires 2.5V to 5.5V supply voltage
- Maintain proper trace width for power distribution (≥0.5mm)
- Use separate ground planes if possible

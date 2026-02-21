# STM32 Tetris - Bare-Metal on Nucleo-F446RE + 32×48 WS2812 RGB LED Matrix

A full-featured, classic **Tetris** game running bare-metal on the **NUCLEO-F446RE** (STM32F446RE @ 180 MHz), driving a large **32×48 RGB LED matrix** built from **6× 16×16 WS2812(B) panels**.

The display driver uses a highly optimized **parallel bit-banging** technique (multi-GPIO simultaneous output with hand-tuned timings in assembly-like style), achieving a smooth **~140 Hz** refresh rate - impressive for software-driven WS2812 protocol on STM32.

## Features

- Authentic Tetris gameplay with **7-bag randomizer** (fair next-piece queue)
- **Ghost piece** (drop shadow / landing preview)
- Level-based gravity (NES-style table, levels up every 5 lines, can be easily customized)
- Line clear animation + game over fade / corruption effect
- Score, level, rows-cleared indicators
- High-score saving (up to 63 entries) + on-screen name entry keyboard
- Leaderboard with sliding marquee text
- Start countdown animation ("3-2-1-Go")
- Animated menu with glowing borders & selector highlight
- Custom 5×3 pixel font rendered directly into framebuffer
- Pure register-level code — **no HAL, no libraries, no CMSIS-DSP**

## Hardware

- **Board** — [NUCLEO-F446RE](https://www.st.com/en/evaluation-tools/nucleo-f446re.html)
- **MCU** — STM32F446RE running at **180 MHz** (HSE + PLL overdrive)
- **Display** — 32×48 RGB pixels made from **6× 16×16 WS2812(B) matrices**  
  - Total: **4608 individually addressable RGB LEDs**  
  - Driven in **parallel** across multiple GPIO pins for high refresh (~140 Hz)
- **Controls** — 5 tactile buttons (debounced in software):
  - Left / Right -> move piece
  - Up -> rotate clockwise
  - Down -> soft drop
  - Special -> hard drop / menu confirm

## Using Arduino IDE (easiest way to get started)

Even though this is a low-level project without the Arduino framework abstractions, you can use the **Arduino IDE** just as a convenient editor + uploader + board manager.

### Quick Setup Steps (Arduino IDE route)

Use the following scematics to connect the parts:

<img width="905" height="791" alt="image" src="https://github.com/user-attachments/assets/0e1a1046-c9f9-4ba7-b4fb-3c28a5decfbf" />

1. **Install Arduino IDE**  
   Download from https://www.arduino.cc/en/software (2.x recommended)

2. **Add STM32 core support**  
  - Open Arduino IDE  
  - Go to **File -> Preferences**  
  - In “Additional Boards Manager URLs” paste: ```https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json```
  - Click OK  
  - Go to **Tools -> Board -> Boards Manager**  
  - Search for **STM32**  
  - Install **“STM32 MCU based boards” by STMicroelectronics** (latest version)

3. **Select your board**  
- Tools -> Board -> STM32 MCU based boards -> **Generic STM32F4 series**  
- Then set:  
- **Board part number**: **Nucleo-64**  
- **Variant**: **Nucleo-F446RE**  
- **Upload method**: **STM32CubeProgrammer (SWD)**  (recommended)  
  or **STLink** if you prefer the old way  
- **Clock configuration**: HSE 8 MHz crystal/ceramic
- **Optimize**: **Fastest (-O3)**

4. **Create new sketch & replace everything**  
- File -> New  
- Delete all default code in the new sketch  
- Copy-paste **all** the code from this project's `main.ino`
- **Important**: The Arduino IDE will still try to add its own `setup()` and `loop()`, but because we define our own `main()`, it will be ignored. This is intentional and safe.

5. **Compile & Upload**
- Connect your Nucleo via USB  
- Make sure the ST-LINK drivers are installed (usually automatic on Windows/macOS)  
- Click **Upload** (right arrow icon)  
- Wait ~10–20 seconds, game should start on the matrix

### Button Mapping (classic + side button)

Your controller layout:

| Position       | Function       | GPIO used in code     |
|----------------|----------------|-----------------------|
| D-pad Left     | Move left      | GPIOB pin 6           |
| D-pad Right    | Move right     | GPIOB pin 3           |
| D-pad Up       | Rotate         | GPIOB pin 5           |
| D-pad Down     | Soft drop      | GPIOB pin 4           |
| **Left side**  | **Special**    | GPIOA pin 10          |

-> The **special button** is the extra one on the **left side** of your controller (not part of the cross).  

If your special button is on a different pin than expected, just search the code for `GPIOA->IDR & GPIO_IDR_IDR_10` and adjust the pin number.

Enjoy your giant 32×48 Tetris screen!

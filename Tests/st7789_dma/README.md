# ST7789 DMA host checks

These tests compile the real screen driver against a small HAL mock. They check
wire byte order, asynchronous ownership, DMA chunking, error propagation and
abort/restart behavior. They do not validate STM32 registers, IRQ timing, the
LVGL semaphore integration or physical display output.

From the project root, using a native GCC (not arm-none-eabi-gcc):

```powershell
gcc -std=c11 -Wall -Wextra -Werror -ITests/st7789_dma -IDrivers/DshanMCU-F407 Tests/st7789_dma/test_st7789_dma.c Drivers/DshanMCU-F407/driver_st7789_f407.c -o D:/codex-workplace/st7789_dma_test.exe
D:/codex-workplace/st7789_dma_test.exe
```

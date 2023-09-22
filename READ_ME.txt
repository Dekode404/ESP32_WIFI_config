@ Size of the code for to track the optimization of the code:-

Used static DRAM:   30740 bytes (149996 remain, 17.0% used)
      .data size:   12828 bytes
      .bss  size:   17912 bytes
Used static IRAM:   87769 bytes (43303 remain, 67.0% used)
      .text size:   86742 bytes
   .vectors size:    1027 bytes
Used Flash size :  586435 bytes
      .text     :  487407 bytes
      .rodata   :   98772 bytes
Total image size:  687032 bytes (.bin may be padded larger)

@ Features of the code:-
1. Light wait not consume more than 10% of the total RAM (Total RAM 520K so RAM consumption is limited to )
2. Give the indication for the WIFI status and the connection status.
3. 
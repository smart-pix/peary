# CoRDIA Device

## Chip Registers
  All below registers can be written from `pearycli` using `setRegister` command and read out using `getRegister` command. Except the last one, each configuration register is 1 bit only.
  Writing to chip registers will automatically initiate upload of the configuration to the chip for each write.

  * `inject_pix0` [1b] (RW)
  * `inject_pix1` [1b] (RW)
  * `inject_pix2` [1b] (RW)
  * `inject_pix3` [1b] (RW)
  * `inject_pix4` [1b] (RW)
  * `inject_pix5` [1b] (RW)
  * `inject_pix6` [1b] (RW)
  * `inject_pix7` [1b] (RW)
  * `inject_pix8` [1b] (RW)
  * `inject_pix9` [1b] (RW)
  * `inject_pix10` [1b] (RW)
  * `inject_pix11` [1b] (RW)
  * `inject_pix12` [1b] (RW)
  * `inject_pix13` [1b] (RW)
  * `inject_pix14` [1b] (RW)
  * `inject_pix15` [1b] (RW)
  * `use_isource` [1b] (RW)
  * `use_pulsec` [1b] (RW)
  * `set_gn1_external` [1b] (RW)
  * `notgn2_cds` [1b] (RW)
  * `notsc_cds` [1b] (RW)
  * `select_biases2adc` [1b] (RW)
  * `mon_spix_addr22`  [1b] (RW)
  * `mon_spix_addr23` [1b] (RW)
  * `full_config`: [24b] (RW) all above configuration bits in one 24b word
  * `pg_freq`: [32b] (WS) Special virtual register for setting frequency of pattern generator and readout. Exists in SW only, has no connection to HW.

## FPGA Registers

  Following registers are accessible from `pearycli` using `setMemory` and `getMemory` commands.
  These typically don't need to be accessed directly. Use provided functions instead.

  * `pg_cmd_rst`: [1b] (W) resets pattern generator
  * `pg_cmd_start`: [1b] (W) starts pattern generator
  * `pg_cmd_stop`: [1b] (W) stops pattern generator
  * `pg_stat_running`: [1b] (R) is `1` if the pattern generator is running.
  * `pg_conf_pattern_length`: [11b] (RW) sets the number of lines in the generator memory
  * `pg_conf_number_of_runs`: [16b] (RW) sets the number of runs through the PG memory
  * `sdi_cmd_rst`: [1b] (W) resets configuration interface
  * `sdi_cmd_write_config`: [1b] (W) initiate writing configuration to the chip
  * `sdi_conf_sanity_check_en`: [1b] (RW) enables or disables readout of the configuration and its checking against sent data
  * `sdi_stat_write_done`: [1b] (R) is `1` when writing configuration was finished
  * `sdi_err_sanity_check`: [1b] (R) is `1` when configuration bits don't match readout bits
  * `sdi_conf_bits`: [24b] (RW) configuration to be sent to the chip
  * `sdi_readout_bits`: [24b] (R) configuration that was read out from the chip
  * `data_fifo_rst`: [1b] (W) resets received data FIFO
  * `readout_en`: [1b] (RW) enables readout - setting it to `0` also resets the FIFO
  * `data_out`: [32b] (R) FIFO output - received data
  * `rx_strobe_delay`: [3b] (RW) sets delay in PG clock cycles between the strobe signal and actual sampling of the ADC data. Default value is `3`, range is `0-7`.
  * `rx_strobe_sel`: [1b] (RW) selects a source of ADC bit strobe signal. If set to `0` (default), the source it `ADC_clk_comp`, if set to `1`, the source is `ADC_clk_SR`.
  * `rx_strobe_neg`: [1b] (RW) If set to `1`, the source signal for the ADC bit strobe is inverted (i.e. strobe is generated on a falling edge of the original signal). If set to `0` (default), strobe is generated on a rising edge.
  * `pg_clk_ce`: (RW) Enable (`1` - default) or disable (`0`) pattern generator and readout clock. Clock is disabled during frequency change.


## Functions

Functions accessible through the peary CLI:
  * `powerStatusLog`: Prints voltage and currents measured on the power supplies.
  * `writeConfig`: Apply the chip configuration from the FPGA register `sdi_conf_bits` to the chip. Done automatically when using chip register access, must be done manually when accessing the FPGA register manually through memory
  * `writePG <F>`: Writes pattern sequence from file `F` to the generator memory.
  * `startPG <N>`: Starts pattern generator which and repeats the sequence `N`-times. If `N` is set to `0`, it will run infinitely until it is stopped manually.
  * `stopPG`: Finishes the current pattern generator run and stops it (e.g. when running infinitely).
  * `printFrames <N>`: Will run the pattern generator `N`-times and print received frames to the CLI output.
  * `storeFrames <N> <F>`: Will run the pattern generator `N`-times and store received frames to the file `F`.
  * `setFreq <N>`: Sets clock frequency for pattern generator (and readout). Input parameter is requested frequency in Hz, range is 10-200MHz.

## Data Format

  Data stored in a file produced by `storeFrames` function are in binary format. Each word is 2 bytes long (16 bits).
  The first word in a frame defines a number of bytes of that frame, excluding this header number. Then the frame data follows. Next frame follows immediately and starts again with the number of bytes.

  Example file content (values are in HEX):
  ```
  root@caribou:~# hexdump -v out.bin
  0000000 0020 0660 0660 0660 0660 0660 0660 0660
  0000010 0260 0660 0260 0660 0260 0660 0660 0260
  0000020 0660 0020 0260 0660 0260 0660 0660 0660
  0000030 0660 0661 0260 0660 0260 0660 0660 0661
  0000040 0260 0660                              
  ```
  The above file content means the following:
  ```
  0000000: 0020: Equals 32 (dec) The next 32 bytes is frame data - each is 2 bytes, thus there are 16 pixels
  0000002: 0660: Frame 1, Pixel 1
  0000004: 0660: Frame 1, Pixel 2
  0000006: 0660: Frame 1, Pixel 3
  0000008: 0660: Frame 1, Pixel 4
  000000A: 0660: Frame 1, Pixel 5
  000000C: 0660: Frame 1, Pixel 6
  000000E: 0660: Frame 1, Pixel 7
  0000010: 0260: Frame 1, Pixel 8
  0000012: 0660: Frame 1, Pixel 9
  0000014: 0260: Frame 1, Pixel 10
  0000016: 0660: Frame 1, Pixel 11
  0000018: 0260: Frame 1, Pixel 12
  000001A: 0660: Frame 1, Pixel 13
  000001C: 0660: Frame 1, Pixel 14
  000001E: 0260: Frame 1, Pixel 15
  0000020: 0660: Frame 1, Pixel 16
  0000022: 0020: New Frame. The next 32 bytes is frame data (16 pixels)
  0000024: 0260: Frame 2, Pixel 1
  0000026: 0660: Frame 2, Pixel 2
  0000028: 0260: Frame 2, Pixel 3
  000002A: 0660: Frame 2, Pixel 4
  000002C: 0660: Frame 2, Pixel 5
  000002E: 0660: Frame 2, Pixel 6
  0000030: 0660: Frame 2, Pixel 7
  0000032: 0661: Frame 2, Pixel 8
  0000034: 0260: Frame 2, Pixel 9
  0000036: 0660: Frame 2, Pixel 10
  0000038: 0260: Frame 2, Pixel 11
  000003A: 0660: Frame 2, Pixel 12
  000003C: 0660: Frame 2, Pixel 13
  000003E: 0661: Frame 2, Pixel 14
  0000040: 0260: Frame 2, Pixel 15
  0000042: 0660: Frame 2, Pixel 16
  0000044: EOF
  ```

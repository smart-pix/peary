# H2M Device
Caribou device dedicated to the H2M (Hybrid-to-Monolithic) chip.

## Description
The H2M ASIC is a monolithic pixel sensor chip designed in a 65 nm CMOS imaging process. It consists of a matrix of 64 x 16 square pixels with 35 um pitch. The on-pixel logic is based on an 8-bit counter that is programmable to operate in 4 different acquisition modes: Time of Arrival (ToA), Time over Threshold (ToT), Photon Counting (CNT) and Triggered (TRG) mode.

For pixel identification, the pixel matrix consisting of 64 columns and 16 rows is assumed to be:
```
 	row 15  |   15      31     ...   1023
	row 14  |   14      30 	   ...   1022
	  ...   |        ...   	   ...
	 row 1  |    1      17     ...   1009
        row  0  |    0      16     ...   1008
	------------------------------------------
 	   col       0       1     ...     64
```
And groups of 4 pixels needed for reading/writing are identified as:
```
	group 0    : pixel    0,    1,    2,    3
	group 1    : pixel    4,    5,    6,    7
	   ...
	group 255  : pixel 1020, 1021, 1022, 1023
```

## Operation

### Through the peary Command Line Interface (CLI):
  * Start peary CLI. \
`pearycli H2M -c /path/to/configuration/file.cfg`
  * Power and configure device 0 (H2M in this case). \
`powerOn 0` \
`configure 0`
  * See available functions (description below): \
`help`
  * ... Have fun!

### Through EUDAQ2:

### Data Taking Modes:
The four acquisition modes can be selected with `acq_mode`. When set to `0b00`, the ToA mode is enable; `0b01` for T0T; `0b10` for CNT; and `0b11` for TRG.

## Software parameters:

These parameters can be placed in the Peary configuration file (alongside registers and memories) and will be interpreted:

* `pg_file`: File of the pattern generator. If present, the PG will be configured upon `configure()` of the chip.
* `clock_internal`: Boolean to select internal clock generation, no locking check for external clock.
* `force_read_from_fifo`: Boolean to force readout from FIFO irregardless of whether device is in `sc_ctrl_mode` or not.

## Chip Parameters:
The following parameters are read from the configuration file to configure the chip behavior:
* `acq_start`: Enables acquisition. When set to 1, the on-pixel counters are in LFSR mode and the chip is ready for acquisition.
* `conf`: When set to 1, the columns are ready to be configured. Configuration is performed by writing the configuration data to the slow control address of each column. By setting conf to 0, the written configuration is latched in the pixels.
* `acq_mode`:  Selects acquisition mode.
* `shutter_select`: When this bit is 0, the externally applied shutter is directly connected to the pixel columns. When set to 1, the shutter is internally synchronized to the slow control clock.
* `latency`: Counter preset for triggered acquisition mode.
* `analog_out_ctrl`: Analog test output control.
* `digital_out_ctrl`: Digital test output control.
* `matrix_ctrl`: Matrix control.
* `shutter_tp_lat`: Test pulse latency after shutter is opened (in slow control clock cycles).
* `link_shutter_tp`: When set to 0,  test pulses independent of shutter. When set to 1, test pulses injected when shutter is opened.
* `tp_polarity`: When set to 0, a negative pulse is injected. When set to 1, a positive pulse is injected.
* `tp_sel_digital`: When set to 1, a digital test pulse is injected. When set to 0, an analog test pulse is injected.
* `tp_num`: Number of test pulses sent to the pixels.
* `tp_on`:  Number of slow control clock cycles the test pulse is high.
* `tp_off`: Test pulse off.
* `dac_ibias`: DAC bias.
* `dac_itrim`: DAC itrim.
* `dac_ikrum`: DAC Ikrum.
* `dac_vthr`: DAC Vthreshold.
* `dac_vref`: DAC Vref.
* `dac_vtpulse`: DAC for injected test pulse voltage.
* `bgr_res_0`: Bandgap res_0 control.
* `bgr_res_1`: Bandgap res_1 control.
* `bgr_res_2`: Bandgap res_2 control.
* `bgr_iref_trim`: Trimming for reference current. Controls the trimming for the bandgap, which allows up to +/−2 µA compensation of the current delivered by the bandgap. It is set as thermometer code (not binary weighted) in steps of 1 uA.
* `enable_bridge`: Enables SLVS transmitter bridges.

The default parameters for the supply voltages and currents usually permit stable operation of the H2M. These parameters are:
 * `vddd = 1.2`
 * `vdda = 1.2`
 * `lvds = 3.3`
 * `v_circuit = 3.3`
 * `v_io = 0.0` // to be decided!
 * `v_sub = -1.2`
 * `v_pwell = -1.2`
 * `vddd_current = 0.3` // to be tested
 * `vdda_current = 0.3` // to be tested
 * `lvds_current = 0.5` // to be tested
 * `v_circuit_current = 0.1` // to be tested
 * `v_io_current = 0.0` // to be decided


## FPGA Parameters

## FPGA Controls

These registers are used to control the firmware in the FPGA. They are not intended to be used directly by a user. Peary software routines should control them instead and user should use these routines. However, direct access to them is possible.
The registers can be readable [`r`], writeable [`w`] or both [`rw`]. Read-only [`r`] registers are used to indicate a status and their value is controlled by the firmware. Writing to a read-only register has no effect.  Write-only [`w`] registers are used to trigger an action by writing a value (typically `1`) into it. The corresponding physical registers in the FPGA are reset by firmware to a default value automatically after each write. Reading a write-only register will return a default value (typically `0`).

 * `debug`: Option to pass a few signals to SMA output `USER_SMA_CLK_N` on the FPGA board. The following mapping is used:
   * `debug = 0 -> readout_idle (default)`
   * `debug = 1 -> readout_acq_err`
   * `debug = 2 -> sc_err_cmd`
   * `debug = 3 -> sc_err_addr`
   * `debug = 4 -> sc_err_timeout`

## Functions
Functions accessible through the Peary CLI:
 * `configure`: Initializer function for H2M.
 * `powerUp`: Turn on the power supply for the H2M chip.
 * `powerDown`: Turn off the H2M power.
 * `daqStart`: Start the data acquisition.
 * `daqStop`: Stop the data acquisition.
 * `powerStatusLog`: Report power status.
 * `configureMatrix`: Configure pixel matrix from mask file.
 * `getData`: Returns the data as 8 bits per pixel as explained below.
 * `getRawData`: Returns the data of each group of 4 pixels (32 bits per group).
 * `programMask`: Write matrixMask to registers.
 * `readMask`: Read the mask file into matrixMask.
 * `generateDefaultMask`: A default masking is generated.
 * `triggerPatternGenerator`: Run pattern generator once.


## Utils
Helpful root macros for the analysis of laboratory measurements and mask file generation are in `devices/H2M/utils`:
 * `write_example_mask.C`: contains a macro for mask file generation. An example of mask file can be found in `devices/H2M/config/example_mask0_tp1.cfg`.


## Data Format:
The data is returned as 8 bits per pixel. It contains the following information with the syntax: identifier[position] = default value:
  * `tp_enable[0] = 0`: if set to 1, the test pulse is enabled in the pixel. Combined with `tp_sel_digital`, a digital or analog test pulse is selected.
  * `mask[1] = 0`: if set to 1, the pixel is masked.
  * `tuning_dac[5:2] = 1111`: for local threshold trimming
  * `not_top_pixel[6] = 0`: only used in triggered acquisition mode. It allows configuring the strip length.
  * `reserved[7] = 0`.

The data is read out column by column, with each column containing four groups of four pixels. For each column, there is a single 32-bit register that is read/written four times, using a shifting mechanism in the firmware. This allows the complete column to be read/written. These 32-bit registers are: `px_cnt_0`, `px_cnt_1`, ..., and `px_cnt_63` with the index indicating the column identification.

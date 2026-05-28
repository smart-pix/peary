# dSiPM Device

Caribou device dedicated to the DESY digital Silicon Photomultiplier (dSiPM).

## Description

The dSiPM consists of a matrix of 32 x 32 pixels, arranged in four square quadrants. Each pixel contains 4 Single-Photon Avalanche Diodes (SPADs) sharing a quenching circuit, a discriminator and a in-pixel counter. The chip is operated in a frame-based mode, defined by a 3 MHz clock. The 256 pixels in a quadrant share a 12-bit Time-to-Digital Converter (TDC), generating a time stamp for the first hit in each quadrant and frame. The dSiPM features two readout modes, the first provides binary hit information (1-bit mode). The so-called 2-bit mode allows to count up to 3 hits per frame, doubling the frame duration compared to the 1-bit mode. A validation logic can be applied to discard events based on topology. A masking mechanism allows to disable any pattern of pixels, which is a useful feature to reduce the impact of noisy pixels. The chip also features test circuits for pixels and SPADs in four flavors surrounded by eight neighbor pixels/SPADs, with (digital output) and without front-end (analog output), and a fifth test circuit for the TDC. A temperature diode on the chip allows to monitor the temperature of the sensor. Finally, there is the possibility to provide trigger pulses to an external LED via the dSiPM chip board, which provides a useful tool for laboratory characterization of the device.

The Caribou setup provides all clocks, signals, supply voltages and currents required to operate the dSiPM. Only the high voltage (about +20 V) for biasing of the SPADs needs to be provided externally to the corresponding LEMO connector.

## Operation

### Through the peary Command Line Interface (CLI):
  * Start peary CLI. \
`pearycli dSiPM -c /path/to/configuration/file.cfg`
  * Power and configure device 0 (dSiPM in this case). \
`powerOn 0` \
`configure 0`
  * See available functions (description below): \
`help`
  * ... Have some fun.

### Through EUDAQ2:
  * Setup EUDAQ2 run control the usual way.
  * Open caribou producer on caribou, with dSiPM device \
`ssh -t username@carbouIP 'euCliProducer -n CaribouProducer -t dSiPM -r tcp://${runControlPcIP}:${runControlPcPORT}'`
  * Setup initialization file the usual way. Make sure to add (path on caribou): \
`config_file = "/path/to/configuration/file.cfg"`
  * Setup configuration file the usual way.
  * ... Take tons of high-quality data.

Make sure to check `peary/devices/dSIPM/config` for some examples of configuration files.

### Data Taking Modes:

The FW has two DAQ modes, in one the shutter is immediately opened when we start the data taking (FW register `daq_start_stdalone` set to `1`).
In the second mode (register `daq_start_tlu` set to `1`) the firmware waits for a `T0` from the TLU and only then opens the shutter.
The bunch counter and other registers on the chip are held on low until the shutter is opened and the start counting.

The DAQ modes are controlled with the Peary configuration key `tlu_daq`. Setting it to `1` will wait for a T0 while `0` will start the DAQ immediately.

## Chip Parameters

The following parameters are read from the configuration file to configure the chip behavior, following the syntax `identifier=default_value`:
  * `validCntr_Q1=0x0`: Validation logic configuration. Exists also for the other tree quadrants. Setting for Validation References: Combinations of ANDs and ORs. E.g. `0000` (0): all ORs, `1000` (1): always 2 rows are combined with an AND, the outputs of these combinations are combined with an OR in the next step and so on.
  * `TDC_Q1=0x2`: Process Corner Settings for TDC in quadrant 1, tt: `01` (2), ss: `00` (0), ff: `11` (3). Exists also for the other tree quadrants.
  * `set2Bit=0x1`: Operating in 1-bit mode (1), or in 2-bit mode (0). Note that a similar parameter for an FPGA has to be adjusted accordingly and the value there is inverted!
  * `mask_filename`: Provide a mask file to define which pixels are enabled/ masked. See e.g. `peary/devices/dSiPM/config/dSiPM_Mask_AllOn.cfg`.
  * `quadrantsEnable=0xF`: Bit mask to enable all pixels in a quadrant. E.g. `0111` enables all quadrants but quadrant 4.
  * `pixelEnable_TC=0x0`: Enables the test circuits with front-end and its neighbors. E.g. (0) all off, (256) only inner pixel/SPAD (of interest) on, (511) all nine pixels/SPADs on.
  * `pixelEnableWOFE_TC=0x0`: Enables ONLY the neighbors of the analog test circuits without front-end. The inner pixel/SPAD is **always** on, because it is connected to an external 20k ohm quenching resistor.
  * `TDC_TC:0x2`: Prozess Corner Settings for TDC test circuit, tt: (2), ss: (0), ff: (3).
  * `temp_offset` : Define temperature pol1 calibration function constant offset for temperature diode calibration, 0.0 if not defined.
  * `temp_slope` : Define temperature pol1 calibration function slope for temperature diode calibration, 1.0 if not defined.

The default parameters for the supply voltages and currents usually permit stable operation of the dSiPM. These parameters are:
  * `dSiPM_VDDIO3V3=3.3`
  * `dSiPM_VDDIO3V3_CURRENT=3`
  * `dSiPM_VDDIO1V8=1.8`
  * `dSiPM_VDDIO1V8_CURRENT=3`
  * `dSiPM_VDDCORE=1.8`
  * `dSiPM_VDDCORE_CURRENT=3`
  * `dSiPM_LED_PWR=3.5`
  * `dSiPM_LED_PWR_CURRENT=3`
  * `dSiPM_DS_PWR=3.3`
  * `dSiPM_DS_PWR_CURRENT=3`
  * `dSiPM_VBQ=0.72`       
  * `dSiPM_VCLAMP=2.0`    
  * `dSiPM_VBQ_TC=0.72`    
  * `dSiPM_VCLAMP_TC=2.0`
  * `dSiPM_TD_IN=100`     
  * `CMOS_OUT_1_TO_4_START_VOLTAGE=1.3`
  * `CMOS_OUT_1_TO_4_VOLTAGE=1.8`
  * `CMOS_OUT_5_TO_8_VOLTAGE=3.3`
  * `dSiPM_LED_ON=1` Enables LED pulsing.
  * `dSiPM_T_ON=` Enables temperature diode.

## FPGA Parameters

Configuration parameters to control the data acquisition system on the FPGA board, following the syntax `identifier=default_value`:
  * `mode_2bit=0x0`: Operating in 1-bit mode (0), or in 2-bit mode (1). Note that a similar parameter for a chip needs to be adjusted accordingly and the value there is inverted!
  * `ext_trg_enable=0x0`: Enable triggers from external source (SMA connector J67 on the ZC706 board).
  * `tlu_trg_enable=0x0`: Enable triggers from the TLU (RS45 connector).
  * `led_trg_enable=0x0`: Enable triggers from the LED trigger pulse.
  * `auto_trg_enable=0x0`: Enable internal triggers in stand alone mode, e.g. for DCR measurements.
  * `wait_for_fifo_empty=0x0`: If the data buffer becomes full and this is set to `1`, DAQ stops storing data until the buffer is completely empty. If set to `0`, it continues storing data as soon as there is any space available.
  * `frames_before_trg=0x0`: Number of frames before the trigger signal arrival to be marked for storage.
  * `frames_after_trg=0x0`: Number of frames after the trigger signal arrival to be marked for storage, including the frame in which the trigger signal arrived.
  * `auto_trg_spacing=0x0`: Period of internal triggers in units of frame length. `0` = trigger generated for each frame.
  * `busy_time=0x0`: Duration of the busy signal to the TLU after a trigger has been received in units of frame length.
  * `led_trigger=0x0`: Starts LED trigger pulsing.
  * `led_start_offset`: Delay of the LED pulse with respect to the beginning of a frame in 2.45ns (408MHz) clock cycles.
  * `led_num_pulses=0x0`: Number of LED pulses per frame *minus one*. `0` = 1 pulse.
  * `led_pulse_width=0x0`: Width of an LED pulse in 2.45ns (408MHz) clock cycles *minus one*. `0` = 1 clock cycle.
  * `led_pulse_spacing=0x0`: Spacing of the LED pulses in a frame in 2.45ns (408MHz) clock cycles *minus one*. `0` = 1 clock cycle.
  * `led_frames=0x0`: Number of frames in which the LED fires.
  * `led_skip_frames=0x0`: A pause between LED-pulsed frames in units of frame length.

## FPGA Controls

These registers are used to control the firmware in the FPGA. They are not intended to be used directly by a user. Peary software routines should control them instead and user should use these routines. However, direct access to them is possible.

The registers can be readable [`r`], writeable [`w`] or both [`rw`]. Read-only [`r`] registers are used to indicate a status and their value is controlled by the firmware. Writing to a read-only register has no effect.  Write-only [`w`] registers are used to trigger an action by writing a value (typically `1`) into it. The corresponding physical registers in the FPGA are reset by firmware to a default value automatically after each write. Reading a write-only register will return a default value (typically `0`).

  * `writeChipConfig` [`w`]: Triggers a write transfer of the chip configuration registers to the physical chip.
  * `addConfClock` [`rw`]: A number of zero bits to add after the chip *matrix* configuration data stream. (6 bits)
  * `matrixConfDirty` [`r`]: Status bit indicating that the chip *matrix* configuration has changed after it was last time written to the chip.
  * `testConfDirty` [`r`]: Status bit indicating that the chip *test circuit* configuration has changed after it was last time written to the chip.
  * `chipConfRunning` [`r`]: Status bit indicating that the chip configuration is currently being written into the chip.
  * `testConfOffset` [`rw`]: A number of zero bits before the chip *test circuit* configuration data stream starts. (11 bits)
  * `testConfEnable` [`rw`]: Enables writing the chip *test circuit* configuration during the configuration write.
  * `daq_start_tlu` [`w`]: Starts DAQ in a TLU mode - Prepares the DAQ circuits and waits for a `T0` signal after which the shutter signal is enabled and the actual DAQ starts.
  * `daq_start_stdalone` [`w`]: Starts DAQ immediately.
  * `daq_stop` [`w`]: Stops DAQ (any mode).
  * `daq_running` [`r`]: Status bit indicating that a DAQ is currently running and storing data.
  * `hv_enable` [`rw`]: Controls an external HV switch.
  * `ds_pd` [`rw`]: Controls `DS_PD` signal on a chipboard.
  * `ds_eq` [`rw`]: Controls `DS_EQ` signal on a chipboard.
  * `ds_pe` [`rw`]: Controls `DS_PE` signal on a chipboard.
  * `fpga_reset` [`rw`]: Resets the firmware. Does **not** reset the AXI bus and the configuration registers.
  * `fifo_pop` [`w`]: Throws away the next data frame from the readout data FIFO.
  * `led_trigger` [`rw`]: Writing `1` starts the LED pulser. The value is reset to `0` by firmware when the pulser finishes the sequence. Thus testing the register for `1` can be used to determine whether the pulser is still running.
  * `rd_data_available` [`r`]: Status bit is set to `1` when there is valid data in the readout data FIFO.

## Functions

Functions accessible through the peary CLI:
  * `configureClock`: Configures the clock generator on the carboard (Si5345 Rev B).
  * `powerStatusLog`: Lists the powering status of the device.
  * `configureMatrix`: Input: `MaskFile.cfg`. Writes a masking pattern to the chip registers. Allows to mask any pattern of pixels.
  * `writeConfig`: Apply the chip configuration stored in the chip registers.
  * `ledOn`: Enable LED trigger pulses.
  * `ledOff`: Disable LED trigger pulses.
  * `tOn`: Power the temperature diode.
  * `tOff`: Turn power for the temperature diode off.
  * `printFrames N`: Prints N frames to (`frameData.txt`) using an ASCII format.
  * `printLEDFrames N`: Triggers the LED and prints N frames to (`frameData.txt`) in an ASCII format. See *Utils* for analysis macros.
  * `storeFrames N filename.bin`: Prints N frames to `filename.bin` uing a binary format. See *Utils* for analysis macros. Filename is optional, defaults to `BINframeData.bin`.
  * `storeLEDFrames N filename.bin`: Triggers the LED and prints N frames to `filename.bin` uing a binary format. See *Utils* for analysis macros. Filename is optional, defaults to `BINframeData.bin`.
  * `printTemperatures N`: Prints N temperature measurements, taken every 10 s, to screen and to file (`temperatureData.txt`) using an ASCII format.

## Utils

Helpful root macros for the analysis of laboratory measurements and mask file generation in `devices\dSiPM\utils`:
  * `dSiPM_MaskGenerator.C` Usage (in root): \
  `.x dSiPM_MaskGenerator.C("frameData.txt","oldmask.cfg","newmask.cfg",CUT)` \
Calculates the Dark Count Rate (DCR) from a provided measurement file (e.g. `frameData.txt`). CUT defines the percentage of pixels to be masked. The masked pixels in `oldmask.cfg` are found and the pixels with the highest DCR are added until the defined percentage is reached.
  * `dSiPMFramePlotter.C` Usage: (in root): \
  `.x dSiPMFramePlotter.C("frameData.txt", SAVE, FRAMENR)` \
Plotting tool for frame data ASCII format. Generates a hitmap and projections. SAVE determines if the output plot should be stored as `.png`. If a FRAMENR is given, an event display of the corresponding frame is generated.
  * `dSiPMFrameDCR.C` Usage: (in root): \
  `.x dSiPMFrameDCR.C+("frameData.txt", SAVE, CUT, NFRAMES)` \
Analysis tool to determine the DCR from frame data in ASCII format. SAVE determines if the output plot should be stored as `.png`. CUT defines the percentage of noisiest pixels to be excluded. NFRAMES defines the number of frames asumed in the DCR calculation. Defaults to 10200.
  * `dSiPMDecodingTime.C+("frameData.txt", TFVAL_Q1, TFVAL_Q2, TFVAL_Q3, TFVAL_Q4, SAVE)` \
Analysis tool to determine the TimeStamps from frame data in ASCII format (`-pe 1` option needed in the converter). TFVAL_Qi is the resolution of the fine TDC in ps (default is 95.2 ps).
Generates a tree with all input data and histograms for analysis of timestamps: Fine & Coarse (TF & TC) time stamps for all data a for single quadrants, TimeOfArrival (TOA) for all the quadrants, Time difference between TOA of all quadrants combinations (TDIFF).
SAVE determines if the tree and output plots should be stored as `.root`

Binary to decode binary data files:
  * `peary/bin/dSiPMBinaryDecoder` Usage:
  `./dSiPMBinaryDecoder -h` \
It tells you how it works. Building needs to be enabled in CMake configuration.

## Data Format

The data is returned as 40 data words of 32 bit, as given in the listing below. The first 32 words contain the hit data from the dSiPM, the first 8 words contain the data from the first quadrant etc. (`chip_data[0]...`). The individual pixel addresses in the quadrant data block are described in the chip manual. The time information and the value of the valid bit (derived applying the configured validation logic) are contained in the following 7 data words (`chip_ts...`). Also their decoding is described in the chip manual. Additional information from the FPGA is contained in the last 3 data words (`rdout_...`), as given in the listing below. \
The data format for the 2-bit mode is the same. The only difference is that the hit information for one frame is distributed over two consecutive data blocks, keeping the association of the first 8 words with the first quadrant, and so on. The mapping between the pixel index and the position of a bit in the data block is again described in the chip manual.
```
rd_data_0  = {chip_data[0][31:0]};
rd_data_1  = {chip_data[0][63:32]};
rd_data_2  = {chip_data[0][95:64]};
rd_data_3  = {chip_data[0][127:96]};
rd_data_4  = {chip_data[0][159:128]};
rd_data_5  = {chip_data[0][191:160]};
rd_data_6  = {chip_data[0][223:192]};
rd_data_7  = {chip_data[0][255:224]};
rd_data_8  = {chip_data[1][31:0]};
rd_data_9  = {chip_data[1][63:32]};
rd_data_10 = {chip_data[1][95:64]};
rd_data_11 = {chip_data[1][127:96]};
rd_data_12 = {chip_data[1][159:128]};
rd_data_13 = {chip_data[1][191:160]};
rd_data_14 = {chip_data[1][223:192]};
rd_data_15 = {chip_data[1][255:224]};
rd_data_16 = {chip_data[2][31:0]};
rd_data_17 = {chip_data[2][63:32]};
rd_data_18 = {chip_data[2][95:64]};
rd_data_19 = {chip_data[2][127:96]};
rd_data_20 = {chip_data[2][159:128]};
rd_data_21 = {chip_data[2][191:160]};
rd_data_22 = {chip_data[2][223:192]};
rd_data_23 = {chip_data[2][255:224]};
rd_data_24 = {chip_data[3][31:0]};
rd_data_25 = {chip_data[3][63:32]};
rd_data_26 = {chip_data[3][95:64]};
rd_data_27 = {chip_data[3][127:96]};
rd_data_28 = {chip_data[3][159:128]};
rd_data_29 = {chip_data[3][191:160]};
rd_data_30 = {chip_data[3][223:192]};
rd_data_31 = {chip_data[3][255:224]};
rd_data_32 = {chip_ts[31:0]};
rd_data_33 = {chip_ts[63:32]};
rd_data_34 = {chip_ts[95:64]};
rd_data_35 = {chip_ts[127:96]};
rd_data_36 = {chip_ts[159:128]};
rd_data_37 = {chip_ts[191:160]};
rd_data_38 = {chip_ts[211:192], rdout_frame_second_2bit, rdout_frame_timestamp_check[2:0], rdout_bunch_timestamp_fine[7:0]};
rd_data_39 = {rdout_bunch_timestamp_coarse[31:0]};
rd_data_40 = {rdout_bunch_timestamp_coarse[39:32], rdout_trigger_id_out[23:0]};
```

### Timestamp and FPGA section

The timestamp section has an eight-fold symmetry. It can be useful to order the bits in word-sized blocks with 8 bits per block:

```
BC_Q1<39>, BC_Q2<39>, BC_Q3<39>, BC_Q4<39>, valid_Q1,  valid_Q2,  valid_Q3,  valid_Q4,
BC_Q1<38>, BC_Q2<38>, BC_Q3<38>, BC_Q4<38>, BC_Q1<12>, BC_Q2<12>, BC_Q3<12>, BC_Q4<12>,
BC_Q1<37>, BC_Q2<37>, BC_Q3<37>, BC_Q4<37>, BC_Q1<11>, BC_Q2<11>, BC_Q3<11>, BC_Q4<11>,
BC_Q1<36>, BC_Q2<36>, BC_Q3<36>, BC_Q4<36>, BC_Q1<10>, BC_Q2<10>, BC_Q3<10>, BC_Q4<10>,

BC_Q1<35>, BC_Q2<35>, BC_Q3<35>, BC_Q4<35>, BC_Q1<9>,  BC_Q2<9>,  BC_Q3<9>,  BC_Q4<9>,
BC_Q1<34>, BC_Q2<34>, BC_Q3<34>, BC_Q4<34>, BC_Q1<8>,  BC_Q2<8>,  BC_Q3<8>,  BC_Q4<8>,
BC_Q1<33>, BC_Q2<33>, BC_Q3<33>, BC_Q4<33>, BC_Q1<7>,  BC_Q2<7>,  BC_Q3<7>,  BC_Q4<7>,
BC_Q1<32>, BC_Q2<32>, BC_Q3<32>, BC_Q4<32>, BC_Q1<6>,  BC_Q2<6>,  BC_Q3<6>,  BC_Q4<6>,

BC_Q1<31>, BC_Q2<31>, BC_Q3<31>, BC_Q4<31>, BC_Q1<5>,  BC_Q2<5>,  BC_Q3<5>,  BC_Q4<5>,
BC_Q1<30>, BC_Q2<30>, BC_Q3<30>, BC_Q4<30>, BC_Q1<4>,  BC_Q2<4>,  BC_Q3<4>,  BC_Q4<4>,
BC_Q1<29>, BC_Q2<29>, BC_Q3<29>, BC_Q4<29>, BC_Q1<3>,  BC_Q2<3>,  BC_Q3<3>,  BC_Q4<3>,
BC_Q1<28>, BC_Q2<28>, BC_Q3<28>, BC_Q4<28>, BC_Q1<2>,  BC_Q2<2>,  BC_Q3<2>,  BC_Q4<2>,

BC_Q1<27>, BC_Q2<27>, BC_Q3<27>, BC_Q4<27>, BC_Q1<1>,  BC_Q2<1>,  BC_Q3<1>,  BC_Q4<1>,
BC_Q1<26>, BC_Q2<26>, BC_Q3<26>, BC_Q4<26>, BC_Q1<0>,  BC_Q2<0>,  BC_Q3<0>,  BC_Q4<0>,
BC_Q1<25>, BC_Q2<25>, BC_Q3<25>, BC_Q4<25>, fT_Q1<4>,  fT_Q2<4>,  fT_Q3<4>,  fT_Q4<4>,
BC_Q1<24>, BC_Q2<24>, BC_Q3<24>, BC_Q4<24>, fT_Q1<3>,  fT_Q2<3>,  fT_Q3<3>,  fT_Q4<3>,

BC_Q1<23>, BC_Q2<23>, BC_Q3<23>, BC_Q4<23>, fT_Q1<2>,  fT_Q2<2>,  fT_Q3<2>,  fT_Q4<2>,
BC_Q1<22>, BC_Q2<22>, BC_Q3<22>, BC_Q4<22>, fT_Q1<1>,  fT_Q2<1>,  fT_Q3<1>,  fT_Q4<1>,
BC_Q1<21>, BC_Q2<21>, BC_Q3<21>, BC_Q4<21>, fT_Q1<0>,  fT_Q2<0>,  fT_Q3<0>,  fT_Q4<0>,
BC_Q1<20>, BC_Q2<20>, BC_Q3<20>, BC_Q4<20>, cT_Q1<6>,  cT_Q2<6>,  cT_Q3<6>,  cT_Q4<6>,

BC_Q1<19>, BC_Q2<19>, BC_Q3<19>, BC_Q4<19>, cT_Q1<5>,  cT_Q2<5>,  cT_Q3<5>,  cT_Q4<5>,
BC_Q1<18>, BC_Q2<18>, BC_Q3<18>, BC_Q4<18>, cT_Q1<4>,  cT_Q2<4>,  cT_Q3<4>,  cT_Q4<4>,
BC_Q1<17>, BC_Q2<17>, BC_Q3<17>, BC_Q4<17>, cT_Q1<3>,  cT_Q2<3>,  cT_Q3<3>,  cT_Q4<3>,
BC_Q1<16>, BC_Q2<16>, BC_Q3<16>, BC_Q4<16>, cT_Q1<2>,  cT_Q2<2>,  cT_Q3<2>,  cT_Q4<2>,

BC_Q1<15>, BC_Q2<15>, BC_Q3<15>, BC_Q4<15>, cT_Q1<1>,  cT_Q2<1>,  cT_Q3<1>,  cT_Q4<1>,
BC_Q1<14>, BC_Q2<14>, BC_Q3<14>, BC_Q4<14>, cT_Q1<0>,  cT_Q2<0>,  cT_Q3<0>,  cT_Q4<0>,
BC_Q1<13>, BC_Q2<13>, BC_Q3<13>, BC_Q4<13>, F_2nd2bit, F_TS_C<2>, F_TS_C<1>, F_TS_C<0>,
F_BCF<7>,  F_BCF<6>,  F_BCF<5>,  F_BCF<4>,  F_BCF<3>,  F_BCF<2>,  F_BCF<1>,  F_BCF<0>,

F_BCC<31>, F_BCC<30>, F_BCC<29>, F_BCC<28>, F_BCC<27>, F_BCC<26>, F_BCC<25>, F_BCC<24>,
F_BCC<23>, F_BCC<22>, F_BCC<21>, F_BCC<20>, F_BCC<19>, F_BCC<18>, F_BCC<17>, F_BCC<16>,
F_BCC<15>, F_BCC<14>, F_BCC<13>, F_BCC<12>, F_BCC<11>, F_BCC<10>, F_BCC<9>,  F_BCC<8>,
F_BCC<7>,  F_BCC<6>,  F_BCC<5>,  F_BCC<4>,  F_BCC<3>,  F_BCC<2>,  F_BCC<1>,  F_BCC<0>,

F_BCC<39>, F_BCC<38>, F_BCC<37>, F_BCC<36>, F_BCC<35>, F_BCC<34>, F_BCC<33>, F_BCC<32>,
F_TID<23>, F_TID<22>, F_TID<21>, F_TID<20>, F_TID<19>, F_TID<18>, F_TID<17>, F_TID<16>,
F_TID<15>, F_TID<14>, F_TID<13>, F_TID<12>, F_TID<11>, F_TID<10>, F_TID<9>,  F_TID<8>,
F_TID<7>,  F_TID<6>,  F_TID<5>,  F_TID<4>,  F_TID<3>,  F_TID<2>,  F_TID<1>,  F_TID<0>,
```

Naming:
- `BC_QX<N>` : chip bunch counter of quadrant `X`, bit `N` (3 MHz)
- `cT_QX<N>` : chip coarse timestamp of quadrant `X`, bit `N` (408 MHz)
- `fT_QX<N>` : chip fine timestamp of quadrant `X`, bit `N` (32 * 408 MHz)
- `valid_QX` : chip valid bit of quadrant `X`
- `F_2nd2bit` : FPGA 2nd frame 2bit
- `F_TS_C<N>` : FPGA timestamp check, bit `N`
- `F_BCC<N>` : FPGA coarse timestamp, bit `N` (3 MHz)
- `F_BCF<N>` : FPGA fine timestamp, bit `N` (408 MHz)
- `F_TID<N>` : FPGA trigger ID, bit `N`

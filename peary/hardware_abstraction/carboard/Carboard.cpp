/**
 * Caribou Carboard HAL class implementation
 */

#include "Carboard.hpp"

using namespace caribou;
using namespace carboard;

Carboard::Carboard() : caribouHAL() {

  // Log the firmware
  LOG(STATUS) << getFirmwareVersion();

  //[AQ] Don't get board ID now, this will cause a crash if we don't have a CaR board.
  LOG(WARNING) << "Skipped getting board ID (Not Implemented for ZCU102)";
  //LOG(STATUS) << "Board ID: " << static_cast<int>(getBoardID()) << " Board Revision: " << to_hex_string(getBoardRev());

  LOG(WARNING) << "Skipped initialization (Not Implemented for ZCU102)";
  //this->Initialize();
}

std::string Carboard::getFirmwareVersion() {

  const auto firmwareVersion = this->readMemory(firmware_mem, CARIBOU_FIRMWARE_VERSION_OFFSET);
  const uint8_t day = (firmwareVersion >> 27) & 0x1F;
  const uint8_t month = (firmwareVersion >> 23) & 0xF;
  const uint32_t year = static_cast<uint32_t>(2000 + ((firmwareVersion >> 17) & 0x3F));
  const uint8_t hour = (firmwareVersion >> 12) & 0x1F;
  const uint8_t minute = (firmwareVersion >> 6) & 0x3F;
  const uint8_t second = firmwareVersion & 0x3F;

  std::stringstream s;
  s << "Firmware version: " << to_hex_string(firmwareVersion) << " (" << static_cast<int>(day) << "/"
    << static_cast<int>(month) << "/" << static_cast<int>(year) << " " << static_cast<int>(hour) << ":"
    << static_cast<int>(minute) << ":" << static_cast<int>(second) << ")";

  return s.str();
}

void Carboard::generalReset() {
  // Disable all Voltage Regulators
  LOG(DEBUG) << "Disabling all Voltage regulators";
  iface_i2c& i2c0 = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_IOEXP));
  i2c0.write(0x2, {0x00, 0x00}); // disable all bits of Port 1-2 (internal register)
  i2c0.write(0x6, {0x00, 0x00}); // set all bits of Port 1-2 in output mode

  LOG(DEBUG) << "Disabling all current sources";
  powerDAC(false, CUR_1.address(), CUR_1.channel());
  powerDAC(false, CUR_2.address(), CUR_2.channel());
  powerDAC(false, CUR_3.address(), CUR_3.channel());
  powerDAC(false, CUR_4.address(), CUR_4.channel());
  powerDAC(false, CUR_5.address(), CUR_5.channel());
  powerDAC(false, CUR_6.address(), CUR_6.channel());
  powerDAC(false, CUR_7.address(), CUR_7.channel());
  powerDAC(false, CUR_8.address(), CUR_8.channel());

  // In the current CaR version it has been disabled
  // Enabling DC/DC converters
  // setDCDCConverter(LTM_VPWR1, 5 );
  // setDCDCConverter(LTM_VPWR2, 5 );
  // setDCDCConverter(LTM_VPWR3, 5 );
  caribouHAL::generalReset();
}

uintptr_t Carboard::getFirmwareRegister(uint16_t) {
  throw FirmwareException("Functionality not implemented.");
}

uint8_t Carboard::getBoardID() {

  LOG(DEBUG) << "Reading board ID from CaR EEPROM";
  iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_EEPROM));

  auto data = myi2c.wordread(ADDR_EEPROM_BOARD_ID, 1);

  if(data.empty()) {
    throw CommunicationError("No data returned");
  }
  return data.front();
}

uint8_t Carboard::readEEPROM(uint16_t addr) {
  iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_EEPROM));

  LOG(DEBUG) << "Reading from EEPROM...";
  auto data = myi2c.wordread(addr, 1);

  LOG(INFO) << "Value at " << to_hex_string(addr) << " is " << to_hex_string(data.front());

  return data.front();
}

void Carboard::writeEEPROM(uint16_t addr, int val) {
  iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_EEPROM));
  iface_i2c::dataVector_type data = {static_cast<uint8_t>(val)};

  LOG(DEBUG) << "Writing to EEPROM...";
  myi2c.wordwrite(addr, data);
}

uint8_t Carboard::getBoardRev() {

  LOG(DEBUG) << "Determining board revision...";
  iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_CMOSDAC));

  if(myi2c.probe(0)) {
    LOG(DEBUG) << "Detected v1.3 board";
    return CAR_REV_1_3;
  } else {
    LOG(DEBUG) << "Detected v1.1 board";
    return CAR_REV_1_1;
  }
}

double Carboard::readTemperature() {

  // Two bytes must be read, containing 12bit of temperature information plus 4bit 0.
  // Negative numbers are represented in binary twos complement format.

  LOG(DEBUG) << "Reading temperature from TMP101";
  iface_i2c& myi2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_TEMP));

  // Read the two temperature bytes from the TMP101:
  auto data = myi2c.read(REG_TEMP_TEMP, 2);
  if(data.size() != 2) {
    throw CommunicationError("No data returned");
  }

  // FIXME correctly handle 2's complement for negative temperatures!
  auto temp = static_cast<int16_t>(((data.front() << 8) | data.back()) >> 4);
  return temp * 0.0625;
}

void Carboard::powerDCDCConverter(const DCDC_CONVERTER_T, const bool) {
  throw FirmwareException("Functionality not implemented.");
}

void Carboard::setDCDCConverter(const DCDC_CONVERTER_T converter, const double voltage) {
  LOG(DEBUG) << "Setting " << voltage << "V "
             << "on " << converter.name();

  if(voltage > 5 || voltage < 0) {
    throw ConfigInvalid("Trying to set DC/DC converter to " + std::to_string(voltage) + " V (range is 0-5 V)");
  }

  powerDAC(false, converter.address(), converter.channel());
  setDACVoltage(converter.address(), converter.channel(), -0.1824 * voltage + 0.9120);
  powerDAC(true, converter.address(), converter.channel());
}

void Carboard::setBiasRegulator(const BIAS_REGULATOR_T regulator, const double voltage) {
  LOG(DEBUG) << "Setting bias " << voltage << "V "
             << "on " << regulator.name();

  if(voltage > 3.5 || voltage < 0) {
    throw ConfigInvalid("Trying to set bias regulator to " + std::to_string(voltage) + " V (range is 0-3.5 V)");
  }

  setDACVoltage(regulator.address(), regulator.channel(), voltage);
}

void Carboard::powerBiasRegulator(const BIAS_REGULATOR_T regulator, const bool enable) {

  if(enable) {
    LOG(DEBUG) << "Powering up " << regulator.name();
    powerDAC(true, regulator.address(), regulator.channel());
  } else {
    LOG(DEBUG) << "Powering down " << regulator.name();
    powerDAC(false, regulator.address(), regulator.channel());
  }
}

void Carboard::setVoltageRegulator(const VOLTAGE_REGULATOR_T regulator,
                                   const double voltage,
                                   const double maxExpectedCurrent) {
  LOG(DEBUG) << "Setting " << voltage << "V "
             << "on " << regulator.name();

  if(voltage > 3.5 || voltage < 0) {
    throw ConfigInvalid("Trying to set Voltage regulator to " + std::to_string(voltage) + " V (range is 0-3.5 V)");
  }

  setDACVoltage(regulator.address(), regulator.channel(), 3.6 - voltage);

  // set current/power monitor
  setCurrentMonitor(regulator.pwrmonitor(), maxExpectedCurrent, true);
}

void Carboard::powerVoltageRegulator(const VOLTAGE_REGULATOR_T regulator, const bool enable) {

  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_IOEXP));

  if(enable) {
    LOG(DEBUG) << "Powering up " << regulator.name();

    // First power on DAC
    powerDAC(true, regulator.address(), regulator.channel());
    usleep(100000);
    // Power on the Voltage regulator
    auto mask = i2c.read(0x03, 1)[0];
    mask = static_cast<uint8_t>(mask | (1 << regulator.pwrswitch()));
    i2c.write(std::make_pair(0x03, mask));
  } else {
    LOG(DEBUG) << "Powering down " << regulator.name();

    // Disable the Volage regulator
    auto mask = i2c.read(0x03, 1)[0];
    mask = static_cast<uint8_t>(mask & ~(1 << regulator.pwrswitch()));
    i2c.write(std::make_pair(0x03, mask));

    // Disable the DAC
    powerDAC(false, regulator.address(), regulator.channel());
  }
}

void Carboard::setCurrentSource(const CURRENT_SOURCE_T source,
                                const unsigned int current,
                                const CURRENT_SOURCE_POLARITY_T polarity) {

  LOG(DEBUG) << "Setting " << current << "uA "
             << "on " << source.name();

  if(current > 1000) {
    throw ConfigInvalid("Trying to set current source to " + std::to_string(current) + " uA (max is 1000uA)");
  }

  // set DAC
  setDACVoltage(source.address(), source.channel(), (current * CAR_VREF_4P0) / 1000);

  // set polarisation
  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_IOEXP));
  auto mask = i2c.read(0x02, 1)[0];

  if(polarity == CURRENT_SOURCE_POLARITY_T::PULL) {
    LOG(DEBUG) << "Polarity switch (" << to_hex_string(source.polswitch()) << ") set to PULL";
    mask = static_cast<uint8_t>(mask & ~(1 << source.polswitch()));
  } else if(polarity == CURRENT_SOURCE_POLARITY_T::PUSH) {
    LOG(DEBUG) << "Polarity switch (" << to_hex_string(source.polswitch()) << ") set to PUSH";
    mask = static_cast<uint8_t>(mask | (1 << source.polswitch()));
  } else {
    throw ConfigInvalid("Invalid polarity setting provided");
  }

  i2c.write(std::make_pair(0x02, mask));
}

void Carboard::powerCurrentSource(const CURRENT_SOURCE_T source, const bool enable) {
  if(enable) {
    LOG(DEBUG) << "Powering up " << source.name();
    powerDAC(true, source.address(), source.channel());
  } else {
    LOG(DEBUG) << "Powering down " << source.name();
    powerDAC(false, source.address(), source.channel());
  }
}

void Carboard::setDACVoltage(const uint8_t device, const uint8_t address, const double voltage) {

  // Control voltages using DAC7678 with QFN packaging
  // All DAc7678 use straight binary encoding since the TWOC pins are pulled low

  // All DAC voltage regulators on the CaR board are on the BUS_I2C3:
  LOG(DEBUG) << "Setting voltage " << voltage << "V "
             << "on DAC7678 at " << to_hex_string(device) << " channel " << to_hex_string(address);
  // Workaround for the new DAC on the v1.3 CaR board, which is on BUS_I2C0
  iface_i2c& myi2c =
    InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(device == 0x48 ? BUS_I2C0 : BUS_I2C3, device));

  // Per default, the internal reference is switched off,
  // with external reference we have: voltage = d_in/4096*v_refin
  auto d_in = static_cast<uint16_t>(voltage * 4096 / CAR_VREF_4P0);

  // with internal reference of 2.5V we have: voltage = d_in/4096*2*2.5
  //  -> d_in = voltage/2.5 * 4096/2

  // Check out of range values
  if(d_in >= 4096) {
    d_in = 4095;
  }

  iface_i2c::dataVector_type command = {static_cast<uint8_t>(d_in >> 4), static_cast<uint8_t>(d_in << 4)};

  // Set DAC and update: combine command with channel via Control&Access byte:
  uint8_t reg = REG_DAC_WRUP_CHANNEL | address;

  // Send I2C write command
  myi2c.write(reg, command);
}

double Carboard::getDACVoltage(const uint8_t device, const uint8_t address) {

  // Control voltages using DAC7678 with QFN packaging
  // All DAc7678 use straight binary encoding since the TWOC pins are pulled low

  // All DAC voltage regulators on the CaR board are on the BUS_I2C3:
  LOG(DEBUG) << "Getting voltage from DAC7678 at " << to_hex_string(device) << " channel " << to_hex_string(address);

  // Workaround for the new DAC on the v1.3 CaR board, which is on BUS_I2C0
  iface_i2c& myi2c =
    InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(device == 0x48 ? BUS_I2C0 : BUS_I2C3, device));

  // Set DAC and update: combine command with channel via Control&Access byte:
  uint8_t reg = REG_DAC_READ_CHANNEL | address;

  auto data = myi2c.read(reg, 2);
  if(data.size() != 2) {
    throw CommunicationError("No data returned");
  }

  auto dac = (static_cast<uint16_t>(data.at(0)) << 4) | (static_cast<uint16_t>(data.at(1)) >> 4);
  LOG(DEBUG) << "Register value read: " << dac;

  // Per default, the internal reference is switched off,
  // with external reference we have: voltage = d_in/4096*v_refin
  return static_cast<double>(dac) * CAR_VREF_4P0 / 4096.;
}



void Carboard::powerDAC(const bool enable, const uint8_t device, const uint8_t address) {

  // Control voltages using DAC7678 with QFN packaging
  // All DAc7678 use straight binary encoding since the TWOC pins are pulled low

  // All DAC voltage regulators on the CaR board are on the BUS_I2C3:
  LOG(DEBUG) << "Powering " << (enable ? "up" : "down") << " channel " << to_hex_string(address) << " on DAC7678 at "
             << to_hex_string(device);
  // Workaround for the new DAC on the v1.3 CaR board, which is on BUS_I2C0
  iface_i2c& myi2c =
    InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(device == 0x48 ? BUS_I2C0 : BUS_I2C3, device));

  // Set the correct channel bit to be powered up/down:
  auto channel_bits = static_cast<uint16_t>(2 << address);
  iface_i2c::dataVector_type command = {
    static_cast<uint8_t>((enable ? (REG_DAC_POWERUP | channel_bits >> 4) : (REG_DAC_POWERDOWN_HZ | channel_bits >> 4)) &
                         0xFF),
    static_cast<uint8_t>((channel_bits << 4) & 0xFF)};

  // Send I2C write command
  myi2c.write(REG_DAC_POWER, command);
}

void Carboard::disableSI5345() {
  LOG(DEBUG) << "Disabling SI5345";

  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_CLKGEN));
  auto page = static_cast<uint8_t>(si5345_revb_registers_clkoff[0].address >> 8); // first page to be used

  i2c.write(std::make_pair(0x01, page)); // set first page

  for(size_t i = 0; i < SI5345_REVB_REG_CONFIG_NUM_REGS; i++) {
    if(page != si5345_revb_registers_clkoff[i].address >> 8) { // adjust page
      page = static_cast<uint8_t>(si5345_revb_registers_clkoff[i].address >> 8);
      i2c.write(std::make_pair(0x01, page));
    }
    i2c.write(std::make_pair(si5345_revb_registers_clkoff[i].address & 0xFF, si5345_revb_registers_clkoff[i].value));
  }
}

void Carboard::configureSI5345(SI5345_REG_T const* const regs, const size_t length) {
  LOG(DEBUG) << "Configuring SI5345";

  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_CLKGEN));
  auto page = static_cast<uint8_t>(regs[0].address >> 8); // first page to be used

  i2c.write(std::make_pair(0x01, page)); // set first page

  for(size_t i = 0; i < length; i++) {
    if(page != regs[i].address >> 8) { // adjust page
      page = static_cast<uint8_t>(regs[i].address >> 8);
      i2c.write(std::make_pair(0x01, page));
    }
    i2c.write(std::make_pair(regs[i].address & 0xFF, regs[i].value));
  }
}

iface_i2c::dataVector_type Carboard::readSI5345(int page) {
  iface_i2c::dataVector_type data;

  iface_i2c& i2c =
    InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(carboard::BUS_I2C0, carboard::ADDR_CLKGEN));
  LOG(DEBUG) << "read SI5345 registers page " << page;
  i2c.write({0x01, page}); // set page

  for(int i = 0; i < 32; i++) {
    // registers have to be accessed 1 by 1, each contains 8 byte
    // 1 page contains 32  register => 256 byte
    // 1st register is located at 0x01
    auto tmp = i2c.read(i2c_reg_t(i * 8), 8);
    for(size_t j = 0; j < tmp.size(); j++) {
      data.push_back(tmp[j]);
    }
  }
  return data;
}

bool Carboard::isLockedSI5345() {
  LOG(DEBUG) << "Checking lock status of SI5345";

  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, ADDR_CLKGEN));
  i2c.write(std::make_pair(0x01, 0x00)); // set first page
  auto rx = i2c.read(static_cast<uint8_t>(0x0E), 1);
  if((rx[0] & 0x2) != 0) {
    LOG(DEBUG) << "SI5345 is not locked";
    return false;
  } else {
    LOG(DEBUG) << "SI5345 is locked";
    return true;
  }
}

void Carboard::setUSRCLK(const uint64_t frequency) {
  // BUS_I2C4
  // ADDR_SI570

  // unbind linux driver:
  // echo 1-005d > /sys/bus/i2c/drivers/si570/unbind

  // code below is based on Si570 linux driver

  const uint32_t FREQ_MAX = 810000000;
  const uint32_t FREQ_MIN = 10000000;
  const uint32_t FXTAL_MAX = 130000000;
  const uint32_t FXTAL_MIN = 100000000;
  const uint64_t FDCO_MAX = 5670000000;
  const uint64_t FDCO_MIN = 4850000000;
  const uint64_t FOUT_DEF = 156250000;

  if(frequency < FREQ_MIN || frequency > FREQ_MAX) {
    LOG(ERROR) << "Requested frequency " << frequency << "Hz is out of permitted range " << FREQ_MIN << "Hz - " << FREQ_MAX
               << "Hz. Aborting.";
    return;
  }

  uint32_t i;
  uint32_t n1, hs_div, out_n1, out_hs_div, fxtal;
  uint64_t out_rfreq, fdco, best_fdco = UINT64_MAX;

  // Available HS_DIV values:
  static const uint8_t si570_hs_div_values[] = {11, 9, 7, 6, 5, 4};

  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C4, ADDR_SI570));

  // To get fxtal, we have to reset the oscillator to default values, read them out and calculate fxtal.
  LOG(DEBUG) << "resetting oscillator";
  i2c.write(std::make_pair(135, 0x01)); // RECALL

  LOG(DEBUG) << "reading oscillator default values";
  auto rx = i2c.read(static_cast<uint8_t>(0x07), 6);

  out_hs_div = ((rx[0] & 0xE0) >> 5) + 4u;
  out_n1 = (((static_cast<uint32_t>(rx[0]) & 0x1F) << 2) | ((static_cast<uint32_t>(rx[1]) & 0xC0) >> 6)) + 1u;
  if(out_n1 > 1)
    out_n1 &= ~1u;
  out_rfreq = static_cast<uint32_t>(rx[5]) | (static_cast<uint32_t>(rx[4]) << 8) | (static_cast<uint32_t>(rx[3]) << 16) |
              (static_cast<uint32_t>(rx[2]) << 24) | ((static_cast<uint64_t>(rx[1]) & 0x3F) << 32);
  fdco = FOUT_DEF * static_cast<uint64_t>(out_hs_div * out_n1);
  fxtal = static_cast<uint32_t>((fdco << 28) / out_rfreq);

  LOG(DEBUG) << "Oscillator default values: HS_DIV = " << out_hs_div << "; N1 = " << out_n1 << "; RFREQ = " << out_rfreq
             << "; FDCO = " << fdco;

  if(fxtal < FXTAL_MIN || fxtal > FXTAL_MAX) {
    LOG(ERROR) << "Calculated crystal frequency " << fxtal << "Hz is out of sensible range. Something went wrong. Aborting.";
    return;
  } else {
    LOG(DEBUG) << "FXTALL = " << fxtal;
  }
  // Calculating fxtal can be done only once per device and then stored for further use.
  // Here, we do it each time because of simplicity. The time overhead is negligable for our use case.
  // However, fxtal from one oscilator shall not be used for a same type of the oscillator on another board.

  LOG(DEBUG) << "Searching parameters to mach new frequency of " << frequency;

  for(i = 0; i < 6; i++) {
    hs_div = si570_hs_div_values[i];
    LOG(DEBUG) << "Searching parameters for HS_DIV = " << hs_div;
    // Find lowest possible value for n1
    n1 = static_cast<uint32_t>((FDCO_MIN / hs_div) / frequency);
    if(!n1 || (n1 & 1))
      n1++;
    while(n1 <= 128) {
      fdco = static_cast<uint64_t>(hs_div * n1) * static_cast<uint64_t>(frequency);
      if(fdco > FDCO_MAX)
        break;
      if(fdco >= FDCO_MIN && fdco < best_fdco) {
        out_n1 = n1;
        out_hs_div = hs_div;
        out_rfreq = ((fdco << 28) / fxtal);
        best_fdco = fdco;
        LOG(DEBUG) << "Found FDCO = " << best_fdco << " with N1 = " << out_n1 << " and RFREQ = " << out_rfreq;
      }
      n1 += (n1 == 1 ? 1 : 2);
    }
  }

  if(best_fdco == UINT64_MAX) {
    // This should not happen for a valid frequency, but just in case...
    LOG(ERROR) << "Did not find valid settings for output frequency = " << frequency;
    return;
  }

  iface_i2c::dataVector_type out_data;

  LOG(DEBUG) << "Freezing DCO";
  auto rconf = i2c.read(static_cast<uint8_t>(137), 1);
  rconf[0] |= 0x10;
  i2c.write(std::make_pair(137, rconf[0])); // Freeze DCO

  // Prepare configuration data:
  out_data.push_back(static_cast<uint8_t>((out_hs_div - 4) << 5) | static_cast<uint8_t>(((out_n1 - 1) >> 2) & 0x1F));
  out_data.push_back(static_cast<uint8_t>((out_n1 - 1) << 6) | static_cast<uint8_t>((out_rfreq >> 32) & 0x3F));
  out_data.push_back((out_rfreq >> 24) & 0xff);
  out_data.push_back((out_rfreq >> 16) & 0xff);
  out_data.push_back((out_rfreq >> 8) & 0xff);
  out_data.push_back(out_rfreq & 0xff);

  LOG(DEBUG) << "Writing new parameters to the oscillator";
  i2c.write(7, out_data); //

  LOG(DEBUG) << "Unfreezing DCO and notifying DSPLL";
  rconf = i2c.read(static_cast<uint8_t>(137), 1);
  rconf[0] &= 0xEF;
  i2c.write(std::make_pair(137, rconf[0])); // Unfreeze DCO
  i2c.write(std::make_pair(135, 0x40));     // NewFreq Alert to DSPLL
  // NewFreq alert must be sent latest 10ms after unfreezing DCO, otherwise parameters will be reset.

  usleep(10000);
  LOG(INFO) << "USRCLK set to new frequency " << frequency << "Hz";
  return;
}

void Carboard::configurePulser(unsigned channel_mask,
                               const bool ext_trigger,
                               const bool ext_trig_edge,
                               const bool idle_state) {
  // use only 4 LSBs as a mask
  channel_mask &= 0x0F;
  auto pulserControl = static_cast<uint32_t>(this->readMemory(pulser, CARIBOU_PULSER_REG_CONTROL_OFFSET));

  // select idle output state
  channel_mask <<= 8;
  if(idle_state) {
    pulserControl |= (channel_mask);
  } else {
    pulserControl &= (~channel_mask);
  }
  // set/unset ext. triggering
  channel_mask <<= 8;
  if(ext_trigger) {
    pulserControl |= (channel_mask);
  } else {
    pulserControl &= (~channel_mask);
  }
  // select ext. trigger edge
  channel_mask <<= 4;
  if(ext_trig_edge) {
    pulserControl |= (channel_mask);
  } else {
    pulserControl &= (~channel_mask);
  }

  this->writeMemory(pulser, CARIBOU_PULSER_REG_CONTROL_OFFSET, pulserControl);
}

void Carboard::startPulser(unsigned channel_mask) {

  // use only 4 LSBs as a mask
  channel_mask &= 0x0F;

  auto pulserControl = static_cast<uint32_t>(this->readMemory(pulser, CARIBOU_PULSER_REG_CONTROL_OFFSET));
  pulserControl |= (channel_mask);
  this->writeMemory(pulser, CARIBOU_PULSER_REG_CONTROL_OFFSET, pulserControl);
}

void Carboard::enablePulser(unsigned channel_mask) {

  // use only 4 LSBs as a mask
  channel_mask &= 0x0F;

  auto pulserControl = static_cast<uint32_t>(this->readMemory(pulser, CARIBOU_PULSER_REG_CONTROL_OFFSET));
  pulserControl &= ~(channel_mask << 4);
  this->writeMemory(pulser, CARIBOU_PULSER_REG_CONTROL_OFFSET, pulserControl);
}

void Carboard::disablePulser(unsigned channel_mask) {

  // use only 4 LSBs as a mask
  channel_mask &= 0x0F;

  auto pulserControl = static_cast<uint32_t>(this->readMemory(pulser, CARIBOU_PULSER_REG_CONTROL_OFFSET));
  pulserControl |= (channel_mask << 4);
  this->writeMemory(pulser, CARIBOU_PULSER_REG_CONTROL_OFFSET, pulserControl);
}

uint32_t Carboard::getPulserRunning() {
  return (this->readMemory(pulser, CARIBOU_PULSER_REG_STATUS_OFFSET) & 0x0F);
}

uint32_t Carboard::getPulseCount(const uint32_t channel) {

  // check for valid channel range 1-4
  if(channel > 4 || channel < 1) {
    throw ConfigInvalid("There is no channel " + std::to_string(channel) + " available. The valid range is 1-4.");
  }

  return static_cast<uint32_t>(
    this->readMemory(pulser, CARIBOU_PULSER_REG_PULSE_COUNT_OFFSET + (CARIBOU_PULSER_CHANNEL_OFFSET * channel)));
}

void Carboard::configurePulseParameters(const unsigned channel_mask,
                                        const uint32_t periods,
                                        const uint32_t time_active,
                                        const uint32_t time_idle,
                                        const double voltage) {

  uint32_t channel = 1;
  uint32_t channel_int = 1;
  // for all 4 channels
  while(true) {
    // apply the settings only for channels enabled in the mask, skip others
    if((channel_mask & channel) != 0) {
      continue;
    }

    switch(channel) {
    case 0b0001:
      setBiasRegulator(INJ_1, voltage);
      powerBiasRegulator(INJ_1, true);
      break;
    case 0b0010:
      setBiasRegulator(INJ_2, voltage);
      powerBiasRegulator(INJ_2, true);
      break;
    case 0b0100:
      setBiasRegulator(INJ_3, voltage);
      powerBiasRegulator(INJ_3, true);
      break;
    case 0b1000:
      setBiasRegulator(INJ_4, voltage);
      powerBiasRegulator(INJ_4, true);
      break;
    default:
      throw FirmwareException("Something went terribly wrong in Carboard::configurePulseParameters() and the function "
                              "reached a state where it should not be.");
    }

    this->writeMemory(pulser, CARIBOU_PULSER_REG_PERIODS_OFFSET + channel_int * CARIBOU_PULSER_CHANNEL_OFFSET, periods);
    this->writeMemory(
      pulser, CARIBOU_PULSER_REG_TIME_ACTIVE_OFFSET + channel_int * CARIBOU_PULSER_CHANNEL_OFFSET, time_active);
    this->writeMemory(pulser, CARIBOU_PULSER_REG_TIME_IDLE_OFFSET + channel_int * CARIBOU_PULSER_CHANNEL_OFFSET, time_idle);

    // go to next channel
    channel <<= 1;
    channel_int++;
    // check if we are still in the range of available channels. If not, we are done.
    if((channel & 0x0F) != 0) {
      return;
    }
  }
}

void Carboard::setCurrentMonitor(const uint8_t device, const double maxExpectedCurrent, bool enableLimit) {
  LOG(DEBUG) << "Setting maxExpectedCurrent " << maxExpectedCurrent << "A "
             << "on INA226 at " << to_hex_string(device);
  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C1, device));

  // Set configuration register:
  uint16_t conf = (1 << 14);
  // Average over 16 samples:
  conf |= (0x7 << 9);
  // Use a bus voltage conversion time of 140 us:
  conf |= (0x0 << 6);
  // Use a shunt voltage conversion time of 140us:
  conf |= (0x0 << 3);
  // Operation mode: continuous mesaurement of shunt and bus voltage:
  conf |= 0x7;
  i2c.write(REG_ADC_CONFIGURATION, {static_cast<i2c_t>(conf >> 8), static_cast<i2c_t>(conf & 0xFF)});

  // set calibration register

  // Calculate current LSB from max expected current:
  double current_lsb = maxExpectedCurrent / (0x1 << 15);
  uint16_t cal = 0;

  // Prevent overflow for currents <= 512mA
  if(maxExpectedCurrent < 0.513) {
    cal = 0x7FFF;
  } else {
    cal = static_cast<uint16_t>(0.00512 / (CAR_INA226_R_SHUNT * current_lsb));
  }
  LOG(DEBUG) << "  cal_register = " << cal << " (" << to_hex_string(cal) << ")";

  // Calculate current LSB from actual calibration register value
  current_lsb = 0.00512 / (cal * CAR_INA226_R_SHUNT);
  LOG(DEBUG) << "  current_lsb  = " << (current_lsb * 1e6) << " uA/bit";

  i2c.write(REG_ADC_CALIBRATION, {static_cast<i2c_t>(cal >> 8), static_cast<i2c_t>(cal & 0xFF)});

  if(getBoardRev() == CAR_REV_1_3) {
    i2c.read(REG_ADC_ENABLE, 2); // Clear Alert Latch

    if(enableLimit) {
      // Shunt voltage LSB is 2.5uV
      auto limit = static_cast<uint16_t>(maxExpectedCurrent * CAR_INA226_R_SHUNT / 2.5e-6);

      LOG(DEBUG) << "  enabling current limit";
      LOG(DEBUG) << "  limit_register = " << static_cast<double>(limit) << " (" << to_hex_string(limit) << ")";

      i2c.write(REG_ADC_LIMIT, {static_cast<uint8_t>(limit >> 8), static_cast<uint8_t>(limit & 0xFF)});
      i2c.write(REG_ADC_ENABLE, {1 << 7, 1}); // Shunt Voltage Over-Voltage and Alert Latch Enable
    } else {
      i2c.write(REG_ADC_ENABLE, {0x00, 0x00});
    }
  }
}

double Carboard::readBias(const BIAS_REGULATOR_T regulator) {
  LOG(DEBUG) << "Reading bias from " << regulator.name();
  return getDACVoltage(regulator.address(), regulator.channel());
}

double Carboard::measureVoltage(const VOLTAGE_REGULATOR_T regulator) {

  const i2c_address_t device = regulator.pwrmonitor();
  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C1, device));

  LOG(DEBUG) << "Reading bus voltage from INA226 at " << to_hex_string(device);
  auto voltage = i2c.read(REG_ADC_BUS_VOLTAGE, 2);

  // INA226: fixed LSB for voltage measurement: 1.25mV
  return (static_cast<unsigned int>(voltage.at(0) << 8) | voltage.at(1)) * 0.00125;
}

// FIXME: somtimes it returns dummy values
double Carboard::measureCurrent(const VOLTAGE_REGULATOR_T regulator) {
  const i2c_address_t device = regulator.pwrmonitor();
  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C1, device));
  LOG(DEBUG) << "Reading current from INA226 at " << to_hex_string(device);

  // Reading back the calibration register:
  auto cal_v = i2c.read(REG_ADC_CALIBRATION, 2);
  double current_lsb = 0.00512 / ((static_cast<uint16_t>(cal_v.at(0) << 8) | cal_v.at(1)) * CAR_INA226_R_SHUNT);
  LOG(DEBUG) << "  current_lsb  = " << (current_lsb * 1e6) << " uA/bit";

  // Reading the current register:
  auto current_raw = i2c.read(REG_ADC_CURRENT, 2);
  return (static_cast<unsigned int>(current_raw.at(0) << 8) | current_raw.at(1)) * current_lsb;
}

// FIXME: sometimes it return dummy values
double Carboard::measurePower(const VOLTAGE_REGULATOR_T regulator) {
  const i2c_address_t device = regulator.pwrmonitor();
  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C1, device));
  LOG(DEBUG) << "Reading power from INA226 at " << to_hex_string(device);

  // Reading back the calibration register:
  auto cal_v = i2c.read(REG_ADC_CALIBRATION, 2);
  // Power LSB is fixed at 25x current LSB
  double power_lsb = (0.00512 * 25) / ((static_cast<uint16_t>(cal_v.at(0) << 8) | cal_v.at(1)) * CAR_INA226_R_SHUNT);
  LOG(DEBUG) << "  power_lsb  = " << (power_lsb * 1e6) << " uW/bit";

  // Reading the power register:
  auto power_raw = i2c.read(REG_ADC_POWER, 2);
  return (static_cast<unsigned int>(power_raw[0] << 8) | power_raw[1]) * power_lsb;
}

double Carboard::readSlowADC(const SLOW_ADC_CHANNEL_T channel) {
  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C3, ADDR_ADC));

  LOG(DEBUG) << "Sampling channel " << channel.name() << " on pin " << static_cast<int>(channel.channel())
             << " of ADS7828 at " << to_hex_string(ADDR_ADC);

  uint8_t command = channel.address();

  // Enable single-ended mode, no differential measurement:
  command = command ^ 0x80;

  // We use the external reference voltage CAR_VREF_4P0, so do not switch to internal,
  // but keeb A/D power on
  command = command ^ 0x04;

  auto volt_raw = i2c.read(command, 2);
  auto readback = static_cast<uint16_t>((volt_raw[0] << 8) | volt_raw[1]);
  return (readback * CAR_VREF_4P0 / 4096);
}

void Carboard::setInputCMOSLevel(double voltage) {
  if(getBoardRev() == CAR_REV_1_3) {
    setDACVoltage(CMOS_IN_1_TO_4.address(), CMOS_IN_1_TO_4.channel(), voltage);
    setDACVoltage(CMOS_IN_5_TO_8.address(), CMOS_IN_5_TO_8.channel(), voltage);
    setDACVoltage(CMOS_IN_9_TO_12.address(), CMOS_IN_9_TO_12.channel(), voltage);
    setDACVoltage(CMOS_IN_13_TO_14.address(), CMOS_IN_13_TO_14.channel(), voltage);
  }
}

void Carboard::setOutputCMOSLevel(double voltage) {
  if(getBoardRev() == CAR_REV_1_3) {
    setDACVoltage(CMOS_OUT_1_TO_4.address(), CMOS_OUT_1_TO_4.channel(), voltage);
    setDACVoltage(CMOS_OUT_5_TO_8.address(), CMOS_OUT_5_TO_8.channel(), voltage);
  }
}

void Carboard::monitorAlert() {

  const i2c_address_t device = ADDR_IOEXP2;
  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, device));

  LOG(DEBUG) << "Reading ALERT Status from PCA9539 with address " << to_hex_string(device);
  auto status = i2c.read(0x0, 2);

  LOG(INFO) << "PWR1 ALERT :" << PWR1_ALERT.status(status);
  LOG(INFO) << "PWR2 ALERT :" << PWR2_ALERT.status(status);
  LOG(INFO) << "PWR3 ALERT :" << PWR3_ALERT.status(status);
  LOG(INFO) << "PWR4 ALERT :" << PWR4_ALERT.status(status);
  LOG(INFO) << "PWR5 ALERT :" << PWR5_ALERT.status(status);
  LOG(INFO) << "PWR6 ALERT :" << PWR6_ALERT.status(status);
  LOG(INFO) << "PWR7 ALERT :" << PWR6_ALERT.status(status);
  LOG(INFO) << "PWR8 ALERT :" << PWR6_ALERT.status(status);
  LOG(INFO) << "SI5345 ALERT :" << SI5345_ALERT.status(status);
}

bool Carboard::getAlertStatus(const GPIO_T IOCHANNEL) {

  const i2c_address_t device = IOCHANNEL.ioaddr();
  iface_i2c& i2c = InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_I2C0, device));

  LOG(DEBUG) << "Reading ALERT Status from PCA9539";
  auto data = i2c.read(0x0, 2);
  return IOCHANNEL.status(data);
}

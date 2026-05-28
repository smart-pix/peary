/**
 * Caribou implementation for the FASTPIX
 */

#include "DPTSDevice.hpp"
#include "utils/log.hpp"

using namespace caribou;

DPTSDevice::DPTSDevice(const caribou::Configuration config) : CaribouDevice(config, InterfaceConfiguration("dummy")) {

  _dispatcher.add("powerStatusLog", &DPTSDevice::powerStatusLog, this);
  _dispatcher.add("setVDD", &DPTSDevice::setVDD, this);
  _dispatcher.add("writeSR", &DPTSDevice::writeSR, this);
  _dispatcher.add("writeSR2", &DPTSDevice::writeSR2, this);
  _dispatcher.add("clearConfig", &DPTSDevice::clearConfig, this);
  _dispatcher.add("maskColumn", &DPTSDevice::maskColumn, this);
  _dispatcher.add("maskRow", &DPTSDevice::maskRow, this);
  _dispatcher.add("maskDiagonal", &DPTSDevice::maskDiagonal, this);
  _dispatcher.add("maskPixel", &DPTSDevice::maskPixel, this);
  _dispatcher.add("writeConfig", &DPTSDevice::writeConfig, this);
  _dispatcher.add("clear", &DPTSDevice::clear, this);
  _dispatcher.add("pulse", &DPTSDevice::pulse, this);
  _dispatcher.add("toggle", &DPTSDevice::toggle, this);
  _dispatcher.add("set", &DPTSDevice::set, this);
  _dispatcher.add("clear", &DPTSDevice::clear, this);
  _dispatcher.add("enablePulser", &DPTSDevice::enablePulser, this);

  // Set up periphery

  _periphery.add("AVDD", carboard::PWR_OUT_5);
  _periphery.add("DVDD", carboard::PWR_OUT_6);
  _periphery.add("BVDD", carboard::PWR_OUT_1);

  _periphery.add("IBIASF", carboard::BIAS_22);
  _periphery.add("VCASB", carboard::BIAS_31);
  _periphery.add("VCASN", carboard::BIAS_25);
  _periphery.add("VH", carboard::BIAS_28);

  _periphery.add("IBIASN", carboard::CUR_5);
  _periphery.add("IBIAS", carboard::CUR_3);
  _periphery.add("IRESET", carboard::CUR_2);

  _periphery.add("IDB", carboard::CUR_1);
  _periphery.add("TEMP", carboard::CUR_6);

  // Add memory pages to the dictionary:
  _memory.add(DPTS_MEMORY);
}

void DPTSDevice::configure() {
  CaribouDevice<carboard::Carboard, DummyInterface>::configure();
}

template <typename Enumeration> auto as_value(Enumeration const value) -> typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

DPTSDevice::~DPTSDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

void DPTSDevice::powerUp() {
  LOG(INFO) << "Powering up";

  setInputCMOSLevel(_config.Get("dvdd", DPTS_DVDD));

  this->setVoltage("avdd", 0);
  this->setVoltage("dvdd", 0);
  this->setVoltage("bvdd", 0);

  this->switchOn("avdd");
  this->switchOn("dvdd");
  this->switchOn("bvdd");

  mDelay(100);

  double vdd = 0;
  while(vdd <= _config.Get("avdd", DPTS_AVDD)) {
    LOG(INFO) << "VDD: " << vdd;
    this->setVoltage("avdd", vdd, _config.Get("avdd_current", DPTS_AVDD_CURRENT));
    this->setVoltage("dvdd", vdd, _config.Get("dvdd_current", DPTS_DVDD_CURRENT));
    this->setVoltage("bvdd", vdd, _config.Get("bvdd_current", DPTS_BVDD_CURRENT));
    vdd += 0.1;
    mDelay(100);
  }

  this->setVoltage("avdd", _config.Get("avdd", DPTS_AVDD), _config.Get("avdd_current", DPTS_AVDD_CURRENT));
  this->setVoltage("dvdd", _config.Get("dvdd", DPTS_DVDD), _config.Get("dvdd_current", DPTS_DVDD_CURRENT));
  this->setVoltage("bvdd", _config.Get("bvdd", DPTS_BVDD), _config.Get("bvdd_current", DPTS_BVDD_CURRENT));

  // ---

  LOG(DEBUG) << " IBIASF: " << _config.Get("ibiasf", DPTS_IBIASF) << "V";
  this->setVoltage("ibiasf", _config.Get("ibiasf", DPTS_IBIASF));
  this->switchOn("ibiasf");

  mDelay(100);

  LOG(DEBUG) << " VCASB: " << _config.Get("vcasb", DPTS_VCASB) << "V";
  this->setVoltage("vcasb", _config.Get("vcasb", DPTS_VCASB));
  this->switchOn("vcasb");

  mDelay(100);

  LOG(DEBUG) << " VCASN: " << _config.Get("vcasn", DPTS_VCASN) << "V";
  this->setVoltage("vcasn", _config.Get("vcasn", DPTS_VCASN));
  this->switchOn("vcasn");

  mDelay(100);

  LOG(DEBUG) << " VH: " << _config.Get("vh", DPTS_VH) << "V";
  this->setVoltage("vh", _config.Get("vh", DPTS_VH));
  this->switchOn("vh");

  mDelay(100);

  LOG(DEBUG) << " IBIASN: " << _config.Get("ibiasn", DPTS_IBIASN) << "uA";
  this->setCurrent("ibiasn", static_cast<unsigned int>(abs(_config.Get("ibiasn", DPTS_IBIASN))), 0);
  this->switchOn("ibiasn");

  mDelay(100);

  LOG(DEBUG) << " IBIAS: " << _config.Get("ibias", DPTS_IBIAS) << "uA";
  this->setCurrent("ibias", static_cast<unsigned int>(abs(_config.Get("ibias", DPTS_IBIAS))), 0);
  this->switchOn("ibias");

  mDelay(100);

  LOG(DEBUG) << " IRESET: " << _config.Get("ireset", DPTS_IRESET) << "uA";
  this->setCurrent("ireset", static_cast<unsigned int>(abs(_config.Get("ireset", DPTS_IRESET))), 0);
  this->switchOn("ireset");

  mDelay(100);

  LOG(DEBUG) << " IDB: " << _config.Get("idb", DPTS_IDB) << "uA";
  this->setCurrent("idb", static_cast<unsigned int>(abs(_config.Get("ireset", DPTS_IDB))), 0);
  this->switchOn("idb");

  // ---

  setMemory("gpio", 0);

  setOutputCMOSLevel(_config.Get("dvdd", DPTS_DVDD));
}

void DPTSDevice::powerDown() {
  LOG(INFO) << "Power off";

  setMemory("gpio", 0);
  setOutputCMOSLevel(0);

  LOG(DEBUG) << "Power off IBIASF";
  this->switchOff("ibiasf");

  LOG(DEBUG) << "Power off VCASB";
  this->switchOff("vcasb");

  LOG(DEBUG) << "Power off VCASN";
  this->switchOff("vcasn");

  LOG(DEBUG) << "Power off VH";
  this->switchOff("vh");

  LOG(DEBUG) << "Power off IBIASN";
  this->switchOff("ibiasn");

  LOG(DEBUG) << "Power off IBIAS";
  this->switchOff("ibias");

  LOG(DEBUG) << "Power off IRESET";
  this->switchOff("ireset");

  LOG(DEBUG) << "Power off IDB";
  this->switchOff("idb");

  LOG(DEBUG) << "Power off BVDD";
  this->switchOff("bvdd");

  LOG(DEBUG) << "Power off DVDD";
  this->switchOff("dvdd");

  LOG(DEBUG) << "Power off AVDD";
  this->switchOff("avdd");

  setInputCMOSLevel(0);
}

void DPTSDevice::daqStart() {
  LOG(INFO) << "DAQ started.";
}

void DPTSDevice::daqStop() {
  LOG(INFO) << "DAQ stopped.";
}

void DPTSDevice::powerStatusLog() {
  LOG(INFO) << "Power status:";

  LOG(INFO) << "DVDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("dvdd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("dvdd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("dvdd") << "W";

  LOG(INFO) << "AVDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("avdd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("avdd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("avdd") << "W";

  LOG(INFO) << "BVDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("bvdd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("bvdd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("bvdd") << "W";
}

void DPTSDevice::reset() {}

void DPTSDevice::setVDD(double vdd) {
  setVoltage("avdd", vdd);
  setVoltage("dvdd", vdd);
  setVoltage("bvdd", vdd);
}

const size_t DPTS_CLK = 0;
const size_t DPTS_EN = 1;
const size_t DPTS_TRG = 5;
const size_t DPTS_SI = 7;

void DPTSDevice::writeBit(uint8_t b) {
  uint8_t val;
  val = static_cast<uint8_t>((b & 1u) << DPTS_SI);
  setMemory("gpio", val); // SI = b, CLK = 0
  usleep(5);

  val = static_cast<uint8_t>((b & 1u) << DPTS_SI) | static_cast<uint8_t>(1u << DPTS_CLK);
  setMemory("gpio", val); // SI = b, CLK = 1
  usleep(5);
}

void DPTSDevice::writeReg(uint32_t r, bool reverse) {
  if(!reverse) {
    for(size_t i = 0; i < 32; i++) {
      writeBit(static_cast<uint8_t>((r & (1u << 31)) >> 31));
      writeBit(static_cast<uint8_t>((r & (1u << 31)) >> 31));
      writeBit(static_cast<uint8_t>((r & (1u << 31)) >> 31));

      r <<= 1;
    }
  } else {
    for(size_t i = 0; i < 32; i++) {
      writeBit(r & 1u);
      writeBit(r & 1u);
      writeBit(r & 1u);

      r >>= 1;
    }
  }
}

void DPTSDevice::writeReg2(uint32_t r) {
  for(size_t i = 0; i < 32; i++) {
    writeBit(r & 1u);
    r >>= 1;
  }
}

void DPTSDevice::writeSR(uint32_t mr, uint32_t cs, uint32_t md, uint32_t mc, uint32_t rs) {
  setMemory("gpio", 0);
  usleep(10);

  writeReg(mr);
  writeReg(cs);
  writeReg(md, true);
  writeReg(mc, true);
  writeReg(rs);

  setMemory("gpio", 1u << DPTS_CLK); // CLK = 1
  usleep(10);
  setMemory("gpio", (1u << DPTS_CLK) | (1u << DPTS_EN)); // CLK = 1, EN = 1
}

void DPTSDevice::writeSR2(uint32_t mr1,
                          uint32_t mr2,
                          uint32_t mr3,
                          uint32_t cs1,
                          uint32_t cs2,
                          uint32_t cs3,
                          uint32_t md1,
                          uint32_t md2,
                          uint32_t md3,
                          uint32_t mc1,
                          uint32_t mc2,
                          uint32_t mc3,
                          uint32_t rs1,
                          uint32_t rs2,
                          uint32_t rs3) {
  setMemory("gpio", 0);
  usleep(10);

  writeReg2(mr1);
  writeReg2(mr2);
  writeReg2(mr3);

  writeReg2(cs1);
  writeReg2(cs2);
  writeReg2(cs3);

  writeReg2(md1);
  writeReg2(md2);
  writeReg2(md3);

  writeReg2(mc1);
  writeReg2(mc2);
  writeReg2(mc3);

  writeReg2(rs1);
  writeReg2(rs2);
  writeReg2(rs3);

  setMemory("gpio", 1u << DPTS_CLK); // CLK = 1
  usleep(10);
  setMemory("gpio", (1u << DPTS_CLK) | (1u << DPTS_EN)); // CLK = 1, EN = 1
}

void DPTSDevice::enablePulser(uint32_t x, uint32_t y) {
  column_pulser |= (1u << x);
  row_pulser |= (1u << y);
}

void DPTSDevice::pulse() {
  set(DPTS_TRG); // TRG = 1
  mDelay(1);
  clear(DPTS_TRG); // TRG = 0
}

void DPTSDevice::toggle(int i) {
  size_t val = getMemory("gpio");
  setMemory("gpio", val ^ (1u << i));
}

void DPTSDevice::set(int i) {
  size_t val = getMemory("gpio");
  setMemory("gpio", val | (1u << i));
}

void DPTSDevice::clear(int i) {
  size_t val = getMemory("gpio");
  setMemory("gpio", val & ~(1u << i));
}

void DPTSDevice::clearConfig() {
  column_mask = 0;
  row_mask = 0;
  diagonal_mask = 0;
  column_pulser = 0;
  row_pulser = 0;
}

void DPTSDevice::writeConfig() {
  LOG(INFO) << "Column mask: " << to_hex_string(column_mask);
  LOG(INFO) << "Row mask: " << to_hex_string(row_mask);
  LOG(INFO) << "Diagonal mask: " << to_hex_string(diagonal_mask);
  LOG(INFO) << "Pulser column: " << to_hex_string(column_pulser);
  LOG(INFO) << "Pulser row: " << to_hex_string(row_pulser);

  writeSR(row_mask, row_pulser, diagonal_mask, column_mask, column_pulser);

  for(int x = 0; x < 32; x++) {
    for(int y = 0; y < 32; y++) {
      if((column_mask >> x) & 1u && (row_mask >> y) & 1u) {
        for(int d = 0; d < 32; d++) {
          if((diagonal_mask >> d) & 1u && (y - x == d || y - x == d - 32)) {
            LOG(INFO) << "Masked pixel " << x << ' ' << y << ' ' << d;
          }
        }
      }
    }
  }
}

void DPTSDevice::maskColumn(int i) {
  column_mask |= (1u << i);
}

void DPTSDevice::maskRow(int i) {
  row_mask |= (1u << i);
}

void DPTSDevice::maskDiagonal(int i) {
  diagonal_mask |= (1u << i);
}

void DPTSDevice::maskPixel(int x, int y) {
  maskColumn(x);
  maskRow(y);

  int d = y - x;

  if(d < 0) {
    d += 32;
  }

  maskDiagonal(d);
  LOG(INFO) << "Masking pixel " << x << ' ' << y << ' ' << d;
}

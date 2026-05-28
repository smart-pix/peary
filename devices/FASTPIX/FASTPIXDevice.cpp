/**
 * Caribou implementation for the FASTPIX
 */

#include "FASTPIXDevice.hpp"
#include "utils/log.hpp"

using namespace caribou;

FASTPIXDevice::FASTPIXDevice(const caribou::Configuration config) : CaribouDevice(config, InterfaceConfiguration("dummy")) {

  _dispatcher.add("setMB", &FASTPIXDevice::setMB, this);
  _dispatcher.add("setPRE", &FASTPIXDevice::setPRE, this);
  _dispatcher.add("setHBRIDGE", &FASTPIXDevice::setHBRIDGE, this);
  _dispatcher.add("setPB", &FASTPIXDevice::setPB, this);
  _dispatcher.add("setCMFB", &FASTPIXDevice::setCMFB, this);
  _dispatcher.add("powerStatusLog", &FASTPIXDevice::powerStatusLog, this);
  _dispatcher.add("setVDD", &FASTPIXDevice::setVDD, this);

  // Set up periphery

  _periphery.add("AVDD", carboard::PWR_OUT_1);
  _periphery.add("DVDD", carboard::PWR_OUT_2);
  _periphery.add("PVDD", carboard::PWR_OUT_4);

  _periphery.add("NGATE", carboard::BIAS_4);
  _periphery.add("DRESET", carboard::BIAS_3);
  _periphery.add("VRESET", carboard::BIAS_1);
  _periphery.add("VPULSE", carboard::BIAS_5);
  _periphery.add("BUFF_VCASP", carboard::BIAS_7);
  _periphery.add("BUFF_VCASN", carboard::BIAS_8);
  _periphery.add("BUFF_VCAS", carboard::BIAS_6);
  _periphery.add("VCAS2", carboard::BIAS_15);
  _periphery.add("VCASP", carboard::BIAS_9);

  _periphery.add("ITHR", carboard::BIAS_23);
  _periphery.add("IDB", carboard::BIAS_17);
  _periphery.add("IBIASP_OFF", carboard::BIAS_20);
  _periphery.add("ITHRN", carboard::BIAS_30);
  _periphery.add("BUFF_IBIASP", carboard::BIAS_31);
  _periphery.add("BUFF_IBIASN", carboard::BIAS_25);
  _periphery.add("IRESET", carboard::BIAS_28);

  _periphery.add("IBIASP", carboard::CUR_3);
  _periphery.add("IBIASN", carboard::CUR_2);

  _periphery.add("PB2", carboard::BIAS_13);
  _periphery.add("PB1", carboard::BIAS_11);
  _periphery.add("PB0", carboard::BIAS_10);
  _periphery.add("EN_CMFB", carboard::BIAS_24);

  // Add memory pages to the dictionary:
  _memory.add(FASTPIX_MEMORY);
}

void FASTPIXDevice::configure() {
  setPB(_config.Get<uint32_t>("pb", FASTPIX_PB));
  setPRE(_config.Get<uint32_t>("pre", FASTPIX_PRE));
  setHBRIDGE(_config.Get<uint32_t>("hbridge", FASTPIX_HBRIDGE));
  setCMFB(_config.Get<uint32_t>("en_cmfb", FASTPIX_EN_CMFB));

  CaribouDevice<carboard::Carboard, DummyInterface>::configure();
}

template <typename Enumeration> auto as_value(Enumeration const value) -> typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

FASTPIXDevice::~FASTPIXDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

void FASTPIXDevice::powerUp() {
  LOG(INFO) << "Powering up";

  setInputCMOSLevel(_config.Get("dvdd", FASTPIX_DVDD));

  this->setVoltage("avdd", 0);
  this->setVoltage("dvdd", 0);
  this->setVoltage("pvdd", 0);

  this->switchOn("avdd");
  this->switchOn("dvdd");
  this->switchOn("pvdd");

  mDelay(100);

  double vdd = 0;
  while(vdd <= _config.Get("avdd", FASTPIX_AVDD)) {
    LOG(INFO) << "VDD: " << vdd;
    this->setVoltage("avdd", vdd, _config.Get("avdd_current", FASTPIX_AVDD_CURRENT));
    this->setVoltage("dvdd", vdd, _config.Get("dvdd_current", FASTPIX_DVDD_CURRENT));
    this->setVoltage("pvdd", vdd, _config.Get("pvdd_current", FASTPIX_PVDD_CURRENT));
    vdd += 0.1;
    mDelay(100);
  }

  this->setVoltage("avdd", _config.Get("avdd", FASTPIX_AVDD), _config.Get("avdd_current", FASTPIX_AVDD_CURRENT));
  this->setVoltage("dvdd", _config.Get("dvdd", FASTPIX_DVDD), _config.Get("dvdd_current", FASTPIX_DVDD_CURRENT));
  this->setVoltage("pvdd", _config.Get("pvdd", FASTPIX_PVDD), _config.Get("pvdd_current", FASTPIX_PVDD_CURRENT));

  // ---

  LOG(DEBUG) << " ITHR: " << _config.Get("ithr", FASTPIX_ITHR) << "V";
  this->setVoltage("ithr", _config.Get("ithr", FASTPIX_ITHR));
  this->switchOn("ithr");

  mDelay(100);

  LOG(DEBUG) << " ITHRN: " << _config.Get("ithrn", FASTPIX_ITHRN) << "V";
  this->setVoltage("ithrn", _config.Get("ithrn", FASTPIX_ITHRN));
  this->switchOn("ithrn");

  mDelay(100);

  LOG(DEBUG) << " IBIASP_OFF: " << _config.Get("ibiasp_off", FASTPIX_IBIASP_OFF) << "V";
  this->setVoltage("ibiasp_off", _config.Get("ibiasp_off", FASTPIX_IBIASP_OFF));
  this->switchOn("ibiasp_off");

  mDelay(100);

  LOG(DEBUG) << " BUFF_IBIASP: " << _config.Get("buff_ibiasp", FASTPIX_BUFF_IBIASP) << "V";
  this->setVoltage("buff_ibiasp", _config.Get("buff_ibiasp", FASTPIX_BUFF_IBIASP));
  this->switchOn("buff_ibiasp");

  mDelay(100);

  LOG(DEBUG) << " BUFF_IBIASN: " << _config.Get("buff_ibiasn", FASTPIX_BUFF_IBIASN) << "V";
  this->setVoltage("buff_ibiasn", _config.Get("buff_ibiasn", FASTPIX_BUFF_IBIASN));
  this->switchOn("buff_ibiasn");

  mDelay(100);

  LOG(DEBUG) << " IRESET: " << _config.Get("ireset", FASTPIX_IRESET) << "V";
  this->setVoltage("ireset", _config.Get("ireset", FASTPIX_IRESET));
  this->switchOn("ireset");

  mDelay(100);

  LOG(DEBUG) << " IBIASP: " << _config.Get("ibiasp", FASTPIX_IBIASP) << "uA";
  this->setCurrent("ibiasp", static_cast<unsigned int>(abs(_config.Get("ibiasp", FASTPIX_IBIASP))), 0);
  this->switchOn("ibiasp");

  mDelay(100);

  LOG(DEBUG) << " IBIASN: " << _config.Get("ibiasn", FASTPIX_IBIASN) << "uA";
  this->setCurrent("ibiasn", static_cast<unsigned int>(abs(_config.Get("ibiasn", FASTPIX_IBIASN))), 1);
  this->switchOn("ibiasn");

  mDelay(100);

  LOG(DEBUG) << " IDB: " << _config.Get("idb", FASTPIX_IDB) << "V";
  this->setVoltage("idb", _config.Get("idb", FASTPIX_IDB));
  this->switchOn("idb");

  mDelay(100);

  // ---

  LOG(DEBUG) << " NGATE: " << _config.Get("ngate", FASTPIX_NGATE) << "V";
  this->setVoltage("ngate", _config.Get("ngate", FASTPIX_NGATE));
  this->switchOn("ngate");

  mDelay(100);

  LOG(DEBUG) << " DRESET: " << _config.Get("dreset", FASTPIX_DRESET) << "V";
  this->setVoltage("dreset", _config.Get("dreset", FASTPIX_DRESET));
  this->switchOn("dreset");

  mDelay(100);

  LOG(DEBUG) << " VRESET: " << _config.Get("vreset", FASTPIX_VRESET) << "V";
  this->setVoltage("vreset", _config.Get("vreset", FASTPIX_VRESET));
  this->switchOn("vreset");

  mDelay(100);

  LOG(DEBUG) << " VPULSE: " << _config.Get("vpulse", FASTPIX_VPULSE) << "V";
  this->setVoltage("vpulse", _config.Get("vpulse", FASTPIX_VPULSE));
  this->switchOn("vpulse");

  mDelay(100);

  LOG(DEBUG) << " BUFF_VCASP: " << _config.Get("buff_vcasp", FASTPIX_BUFF_VCASP) << "V";
  this->setVoltage("buff_vcasp", _config.Get("buff_vcasp", FASTPIX_BUFF_VCASP));
  this->switchOn("buff_vcasp");

  mDelay(100);

  LOG(DEBUG) << " BUFF_VCASN: " << _config.Get("buff_vcasn", FASTPIX_BUFF_VCASN) << "V";
  this->setVoltage("buff_vcasn", _config.Get("buff_vcasn", FASTPIX_BUFF_VCASN));
  this->switchOn("buff_vcasn");

  mDelay(100);

  LOG(DEBUG) << " BUFF_VCAS: " << _config.Get("buff_vcas", FASTPIX_BUFF_VCAS) << "V";
  this->setVoltage("buff_vcas", _config.Get("buff_vcas", FASTPIX_BUFF_VCAS));
  this->switchOn("buff_vcas");

  mDelay(100);

  LOG(DEBUG) << " VCAS2: " << _config.Get("vcas2", FASTPIX_VCAS2) << "V";
  this->setVoltage("vcas2", _config.Get("vcas2", FASTPIX_VCAS2));
  this->switchOn("vcas2");

  mDelay(100);

  LOG(DEBUG) << " VCASP: " << _config.Get("vcasp", FASTPIX_VCASP) << "V";
  this->setVoltage("vcasp", _config.Get("vcasp", FASTPIX_VCASP));
  this->switchOn("vcasp");

  mDelay(100);

  // ---

  setMemory("gpio", 0);

  this->setPB(0);
  this->setCMFB(0);

  this->switchOn("pb2");
  this->switchOn("pb1");
  this->switchOn("pb0");
  this->switchOn("en_cmfb");

  setOutputCMOSLevel(_config.Get("dvdd", FASTPIX_DVDD));
}

void FASTPIXDevice::powerDown() {
  LOG(INFO) << "Power off";

  setMemory("gpio", 0);
  setOutputCMOSLevel(0);

  LOG(DEBUG) << "Power off NGATE";
  this->switchOff("ngate");

  LOG(DEBUG) << "Power off DRESET";
  this->switchOff("dreset");

  LOG(DEBUG) << "Power off VRESET";
  this->switchOff("vreset");

  LOG(DEBUG) << "Power off VPULSE";
  this->switchOff("vpulse");

  LOG(DEBUG) << "Power off BUFF_VCASP";
  this->switchOff("buff_vcasp");

  LOG(DEBUG) << "Power off BUFF_VCASN";
  this->switchOff("buff_vcasn");

  LOG(DEBUG) << "Power off BUFF_VCAS";
  this->switchOff("buff_vcas");

  LOG(DEBUG) << "Power off VCAS2";
  this->switchOff("vcas2");

  LOG(DEBUG) << "Power off VCASP";
  this->switchOff("vcasp");

  LOG(DEBUG) << "Power off ITHR";
  this->switchOff("ithr");

  LOG(DEBUG) << "Power off IDB";
  this->switchOff("idb");

  LOG(DEBUG) << "Power off IBIASP_OFF";
  this->switchOff("ibiasp_off");

  LOG(DEBUG) << "Power off ITHRN";
  this->switchOff("ithrn");

  LOG(DEBUG) << "Power off BUFF_IBIASP";
  this->switchOff("buff_ibiasp");

  LOG(DEBUG) << "Power off BUFF_IBIASN";
  this->switchOff("buff_ibiasn");

  LOG(DEBUG) << "Power off IRESET";
  this->switchOff("ireset");

  LOG(DEBUG) << "Power off IBIASP";
  this->switchOff("ibiasp");

  LOG(DEBUG) << "Power off IBIASN";
  this->switchOff("ibiasn");

  LOG(DEBUG) << "Power off PB2";
  this->switchOff("pb2");

  LOG(DEBUG) << "Power off PB1";
  this->switchOff("pb1");

  LOG(DEBUG) << "Power off PB0";
  this->switchOff("pb0");

  LOG(DEBUG) << "Power off EN_CMFB";
  this->switchOff("en_cmfb");

  LOG(DEBUG) << "Power off PVDD";
  this->switchOff("pvdd");

  LOG(DEBUG) << "Power off DVDD";
  this->switchOff("dvdd");

  LOG(DEBUG) << "Power off AVDD";
  this->switchOff("avdd");

  setInputCMOSLevel(0);
}

void FASTPIXDevice::daqStart() {
  LOG(INFO) << "DAQ started.";
}

void FASTPIXDevice::daqStop() {
  LOG(INFO) << "DAQ stopped.";
}

void FASTPIXDevice::powerStatusLog() {
  LOG(INFO) << "Power status:";

  LOG(INFO) << "DVDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("dvdd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("dvdd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("dvdd") << "W";

  LOG(INFO) << "AVDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("avdd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("avdd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("avdd") << "W";

  LOG(INFO) << "PVDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("pvdd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("pvdd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("pvdd") << "W";
}

void FASTPIXDevice::reset() {}

void FASTPIXDevice::setMB(uint32_t mb1, uint32_t mb2) {
  size_t val = getMemory("gpio");
  uint32_t mb = (mb1 << 2) | ((mb2 & 1) << 1) | ((mb2 & 2) >> 1);

  setMemory("gpio", (val & 0xE0) | (mb & 0x1F));
}

void FASTPIXDevice::setPRE(uint32_t pre) {
  size_t val = getMemory("gpio");
  setMemory("gpio", (val & 0x9F) | (pre & 0x03) << 5);
}

void FASTPIXDevice::setHBRIDGE(uint32_t h) {
  size_t val = getMemory("gpio");
  setMemory("gpio", (val & 0x7F) | (h & 0x01) << 7);
}

void FASTPIXDevice::setPB(uint32_t pb) {
  setVoltage("pb0", pb & (1 << 0) ? _config.Get("dvdd", FASTPIX_DVDD) : 0);
  setVoltage("pb1", pb & (1 << 1) ? _config.Get("dvdd", FASTPIX_DVDD) : 0);
  setVoltage("pb2", pb & (1 << 2) ? _config.Get("dvdd", FASTPIX_DVDD) : 0);
}

void FASTPIXDevice::setCMFB(uint32_t cmfb) {
  setVoltage("en_cmfb", cmfb & (1 << 0) ? _config.Get("dvdd", FASTPIX_DVDD) : 0);
}

void FASTPIXDevice::setVDD(double vdd) {
  setVoltage("avdd", vdd);
  setVoltage("dvdd", vdd);
  setVoltage("pvdd", vdd);
}

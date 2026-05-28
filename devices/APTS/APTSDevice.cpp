/**
 * Caribou implementation for the FASTPIX
 */

#include "APTSDevice.hpp"
#include "utils/log.hpp"

using namespace caribou;

APTSDevice::APTSDevice(const caribou::Configuration config) : CaribouDevice(config, InterfaceConfiguration("dummy")) {

  _dispatcher.add("powerStatusLog", &APTSDevice::powerStatusLog, this);
  _dispatcher.add("pulse", &APTSDevice::pulse, this);

  // Set up periphery

  _periphery.add("AVDD", carboard::PWR_OUT_5);
  _periphery.add("VD", carboard::PWR_OUT_6);

  _periphery.add("IBIAS4_P", carboard::BIAS_4);
  _periphery.add("IBIAS4_N", carboard::BIAS_3);
  _periphery.add("VH", carboard::BIAS_23);
  _periphery.add("SEL_0", carboard::BIAS_17);
  _periphery.add("SEL_1", carboard::BIAS_20);
  _periphery.add("TRG", carboard::BIAS_30);
  _periphery.add("VCASN", carboard::BIAS_31);
  _periphery.add("VCASP", carboard::BIAS_25);
  _periphery.add("VRESET", carboard::BIAS_28);
  _periphery.add("IRESET_N", carboard::BIAS_18);
  _periphery.add("IRESET_P", carboard::BIAS_32);

  _periphery.add("IBIASN", carboard::CUR_5);
  _periphery.add("IBIASP", carboard::CUR_3);
  _periphery.add("IBIAS3", carboard::CUR_2);
  _periphery.add("CUR1", carboard::CUR_1);

  // Add memory pages to the dictionary:
  _memory.add(APTS_MEMORY);
}

void APTSDevice::configure() {
  CaribouDevice<carboard::Carboard, DummyInterface>::configure();
}

template <typename Enumeration> auto as_value(Enumeration const value) -> typename std::underlying_type<Enumeration>::type {
  return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

APTSDevice::~APTSDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

void APTSDevice::powerUp() {
  LOG(INFO) << "Powering up";

  setInputCMOSLevel(_config.Get("avdd", APTS_AVDD));

  this->setVoltage("avdd", 0);
  this->setVoltage("vd", 0);

  this->switchOn("avdd");
  this->switchOn("vd");

  mDelay(100);

  double vdd = 0;
  while(vdd <= _config.Get("avdd", APTS_AVDD)) {
    LOG(INFO) << "VDD: " << vdd;
    this->setVoltage("avdd", vdd, _config.Get("avdd_current", APTS_AVDD_CURRENT));
    vdd += 0.1;
    mDelay(100);
  }

  this->setVoltage("avdd", _config.Get("avdd", APTS_AVDD), _config.Get("avdd_current", APTS_AVDD_CURRENT));
  this->setVoltage("vd", _config.Get("vd", APTS_VD), _config.Get("vd_current", APTS_VD_CURRENT));

  // ---

  LOG(DEBUG) << " IBIAS4_P: " << _config.Get("ibias4_p", APTS_IBIAS4_P) << "V";
  this->setVoltage("ibias4_p", _config.Get("ibias4_p", APTS_IBIAS4_P));

  LOG(DEBUG) << " IBIAS4_N: " << _config.Get("ibias4_n", APTS_IBIAS4_N) << "V";
  this->setVoltage("ibias4_n", _config.Get("ibias4_n", APTS_IBIAS4_N));

  this->switchOn("ibias4_p");
  this->switchOn("ibias4_n");

  mDelay(100);

  LOG(DEBUG) << " IRESET_P: " << _config.Get("ireset_p", APTS_IRESET_P) << "V";
  this->setVoltage("ireset_p", _config.Get("ireset_p", APTS_IRESET_P));

  LOG(DEBUG) << " IRESET_N: " << _config.Get("ireset_n", APTS_IRESET_N) << "V";
  this->setVoltage("ireset_n", _config.Get("ireset_n", APTS_IRESET_N));

  this->switchOn("ireset_p");
  this->switchOn("ireset_n");

  mDelay(100);

  LOG(DEBUG) << " VH: " << _config.Get("vh", APTS_VH) << "V";
  this->setVoltage("vh", _config.Get("vh", APTS_VH));
  this->switchOn("vh");

  mDelay(100);

  LOG(DEBUG) << " SEL_0: " << _config.Get("sel_0", APTS_SEL_0) << "V";
  this->setVoltage("sel_0", _config.Get("sel_0", APTS_SEL_0));
  this->switchOn("sel_0");

  mDelay(100);

  LOG(DEBUG) << " SEL_1: " << _config.Get("sel_1", APTS_SEL_1) << "V";
  this->setVoltage("sel_1", _config.Get("sel_1", APTS_SEL_1));
  this->switchOn("sel_1");

  mDelay(100);

  LOG(DEBUG) << " TRG: " << _config.Get("trg", APTS_TRG) << "V";
  this->setVoltage("trg", _config.Get("trg", APTS_TRG));
  this->switchOn("trg");

  mDelay(100);

  LOG(DEBUG) << " VRESET: " << _config.Get("vreset", APTS_VRESET) << "V";
  this->setVoltage("vreset", _config.Get("vreset", APTS_VRESET));
  this->switchOn("vreset");

  mDelay(100);

  LOG(DEBUG) << " VCASN: " << _config.Get("vcasn", APTS_VCASN) << "V";
  this->setVoltage("vcasn", _config.Get("vcasn", APTS_VCASN));
  this->switchOn("vcasn");

  mDelay(100);

  LOG(DEBUG) << " VCASP: " << _config.Get("vcasp", APTS_VCASP) << "V";
  this->setVoltage("vcasp", _config.Get("vcasp", APTS_VCASP));
  this->switchOn("vcasp");

  mDelay(100);

  LOG(DEBUG) << " IBIASN: " << _config.Get("ibiasn", APTS_IBIASN) << "uA";
  this->setCurrent("ibiasn", static_cast<unsigned int>(abs(_config.Get("ibiasn", APTS_IBIASN))), 1);
  this->switchOn("ibiasn");

  mDelay(100);

  LOG(DEBUG) << " IBIASP: " << _config.Get("ibiasp", APTS_IBIASP) << "uA";
  this->setCurrent("ibiasp", static_cast<unsigned int>(abs(_config.Get("ibiasp", APTS_IBIASP))), 0);
  this->switchOn("ibiasp");

  mDelay(100);

  LOG(DEBUG) << " IBIAS3: " << _config.Get("ibias3", APTS_IBIAS3) << "uA";
  this->setCurrent("ibias3", static_cast<unsigned int>(abs(_config.Get("ibias3", APTS_IBIAS3))), 0);
  this->switchOn("ibias3");

  mDelay(100);

  LOG(DEBUG) << " CUR1: " << _config.Get("cur1", APTS_CUR1) << "uA";
  this->setCurrent("cur1", static_cast<unsigned int>(abs(_config.Get("cur1", APTS_CUR1))), 0);
  this->switchOn("cur1");

  // ---

  setMemory("gpio", 0);

  setOutputCMOSLevel(_config.Get("avdd", APTS_AVDD));

  powerStatusLog();
}

void APTSDevice::powerDown() {
  LOG(INFO) << "Power off";

  setMemory("gpio", 0);
  setOutputCMOSLevel(0);

  LOG(DEBUG) << "Power off CUR1";
  this->switchOff("cur1");

  LOG(DEBUG) << "Power off IBIAS3";
  this->switchOff("ibias3");

  LOG(DEBUG) << "Power off IBIASP";
  this->switchOff("ibiasp");

  LOG(DEBUG) << "Power off IBIASN";
  this->switchOff("ibiasn");

  LOG(DEBUG) << "Power off VCASP";
  this->switchOff("vcasp");

  LOG(DEBUG) << "Power off VCASN";
  this->switchOff("vcasn");

  LOG(DEBUG) << "Power off VRESET";
  this->switchOff("vreset");

  LOG(DEBUG) << "Power off TRG";
  this->switchOff("trg");

  LOG(DEBUG) << "Power off SEL_1";
  this->switchOff("sel_1");

  LOG(DEBUG) << "Power off SEL_0";
  this->switchOff("sel_0");

  LOG(DEBUG) << "Power off VH";
  this->switchOff("vh");

  LOG(DEBUG) << "Power off IRESET_N";
  this->switchOff("ireset_n");

  LOG(DEBUG) << "Power off IRESET_P";
  this->switchOff("ireset_p");

  LOG(DEBUG) << "Power off IBIAS4_N";
  this->switchOff("ibias4_n");

  LOG(DEBUG) << "Power off IBIAS4_P";
  this->switchOff("ibias4_p");

  this->switchOff("avdd");
  this->switchOff("vd");

  setInputCMOSLevel(0);
}

void APTSDevice::daqStart() {
  LOG(INFO) << "DAQ started.";
}

void APTSDevice::daqStop() {
  LOG(INFO) << "DAQ stopped.";
}

void APTSDevice::powerStatusLog() {
  LOG(INFO) << "Power status:";

  LOG(INFO) << "AVDD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("avdd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("avdd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("avdd") << "W";

  LOG(INFO) << "VD:";
  LOG(INFO) << "\tBus voltage: " << this->getVoltage("vd") << "V";
  LOG(INFO) << "\tBus current: " << this->getCurrent("vd") << "A";
  LOG(INFO) << "\tBus power  : " << this->getPower("vd") << "W";
}

void APTSDevice::reset() {}

void APTSDevice::pulse() {
  setVoltage("trg", 1.2);
  mDelay(10);
  setVoltage("trg", 0);
  mDelay(10);
}

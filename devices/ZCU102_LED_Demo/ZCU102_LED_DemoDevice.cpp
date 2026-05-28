/**
 * Caribou C++ Device for ZCU102_LED_Demo
 */

#include "ZCU102_LED_DemoDevice.hpp"
#include "utils/log.hpp"

using namespace caribou;

ZCU102_LED_DemoDevice::ZCU102_LED_DemoDevice(const caribou::Configuration config)
    : CaribouDevice(config, iface_mem::configuration_type(MEM_PATH, FPGA_MEM)) {


  // (1) ADD CUSTOM FUNCTIONS TO THE DISPATCHER
  //     This allows them to be called dynamically from Spacely.

  _dispatcher.add("setUsrclkFreq", &ZCU102_LED_DemoDevice::setUsrclkFreq, this);


  // (2) SET UP PERIPHERY WITH CARBOARD DEVICES 
  _periphery.add("PWR_OUT_1", carboard::PWR_OUT_1);
  _periphery.add("PWR_OUT_2", carboard::PWR_OUT_2);
  _periphery.add("PWR_OUT_3", carboard::PWR_OUT_3);
  _periphery.add("PWR_OUT_4", carboard::PWR_OUT_4);
  _periphery.add("PWR_OUT_5", carboard::PWR_OUT_5);

  _periphery.add("BIAS_1", carboard::BIAS_1);
  _periphery.add("BIAS_2", carboard::BIAS_2);
  _periphery.add("BIAS_3", carboard::BIAS_3);
  _periphery.add("BIAS_4", carboard::BIAS_4);
  _periphery.add("BIAS_5", carboard::BIAS_5);

  _periphery.add("CUR_1", carboard::CUR_1);
  _periphery.add("CUR_2", carboard::CUR_2);
  _periphery.add("CUR_3", carboard::CUR_3);

  _periphery.add("INJ_1", carboard::INJ_1);

  _periphery.add("CMOS_OUT_1_TO_4", carboard::CMOS_OUT_1_TO_4);
  _periphery.add("CMOS_OUT_5_TO_8", carboard::CMOS_OUT_5_TO_8);
  _periphery.add("CMOS_IN_1_TO_4", carboard::CMOS_IN_1_TO_4);
  _periphery.add("CMOS_IN_5_TO_8", carboard::CMOS_IN_5_TO_8);
  _periphery.add("CMOS_IN_9_TO_12", carboard::CMOS_IN_9_TO_12);
  _periphery.add("CMOS_IN_13_TO_14", carboard::CMOS_IN_13_TO_14);

  // (3) ADD MEMORY PAGES TO THE DICTIONARY
  _memory.add(FPGA_REGS);

}

// CUSTOM FUNCTIONS

void ZCU102_LED_DemoDevice::setUsrclkFreq(const uint64_t freq) {


  LOG(DEBUG) << "Unbinding Linux driver for Si570";
  std::ofstream dfout;
  dfout.open("/sys/bus/i2c/drivers/si570/unbind", std::ios_base::app);
  if(dfout.fail()) {
    LOG(INFO) << "Can not unbind driver. Maybe it is not needed.";
  } else {

    //Explanation: "9" is because the Si570 sits on the i2c-9 bus, on ZCU102.
    //             the following 005d is simply the hex address of the Si570.
    //             You can confirm which address is correct because there should
    //             be a file called "9-005d" in /sys/bus/i2c/drivers/si570
    dfout.write("9-005d", 6);
    dfout.close();
  }

  _hal->setUSRCLK(freq);

}

// SKELETON METHODS
//Need to provide definitions for these superclass functions
//or it will throw an error.
void ZCU102_LED_DemoDevice::powerUp() {
  LOG(INFO) << "Powering up (Not Implemented)";
  return;
}

void ZCU102_LED_DemoDevice::powerDown() {
  LOG(INFO) << "Powering down (Not Implemented)";
  return;
}


void ZCU102_LED_DemoDevice::daqStart() {
  LOG(INFO) << "DAQ Starting (Not Implemented)";
  return;
}


void ZCU102_LED_DemoDevice::daqStop() {
  LOG(INFO) << "DAQ Stopping (Not Implemented)";
  return;
}


ZCU102_LED_DemoDevice::~ZCU102_LED_DemoDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}

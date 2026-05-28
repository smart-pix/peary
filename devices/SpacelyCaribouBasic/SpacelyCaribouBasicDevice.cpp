/**
 * Spacely-Caribou Basic C++ Device 
 */

#include "SpacelyCaribouBasicDevice.hpp"
#include "utils/log.hpp"

using namespace caribou;
using namespace carboard;

SpacelyCaribouBasicDevice::SpacelyCaribouBasicDevice(const caribou::Configuration config)
    : CaribouDevice(config, iface_mem::configuration_type(MEM_PATH, FPGA_MEM)) {


  // (1) ADD CUSTOM FUNCTIONS TO THE DISPATCHER
  //     This allows them to be called dynamically from Spacely.

  _dispatcher.add("setUsrclkFreq", &SpacelyCaribouBasicDevice::setUsrclkFreq, this);
  _dispatcher.add("car_i2c_write", &SpacelyCaribouBasicDevice::car_i2c_write, this);
  _dispatcher.add("car_i2c_read", &SpacelyCaribouBasicDevice::car_i2c_read, this);

  _dispatcher.add("streamMemoryToFile", &SpacelyCaribouBasicDevice::streamMemoryToFile,this);
  
  _dispatcher.add("configureSI5345", &SpacelyCaribouBasicDevice::configureSI5345, this);
  _dispatcher.add("disableSI5345", &SpacelyCaribouBasicDevice::disableSI5345, this);
  _dispatcher.add("checkSI5345Locked", &SpacelyCaribouBasicDevice::checkSI5345Locked, this);

  _dispatcher.add("setOutputCMOSLevel", &SpacelyCaribouBasicDevice::setOutputCMOSLevel, this);
  _dispatcher.add("setInputCMOSLevel", &SpacelyCaribouBasicDevice::setInputCMOSLevel, this);


  // (2) SET UP PERIPHERY WITH CARBOARD DEVICES 
  _periphery.add("PWR_OUT_1", carboard::PWR_OUT_1);
  _periphery.add("PWR_OUT_2", carboard::PWR_OUT_2);
  _periphery.add("PWR_OUT_3", carboard::PWR_OUT_3);
  _periphery.add("PWR_OUT_4", carboard::PWR_OUT_4);
  _periphery.add("PWR_OUT_5", carboard::PWR_OUT_5);
  _periphery.add("PWR_OUT_6", carboard::PWR_OUT_6);
  _periphery.add("PWR_OUT_7", carboard::PWR_OUT_7);
  _periphery.add("PWR_OUT_8", carboard::PWR_OUT_8);

  _periphery.add("VOL_IN_1", carboard::VOL_IN_1);
  _periphery.add("VOL_IN_2", carboard::VOL_IN_2);
  _periphery.add("VOL_IN_3", carboard::VOL_IN_3);
  _periphery.add("VOL_IN_4", carboard::VOL_IN_4);

  _periphery.add("BIAS_1", carboard::BIAS_1);
  _periphery.add("BIAS_2", carboard::BIAS_2);
  _periphery.add("BIAS_3", carboard::BIAS_3);
  _periphery.add("BIAS_4", carboard::BIAS_4);
  _periphery.add("BIAS_5", carboard::BIAS_5);
  _periphery.add("BIAS_26", carboard::BIAS_26);    
  _periphery.add("BIAS_15", carboard::BIAS_15);    
  _periphery.add("BIAS_6", carboard::BIAS_6);    
  _periphery.add("BIAS_7", carboard::BIAS_7);
  _periphery.add("BIAS_9", carboard::BIAS_9);
  _periphery.add("BIAS_13", carboard::BIAS_13);

  _periphery.add("CUR_1", carboard::CUR_1);
  _periphery.add("CUR_2", carboard::CUR_2);
  _periphery.add("CUR_3", carboard::CUR_3);
  _periphery.add("CUR_4", carboard::CUR_4);
  _periphery.add("CUR_5", carboard::CUR_5);
  _periphery.add("CUR_6", carboard::CUR_6);
  _periphery.add("CUR_7", carboard::CUR_7);
  _periphery.add("CUR_8", carboard::CUR_8);

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

void SpacelyCaribouBasicDevice::streamMemoryToFile(const std::string& name, const unsigned int N) {

  // Based on the name, figure out the memory base address and offset.
  memory_map mem = _memory.get(name).first;
  size_t offset  = _memory.get(name).second.address();

  // Call the appropriate HAL function.
  _hal->streamMemoryToFile(mem, offset, N, "memory_dump.txt");


}


void SpacelyCaribouBasicDevice::setUsrclkFreq(const uint64_t freq) {


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

void SpacelyCaribouBasicDevice::car_i2c_write(const uint32_t bus, const uint32_t component_addr, const uint32_t mem_addr, const uint32_t data) {

  LOG(DEBUG) << "Writing to CaR Board I2C" << bus << " " << to_hex_string((unsigned char)component_addr) << " register " << to_hex_string((unsigned char)mem_addr)
	     << "(data:" << to_hex_string(data) << ")";


  const char* BUS_NAME = "none";

  if ( bus == 0 ) {
    BUS_NAME = BUS_I2C0;
  } else if ( bus == 1 ) {
    BUS_NAME = BUS_I2C1;
  } else if ( bus == 2 ) {
    BUS_NAME = BUS_I2C2;
  } else if ( bus == 3 ) {
    BUS_NAME = BUS_I2C3;
  }

  if ( !strcmp(BUS_NAME,"none") ) {
    LOG(DEBUG) << "Error, please select a bus 0 ~ 3";
    return;
  }
  
  iface_i2c& myi2c =
    InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_NAME, component_addr));


  iface_i2c::dataVector_type command = {static_cast<uint8_t>(data >> 4), static_cast<uint8_t>(data << 4)};

  
  myi2c.write(mem_addr, command);

}

uint16_t SpacelyCaribouBasicDevice::car_i2c_read(const uint32_t bus, const uint32_t component_addr, const uint32_t mem_addr, const uint32_t len) {
  LOG(DEBUG) << "Reading from the CaR Board I2C" << bus << " " << to_hex_string((unsigned char)component_addr) << " register " << to_hex_string((unsigned char)mem_addr)
	     << "(len:" << len << ")";


  const char* BUS_NAME = "none";

  if ( bus == 0 ) {
    BUS_NAME = BUS_I2C0;
  } else if ( bus == 1 ) {
    BUS_NAME = BUS_I2C1;
  } else if ( bus == 2 ) {
    BUS_NAME = BUS_I2C2;
  } else if ( bus == 3 ) {
    BUS_NAME = BUS_I2C3;
  }

  if ( !strcmp(BUS_NAME,"none") ) {
    LOG(DEBUG) << "Error, please select a bus 0 ~ 3";
    return 1;
  }
  
  iface_i2c& myi2c =
    InterfaceManager::getInterface<iface_i2c>(iface_i2c::configuration_type(BUS_NAME, component_addr));

  auto data = myi2c.read(mem_addr, len);

  if (data.empty()) {
    throw CommunicationError("No data returned");
  }


  //std::vector<uint8_t> result;
  //for( int i = 0; i < len; i++) {
  //  result.push_back(data.front())
  //    }
  //return result;

  uint16_t data1 = data.front();
  uint16_t data2 = data.front();
  uint16_t data_merged = (data1 << 8) + data2;

  LOG(DEBUG) << "Data1:" << data1 << " Data2:" << data2 << " Merged:" << data_merged;
  
  return data_merged;

  /*for(int i = 0; i < len; i++) {
    LOG(INFO) << "Returned: " << to_hex_string(data.front());
    }*/

}


void SpacelyCaribouBasicDevice::configureSI5345(int config_num) {

  LOG(DEBUG) << "Configuring Si5345 clock source with config " << config_num;

  if(config_num == 0) {
    _hal->configureSI5345(si5345_revd_registers, SI5345_REVD_REG_CONFIG_NUM_REGS);
  }
  if (config_num == 1) {
    _hal->configureSI5345(si5345_revd_registers_1, SI5345_REVD_REG_CONFIG_NUM_REGS_1);
  }
  if (config_num == 2) {
    _hal->configureSI5345(si5345_revd_registers_2, SI5345_REVD_REG_CONFIG_NUM_REGS_2);
  }
  /*if (config_num == 3) {   
    _hal->configureSI5345(si5345_revd_registers_3, SI5345_REVD_REG_CONFIG_NUM_REGS_3);
    }*/
  mDelay(100); // let the PLL lock

  // If required, check whether we are locked to external clock:
  /*if(!internal) {
    LOG(DEBUG) << "Waiting for clock to lock...";
    // Try for a limited time to lock, otherwise abort:
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while(!_hal->isLockedSI5345()) {
      auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
      if(dur.count() > 3)
        throw DeviceException("Cannot lock to external clock.");
    }
    }*/
}

void SpacelyCaribouBasicDevice::checkSI5345Locked() {
  LOG(INFO) << "The clock is " << ((_hal->isLockedSI5345()) ? " " : "NOT ") << "locked";
}

void SpacelyCaribouBasicDevice::disableSI5345() {
  LOG(DEBUG) << "Disabling Si5345 clock source";
  _hal->disableSI5345();
}


void SpacelyCaribouBasicDevice::setOutputCMOSLevel(double voltage) {
  LOG(DEBUG) << "Setting output CMOS level to " << voltage << " V";
  _hal->setOutputCMOSLevel(voltage);
}

void SpacelyCaribouBasicDevice::setInputCMOSLevel(double voltage) {
  LOG(DEBUG) << "Setting input CMOS level to " << voltage << " V";
  _hal->setInputCMOSLevel(voltage);
}


// SKELETON METHODS
//Need to provide definitions for these superclass functions
//or it will throw an error.
void SpacelyCaribouBasicDevice::powerUp() {
  LOG(INFO) << "Powering up (Not Implemented)";
  return;
}

void SpacelyCaribouBasicDevice::powerDown() {
  LOG(INFO) << "Powering down (Not Implemented)";
  return;
}


void SpacelyCaribouBasicDevice::daqStart() {
  LOG(INFO) << "DAQ Starting (Not Implemented)";
  return;
}


void SpacelyCaribouBasicDevice::daqStop() {
  LOG(INFO) << "DAQ Stopping (Not Implemented)";
  return;
}


SpacelyCaribouBasicDevice::~SpacelyCaribouBasicDevice() {
  LOG(INFO) << "Shutdown, delete device.";
  powerOff();
}



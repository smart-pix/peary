/**
 * Caribou Fastree3D poistioning stage
 */

#ifndef DEVICE_F3DSTAGEDEVICE_H
#define DEVICE_F3DSTAGEDEVICE_H

#include <string>

#include "device/AuxiliaryDevice.hpp"
#include "interfaces/IPSocket/ipsocket.hpp"
#include "utils/datatypes.hpp"
#include "utils/dictionary.hpp"

#define DEFAULT_DEVICEPATH "127.0.0.1:8888"

// clang-format off
#define F3DStage_REGISTERS					                         \
  {								                         \
   {"position", register_t<std::string>("p", std::string(), std::string())},	         \
   {"position_nonblocking", register_t<std::string>("n", std::string(), std::string(), false, true)},	         \
   {"velocity" , register_t<std::string>("v", std::string(), std::string())},		 \
   {"acceleration", register_t<std::string>("a", std::string(), std::string())},	 \
   {"current_position", register_t<std::string>("P", std::string(), std::string(), true, false)},	         \
   {"current_velocity" , register_t<std::string>("V", std::string(), std::string(), true, false)},		 \
   {"delay", register_t<std::string>("d", std::string(), std::string())},                \
   {"firmware", register_t<std::string>("f", std::string(), std::string(), true, false)},\
   {"reset", register_t<std::string>("r", std::string(), std::string(), false, true)},   \
  }
// clang-format on

namespace caribou {

  class F3DStageDevice : public AuxiliaryDevice<iface_ipsocket> {

  public:
    F3DStageDevice(const caribou::Configuration config);
    ~F3DStageDevice();

    void reset() override;
    void configure() override;
    void setRegister(std::string name, uintptr_t value) override;
    uintptr_t getRegister(std::string name) override;

  protected:
    caribou::dictionary<register_t<std::string>> _registers;
  };

} // namespace caribou

#endif /* DEVICE_F3DSTAGEDEVICE_H */

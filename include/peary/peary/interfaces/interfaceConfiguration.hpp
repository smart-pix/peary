/* This file contains the configuration class used by the Caribou libraries
 */

#ifndef CARIBOU_INTERFACE_CONFIG_H
#define CARIBOU_INTERFACE_CONFIG_H
#include <any>
#include <map>
#include <string>

namespace caribou {

  class InterfaceConfiguration {
  public:
    explicit InterfaceConfiguration(std::string);

    virtual bool operator<(const InterfaceConfiguration& rhs) const;

    std::string _devpath;
  };

} // namespace caribou
#endif

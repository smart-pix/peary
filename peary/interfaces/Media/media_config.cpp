/**
 * Caribou media interface class implementation
 */

#include "media.hpp"

using namespace caribou;

iface_media_config::iface_media_config(const std::string& devpath,
                                       std::string outputEntity,
                                       std::string links,
                                       std::string routes,
                                       std::string formats,
                                       const unsigned int timeout,
                                       const unsigned int noBuffers)
    : InterfaceConfiguration(devpath), _outputEntity(std::move(outputEntity)), _links(std::move(links)),
      _routes(std::move(routes)), _formats(std::move(formats)), _timeout(timeout), _noBuffers(noBuffers) {}

bool iface_media_config::operator<(const iface_media_config& rhs) const {
  if(!InterfaceConfiguration::operator<(rhs) && !rhs.InterfaceConfiguration::operator<(*this)) {
    if(!_outputEntity.compare(rhs._outputEntity)) {
      if(!_links.compare(rhs._links)) {
        if(!_routes.compare(rhs._routes)) {
          if(!_formats.compare(rhs._formats)) {
            if(_timeout == rhs._timeout) {
              return _noBuffers < rhs._noBuffers;
            } else {
              return _timeout < rhs._timeout;
            }
          } else {
            return _formats < rhs._formats;
          }
        } else {
          return _routes < rhs._routes;
        }
      } else {
        return _links < rhs._links;
      }
    } else {
      return _outputEntity < rhs._outputEntity;
    }
  } else {
    return InterfaceConfiguration::operator<(rhs);
  }
}

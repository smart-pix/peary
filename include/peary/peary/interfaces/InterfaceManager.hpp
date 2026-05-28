#ifndef INTERFACE_MANAGER_HPP
#define INTERFACE_MANAGER_HPP

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "Interface.hpp"

namespace caribou {

  class InterfaceManager {

  protected:
    // Default constructor: protected for singleton class
    InterfaceManager() = default;

  private:
    // Static instance of this singleton class
    static InterfaceManager instance;

    // Mutex protecting interface generation
    static std::mutex mutex;

  public:
    /* Get instance of the interface class
     *
     */
    template <typename T> static T& getInterface(typename T::configuration_type const& config) {
      static std::map<typename T::configuration_type, std::unique_ptr<T, void (*)(T*)>> interfaces;

      std::lock_guard<std::mutex> lock(mutex);

      try {
        return *interfaces.at(config);
      }
      // such interface has not been opened yet
      catch(const std::out_of_range&) {

        interfaces.emplace(std::piecewise_construct,
                           std::forward_as_tuple(config),
                           std::forward_as_tuple(new T(config), InterfaceManager::deleteInterface<T>));
        return *interfaces.at(config);
      }
    }

    template <typename T> static void deleteInterface(T* interface) { delete interface; }

    /* Delete unwanted functions from singleton class (C++11)
     */
    InterfaceManager(InterfaceManager const&) = delete;
    void operator=(InterfaceManager const&) = delete;
  };

} // namespace caribou

#endif /*INTERFACE_MANAGER_HPP*/

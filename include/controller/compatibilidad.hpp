// include/compatibility.hpp
#ifndef COMPATIBILITY_HPP
#define COMPATIBILITY_HPP

#include "config/version.hpp"
#include "shared/version_comparator.hpp"
#include <string>

namespace Maxicajero {

class CompatibilityManager {
private:
    VersionUtils::Version currentVersion;
    VersionUtils::Version minClientVersion;
    VersionUtils::Version maxClientVersion;
    
public:
    CompatibilityManager() {
        // Versión actual del servidor
        currentVersion = VersionUtils::Version::fromString(
            Maxicajero::Version::getVersion()
        );
        
        // Definir políticas de compatibilidad
        // Ejemplo: Clientes 1.0.0 a 2.0.0 son compatibles
        minClientVersion = VersionUtils::Version(1, 0, 0);
        maxClientVersion = VersionUtils::Version(2, 0, 0);
    }
    
    // Verificar cliente individual
    bool isClientCompatible(const std::string& clientVersionStr) {
        auto clientVersion = VersionUtils::Version::fromString(clientVersionStr);
        
        // Política: BACKWARD (cliente <= servidor)
        return VersionUtils::CompatibilityChecker::isCompatible(
            clientVersion,
            currentVersion,
            VersionUtils::CompatibilityChecker::Policy::BACKWARD
        );
    }
    
    // Verificar con rango específico
    bool isClientInSupportedRange(const std::string& clientVersionStr) {
        auto clientVersion = VersionUtils::Version::fromString(clientVersionStr);
        return VersionUtils::CompatibilityChecker::isInRange(
            clientVersion,
            minClientVersion,
            maxClientVersion
        );
    }
    
    // Para API REST
    struct CompatibilityResponse {
        bool compatible;
        std::string message;
        std::string serverVersion;
        std::string minVersion;
        std::string maxVersion;
    };
    
    CompatibilityResponse checkCompatibility(const std::string& clientVersionStr) {
        CompatibilityResponse response;
        response.serverVersion = currentVersion.toString();
        response.minVersion = minClientVersion.toString();
        response.maxVersion = maxClientVersion.toString();
        
        auto clientVersion = VersionUtils::Version::fromString(clientVersionStr);
        
        // Primero verificar rango
        if (!isClientInSupportedRange(clientVersionStr)) {
            response.compatible = false;
            response.message = "Versión de cliente fuera del rango soportado. "
                             "Rango soportado: " + minClientVersion.toString() + 
                             " a " + maxClientVersion.toString();
            return response;
        }
        
        // Luego verificar compatibilidad específica
        response.compatible = isClientCompatible(clientVersionStr);
        response.message = VersionUtils::CompatibilityChecker::getCompatibilityMessage(
            clientVersion,
            currentVersion,
            VersionUtils::CompatibilityChecker::Policy::BACKWARD
        );
        
        return response;
    }
    
    // Getters
    std::string getCurrentVersion() const { return currentVersion.toString(); }
    std::string getMinClientVersion() const { return minClientVersion.toString(); }
    std::string getMaxClientVersion() const { return maxClientVersion.toString(); }
};

} // namespace Maxicajero

#endif // COMPATIBILITY_HPP
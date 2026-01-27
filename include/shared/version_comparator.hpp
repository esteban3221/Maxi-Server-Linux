// include/utils/version_comparator.hpp
#ifndef VERSION_COMPARATOR_HPP
#define VERSION_COMPARATOR_HPP

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdint>

namespace Maxicajero {
namespace VersionUtils {

class Version {
private:
    int major;
    int minor;
    int patch;
    std::string suffix; // opcional: "-alpha", "-beta", "-rc1", etc.
    
public:
    Version(int major = 0, int minor = 0, int patch = 0, const std::string& suffix = "")
        : major(major), minor(minor), patch(patch), suffix(suffix) {}
    
    // Parsear string "1.2.3" o "1.2.3-beta"
    static Version fromString(const std::string& versionStr) {
        Version version;
        std::string cleanStr = versionStr;
        
        // Separar sufijo si existe
        size_t dashPos = versionStr.find('-');
        if (dashPos != std::string::npos) {
            cleanStr = versionStr.substr(0, dashPos);
            version.suffix = versionStr.substr(dashPos + 1);
        }
        
        // Parsear números
        std::replace(cleanStr.begin(), cleanStr.end(), '.', ' ');
        std::istringstream iss(cleanStr);
        
        if (!(iss >> version.major)) version.major = 0;
        if (!(iss >> version.minor)) version.minor = 0;
        if (!(iss >> version.patch)) version.patch = 0;
        
        return version;
    }
    
    std::string toString() const {
        std::ostringstream oss;
        oss << major << "." << minor << "." << patch;
        if (!suffix.empty()) {
            oss << "-" << suffix;
        }
        return oss.str();
    }
    
    // Getters
    int getMajor() const { return major; }
    int getMinor() const { return minor; }
    int getPatch() const { return patch; }
    const std::string& getSuffix() const { return suffix; }
    
    // Operadores de comparación
    bool operator==(const Version& other) const {
        return major == other.major && 
               minor == other.minor && 
               patch == other.patch &&
               suffix == other.suffix;
    }
    
    bool operator!=(const Version& other) const {
        return !(*this == other);
    }
    
    bool operator<(const Version& other) const {
        if (major != other.major) return major < other.major;
        if (minor != other.minor) return minor < other.minor;
        if (patch != other.patch) return patch < other.patch;
        
        // Comparar sufijos (simplificado)
        return compareSuffix(suffix, other.suffix) < 0;
    }
    
    bool operator>(const Version& other) const {
        return other < *this;
    }
    
    bool operator<=(const Version& other) const {
        return !(*this > other);
    }
    
    bool operator>=(const Version& other) const {
        return !(*this < other);
    }
    
private:
    // Comparador simple de sufijos
    static int compareSuffix(const std::string& a, const std::string& b) {
        if (a.empty() && b.empty()) return 0;
        if (a.empty()) return 1;  // Sin sufijo > con sufijo
        if (b.empty()) return -1; // Con sufijo < sin sufijo
        
        // Orden: alpha < beta < rc < (nada) < dev
        static const std::vector<std::string> order = {
            "alpha", "beta", "rc", "", "dev"
        };
        
        auto itA = std::find(order.begin(), order.end(), a);
        auto itB = std::find(order.begin(), order.end(), b);
        
        if (itA == order.end()) itA = order.begin();
        if (itB == order.end()) itB = order.begin();
        
        return std::distance(order.begin(), itA) - std::distance(order.begin(), itB);
    }
};

class CompatibilityChecker {
public:
    // Política de compatibilidad
    enum class Policy {
        STRICT,      // Versiones exactamente iguales
        BACKWARD,    // Cliente <= Servidor
        FORWARD,     // Cliente >= Servidor
        RANGE,       // Versión mínima y máxima
        MAJOR_ONLY   // Solo mayor igual
    };
    
    // Verificar compatibilidad
    static bool isCompatible(const Version& clientVersion,
                            const Version& serverVersion,
                            Policy policy = Policy::BACKWARD) {
        switch (policy) {
            case Policy::STRICT:
                return clientVersion == serverVersion;
                
            case Policy::BACKWARD:
                // Cliente puede ser igual o anterior al servidor
                return clientVersion <= serverVersion;
                
            case Policy::FORWARD:
                // Cliente puede ser igual o posterior al servidor
                return clientVersion >= serverVersion;
                
            case Policy::MAJOR_ONLY:
                // Solo verificar versión mayor
                return clientVersion.getMajor() == serverVersion.getMajor();
                
            default:
                return false;
        }
    }
    
    // Verificar con rango específico
    static bool isInRange(const Version& version,
                         const Version& minVersion,
                         const Version& maxVersion) {
        return version >= minVersion && version <= maxVersion;
    }
    
    // Obtener mensaje de error detallado
    static std::string getCompatibilityMessage(const Version& clientVersion,
                                              const Version& serverVersion,
                                              Policy policy = Policy::BACKWARD) {
        std::ostringstream oss;
        oss << "Cliente: " << clientVersion.toString() 
            << ", Servidor: " << serverVersion.toString() << ". ";
            
        if (isCompatible(clientVersion, serverVersion, policy)) {
            oss << "Versiones compatibles.";
        } else {
            oss << "Versiones INCOMPATIBLES. ";
            
            switch (policy) {
                case Policy::BACKWARD:
                    oss << "El cliente debe ser versión " << serverVersion.toString() 
                        << " o anterior.";
                    break;
                case Policy::FORWARD:
                    oss << "El cliente debe ser versión " << serverVersion.toString()
                        << " o posterior.";
                    break;
                case Policy::STRICT:
                    oss << "Se requiere versión exacta " << serverVersion.toString() << ".";
                    break;
                case Policy::MAJOR_ONLY:
                    oss << "Se requiere misma versión mayor (" 
                        << serverVersion.getMajor() << ".x.x).";
                    break;
                default:
                    oss << "Política de compatibilidad desconocida.";
            }
        }
        
        return oss.str();
    }
};

} // namespace VersionUtils
} // namespace Maxicajero

#endif // VERSION_COMPARATOR_HPP
#include <qsox/Error.hpp>
#include <string.h>

namespace qsox {

Error Error::lastOsError(bool) {
    // TODO: map into other types than Other
    return fromOs(errno);
}

std::string Error::osMessage(int osCode) {
    return strerror(osCode);
}

}
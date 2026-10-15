# CPack configuration script for NativeGate packaging

# Ensure only DEB or RPM generators are used
if(NOT CPACK_GENERATOR STREQUAL "DEB" AND NOT CPACK_GENERATOR STREQUAL "RPM")
    message(FATAL_ERROR "Unsupported generator. Only DEB and RPM are allowed.")
endif()

# Check if the build environment matches the target environment
if(CPACK_G_HTOP_BUILD_FAMILY STREQUAL "unsupported")
    message(FATAL_ERROR "Native target environment required: Do not repackage a Manjaro-built binary. Configure and rebuild from a clean tree inside the target distribution.")
elseif(NOT CPACK_GENERATOR STREQUAL CPACK_G_HTOP_BUILD_FAMILY)
    message(FATAL_ERROR "Mismatch between build family (${CPACK_G_HTOP_BUILD_FAMILY}) and generator (${CPACK_GENERATOR}).")
endif()

# Reuse the same parser at configure and package time.
include("${CMAKE_CURRENT_LIST_DIR}/HostFamily.cmake")
g_htop_host_family(current_id CURRENT_FAMILY)
if(NOT CPACK_GENERATOR STREQUAL CPACK_G_HTOP_CONFIGURED_FORMAT)
  message(FATAL_ERROR "Configure a clean tree with G_HTOP_PACKAGE_FORMAT matching the requested generator.")
endif()

# Check if the current family matches the generator
if(NOT CPACK_GENERATOR STREQUAL CURRENT_FAMILY)
    message(FATAL_ERROR "Mismatch between current OS family (${CURRENT_FAMILY}) and generator (${CPACK_GENERATOR}).")
endif()

# Validate maintainer email
if(CPACK_G_HTOP_MAINTAINER_EMAIL MATCHES "^[^ ]+@[^ ]+$")
    # Email is valid, continue packaging
else()
    message(FATAL_ERROR "Invalid maintainer email. Please pass -DG_HTOP_MAINTAINER_EMAIL using an owner-approved email address.")
endif()


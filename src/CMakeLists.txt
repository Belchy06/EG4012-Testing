set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

set(OVB_COMMON_SOURCES
    "ovb_common/platform/platform.cpp"
    "ovb_common/rtp/packet.cpp")

set(OVB_RECV_SOURCES
    "ovb_recv/decoders/avc/avc.cpp"
    "ovb_recv/decoders/ovc/ovc.cpp"
    "ovb_recv/decoders/vvc/vvc.cpp"
    "ovb_recv/decoders/xvc/xvc.cpp"
    "ovb_recv/decoders/decoder_factory.cpp"
    "ovb_recv/decoders/decoder.cpp"
    "ovb_recv/depacketizer/avc/avc_depacketizer.cpp"
    "ovb_recv/depacketizer/ovc/ovc_depacketizer.cpp"
    "ovb_recv/depacketizer/vvc/vvc_depacketizer.cpp"
    "ovb_recv/depacketizer/xvc/xvc_depacketizer.cpp"
    "ovb_recv/depacketizer/depacketizer_factory.cpp"
    "ovb_recv/rtp_receiver/rtp_receiver.cpp"
    "ovb_recv/socket/socket.cpp"
    "ovb_recv/vmaf/vmaf.cpp"
    "ovb_recv/receiver.cpp"
    "ovb_recv/y4m_writer.cpp"
    "ovb_recv/main.cpp")

set(OVB_SEND_SOURCES
    "ovb_send/encoders/avc/avc.cpp"
    "ovb_send/encoders/ovc/ovc.cpp"
    "ovb_send/encoders/vvc/vvc.cpp"
    "ovb_send/encoders/xvc/xvc.cpp"
    "ovb_send/encoders/encoder_factory.cpp"
    "ovb_send/encoders/encoder.cpp"
    "ovb_send/packetizer/avc/avc_packetizer.cpp"
    "ovb_send/packetizer/ovc/ovc_packetizer.cpp"
    "ovb_send/packetizer/vvc/vvc_packetizer.cpp"
    "ovb_send/packetizer/xvc/xvc_packetizer.cpp"
    "ovb_send/packetizer/packetizer_factory.cpp"
    "ovb_send/rtp_sender/rtp_sender.cpp"
    "ovb_send/socket/socket.cpp"
    "ovb_send/sender.cpp"
    "ovb_send/y4m_reader.cpp"
    "ovb_send/main.cpp")

set(OVB_RELAY_SOURCES
    "ovb_relay/drop/simple_bursty/simple_bursty_dropper.cpp"
    "ovb_relay/drop/complex_bursty/complex_bursty_dropper.cpp"
    "ovb_relay/drop/continuous/continuous_dropper.cpp"
    "ovb_relay/drop/dropper_factory.cpp"
    "ovb_relay/tamper/tamperer.cpp"
    "ovb_relay/recv_socket/socket.cpp"
    "ovb_relay/send_socket/socket.cpp"
    "ovb_relay/relay.cpp"
    "ovb_relay/main.cpp")

if(MSVC)
  set(cxx_base_flags /GS /W4 /WX)
  set(cxx_exception_flags "-EHsc")
  set(cxx_strict_flags
    /we4254 # error: conversion, possible loss of data
    /we4296 # error: expression is always true/false
    /we4388 # error: signed/unsigned mismatch
    /we4389 # error: signed/unsigned mismatch
    /we4706 # error: assignment within conditional expression
    /wd4100 # disabled: unreferenced formal parameter
    /wd4018 # disabled: signed/unsigned mismatch
    /wd4127 # disabled: conditional expression is constant
    /wd4752 # disabled: found Intel(R) Advanced Vector Extensions; consider using /arch:AVX
    /wd4244
    /wd4458
  )
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(cxx_base_flags -Wall -Wshadow)
  set(cxx_exception_flags "-fexceptions")
  set(cxx_strict_flags -Werror -Wextra -Wunused -Wold-style-cast -Wlogical-op -Wpointer-arith
        -Wno-unused-parameter -Wno-missing-field-initializers)
  if(OVB_TARGET_ARCH STREQUAL "x86" AND MINGW)
    list(APPEND cxx_base_flags "-mxsave")
  endif()
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    message(FATAL_ERROR "thread_local requires gcc >= 4.8")
    set(cxx_strict_flags ${cxx_strict_flags} -Wno-error=missing-braces)
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(cxx_base_flags -Wall -Wshadow)
  set(cxx_exception_flags "-fexceptions")
  set(cxx_strict_flags -Werror -Wextra -Wno-missing-braces -Wno-unused-const-variable -Wno-unused-parameter)
  if(OVB_TARGET_ARCH STREQUAL "x86" AND MINGW)
    list(APPEND cxx_base_flags "-mxsave")
  endif()
endif()
set(cxx_default ${cxx_base_flags} ${cxx_exception_flags})
set(cxx_strict ${cxx_strict_flags})
set(linker_flags "")

set(ovb_recv_src_extra "")
set(ovb_send_src_extra "")
set(ovb_relay_src_extra "")

include_directories(
    "third_party/ovc/src" 
    "third_party/xvc/src"
    "third_party/vvc/vvenc/include"
    "third_party/vvc/vvdec/include"
    "third_party/avc/codec/api"
    "third_party/vmaf/libvmaf/include")
link_directories(
    "third_party/ovc/build/src/Debug"
    "third_party/xvc/build/src/Debug"
    "third_party/vvc/vvenc/lib/debug-static"
    "third_party/vvc/vvdec/lib/debug-static"
    "third_party/avc/bin/x64/Debug")

# ovb_common
add_library (ovb_common OBJECT ${OVB_COMMON_SOURCES})
target_compile_options(ovb_common PRIVATE ${cxx_default} ${cxx_strict})
target_include_directories(ovb_common PUBLIC .)

# ovb_recv
add_executable(ovb_recv ${OVB_RECV_SOURCES} $<TARGET_OBJECTS:ovb_common> ${ovb_recv_src_extra})
set_target_properties(ovb_recv PROPERTIES OUTPUT_NAME "ovbrecv")
target_compile_options(ovb_recv PRIVATE ${cxx_default} ${cxx_strict})
target_include_directories (ovb_recv PUBLIC .)
target_link_libraries(ovb_recv INTERFACE ${linker_flags} PUBLIC Threads::Threads ${ws2_32_LIBRARY_PATH} xvcdecd ovcdec vvdec welsdcore welsdec)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/vmaf/vmaf-install/bin/libvmaf.dll")
  add_custom_command(TARGET ovb_recv POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/third_party/vmaf/vmaf-install/bin/libvmaf.dll"      # <--this is in-file
      $<TARGET_FILE_DIR:ovb_recv>)   
endif()

# ovb_send
add_executable(ovb_send ${OVB_SEND_SOURCES} $<TARGET_OBJECTS:ovb_common> ${ovb_send_src_extra})
set_target_properties(ovb_send PROPERTIES OUTPUT_NAME "ovbsend")
target_compile_options(ovb_send PRIVATE ${cxx_default} ${cxx_strict})
target_include_directories (ovb_send PUBLIC .)
target_link_libraries(ovb_send INTERFACE ${linker_flags} PUBLIC Threads::Threads ${ws2_32_LIBRARY_PATH} xvcencd ovcenc vvenc welsecore welsenc)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/avc/bin/x64/Debug/welsenc.dll")
  add_custom_command(TARGET ovb_send POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/third_party/avc/bin/x64/Debug/welsenc.dll"      # <--this is in-file
      $<TARGET_FILE_DIR:ovb_send>)   
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/avc/bin/x64/Debug/welsvp.dll")
  add_custom_command(TARGET ovb_send POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/third_party/avc/bin/x64/Debug/welsvp.dll"      # <--this is in-file
      $<TARGET_FILE_DIR:ovb_send>)   
endif()

# ovb_relay
add_executable(ovb_relay ${OVB_RELAY_SOURCES} $<TARGET_OBJECTS:ovb_common> ${ovb_relay_src_extra})
set_target_properties(ovb_relay PROPERTIES OUTPUT_NAME "ovbrelay")
target_compile_options(ovb_relay PRIVATE ${cxx_default} ${cxx_strict})
target_include_directories (ovb_relay PUBLIC .)
target_link_libraries(ovb_relay INTERFACE ${linker_flags} PUBLIC Threads::Threads ${ws2_32_LIBRARY_PATH})

install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/ovb_send/sender.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ovb_recv/receiver.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS ovb_recv ovb_send DESTINATION "${CMAKE_INSTALL_LIBDIR}")
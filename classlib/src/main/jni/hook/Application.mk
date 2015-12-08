# The ARMv7 is significanly faster due to the use of the hardware FPU
APP_ABI := armeabi
APP_PLATFORM := android-9
APP_STL := gnustl_static
APP_STL += stlport_static
APP_STL +=c++_shared
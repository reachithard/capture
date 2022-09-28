file(GLOB CaptureSource *.c *.cpp)

add_library(modcapture STATIC ${CaptureSource})
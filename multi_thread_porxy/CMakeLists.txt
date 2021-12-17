cmake_minimum_required(VERSION 3.20)
project(multi_thread_porxy)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

add_executable(multi_thread_porxy main.cpp Proxy.cpp Proxy.h Connections/ClientConnection.cpp Connections/ClientConnection.h Connections/ServerConnection.cpp Connections/ServerConnection.h Cache/Cache.h Cache/CacheEntry.h Constants.h Connections/Connection.h HttpParser/httprequestparser.h HttpParser/httpresponseparser.h HttpParser/request.h HttpParser/response.h HttpParser/urlparser.h CurTime.h utils/WaitersMap.h Cache/Buffer.h)

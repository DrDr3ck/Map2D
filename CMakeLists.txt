cmake_minimum_required(VERSION 3.0)

project(Map2D)

# SDL2 search components
  INCLUDE(FindPkgConfig)

  PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
  PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
  PKG_SEARCH_MODULE(SDL2MIXER REQUIRED SDL2_mixer>=2.0.0)
  PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)


# copy resources (not in a smart way)
file(COPY ${CMAKE_SOURCE_DIR}/6224_out.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/bakhar.opt DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/buttons DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/craft.xml DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/FreePixel.ttf DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/images DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/items DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/language DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/none.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/objects DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/pixel11.ttf DESTINATION ${CMAKE_BINARY_DIR})



# list all sources:
set(Map2D_sources
  action.cpp
  archive.cpp
  button.cpp
  camera.cpp
  character.cpp
  craft_mgr.cpp
  dialog.cpp
  font.cpp
  items.cpp
  job.cpp
  logger.cpp
  main.cpp
  map.cpp
  object.cpp
  path_finding.cpp
  perlin_noise.cpp
  sdl_background.cpp
  sdl_button.cpp
  sdl_camera.cpp
  sdl_tool.cpp
  session.cpp
  tests.cpp
  texture_mgr.cpp
  translator.cpp
  xml_document.cpp
)

# list headers
set(Map2D_headers
  action.h
  archive.h
  button.h
  camera.h
  character.h
  craft_mgr.h
  dialog.h
  font.h
  items.h
  job.h
  logger.h
  map.h
  object.h
  path_finding.h
  perlin_noise.h
  sdl_background.h
  sdl_button.h
  sdl_camera.h
  sdl_tool.h
  session.h
  tests.h
  texture_mgr.h
  translator.h
  utility.h
  xml_document.h
)

add_executable(Map2D ${Map2D_headers} ${Map2D_sources})
target_link_libraries(Map2D
  ${SDL2_LIBRARIES}
  ${SDL2IMAGE_LIBRARIES}
  ${SDL2MIXER_LIBRARIES}
  ${SDL2TTF_LIBRARIES}
)

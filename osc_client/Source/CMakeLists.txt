file(GLOB source_files  Config.h
                        Main.cpp
                        MainComponent.h
                        MainComponent.cpp
                        M1Checkbox.h)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${source_files})
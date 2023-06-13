file(GLOB source_files  Config.h
                        Main.cpp
                        MainComponent.h
                        MainComponent.cpp)

target_sources(M1-OrientationClient PRIVATE ${source_files})
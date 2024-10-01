#! /bin/bash

# MACH1 ORIENTATION MANAGER Makefile

# getting OS type
ifeq ($(OS),Windows_NT)
	detected_OS := Windows
else
	detected_OS := $(shell uname)
endif

setup:

clean:
ifeq ($(detected_OS),Windows)
	@if exist m1-orientationmanager\\build (rmdir /s /q m1-orientationmanager\\build)
	@if exist m1-orientationmanager\\osc_client\\build (rmdir /s /q m1-orientationmanager\\osc_client\\build)
else
	rm -rf m1-orientationmanager/build
	rm -rf m1-orientationmanager/osc_client/build
endif

clean-dev:
ifeq ($(detected_OS),Windows)
	@if exist m1-orientationmanager\\build-dev (rmdir /s /q m1-orientationmanager\\build-dev)
	@if exist m1-orientationmanager\\osc_client\\build-dev (rmdir /s /q m1-orientationmanager\\osc_client\\build-dev)
else
	rm -rf m1-orientationmanager/build-dev
	rm -rf m1-orientationmanager/osc_client/build-dev
endif

clean-installs:
ifeq ($(detected_OS),Darwin)
	sudo rm -rf /Library/LaunchAgents/com.mach1.spatial.orientationmanager.plist
	sudo rm -rf /Library/Application Support/Mach1/m1-orientationmanager
endif

# configure for debug and setup dev envs with common IDEs
dev: clean-dev
ifeq ($(detected_OS),Darwin)
	cmake . -Bbuild-dev -G "Xcode" -DENABLE_DEBUG_EMULATOR_DEVICE=ON -DCMAKE_INSTALL_PREFIX="/Library/Application Support/Mach1"
	cmake osc_client -Bosc_client/build-dev -G "Xcode"
else ifeq ($(detected_OS),Windows)
	cmake . -Bbuild-dev -DENABLE_DEBUG_EMULATOR_DEVICE=ON -DCMAKE_INSTALL_PREFIX="\Documents and Settings\All Users\Application Data\Mach1"
	cmake osc_client -Bosc_client/build-dev
else
	cmake . -Bbuild-dev -DENABLE_DEBUG_EMULATOR_DEVICE=ON -DCMAKE_INSTALL_PREFIX="/opt/Mach1"
	cmake osc_client -Bosc_client/build-dev
endif

# clean and configure for release
configure: clean
ifeq ($(detected_OS),Darwin)
	cmake . -Bbuild -G "Xcode" -DM1_ORIENTATION_MANAGER_EMBEDDED=1
	cmake osc_client -Bosc_client/build -G "Xcode" -DM1_ORIENTATION_MANAGER_EMBEDDED=1
else
	cmake . -Bbuild -DM1_ORIENTATION_MANAGER_EMBEDDED=1
	cmake osc_client -Bosc_client/build -DM1_ORIENTATION_MANAGER_EMBEDDED=1
endif

build: 
	cmake --build build --config "Release"
	cmake --build osc_client/build --config "Release"

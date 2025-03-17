# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(camera)

FILE(GLOB app_sources src/main.c)
target_sources(app PRIVATE src/mac_header_field.c)
target_sources(app PRIVATE src/physical_header_field.c)
target_sources(app PRIVATE src/mac_message.c)
target_sources(app PRIVATE src/procedures.c)
target_sources(app PRIVATE ${app_sources})

#==========================================================================#
#                                                                          #
#                    (C) Copyright 2018 Devil7 Softwares.                  #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#                http://www.apache.org/licenses/LICENSE-2.0                #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
#                                                                          #
# Contributors :                                                           #
#     Dineshkumar T                                                        #
#                                                                          #
#==========================================================================#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE					:= rw-afterboot
LOCAL_MODULE_TAGS				:= optional
LOCAL_SRC_FILES 				:= rw-afterboot.cpp
LOCAL_C_INCLUDES				:= external/libcxx/include external/selinux/libsepol/include
LOCAL_STATIC_LIBRARIES 			:= libc libselinux-inject liblog libsepol
LOCAL_LDFLAGS 					:= -static 
LOCAL_FORCE_STATIC_EXECUTABLE	:= true

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE					:= libselinux-inject
LOCAL_MODULE_TAGS				:= optional
LOCAL_C_INCLUDES				:= external/selinux/libsepol/include
LOCAL_SRC_FILES					:= selinux-inject.c
LOCAL_MODULE_CLASS				:= STATIC_LIBRARIES
LOCAL_STATIC_LIBRARIES			:= libsepol liblog

include $(BUILD_STATIC_LIBRARY)



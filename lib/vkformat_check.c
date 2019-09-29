
/***************************** Do not edit.  *****************************
 Automatically generated from vulkan_core.h version 121 by mkvkformatfiles.
 *************************************************************************/

/*
** Copyright (c) 2015-2019 The Khronos Group Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <stdint.h>
#include <stdbool.h>

#include "vkformat_enum.h"

bool
isProhibitedFormat(VkFormat format)
{
    switch (format) {
      case VK_FORMAT_R8_USCALED:
      case VK_FORMAT_R8_SSCALED:
      case VK_FORMAT_R8G8_USCALED:
      case VK_FORMAT_R8G8_SSCALED:
      case VK_FORMAT_R8G8B8_USCALED:
      case VK_FORMAT_R8G8B8_SSCALED:
      case VK_FORMAT_B8G8R8_USCALED:
      case VK_FORMAT_B8G8R8_SSCALED:
      case VK_FORMAT_R8G8B8A8_USCALED:
      case VK_FORMAT_R8G8B8A8_SSCALED:
      case VK_FORMAT_B8G8R8A8_USCALED:
      case VK_FORMAT_B8G8R8A8_SSCALED:
      case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_UINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
      case VK_FORMAT_R16_USCALED:
      case VK_FORMAT_R16_SSCALED:
      case VK_FORMAT_R16G16_USCALED:
      case VK_FORMAT_R16G16_SSCALED:
      case VK_FORMAT_R16G16B16_USCALED:
      case VK_FORMAT_R16G16B16_SSCALED:
      case VK_FORMAT_R16G16B16A16_USCALED:
      case VK_FORMAT_R16G16B16A16_SSCALED:
      case VK_FORMAT_G8B8G8R8_422_UNORM:
      case VK_FORMAT_B8G8R8G8_422_UNORM:
      case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
      case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
      case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
      case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
      case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
      case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      case VK_FORMAT_G16B16G16R16_422_UNORM:
      case VK_FORMAT_B16G16R16G16_422_UNORM:
      case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
      case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
      case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
      case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
      case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
        return true;
      default:
        return false;
    }
}

bool
isValidFormat(VkFormat format)
{
    if (format < VK_FORMAT_END_RANGE)
        return true;
    else switch(format) {
        case VK_FORMAT_R10X6_UNORM_PACK16:
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        case VK_FORMAT_R12X4_UNORM_PACK16:
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT:
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT:
        return true;
      default:
        return false;
    }
}


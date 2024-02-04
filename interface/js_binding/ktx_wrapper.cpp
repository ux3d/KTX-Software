/* -*- tab-width: 4; -*- */
/* vi: set sw=2 ts=4 expandtab textwidth=80: */

/*
 * Copyright 2019-2020 Khronos Group, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <emscripten/bind.h>
#include <ktx.h>
#include <iostream>
#include <string>
#include "vkformat_enum.h"

using namespace emscripten;

#define ktxTexture2(t) reinterpret_cast<ktxTexture2*>(m_ptr.get())

namespace ktx {
    enum class SwizzleChannel : char { R = 'r', G = 'g', B = 'b', A = 'a' };

    struct BasisParams {
    bool uastc = false;
    uint32_t threadCount;
    uint32_t compressionLevel;
    uint32_t qualityLevel;
    uint32_t maxEndpoints;
    float endpointRDOThreshold;
    uint32_t maxSelectors;
    float selectorRDOThreshold;
    SwizzleChannel inputSwizzle[4];
    bool normalMap;
    bool preSwizzle;
    bool noEndpointRDO;
    bool noSelectorRDO;
    ktx_pack_uastc_flag_bits_e uastcFlags;
    bool uastcRDOQualityScalar;
    uint32_t uastcRDODictSize;
    float uastcRDOMaxSmoothBlockErrorScale;
    float uastcRDOMaxSmoothBlockStdDev;
    bool uastcRDODontFavorSimplerModes;
    bool uastcRDONoMultithreading;
    };

    struct CreateInfo {
        VkFormat vkFormat;   /*!< VkFormat for texture. Ignored when creating a
                                    ktxTexture1. */
        uint32_t baseWidth;  /*!< Width of the base level of the texture. */
        uint32_t baseHeight; /*!< Height of the base level of the texture. */
        uint32_t baseDepth;  /*!< Depth of the base level of the texture. */
        uint32_t numDimensions; /*!< Number of dimensions in the texture, 1, 2
                                        or 3. */
        uint32_t numLevels; /*!< Number of mip levels in the texture. Should be
                                    1 if @c generateMipmaps is KTX_TRUE; */
        uint32_t numLayers; /*!< Number of array layers in the texture. */
        uint32_t numFaces;  /*!< Number of faces: 6 for cube maps, 1 otherwise. */
        bool   isArray;  /*!< Set to KTX_TRUE if the texture is to be an
                                    array texture. Means OpenGL will use a
                                    GL_TEXTURE_*_ARRAY target. */
        bool   generateMipmaps; /*!< Set to KTX_TRUE if mipmaps should be
                                        generated for the texture when loading
                                        into a 3D API. */
    };
    
    class texture
    {
    public:
        texture(texture&) = delete;
        texture(texture&& other) = default;

        static texture createEmpty() {
            return texture(nullptr);
        }

        static texture createFromMemory(const emscripten::val &data) {
            std::vector<uint8_t> bytes{};
            bytes.resize(data["byteLength"].as<size_t>());
            // Yes, this code IS copying the data. Sigh! According to Alon
            // Zakai:
            //     "There isn't a way to let compiled code access a new
            //     ArrayBuffer. The compiled code has hardcoded access to the
            //     wasm Memory it was instantiated with - all the pointers it
            //     can understand are indexes into that Memory. It can't refer
            //     to anything else, I'm afraid."
            //
            //     "In the future using different address spaces or techniques
            //     with reference types may open up some possibilities here."
            emscripten::val memory = emscripten::val::module_property("HEAP8")["buffer"];
            emscripten::val memoryView = data["constructor"].new_(memory, reinterpret_cast<uintptr_t>(bytes.data()), data["length"].as<uint32_t>());
            memoryView.call<void>("set", data);

            // Load the image data now, otherwise we'd have to  copy it from
            // JS into a buffer only for it to be copied from that buffer into
            // the texture later.
            ktxTexture* ptr = nullptr;
            KTX_error_code result = ktxTexture_CreateFromMemory(
                                        bytes.data(),
                                        bytes.size(),
                                        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
                                        &ptr);
            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to create from memory: " << ktxErrorString(result) << std::endl;
                return texture(nullptr);
            }

            return texture(ptr);
        }

        static texture create(const CreateInfo &createInfo) {
            ktxTexture2 *ptr = nullptr;

            ktxTextureCreateInfo ktxCreateInfo;
            ktxCreateInfo.vkFormat = createInfo.vkFormat;
            ktxCreateInfo.baseWidth = createInfo.baseWidth;
            ktxCreateInfo.baseHeight = createInfo.baseHeight;
            ktxCreateInfo.baseDepth = createInfo.baseDepth;
            ktxCreateInfo.numDimensions = createInfo.numDimensions;
            ktxCreateInfo.numLevels = createInfo.numLevels;
            ktxCreateInfo.numLayers = createInfo.numLayers;
            ktxCreateInfo.numFaces = createInfo.numFaces;
            ktxCreateInfo.isArray = createInfo.isArray;
            ktxCreateInfo.generateMipmaps = createInfo.generateMipmaps;
            
            KTX_error_code result = ktxTexture2_Create(&ktxCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &ptr);
            if (result!= KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to create texture: " << ktxErrorString(result) << std::endl;
                return texture(nullptr);
            }

            return texture(ktxTexture(ptr));
        }

        ktx_error_code_e setImageFromMemory(const emscripten::val &data, const uint32_t level, const uint32_t layer, const uint32_t face) {
            std::vector<uint8_t> bytes{};
            bytes.resize(data["byteLength"].as<size_t>());
            emscripten::val memory = emscripten::val::module_property("HEAP8")["buffer"];
            emscripten::val memoryView = data["constructor"].new_(memory, reinterpret_cast<uintptr_t>(bytes.data()), data["length"].as<uint32_t>());
            memoryView.call<void>("set", data);

            KTX_error_code result = ktxTexture_SetImageFromMemory(ktxTexture(m_ptr.get()), level, layer, face, bytes.data(), bytes.size());

            if (result != KTX_SUCCESS) {
                std::cout << "ERROR: Could not setImageFromMemory: "
                          << ktxErrorString(result) << std::endl;
            }

            return result;
        }

        ktx_error_code_e hashListAddKVPair(const std::string &key, const std::string &value) {
            KTX_error_code result = ktxHashList_AddKVPair(&ktxTexture2(m_ptr.get())->kvDataHead, key.c_str(), value.length() + 1, value.c_str());

            if (result != KTX_SUCCESS) {
                std::cout << "ERROR: Could not add KV pair: "
                          << ktxErrorString(result) << std::endl;
            }

            return result;
        }

        ktx_error_code_e hashListsetKTXWriter(const std::string &value) {
            return this->hashListAddKVPair(KTX_WRITER_KEY, value);
        }

        // Returns a memory view that owns the memory. It must be freed from JS
        // with .delete()
        emscripten::val writeToMemory() {
            ktx_size_t size;
            ktx_uint8_t* bytes;
            KTX_error_code result = ktxTexture_WriteToMemory(ktxTexture(m_ptr.get()), &bytes, &size);

            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to write to memory: " << ktxErrorString(result) << std::endl;
                return emscripten::val::null();
            }

            return emscripten::val(emscripten::typed_memory_view(size, bytes));
        }



        uint32_t baseWidth() const
        {
            return m_ptr->baseWidth;
        }

        uint32_t baseHeight() const
        {
            return m_ptr->baseHeight;
        }

        bool needsTranscoding() const
        {
            return ktxTexture_NeedsTranscoding(m_ptr.get());
        }

        bool isPremultiplied() const
        {
            return (m_ptr->classId == ktxTexture2_c
                    && ktxTexture2_GetPremultipliedAlpha(ktxTexture2(m_ptr.get())));
        }

        // @copydoc ktxTexture2::GetNumComponents
        uint32_t numComponents() const
        {
            if (m_ptr->classId != ktxTexture2_c)
            {
                std::cout << "ERROR: numComponents is only supported for KTX2" << std::endl;
                return 0;
            }

            return ktxTexture2_GetNumComponents(ktxTexture2(m_ptr.get()));
        }

        enum ktxSupercmpScheme supercompressionScheme() const
        {
            if (m_ptr->classId == ktxTexture1_c)
                return KTX_SS_NONE;
            else
                return ktxTexture2(m_ptr.get())->supercompressionScheme;
        }

        uint32_t vkFormat() const
        {
            if (m_ptr->classId != ktxTexture2_c)
            {
                std::cout << "ERROR: vkFormat is only supported for KTX2" << std::endl;
                return 0;
            }

            return ktxTexture2(m_ptr.get())->vkFormat;
        }

        struct ktxOrientation orientation() const
        {
            return m_ptr->orientation;
        }

        ktx_error_code_e transcodeBasis(const val& targetFormat, const val& decodeFlags)
        {
            if (m_ptr->classId != ktxTexture2_c)
            {
                std::cout << "ERROR: transcodeBasis is only supported for KTX2" << std::endl;
                return KTX_INVALID_VALUE;
            }

            KTX_error_code result = ktxTexture2_TranscodeBasis(
                ktxTexture2(m_ptr.get()),
                targetFormat.as<ktx_texture_transcode_fmt_e>(),
                decodeFlags.as<ktx_transcode_flags>());

            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to transcode: " << ktxErrorString(result) << std::endl;
            }
            return result;
        }

        ktx_error_code_e compressBasisEx(const BasisParams& basisParams)
        {
            if (m_ptr->classId != ktxTexture2_c)
            {
                std::cout << "ERROR: compressBasisEx is only supported for KTX2" << std::endl;
                return KTX_INVALID_VALUE;
            }

            if (ktxTexture2_NeedsTranscoding(ktxTexture2(m_ptr.get()))) {
                std::cout << "ERROR: Texture needs transcoding" << std::endl;
                return KTX_INVALID_OPERATION;
            }

            ktxBasisParams params{};
            params.structSize = sizeof(params);
            params.uastc = basisParams.uastc;
            params.threadCount = basisParams.threadCount;
            params.compressionLevel = basisParams.compressionLevel;
            params.qualityLevel = basisParams.qualityLevel;
            params.maxEndpoints = basisParams.maxEndpoints;
            params.endpointRDOThreshold = basisParams.endpointRDOThreshold;
            params.maxSelectors = basisParams.maxSelectors;
            params.selectorRDOThreshold = basisParams.selectorRDOThreshold;
            params.inputSwizzle[0] = static_cast<char>(basisParams.inputSwizzle[0]);
            params.inputSwizzle[1] = static_cast<char>(basisParams.inputSwizzle[1]);
            params.inputSwizzle[2] = static_cast<char>(basisParams.inputSwizzle[2]);
            params.inputSwizzle[3] = static_cast<char>(basisParams.inputSwizzle[3]);
            params.normalMap = basisParams.normalMap;
            params.preSwizzle = basisParams.preSwizzle;
            params.noEndpointRDO = basisParams.noEndpointRDO;
            params.noSelectorRDO = basisParams.noSelectorRDO;
            params.uastcFlags = basisParams.uastcFlags;
            params.uastcRDOQualityScalar = basisParams.uastcRDOQualityScalar;
            params.uastcRDODictSize = basisParams.uastcRDODictSize;
            params.uastcRDOMaxSmoothBlockErrorScale =
                basisParams.uastcRDOMaxSmoothBlockErrorScale;
            params.uastcRDOMaxSmoothBlockStdDev =
                basisParams.uastcRDOMaxSmoothBlockStdDev;
            params.uastcRDODontFavorSimplerModes =
                basisParams.uastcRDODontFavorSimplerModes;
            params.uastcRDONoMultithreading =
                basisParams.uastcRDONoMultithreading;

            
            // Set other BasisLZ/ETC1S or UASTC params to change default quality settings.
            KTX_error_code result = ktxTexture2_CompressBasisEx(ktxTexture2(m_ptr.get()), &params);

            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to compress: " << ktxErrorString(result) << std::endl;
            }
            return result;
        }

        ktx_error_code_e compressBasis(const unsigned int quality)
        {
            if (m_ptr->classId != ktxTexture2_c)
            {
                std::cout << "ERROR: compressBasis is only supported for KTX2" << std::endl;
                return KTX_INVALID_VALUE;
            }

            if (ktxTexture2_NeedsTranscoding(ktxTexture2(m_ptr.get()))) {
                std::cout << "ERROR: Texture needs transcoding" << std::endl;
                return KTX_INVALID_OPERATION;
            }
            
            // Set other BasisLZ/ETC1S or UASTC params to change default quality settings.
            KTX_error_code result = ktxTexture2_CompressBasis(ktxTexture2(m_ptr.get()), quality);

            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to compress: " << ktxErrorString(result) << std::endl;
            }
            return result;
        }

        ktx_error_code_e deflateZstd(const unsigned int compressionLevel) {
            if (m_ptr->classId != ktxTexture2_c)
            {
                std::cout << "ERROR: deflateZstd is only supported for KTX2" << std::endl;
                return KTX_INVALID_VALUE;
            }

            KTX_error_code result = ktxTexture2_DeflateZstd(
                ktxTexture2(m_ptr.get()), compressionLevel);

            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to deflate: " << ktxErrorString(result) << std::endl;
            }
            return result;
        }

        // NOTE: WebGLTexture objects are completely opaque so the option of passing in the texture
        // to use is not viable. Unknown at present is how to find the WebGLTexture for the texture
        // created by ktxTexture_GLUpload via the Emscripten OpenGL ES emulation.
        emscripten::val glUpload()
        {
            GLuint texname = 0;
            GLenum target = 0;
            GLenum error = 0;
            KTX_error_code result = ktxTexture_GLUpload(m_ptr.get(), &texname, &target, &error);
            if (result != KTX_SUCCESS)
            {
                std::cout << "ERROR: Failed to GL upload: " << ktxErrorString(result) << std::endl;
            }

            val ret = val::object();
            // Find the WebGLTexture for texture.
            val texture = val::module_property("GL")["textures"][texname];
            ret.set("texture", texture);
            ret.set("target", target);
            ret.set("error", error);
            return ret;
        }

    private:
        texture(ktxTexture* ptr)
            : m_ptr{ ptr, &destroy }
        {
        }

        static void destroy(ktxTexture* ptr)
        {
            ktxTexture_Destroy(ptr);
        }

        std::unique_ptr<ktxTexture, decltype(&destroy)> m_ptr;
    };
}

/** @mainpage

 Javascript bindings are provided to:

 @li @subpage libktx_js "libktx (in libktx.js)"
 @li @subpage msc_basis_transcoder "Basis Universal Transcoder (in msc_basis_transcoder.js)


 ---
 @par This page last modified $Date$
*/

/** @page libktx_js libktx Binding

## WebIDL for the binding

@code{.unparsed}
interface ktxOrientation {
    readonly attribute OrientationX x;
    readonly attribute OrientationY y;
    readonly attribute OrientationZ z;
};

interface ktxUploadResult {
    readonly attribute WebGLTexture texture;
    readonly attribute GLenum target;
    readonly attribute GLenum error;
};

interface ktxTexture {
    void ktxTexture(ArrayBufferView fileData);
    UploadResult glUpload();
    ErrorCode transcodeBasis();

    readonly attribute long baseWidth;
    readonly attribute long baseHeight;
    readonly attribute bool isPremultiplied;
    readonly attribute bool needsTranscoding;
    readonly attribute long numComponents;
    readonly attribute long vkFormat;
    readonly attribute SupercmpScheme supercompressionScheme;
    readonly attribute ktxOrientation orientation;
};

enum ErrorCode = {
    "SUCCESS",
    "FILE_DATA_ERROR",
    "FILE_ISPIPE",
    "FILE_OPEN_FAILED",
    "FILE_OVERFLOW",
    "FILE_READ_ERROR",
    "FILE_SEEK_ERROR",
    "FILE_UNEXPECTED_ERROR",
    "FILE_WRITE_ERROR",
    "GL_ERROR",
    "INVALID_OPERATION",
    "INVALID_VALUE",
    "NOT_FOUND",
    "OUT_OF_MEMORY",
    "TRANSCODE_FAILED",
    "UNKNOWN_FILE_FORMAT",
    "UNSUPPORTED_TEXTURE_TYPE",
    "UNSUPPORTED_FEATURE",
    "LIBRARY_NOT_LINKED"
};

// Some targets may not be available depending on options used when compiling
// the web assembly. ktxTexture.transcodeBasis will report this.
enum TranscodeTarget = {
    "ETC1_RGB",
    "BC1_RGB",
    "BC4_R",
    "BC5_RG",
    "BC3_RGBA",
    "BC1_OR_3",
    "PVRTC1_4_RGB",
    "PVRTC1_4_RGBA",
    "BC7_RGBA",
    "BC7_M6_RGB",  // Deprecated. Use BC7_RGBA.
    "BC7_M5_RGBA", // Deprecated. Use BC7_RGBA.
    "ETC2_RGBA",
    "ASTC_4x4_RGBA",
    "RGBA32",
    "RGB565",
    "BGR565",
    "RGBA4444",
    "PVRTC2_4_RGB",
    "PVRTC2_4_RGBA",
    "ETC",
    "EAC_R11",
    "EAC_RG11"
};

enum TranscodeFlagBits {
   "TRANSCODE_ALPHA_DATA_TO_OPAQUE_FORMATS"
};

enum OrientationX {
    "LEFT",
    "RIGHT"
};
enum OrientationY {
    "UP",
    "DOWN"
};
enum OrientationZ {
    "IN",
    "OUT"
};

enum SupercmpScheme {
    "NONE",
    "BASIS_LZ",
    "ZSTD"
    "ZLIB"
};

@endcode

## How to use

Put libktx.js and libktx.wasm in a directory on your server. Create a script
tag with libktx.js as the @c src as shown below, changing the path as necessary
for the relative locations of your .html file and the script source. libktx.js
will automatically load msc_basis_transcoder.wasm.

### Create an instance of the LIBKTX module

Add this to the .html file to initialize the libktx module and make it available on the main window.
@code{.unparsed}
    &lt;script src="libktx.js">&lt;/script>
    &lt;script type="text/javascript">
      LIBKTX({preinitializedWebGLContext: gl}).then(module => {
        window.LIBKTX = module;
        // Make existing WebGL context current for Emscripten OpenGL.
        LIBKTX.GL.makeContextCurrent(LIBKTX.GL.createContext(null, { majorVersion: 2.0 }));
        texture = loadTexture(gl, 'ktx_app.ktx2');
      });
    &lt;/script>
@endcode

@e After the module is initialized, invoke code that will directly or indirectly cause
a function like the following to be executed.

### loadTexture function

@code{.unparsed}
    // Set up an XHR to fetch the url into an ArrayBuffer.
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url);
    xhr.responseType = "arrayBuffer";
    xhr.onload = function() {
      const { ktxTexture, TranscodeTarget, OrientationX, OrientationY } = LIBKTX;
      // this.response is a generic binary buffer which
      // we can interpret as Uint8Array.
      var ktxdata = new Uint8Array(this.response);
      ktexture = new ktxTexture(ktxdata);

      if (ktexture.needsTranscoding) {
        var formatString;
        var format;
        if (astcSupported) {
          formatString = 'ASTC';
          format = TranscodeTarget.ASTC_4x4_RGBA;
        } else if (dxtSupported) {
          formatString = 'BC1 or BC3';
          format = TranscodeTarget.BC1_OR_3;
        } else if (pvrtcSupported) {
          formatString = 'PVRTC1';
          format = TranscodeTarget.PVRTC1_4_RGBA;
        } else if (etcSupported) {
          formatString = 'ETC';
          format = TranscodeTarget.ETC;
        } else {
          formatString = 'RGBA4444';
          format = TranscodeTarget.RGBA4444;
        }
        if (ktexture.transcodeBasis(format, 0) != LIBKTX.ErrorCode.SUCCESS) {
          alert('Texture transcode failed. See console for details.');
          return undefined;
        }
      }

      // If there is no global variable "texture".
      //const {texture, target, error} = ktexture.glUpload();
      // If there is a globla variable "texture"
      const result = ktexture.glUpload();
      const {target, error} = result;
      texture = result.texture;

      if (error != gl.NO_ERROR) {
        alert('WebGL error when uploading texture, code = ' + error.toString(16));
        return undefined;
      }
      if (texture === undefined) {
        alert('Texture upload failed. See console for details.');
        return undefined;
      }
      if (target != gl.TEXTURE_2D) {
        alert('Loaded texture is not a TEXTURE2D.');
        return undefined;
      }

      gl.bindTexture(target, texture);
      // If using a placeholder texture during loading, delete
      // it now.
      //gl.deleteTexture(placeholder);

      if (ktexture.numLevels > 1 || ktexture.generateMipmaps)
         // Enable bilinear mipmapping.
         gl.texParameteri(target,
                          gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
      else
        gl.texParameteri(target, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
      gl.texParameteri(target, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

      if (ktexture.orientation.x == OrientationX.LEFT) {
        // Adjust u coords, e.g. by setting up a uv transform
      }
      if (ktexture.orientation.y == OrientationY.DOWN) {
        // Adjust v coords, e.g. by setting up a uv transform
      }
      ktexture.delete();
    }

    xhr.send();
@endcode

@note It is not clear if glUpload can be used with, e.g. THREE.js. It may
be necessary to expose the ktxTexture_IterateLevelFaces or
ktxTexture_IterateLoadLevelFaces API to JS with those calling a
callback in JS to upload each image to WebGL.


*/

EMSCRIPTEN_BINDINGS(ktx) {
    enum_<ktx_error_code_e>("ErrorCode")
        .value("SUCCESS", KTX_SUCCESS)
        .value("FILE_DATA_ERROR", KTX_FILE_DATA_ERROR)
        .value("FILE_ISPIPE", KTX_FILE_ISPIPE)
        .value("FILE_OPEN_FAILED", KTX_FILE_OPEN_FAILED)
        .value("FILE_OVERFLOW", KTX_FILE_OVERFLOW)
        .value("FILE_READ_ERROR", KTX_FILE_READ_ERROR)
        .value("FILE_SEEK_ERROR", KTX_FILE_SEEK_ERROR)
        .value("FILE_UNEXPECTED_ERROR", KTX_FILE_UNEXPECTED_EOF)
        .value("FILE_WRITE_ERROR", KTX_FILE_WRITE_ERROR)
        .value("GL_ERROR", KTX_GL_ERROR)
        .value("INVALID_OPERATION", KTX_INVALID_OPERATION)
        .value("INVALID_VALUE", KTX_INVALID_VALUE)
        .value("NOT_FOUND", KTX_NOT_FOUND)
        .value("OUT_OF_MEMORY", KTX_OUT_OF_MEMORY)
        .value("TRANSCODE_FAILED", KTX_TRANSCODE_FAILED)
        .value("UNKNOWN_FILE_FORMAT", KTX_UNKNOWN_FILE_FORMAT)
        .value("UNSUPPORTED_TEXTURE_TYPE", KTX_UNSUPPORTED_TEXTURE_TYPE)
        .value("UNSUPPORTED_FEATURE", KTX_UNSUPPORTED_FEATURE)
        .value("LIBRARY_NOT_LINKED", KTX_LIBRARY_NOT_LINKED)
        .value("DECOMPRESS_LENGTH_ERROR", KTX_DECOMPRESS_LENGTH_ERROR)
        .value("DECOMPRESS_CHECKSUM_ERROR", KTX_DECOMPRESS_CHECKSUM_ERROR)
        ;

    enum_<ktx_texture_transcode_fmt_e>("TranscodeTarget")
        .value("ETC1_RGB", KTX_TTF_ETC1_RGB)
        .value("BC1_RGB", KTX_TTF_BC1_RGB)
        .value("BC4_R", KTX_TTF_BC4_R)
        .value("BC5_RG", KTX_TTF_BC5_RG)
        .value("BC3_RGBA", KTX_TTF_BC3_RGBA)
        .value("BC1_OR_3", KTX_TTF_BC1_OR_3)
        .value("PVRTC1_4_RGB", KTX_TTF_PVRTC1_4_RGB)
        .value("PVRTC1_4_RGBA", KTX_TTF_PVRTC1_4_RGBA)
        .value("BC7_RGBA", KTX_TTF_BC7_RGBA)
        .value("BC7_M6_RGB", KTX_TTF_BC7_M6_RGB)
        .value("BC7_M5_RGBA", KTX_TTF_BC7_M5_RGBA)
        .value("ETC2_RGBA", KTX_TTF_ETC2_RGBA)
        .value("ASTC_4x4_RGBA", KTX_TTF_ASTC_4x4_RGBA)
        .value("RGBA32", KTX_TTF_RGBA32)
        .value("RGB565", KTX_TTF_RGB565)
        .value("BGR565", KTX_TTF_BGR565)
        .value("RGBA4444", KTX_TTF_RGBA4444)
        .value("PVRTC2_4_RGB", KTX_TTF_PVRTC2_4_RGB)
        .value("PVRTC2_4_RGBA", KTX_TTF_PVRTC2_4_RGBA)
        .value("ETC", KTX_TTF_ETC)
        .value("EAC_R11", KTX_TTF_ETC2_EAC_R11)
        .value("EAC_RG11", KTX_TTF_ETC2_EAC_RG11)
    ;
    enum_<ktx_transcode_flag_bits_e>("TranscodeFlagBits")
        .value("TRANSCODE_ALPHA_DATA_TO_OPAQUE_FORMATS",
               KTX_TF_TRANSCODE_ALPHA_DATA_TO_OPAQUE_FORMATS)
    ;
    enum_<ktxOrientationX>("OrientationX")
        .value("LEFT", KTX_ORIENT_X_LEFT)
        .value("RIGHT", KTX_ORIENT_X_RIGHT)
    ;
    enum_<ktxOrientationY>("OrientationY")
        .value("UP", KTX_ORIENT_Y_UP)
        .value("DOWN", KTX_ORIENT_Y_DOWN)
    ;
    enum_<ktxOrientationZ>("OrientationZ")
        .value("IN", KTX_ORIENT_Z_IN)
        .value("OUT", KTX_ORIENT_Z_OUT)
    ;
    enum_<ktxSupercmpScheme>("SupercmpScheme")
        .value("NONE", KTX_SS_NONE)
        .value("BASIS_LZ", KTX_SS_BASIS_LZ)
        .value("ZSTD", KTX_SS_ZSTD)
        .value("ZLIB", KTX_SS_ZLIB)
    ;

    value_object<ktxOrientation>("Orientation")
        .field("x", &ktxOrientation::x)
        .field("y", &ktxOrientation::y)
        .field("z", &ktxOrientation::z);

    enum_<VkFormat>("VkFormat")
        .value("UNDEFINED", VK_FORMAT_UNDEFINED)
        .value("R4G4_UNORM_PACK8", VK_FORMAT_R4G4_UNORM_PACK8)
        .value("R4G4B4A4_UNORM_PACK16", VK_FORMAT_R4G4B4A4_UNORM_PACK16)
        .value("B4G4R4A4_UNORM_PACK16", VK_FORMAT_B4G4R4A4_UNORM_PACK16)
        .value("R5G6B5_UNORM_PACK16", VK_FORMAT_R5G6B5_UNORM_PACK16)
        .value("B5G6R5_UNORM_PACK16", VK_FORMAT_B5G6R5_UNORM_PACK16)
        .value("R5G5B5A1_UNORM_PACK16", VK_FORMAT_R5G5B5A1_UNORM_PACK16)
        .value("B5G5R5A1_UNORM_PACK16", VK_FORMAT_B5G5R5A1_UNORM_PACK16)
        .value("A1R5G5B5_UNORM_PACK16", VK_FORMAT_A1R5G5B5_UNORM_PACK16)
        .value("R8_UNORM", VK_FORMAT_R8_UNORM)
        .value("R8_SNORM", VK_FORMAT_R8_SNORM)
        .value("R8_USCALED", VK_FORMAT_R8_USCALED)
        .value("R8_SSCALED", VK_FORMAT_R8_SSCALED)
        .value("R8_UINT", VK_FORMAT_R8_UINT)
        .value("R8_SINT", VK_FORMAT_R8_SINT)
        .value("R8_SRGB", VK_FORMAT_R8_SRGB)
        .value("R8G8_UNORM", VK_FORMAT_R8G8_UNORM)
        .value("R8G8_SNORM", VK_FORMAT_R8G8_SNORM)
        .value("R8G8_USCALED", VK_FORMAT_R8G8_USCALED)
        .value("R8G8_SSCALED", VK_FORMAT_R8G8_SSCALED)
        .value("R8G8_UINT", VK_FORMAT_R8G8_UINT)
        .value("R8G8_SINT", VK_FORMAT_R8G8_SINT)
        .value("R8G8_SRGB", VK_FORMAT_R8G8_SRGB)
        .value("R8G8B8_UNORM", VK_FORMAT_R8G8B8_UNORM)
        .value("R8G8B8_SNORM", VK_FORMAT_R8G8B8_SNORM)
        .value("R8G8B8_USCALED", VK_FORMAT_R8G8B8_USCALED)
        .value("R8G8B8_SSCALED", VK_FORMAT_R8G8B8_SSCALED)
        .value("R8G8B8_UINT", VK_FORMAT_R8G8B8_UINT)
        .value("R8G8B8_SINT", VK_FORMAT_R8G8B8_SINT)
        .value("R8G8B8_SRGB", VK_FORMAT_R8G8B8_SRGB)
        .value("B8G8R8_UNORM", VK_FORMAT_B8G8R8_UNORM)
        .value("B8G8R8_SNORM", VK_FORMAT_B8G8R8_SNORM)
        .value("B8G8R8_USCALED", VK_FORMAT_B8G8R8_USCALED)
        .value("B8G8R8_SSCALED", VK_FORMAT_B8G8R8_SSCALED)
        .value("B8G8R8_UINT", VK_FORMAT_B8G8R8_UINT)
        .value("B8G8R8_SINT", VK_FORMAT_B8G8R8_SINT)
        .value("B8G8R8_SRGB", VK_FORMAT_B8G8R8_SRGB)
        .value("R8G8B8A8_UNORM", VK_FORMAT_R8G8B8A8_UNORM)
        .value("R8G8B8A8_SNORM", VK_FORMAT_R8G8B8A8_SNORM)
        .value("R8G8B8A8_USCALED", VK_FORMAT_R8G8B8A8_USCALED)
        .value("R8G8B8A8_SSCALED", VK_FORMAT_R8G8B8A8_SSCALED)
        .value("R8G8B8A8_UINT", VK_FORMAT_R8G8B8A8_UINT)
        .value("R8G8B8A8_SINT", VK_FORMAT_R8G8B8A8_SINT)
        .value("R8G8B8A8_SRGB", VK_FORMAT_R8G8B8A8_SRGB)
        .value("B8G8R8A8_UNORM", VK_FORMAT_B8G8R8A8_UNORM)
        .value("B8G8R8A8_SNORM", VK_FORMAT_B8G8R8A8_SNORM)
        .value("B8G8R8A8_USCALED", VK_FORMAT_B8G8R8A8_USCALED)
        .value("B8G8R8A8_SSCALED", VK_FORMAT_B8G8R8A8_SSCALED)
        .value("B8G8R8A8_UINT", VK_FORMAT_B8G8R8A8_UINT)
        .value("B8G8R8A8_SINT", VK_FORMAT_B8G8R8A8_SINT)
        .value("B8G8R8A8_SRGB", VK_FORMAT_B8G8R8A8_SRGB)
        .value("A8B8G8R8_UNORM_PACK32", VK_FORMAT_A8B8G8R8_UNORM_PACK32)
        .value("A8B8G8R8_SNORM_PACK32", VK_FORMAT_A8B8G8R8_SNORM_PACK32)
        .value("A8B8G8R8_USCALED_PACK32", VK_FORMAT_A8B8G8R8_USCALED_PACK32)
        .value("A8B8G8R8_SSCALED_PACK32", VK_FORMAT_A8B8G8R8_SSCALED_PACK32)
        .value("A8B8G8R8_UINT_PACK32", VK_FORMAT_A8B8G8R8_UINT_PACK32)
        .value("A8B8G8R8_SINT_PACK32", VK_FORMAT_A8B8G8R8_SINT_PACK32)
        .value("A8B8G8R8_SRGB_PACK32", VK_FORMAT_A8B8G8R8_SRGB_PACK32)
        .value("A2R10G10B10_UNORM_PACK32", VK_FORMAT_A2R10G10B10_UNORM_PACK32)
        .value("A2R10G10B10_SNORM_PACK32", VK_FORMAT_A2R10G10B10_SNORM_PACK32)
        .value("A2R10G10B10_USCALED_PACK32",
               VK_FORMAT_A2R10G10B10_USCALED_PACK32)
        .value("A2R10G10B10_SSCALED_PACK32",
               VK_FORMAT_A2R10G10B10_SSCALED_PACK32)
        .value("A2R10G10B10_UINT_PACK32", VK_FORMAT_A2R10G10B10_UINT_PACK32)
        .value("A2R10G10B10_SINT_PACK32", VK_FORMAT_A2R10G10B10_SINT_PACK32)
        .value("A2B10G10R10_UNORM_PACK32", VK_FORMAT_A2B10G10R10_UNORM_PACK32)
        .value("A2B10G10R10_SNORM_PACK32", VK_FORMAT_A2B10G10R10_SNORM_PACK32)
        .value("A2B10G10R10_USCALED_PACK32",
               VK_FORMAT_A2B10G10R10_USCALED_PACK32)
        .value("A2B10G10R10_SSCALED_PACK32",
               VK_FORMAT_A2B10G10R10_SSCALED_PACK32)
        .value("A2B10G10R10_UINT_PACK32", VK_FORMAT_A2B10G10R10_UINT_PACK32)
        .value("A2B10G10R10_SINT_PACK32", VK_FORMAT_A2B10G10R10_SINT_PACK32)
        .value("R16_UNORM", VK_FORMAT_R16_UNORM)
        .value("R16_SNORM", VK_FORMAT_R16_SNORM)
        .value("R16_USCALED", VK_FORMAT_R16_USCALED)
        .value("R16_SSCALED", VK_FORMAT_R16_SSCALED)
        .value("R16_UINT", VK_FORMAT_R16_UINT)
        .value("R16_SINT", VK_FORMAT_R16_SINT)
        .value("R16_SFLOAT", VK_FORMAT_R16_SFLOAT)
        .value("R16G16_UNORM", VK_FORMAT_R16G16_UNORM)
        .value("R16G16_SNORM", VK_FORMAT_R16G16_SNORM)
        .value("R16G16_USCALED", VK_FORMAT_R16G16_USCALED)
        .value("R16G16_SSCALED", VK_FORMAT_R16G16_SSCALED)
        .value("R16G16_UINT", VK_FORMAT_R16G16_UINT)
        .value("R16G16_SINT", VK_FORMAT_R16G16_SINT)
        .value("R16G16_SFLOAT", VK_FORMAT_R16G16_SFLOAT)
        .value("R16G16B16_UNORM", VK_FORMAT_R16G16B16_UNORM)
        .value("R16G16B16_SNORM", VK_FORMAT_R16G16B16_SNORM)
        .value("R16G16B16_USCALED", VK_FORMAT_R16G16B16_USCALED)
        .value("R16G16B16_SSCALED", VK_FORMAT_R16G16B16_SSCALED)
        .value("R16G16B16_UINT", VK_FORMAT_R16G16B16_UINT)
        .value("R16G16B16_SINT", VK_FORMAT_R16G16B16_SINT)
        .value("R16G16B16_SFLOAT", VK_FORMAT_R16G16B16_SFLOAT)
        .value("R16G16B16A16_UNORM", VK_FORMAT_R16G16B16A16_UNORM)
        .value("R16G16B16A16_SNORM", VK_FORMAT_R16G16B16A16_SNORM)
        .value("R16G16B16A16_USCALED", VK_FORMAT_R16G16B16A16_USCALED)
        .value("R16G16B16A16_SSCALED", VK_FORMAT_R16G16B16A16_SSCALED)
        .value("R16G16B16A16_UINT", VK_FORMAT_R16G16B16A16_UINT)
        .value("R16G16B16A16_SINT", VK_FORMAT_R16G16B16A16_SINT)
        .value("R16G16B16A16_SFLOAT", VK_FORMAT_R16G16B16A16_SFLOAT)
        .value("R32_UINT", VK_FORMAT_R32_UINT)
        .value("R32_SINT", VK_FORMAT_R32_SINT)
        .value("R32_SFLOAT", VK_FORMAT_R32_SFLOAT)
        .value("R32G32_UINT", VK_FORMAT_R32G32_UINT)
        .value("R32G32_SINT", VK_FORMAT_R32G32_SINT)
        .value("R32G32_SFLOAT", VK_FORMAT_R32G32_SFLOAT)
        .value("R32G32B32_UINT", VK_FORMAT_R32G32B32_UINT)
        .value("R32G32B32_SINT", VK_FORMAT_R32G32B32_SINT)
        .value("R32G32B32_SFLOAT", VK_FORMAT_R32G32B32_SFLOAT)
        .value("R32G32B32A32_UINT", VK_FORMAT_R32G32B32A32_UINT)
        .value("R32G32B32A32_SINT", VK_FORMAT_R32G32B32A32_SINT)
        .value("R32G32B32A32_SFLOAT", VK_FORMAT_R32G32B32A32_SFLOAT)
        .value("R64_UINT", VK_FORMAT_R64_UINT)
        .value("R64_SINT", VK_FORMAT_R64_SINT)
        .value("R64_SFLOAT", VK_FORMAT_R64_SFLOAT)
        .value("R64G64_UINT", VK_FORMAT_R64G64_UINT)
        .value("R64G64_SINT", VK_FORMAT_R64G64_SINT)
        .value("R64G64_SFLOAT", VK_FORMAT_R64G64_SFLOAT)
        .value("R64G64B64_UINT", VK_FORMAT_R64G64B64_UINT)
        .value("R64G64B64_SINT", VK_FORMAT_R64G64B64_SINT)
        .value("R64G64B64_SFLOAT", VK_FORMAT_R64G64B64_SFLOAT)
        .value("R64G64B64A64_UINT", VK_FORMAT_R64G64B64A64_UINT)
        .value("R64G64B64A64_SINT", VK_FORMAT_R64G64B64A64_SINT)
        .value("R64G64B64A64_SFLOAT", VK_FORMAT_R64G64B64A64_SFLOAT);


    enum_<ktx_pack_uastc_flag_bits_e>("PackUastcFlagBits")
        .value("KTX_PACK_UASTC_LEVEL_FASTEST", KTX_PACK_UASTC_LEVEL_FASTEST)
        .value("KTX_PACK_UASTC_LEVEL_FASTER", KTX_PACK_UASTC_LEVEL_FASTER)
        .value("KTX_PACK_UASTC_LEVEL_DEFAULT", KTX_PACK_UASTC_LEVEL_DEFAULT)
        .value("KTX_PACK_UASTC_LEVEL_SLOWER", KTX_PACK_UASTC_LEVEL_SLOWER)
        .value("KTX_PACK_UASTC_LEVEL_VERYSLOW", KTX_PACK_UASTC_LEVEL_VERYSLOW)
        .value("KTX_PACK_UASTC_MAX_LEVEL", KTX_PACK_UASTC_MAX_LEVEL)
        .value("KTX_PACK_UASTC_LEVEL_MASK", KTX_PACK_UASTC_LEVEL_MASK)
        .value("KTX_PACK_UASTC_FAVOR_UASTC_ERROR",
               KTX_PACK_UASTC_FAVOR_UASTC_ERROR)
        .value("KTX_PACK_UASTC_FAVOR_BC7_ERROR", KTX_PACK_UASTC_FAVOR_BC7_ERROR)
        .value("KTX_PACK_UASTC_ETC1_FASTER_HINTS",
               KTX_PACK_UASTC_ETC1_FASTER_HINTS)
        .value("KTX_PACK_UASTC_ETC1_FASTEST_HINTS",
               KTX_PACK_UASTC_ETC1_FASTEST_HINTS)
        .value("KTX_PACK_UASTC__ETC1_DISABLE_FLIP_AND_INDIVIDUAL", KTX_PACK_UASTC__ETC1_DISABLE_FLIP_AND_INDIVIDUAL);

    enum_<ktx::SwizzleChannel>("SwizzleChannel")
        .value("R", ktx::SwizzleChannel::R)
        .value("G", ktx::SwizzleChannel::G)
        .value("B", ktx::SwizzleChannel::B)
        .value("A", ktx::SwizzleChannel::A)
        ;

    value_object<ktx::BasisParams>("BasisParams")
        .field("uastc", &ktx::BasisParams::uastc)
        .field("threadCount", &ktx::BasisParams::threadCount)
        .field("compressionLevel", &ktx::BasisParams::compressionLevel)
        .field("qualityLevel", &ktx::BasisParams::qualityLevel)
        .field("maxEndpoints", &ktx::BasisParams::maxEndpoints)
        .field("endpointRDOThreshold", &ktx::BasisParams::endpointRDOThreshold)
        .field("maxSelectors", &ktx::BasisParams::maxSelectors)
        .field("selectorRDOThreshold", &ktx::BasisParams::selectorRDOThreshold)
        .field("inputSwizzle", &ktx::BasisParams::inputSwizzle)
        .field("normalMap", &ktx::BasisParams::normalMap)
        .field("preSwizzle", &ktx::BasisParams::preSwizzle)
        .field("noEndpointRDO", &ktx::BasisParams::noEndpointRDO)
        .field("noSelectorRDO", &ktx::BasisParams::noSelectorRDO)
        .field("uastcFlags", &ktx::BasisParams::uastcFlags)
        .field("uastcRDOQualityScalar",
               &ktx::BasisParams::uastcRDOQualityScalar)
        .field("uastcRDODictSize", &ktx::BasisParams::uastcRDODictSize)
        .field("uastcRDOMaxSmoothBlockErrorScale",
               &ktx::BasisParams::uastcRDOMaxSmoothBlockErrorScale)
        .field("uastcRDOMaxSmoothBlockStdDev",
               &ktx::BasisParams::uastcRDOMaxSmoothBlockStdDev)
        .field("uastcRDODontFavorSimplerModes",
               &ktx::BasisParams::uastcRDODontFavorSimplerModes)
        .field("uastcRDONoMultithreading",
               &ktx::BasisParams::uastcRDONoMultithreading);

    value_object<ktx::CreateInfo>("CreateInfo")
        .field("vkFormat", &ktx::CreateInfo::vkFormat)
        .field("baseWidth", &ktx::CreateInfo::baseWidth)
        .field("baseHeight", &ktx::CreateInfo::baseHeight)
        .field("baseDepth", &ktx::CreateInfo::baseDepth)
        .field("numDimensions", &ktx::CreateInfo::numDimensions)
        .field("numLevels", &ktx::CreateInfo::numLevels)
        .field("numLayers", &ktx::CreateInfo::numLayers)
        .field("numFaces", &ktx::CreateInfo::numFaces)
        .field("isArray", &ktx::CreateInfo::isArray)
        .field("generateMipmaps", &ktx::CreateInfo::generateMipmaps);

    value_array<std::array<ktx::SwizzleChannel, 4>>("SwizzleChannelArrayRGBA")
        .element(emscripten::index<0>())
        .element(emscripten::index<1>())
        .element(emscripten::index<2>())
        .element(emscripten::index<3>())
        ;

    function("createFromMemory", &ktx::texture::createFromMemory);
    function("create", &ktx::texture::create);

    class_<ktx::texture>("Ktx2Texture")
        .constructor(&ktx::texture::create)

        //.class_function("createFromMemory", &ktx::texture::createFromMemory)
        // .property("data", &ktx::texture::getData)
        .property("baseWidth", &ktx::texture::baseWidth)
        .property("baseHeight", &ktx::texture::baseHeight)
        .property("isPremultiplied", &ktx::texture::isPremultiplied)
        .property("needsTranscoding", &ktx::texture::needsTranscoding)
        .property("numComponents", &ktx::texture::numComponents)
        .property("orientation", &ktx::texture::orientation)
        .property("supercompressScheme", &ktx::texture::supercompressionScheme)
        .property("vkFormat", &ktx::texture::vkFormat)
        .function("writeToMemory", &ktx::texture::writeToMemory)
        .function("setImageFromMemory", &ktx::texture::setImageFromMemory)
        .function("hashListAddKVPair", &ktx::texture::hashListAddKVPair)
        .function("hashListsetKTXWriter", &ktx::texture::hashListsetKTXWriter)
        .function("transcodeBasis", &ktx::texture::transcodeBasis)
        .function("deflateZstd", &ktx::texture::deflateZstd)
        .function("compressBasisEx", &ktx::texture::compressBasisEx)
        .function("compressBasis", &ktx::texture::compressBasis)
        .function("glUpload", &ktx::texture::glUpload)
    ;
}

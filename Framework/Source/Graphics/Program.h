/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#pragma once
#include "Framework.h"
#include <string>
#include <map>
#include <vector>
#include "Core/ProgramVersion.h"
#include "Core/UniformBuffer.h"

namespace Falcor
{
    class Texture;
    class Sampler;
    class Shader;
    class RenderContext;

    /** High-level abstraction of a program class.
        This class manages different versions of the same program. Different versions means same shader files, different macro definitions. This allows simple usage in case different macros are required - for example static vs. animated models.
    */
    class Program : public std::enable_shared_from_this<Program>
    {
    public:
        class SharedPtr : public std::shared_ptr<Program>
        {
        public:
            SharedPtr() : std::shared_ptr<Program>() {}
            SharedPtr(Program* pProg) : std::shared_ptr<Program>(pProg) {}
            UniformBuffer::SharedPtr operator[](const std::string& uboName) { return get()->getUniformBuffer(uboName); }
        };

        using SharedConstPtr = std::shared_ptr<const Program>;

        class DefineList : public std::map<std::string, std::string>
        {
        public:
            void add(const std::string& name, const std::string& val = "") { (*this)[name] = val; }
            void remove(const std::string& name) {(*this).erase(name); }
        };

        /** create a new program object.
            \param[in] vertexFile Vertex shader filename. If this string is empty (""), it will use a default vertex shader which transforms and outputs all the vertex attributes.
            \param[in] fragmentFile Fragment shader filename.
            \param[in] programDefines A list of macro definitions to set into the shaders. The macro definitions will be assigned to all the shaders.
            \return A new object, or nullptr if creation failed.

            Note that this call merely creates a program object. The actual compilation and link happens when calling Program#getActiveProgramVersion().
        */
        static SharedPtr createFromFile(const std::string& vertexFile, const std::string& fragmentFile, const DefineList& programDefines = DefineList());

        /** create a new program object.
        \param[in] vertexFile Vertex shader string.
        \param[in] fragmentFile Fragment shader string.
        \param[in] programDefines A list of macro definitions to set into the shaders. The macro definitions will be assigned to all the shaders.

        \return A new object, or nullptr if creation failed.
        Note that this call merely creates a program object. The actual compilation and link happens when calling Program#getActiveProgramVersion().
        */
        static SharedPtr createFromString(const std::string& vertexShader, const std::string& fragmentShader, const DefineList& programDefines = DefineList());

        /** create a new program object.
            \param[in] vertexFile Vertex shader filename. If this string is empty (""), it will use a default vertex shader which transforms and outputs all the vertex attributes.
            \param[in] fragmentFile Fragment shader filename.
            \param[in] geometryFile Geometry shader filename.
            \param[in] hullFile Hull shader filename.
            \param[in] domainFile Domain shader filename.
            \param[in] programDefines A list of macro definitions to set into the shaders.

            \return A new object, or nullptr if creation failed.

            Note that this call merely creates a program object. The actual compilation and link happens when calling Program#getActiveProgramVersion().
        */
        static SharedPtr createFromFile(const std::string& vertexFile, const std::string& fragmentFile, const std::string& geometryFile, const std::string& hullFile, const std::string& domainFile, const DefineList& programDefines = DefineList());

        /** create a new program object.
        \param[in] vertexShader Vertex shader string.
        \param[in] fragmentShader Fragment shader string.
        \param[in] geometryShader Geometry shader string.
        \param[in] hullShader Hull shader string.
        \param[in] domainShader Domain shader string.
        \param[in] programDefines A list of macro definitions to set into the shaders.
        \return A new object, or nullptr if creation failed.

        Note that this call merely creates a program object. The actual compilation and link happens when calling Program#getActiveProgramVersion().
        */
        static SharedPtr createFromString(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader, const std::string& hullShader, const std::string& domainShader, const DefineList& programDefines = DefineList());

        ~Program();
        /** Get a shader object associated with this program
            \param[in] Type The Type of the shader object to fetch.
            \return The requested shader object, or nullptr if the shader doesn't exist.
        */
        const Shader* getShader(ShaderType type) const;

        /** Get the API handle of the active program
        */
        ProgramVersion::SharedConstPtr getActiveProgramVersion() const;

        /** Adds a macro definition to the program. If the macro already exists, its will be replaced.

            \param[in] name The name of define. Must be valid
            \param[in] value Optional. The value of the define string
        */
        void addDefine(const std::string& name, const std::string& value = "");

        /** Remove a macro definition from the program. If the definition doesn't exist, the function call will be silently ignored.
            \param[in] name The name of define. Must be valid
        */
        void removeDefine(const std::string& name);

        /** Clear the macro definition list
        */
        void clearDefines() { mDefineList.clear(); }
    
        /** Get the location of an input attribute for the active program version. Note that different versions might return different locations.
            \param[in] Attribute The attribute name in the program
            \return The index of the attribute if it is found, otherwise CApiProgram#InvalidLocation
        */
        int32_t getAttributeLocation(const std::string& attribute) const;

        /** Get the location of a uniform buffer for the active program version. Note that different versions might return different locations.
            \param[in] Attribute The attribute name in the program
            \return The index of the attribute if it is found, otherwise CApiProgram#InvalidLocation
        */
        uint32_t getUniformBufferBinding(const std::string& name) const;

        /** Get the macro definition string of the active program version
        */
        const DefineList& getActiveDefinesList() const { return mDefineList; }

        std::string getActiveDefinesString() const;

        /** Reload and relink all programs.
        */
        static void reloadAllPrograms();

        /** Get a uniform-buffer object associated with this program. the function will return one of the following:
            - A new UniformBuffer object if no buffer was associated with bufName
            - An already existing buffer associated with bufName. The existing buffer might have been created using a previous getUniformBuffer() call or bindUniformBuffer() call
            - nullptr if bufName is not a name of a buffer declared in the program
            Note that setting a new define string will force the creation of a new UniformBuffer object
        */
        UniformBuffer::SharedPtr getUniformBuffer(const std::string& bufName);

        /** Binds a uniform buffer into the program. This is useful in cases when we want to share UBOs between programs.
            The first program will call getUniformBuffer() to generate a buffer, then bind it to other programs that we'd like to share it with.
            Note that there is not data layout check here. It's your responsibility to make sure that the layouts match,
        */
        void bindUniformBuffer(const std::string& bufName, UniformBuffer::SharedPtr& pUbo);

        /** Set all uniform-buffers into the render-context            
        */
        void setUniformBuffersIntoContext(RenderContext* pContext);
    private:
        static const uint32_t kShaderCount = (uint32_t)ShaderType::Count;

        Program();
        static SharedPtr createInternal(const std::string& vs, const std::string& fs, const std::string& gs, const std::string& hs, const std::string& ds, const DefineList& programDefines, bool createdFromFile);
        bool link() const;
        std::string mShaderStrings[kShaderCount]; // Either a filename or a string, depending on the value of mCreatedFromFile

        DefineList mDefineList;

        // We are doing lazy compilation, so these are mutable
        mutable bool mLinkRequired = true;
        mutable std::map<const DefineList, ProgramVersion::SharedConstPtr> mProgramVersions;
        mutable ProgramVersion::SharedConstPtr mpActiveProgram = nullptr;
        mutable std::map<const std::string, UniformBuffer::SharedPtr> mUboMap;

        std::string getProgramDescString() const;
        static std::vector<Program*> sPrograms;

        bool mCreatedFromFile = false;
        using string_time_map = std::unordered_map<std::string, time_t>;
        mutable string_time_map mFileTimeMap;

        bool checkIfFilesChanged();
        void reset();
    };
}
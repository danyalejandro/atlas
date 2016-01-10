#include "atlas/gl/Shader.hpp"
#include "atlas/core/Log.hpp"
#include "atlas/core/Platform.hpp"

#include <iostream>

namespace atlas
{
    namespace gl
    {
        struct Shader::GLShaderImpl
        {
            GLShaderImpl() :
                shaderProgram(0)
            { }

            GLShaderImpl(GLShaderImpl const& impl) = default;

            ~GLShaderImpl()
            { }

            GLShaderImpl* clone() const
            {
                return new GLShaderImpl(*this);
            }

            const GLchar* readShaderSource(std::string const& filename)
            {
                FILE* infile;
#if defined(ATLAS_PLATFORM_WINDOWS)
                fopen_s(&infile, filename.c_str(), "rb");
#else
                infile = fopen(filename.c_str(), "rb");
#endif
                if (!infile)
                {
                    ERROR_LOG("Cannot open file: " + filename);
                    return NULL;
                }

                fseek(infile, 0, SEEK_END);
                int length = ftell(infile);
                fseek(infile, 0, SEEK_SET);

                if (length == 0)
                {
                    fclose(infile);
                    return nullptr;
                }

                GLchar* source = new GLchar[length + 1];
                fread(source, 1, length, infile);
                fclose(infile);
                source[length] = '\0';
                return const_cast<const GLchar*>(source);
            }

            bool checkShaderProgram() const
            {
                if (shaderProgram == 0)
                {
                    ERROR_LOG(std::string("The shader program is null. ") +
                        std::string("Have you called compileShaders?"));
                    return false;
                }

                if (!glIsProgram(shaderProgram))
                {
                    ERROR_LOG(std::string("The shader program is invalid. ") +
                        std::string("Did your shader get deleted?"));
                    return false;
                }

                return true;
            }

            GLint shaderProgram;
            std::vector<ShaderInfo> shaders;
        };

        Shader::Shader() :
            mImpl(new GLShaderImpl)
        { }

        Shader::Shader(Shader const& shader) :
            mImpl(shader.mImpl->clone())
        { }

        Shader::~Shader()
        {
            deleteShaders();
        }

        void Shader::compileShaders(std::vector<ShaderInfo> const& shaders)
        {
            if (shaders.empty())
            {
                WARN_LOG("Received empty shader list.");
                return;
            }

            if (mImpl->shaderProgram)
            {
                WARN_LOG(std::string("Cannot create a new shader program ") +
                    std::string("when one already exists."));
                return;
            }

            mImpl->shaderProgram = glCreateProgram();

            for (auto shader : shaders)
            {
                auto handle = glCreateShader(shader.shaderType);

                const GLchar* source = mImpl->readShaderSource(shader.shaderFile);
                if (!source)
                {
                    ERROR_LOG("Cannot compile empty shader.");
                    for (auto& delShader : mImpl->shaders)
                    {
                        glDeleteShader(delShader.shaderHandle);
                    }

                    mImpl->shaders.clear();
                    glDeleteProgram(mImpl->shaderProgram);
                    mImpl->shaderProgram = 0;

                    return;
                }

                glShaderSource(handle, 1, &source, NULL);
                delete[] source;

                glCompileShader(handle);

                GLint compiled;
                glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
                if (!compiled)
                {
                    GLsizei len;
                    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &len);

                    GLchar* log = new GLchar[len + 1];
                    glGetShaderInfoLog(handle, len, &len, log);

                    ERROR_LOG("Shader compilation failed: " + std::string(log));

                    deleteShaders();
                    delete[] log;
                    return;
                }

                glAttachShader(mImpl->shaderProgram, handle);
                mImpl->shaders.push_back(ShaderInfo(shader, handle));
            }
        }

        void Shader::linkShaders()
        {
            if (!mImpl->shaderProgram)
            {
                ERROR_LOG("Cannot link with an empty program.");
                return;
            }

            glLinkProgram(mImpl->shaderProgram);

            GLint linked;
            glGetProgramiv(mImpl->shaderProgram, GL_LINK_STATUS, &linked);
            if (!linked)
            {
                GLsizei len;
                glGetShaderiv(mImpl->shaderProgram, GL_INFO_LOG_LENGTH, &len);

                GLchar* log = new GLchar[len + 1];
                glGetShaderInfoLog(mImpl->shaderProgram, len, &len, log);

                ERROR_LOG("Shader linking failed: " + std::string(log));

                deleteShaders();
                delete[] log;
                return;
            }
        }

        void Shader::deleteShaders()
        {
            for (auto& shader : mImpl->shaders)
            {
                glDetachShader(mImpl->shaderProgram, shader.shaderHandle);
                glDeleteShader(shader.shaderHandle);
            }

            mImpl->shaders.clear();

            if (mImpl->shaderProgram)
            {
                glDeleteProgram(mImpl->shaderProgram);
                mImpl->shaderProgram = 0;
            }
        }

        void Shader::bindAttribute(GLuint location, 
            std::string const& name) const
        {
            if (mImpl->checkShaderProgram())
            {
                glBindAttribLocation(mImpl->shaderProgram, location,
                    name.c_str());
            }
            else
            {
                ERROR_LOG("Cannot bind attribute without a shader program.");
            }
        }

        void Shader::enableShaders() const
        {
            if (mImpl->checkShaderProgram())
            {
                glUseProgram(mImpl->shaderProgram);
            }
            else
            {
                ERROR_LOG("Cannot enable shaders without a shader program.");
            }
        }

        void Shader::disableShaders() const
        {
            glUseProgram(0);
        }

        GLint Shader::getShaderProgram() const
        {
            return mImpl->shaderProgram;
        }

        GLint Shader::getUniformVariable(std::string const& name) const
        {
            if (mImpl->checkShaderProgram())
            {
                GLint ret = glGetUniformLocation(mImpl->shaderProgram, 
                    name.c_str());
                if (ret == -1)
                {
                    ERROR_LOG(std::string("The uniform location \"") + name +
                        std::string("\" is invalid."));
                }
                return ret;
            }
            else
            {
                ERROR_LOG(
                    std::string("Cannot access uniform variables without ") +
                    std::string("a shader program."));
                return -1;
            }
        }

        GLint Shader::getAttributeVariable(std::string const& name) const
        {
            if (mImpl->checkShaderProgram())
            {
                GLint ret = glGetAttribLocation(mImpl->shaderProgram,
                    name.c_str());
                if (ret == -1)
                {
                    ERROR_LOG(std::string("The attribute location \"") + name +
                        std::string("\" is invalid."));
                }
                
                return ret;
            }
            else
            {
                ERROR_LOG(
                    std::string("Cannot access attribute location without ") +
                    std::string("a shader program."));
                return -1;
            }
        }
    }
}
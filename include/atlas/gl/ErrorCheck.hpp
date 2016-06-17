/**
 *	\file ErrorCheck.hpp
 *	\brief Defines the OpenGL error handling system.
 *
 * Recall that OpenGL is essentially a state-machine. As a result,
 * whenever an error is triggered, the error flag is added to a queue,
 * which can then be checked until all the errors have been retrieved.
 * This class (and function) provide a mechanism for doing this 
 * automatically. Errors are retrieved from OpenGL and then 
 * printed out using the Log provided by Atlas. For convenience,
 * use the provided macro.
 * 
 * \note
 * Modern OpenGL (4.3 and above) provides a new mechanism for retrieving
 * errors using an error callback function. The reason why this isn't
 * used in Atlas is due to the platform support. Not all Linux 
 * and Apple machines have access to OpenGL 4.3 so this functionality
 * would be lost on them. At the same time, adding the code
 * necessary for the callback would just add extra over-head and hence
 * it is not implemented.
 * 
 * \warning
 * Atlas does <b> not </b> provide support for OpenGL 2. As such,
 * any errors generated from this API will not be parsed by this
 * function. Please use OpenGL 3 and above instead.
 */

#ifndef ATLAS_INCLUDE_ATLAS_GL_ERROR_CHECK_HPP
#define ATLAS_INCLUDE_ATLAS_GL_ERROR_CHECK_HPP

#pragma once

#include "atlas/core/Macros.hpp"
#include "GL.hpp"

namespace atlas
{
    namespace gl
    {
        /**
         * Loops through the OpenGL queue retrieving all of the errors
         * and prints them to the Log.
         */
        void checkGLErrors();

        /**
         *	Loops through the OpenGL error queue and cleans it out.
         *	
         *	\warning
         *	This function does not print errors! Use with care.
         */
        void clearGLErrors();

        void GLAPIENTRY openGLErrorCallback(GLenum source, GLenum type,
            GLuint id, GLenum severity, GLsizei length, const GLchar* message,
            const void* userParam);
    }
}

/**
 * \def GL_ERROR_CHECK()
 * Invokes the checkGLErrors function without having to explicity type
 * all of the namespaces.
 */
#ifdef ATLAS_DEBUG
#define GL_ERROR_CHECK() atlas::gl::checkGLErrors()
#else
#define GL_ERROR_CHECK()
#endif

/**
 *	\def GL_CLEAR_ERRORS()
 *	Clears out the OpenGL error queue without printing them to the screen.
 *	
 *	\warning
 *	This will not print errors!
 */
#define GL_CLEAR_ERRORS() atlas::gl::clearGLErrors()

#endif
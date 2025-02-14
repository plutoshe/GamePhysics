/*
	This class defines a Maya file translator
*/

#ifndef EAE6320_CMAYAGEOMETRYEXPORTER_H
#define EAE6320_CMAYAGEOMETRYEXPORTER_H

// Includes
//=========

// This must come before any Maya header files
// to make sure that windows.h is #included in a consistent way
#include <Engine/Windows/Includes.h>

#include <maya/MPxFileTranslator.h>

// Class Definition
//=================

namespace eae6320
{
	class cMayaGeometryExporter : public MPxFileTranslator
	{
		// Inherited Interface
		//====================

	public:

		// The writer method is what exports the file
		virtual bool haveWriteMethod() const override { return true; }
		virtual MStatus writer( const MFileObject& i_file, const MString& i_options, FileAccessMode i_mode ) override;

		// We won't implement a Maya importer in our class.
		// If you wanted to be able to import your geometry files into Maya
		// you would have to return true from haveReadMethod()
		// and then implement reader().

		// You can choose what the default file extension of an exported geometry is
		virtual MString defaultExtension() const override { return "bin"; }	// A string literal like "geometry" will convert automatically to an MString

		// Interface
		//==========

	public:

		// This function is used by Maya to create an instance of the exporter (see registerFileTranslator() in EntryPoint.cpp)
		static void* Create()
		{
			return new cMayaGeometryExporter;
		}
	};
}

#endif	// EAE6320_CMAYAGEOMETRYEXPORTER_H

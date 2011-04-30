#include "DecryptDirectoryInputStreamArchive.h"
#include "DecryptInputStream.h"
#include "CryptUtil.h"
#include <io/File.h>
#include <io/IOException.h>
#include <io/InputStream.h>
#include <io/FileNotFoundException.h>
#include <io/DirectoryInputStreamArchive.h>
#include <lang/Array.h>
#include <lang/Debug.h>
#include "config.h"

//-----------------------------------------------------------------------------

using namespace io;
using namespace lang;

//-----------------------------------------------------------------------------

namespace crypt
{


class DecryptDirectoryInputStreamArchive::DecryptDirectoryInputStreamArchiveImpl :
	public Object
{
public:
	P(DirectoryInputStreamArchive)	arch;
	P(DecryptInputStream)			din;
};

//-----------------------------------------------------------------------------

DecryptDirectoryInputStreamArchive::DecryptDirectoryInputStreamArchive()
{
	m_this = new DecryptDirectoryInputStreamArchiveImpl;
	m_this->arch = new DirectoryInputStreamArchive;
}

DecryptDirectoryInputStreamArchive::~DecryptDirectoryInputStreamArchive()
{
}
	
void DecryptDirectoryInputStreamArchive::addPath( const String& path )
{
	m_this->arch->addPath( path );
}

void DecryptDirectoryInputStreamArchive::addPaths( const String& path )
{
	m_this->arch->addPaths( path );
}

void DecryptDirectoryInputStreamArchive::removePaths()
{
	m_this->arch->removePaths();
}

void DecryptDirectoryInputStreamArchive::close()
{
	m_this->arch->close();
}

InputStream* DecryptDirectoryInputStreamArchive::getInputStream( const String& name )
{
	/*try
	{*/
		P(InputStream) in = m_this->arch->getInputStream( name );
		m_this->din = new DecryptInputStream( in );
		return m_this->din;
	/*}
	catch ( FileNotFoundException& )
	{
		String cryptedName = CryptUtil::cryptFileName( name );
		P(InputStream) in = m_this->arch->getInputStream( cryptedName );
		m_this->din = new DecryptInputStream( in );
		return m_this->din;
	}
	return 0;*/
}

InputStream* DecryptDirectoryInputStreamArchive::getInputStream( int index )
{
	throw IOException( Format("DecryptDirectoryInputStreamArchive does not support stream indexing") );
	return m_this->arch->getInputStream( index );
}

String DecryptDirectoryInputStreamArchive::getEntry( int index ) const
{
	throw IOException( Format("DecryptDirectoryInputStreamArchive does not support stream indexing") );
	return m_this->arch->getEntry( index );
}

int DecryptDirectoryInputStreamArchive::size() const
{
	throw IOException( Format("DecryptDirectoryInputStreamArchive does not support stream indexing") );
	return m_this->arch->size();
}

String DecryptDirectoryInputStreamArchive::toString() const
{
	return m_this->arch->toString();
}


} // crypt

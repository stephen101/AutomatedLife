/*

this is a special class definition to locate a static data file for use by our program.

the data file location will vary depending on our specific platform, so the actual implementation
of this interface is provided in platform-specific files.

*/

#ifndef __LOCATE_DATAFILE_HPP__
#define __LOCATE_DATAFILE_HPP__

// Qt includes
#include <QString>


class DatafileLocator {
	public:
		static QString getDatafileLocation();
};

#endif

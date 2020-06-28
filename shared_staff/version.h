#ifndef VERSION_H
#define VERSION_H

#include <cstdio>
#include <string>
#include <iostream>
using std::string;

/**
 * @struct Version
 * @brief The Version struct helps the program to identify the actual version.
 */

struct Version {
    int MAJOR=0;
    int MINOR=0;
    int PATCH=0;

    Version(string i_versionToCheck){
        std::sscanf(i_versionToCheck.c_str(),"%d.%d.%d",&MAJOR,&MINOR,&PATCH);
    }

    bool operator < (const Version& other){
        if (MAJOR < other.MAJOR)
            return true;
        if (MINOR < other.MINOR)
            return true;
        if (PATCH < other.PATCH)
            return true;

        return false;
    }

    bool operator == (const Version& other){
        return MAJOR == other.MAJOR && MINOR == other.MINOR && PATCH == other.PATCH;
    }
};

#endif // VERSION_H

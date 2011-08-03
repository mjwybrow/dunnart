/*
 * $Revision: 1.6 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2009-07-10 06:59:38 +1000 (Fri, 10 Jul 2009) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements the license mechanism used in OREAS applications
 * 
 * \author Carsten Gutwenger
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/

// If the symbol OGDF_ALWAYS_CHECK_HOST is defined the hostname
// id is always checked, i.e. license type "multi" is ignored
// We use this to create API packages for evaluation that
// cannot be used with a multi-machine license.
//#define OGDF_ALWAYS_CHECK_HOST


#include <ogdf/Licenser.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <limits.h>


#ifdef OGDF_SYSTEM_UNIX
#include <unistd.h>
#endif

#ifdef OGDF_SYSTEM_WINDOWS
#if defined(_MSC_VER) && defined(UNICODE)
#define LPCTSTR LPCSTR
#define LPTSTR LPSTR
#undef GetVolumeInformation
#undef GetComputerName
#define GetVolumeInformation  GetVolumeInformationA
#define GetComputerName  GetComputerNameA
#endif
#endif


namespace ogdf {

//
// abbreviations for month names
//
const char *Licenser::monthName[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};


//
// used type for 32-bit words
//
typedef unsigned long int uint4;


// maximal length of a license string
const int maxLine = 1024;


//
// shift constants used by md5-algorithm
//

const int S11 = 7, S12 = 12, S13 = 17, S14 = 22,
          S21 = 5, S22 =  9, S23 = 14, S24 = 20,
          S31 = 4, S32 = 11, S33 = 16, S34 = 23,
          S41 = 6, S42 = 10, S43 = 15, S44 = 21;



//
// basic md5 functions
//

inline uint4 F(uint4 X, uint4 Y, uint4 Z)
{
	return (X & Y) | (~X & Z);
}

inline uint4 G(uint4 X, uint4 Y, uint4 Z)
{
	return (X & Z) | (Y & ~Z);
}

inline uint4 H(uint4 X, uint4 Y, uint4 Z)
{
	return X ^ Y ^ Z;
}

inline uint4 I(uint4 X, uint4 Y, uint4 Z)
{
	return Y ^ (X | ~Z);
}


inline uint4 rotate_left(uint4 x, int n)
{
	return (x << n ) | (x >> (32-n));
}



inline void FF(uint4 &a, uint4 b, uint4 c, uint4 d,
	uint4 x, int s, uint4 ac)
{
	a += F(b,c,d) + x + ac;
	a = rotate_left(a,s);
	a += b;
}

inline void GG(uint4 &a, uint4 b, uint4 c, uint4 d,
	uint4 x, int s, uint4 ac)
{
	a += G(b,c,d) + x + ac;
	a = rotate_left(a,s);
	a += b;
}

inline void HH(uint4 &a, uint4 b, uint4 c, uint4 d,
	uint4 x, int s, uint4 ac)
{
	a += H(b,c,d) + x + ac;
	a = rotate_left(a,s);
	a += b;
}

inline void II(uint4 &a, uint4 b, uint4 c, uint4 d,
	uint4 x, int s, uint4 ac)
{
	a += I(b,c,d) + x + ac;
	a = rotate_left(a,s);
	a += b;
}


// sets the digest in 'out'
// if out[0] == 25, we use the new digest-calculation form with 25 digits instead of 32
inline void encode (char *out, uint4 word1, uint4 word2, uint4 word3, uint4 word4)
{
    if (out[0] != 25) {
		ogdf::sprintf(out, 33,  "%02x%02x%02x%02x", word1 & 0xff, (word1 >>  8) & 0xff,
		    (word1 >> 16) & 0xff, (word1 >> 24) & 0xff);
	    ogdf::sprintf(out+8, 25,  "%02x%02x%02x%02x", word2 & 0xff, (word2 >>  8) & 0xff,
		    (word2 >> 16) & 0xff, (word2 >> 24) & 0xff);
	    ogdf::sprintf(out+16, 17,  "%02x%02x%02x%02x", word3 & 0xff, (word3 >>  8) & 0xff,
		    (word3 >> 16) & 0xff, (word3 >> 24) & 0xff);
	    ogdf::sprintf(out+24, 9,   "%02x%02x%02x%02x", word4 & 0xff, (word4 >>  8) & 0xff,
		    (word4 >> 16) & 0xff, (word4 >> 24) & 0xff);
        out[32] = 0;
    }
    else {
        static uint4 pow36[] = {
            1,                // 36 ^ 0
            36,               // 36 ^ 1
            36*36,            // 36 ^ 2
            36*36*36,         // 36 ^ 3
            36*36*36*36,      // 36 ^ 4
            36*36*36*36*36,   // 36 ^ 5
            36*36*36*36*36    // 36 ^ 6
        };
        if (pow36[5] == pow36[6])
            pow36[6] *= 36;

        int b1 = (word1 > pow36[6]) ? 1 : 0;
        int b2 = (word2 > pow36[6]) ? 1 : 0;
        int b3 = (word3 > pow36[6]) ? 1 : 0;
        int b4 = (word4 > pow36[6]) ? 1 : 0;

        for (int i = 5; i >= 0; --i) {
            word1 %= pow36[i + 1];
            word2 %= pow36[i + 1];
            word3 %= pow36[i + 1];
            word4 %= pow36[i + 1];

            int v1 = word1 / pow36[i];
            int v2 = word2 / pow36[i];
            int v3 = word3 / pow36[i];
            int v4 = word4 / pow36[i];
    
            char a1 = (v1 < 10) ? '0' + v1 : 'a' + v1 - 10;
            char a2 = (v2 < 10) ? '0' + v2 : 'a' + v2 - 10;
            char a3 = (v3 < 10) ? '0' + v3 : 'a' + v3 - 10;
            char a4 = (v4 < 10) ? '0' + v4 : 'a' + v4 - 10;

            out[ 5-i] = a1;
            out[11-i] = a2;
            out[17-i] = a3;
            out[24-i] = a4;
        }

        int  v = b1 * 8 + b2 * 4 + b3 * 2 + b4;
        char a = (v < 10) ? '0' + v : 'a' + v - 10;
        out[18] = a;
        out[25] = 0;
    }
}


//
// md5-algorithm
//  input points to string of length len (may contain 0's), and digest points
//  to a character array of length at least 33 (32 characters for the digest
//  + final 0)
//
void Licenser::md5 (const char *input, int len, char *digest)
{
	// we assume that uint4 is 32-bit integer
	// if this assertion fails, adapt definition of uint4 for current system!!
	OGDF_ASSERT(sizeof(uint4) == 4);


	// Append padding bits

	int m = (len+1) % 64;
	int zeros = (m <= 56) ? (56 - m) : (120 - m);

	int N = (len+1+zeros+8)/4;

	unsigned char *buffer = new unsigned char [len+1+zeros];
	uint4 *M = new uint4[N];

	memcpy((char *)buffer, input, len);
	//strcpy ((char *)buffer, input);
	buffer[len] = 0x80;

	int i, j;
	for (i = 1; i <= zeros; i++)
		buffer[len+i] = 0;
	

	// Build word message and append length

	for (i = 0, j = 0; j < N-2; j++, i+=4)
		M[j] = ((uint4)buffer[i]) | (((uint4)buffer[i+1]) << 8) |
			(((uint4)buffer[i+2]) << 16) | (((uint4)buffer[i+3]) << 24);

	delete [] buffer;

	M[N-2] = len*8;   // we assume that length fits into 32-bit
	M[N-1] = 0;


	// Initialize MD buffer

	uint4 a = 0x67452301;
	uint4 b = 0xefcdab89;
	uint4 c = 0x98badcfe;
	uint4 d = 0x10325476;

	
	// Proces message in 16-word blocks

	uint4 X[16];

	for (i = 0; i < N/16; i++) {
		for (j = 0; j < 16; j++) {
			X[j] = M[i*16+j];
		}

		uint4 aa = a, bb = b, cc = c, dd = d;

		// Round 1
		FF (a, b, c, d, X[ 0], S11, 0xd76aa478); /* 1 */
		FF (d, a, b, c, X[ 1], S12, 0xe8c7b756); /* 2 */
		FF (c, d, a, b, X[ 2], S13, 0x242070db); /* 3 */
		FF (b, c, d, a, X[ 3], S14, 0xc1bdceee); /* 4 */
		FF (a, b, c, d, X[ 4], S11, 0xf57c0faf); /* 5 */
		FF (d, a, b, c, X[ 5], S12, 0x4787c62a); /* 6 */
		FF (c, d, a, b, X[ 6], S13, 0xa8304613); /* 7 */
		FF (b, c, d, a, X[ 7], S14, 0xfd469501); /* 8 */
		FF (a, b, c, d, X[ 8], S11, 0x698098d8); /* 9 */
		FF (d, a, b, c, X[ 9], S12, 0x8b44f7af); /* 10 */
		FF (c, d, a, b, X[10], S13, 0xffff5bb1); /* 11 */
		FF (b, c, d, a, X[11], S14, 0x895cd7be); /* 12 */
		FF (a, b, c, d, X[12], S11, 0x6b901122); /* 13 */
		FF (d, a, b, c, X[13], S12, 0xfd987193); /* 14 */
		FF (c, d, a, b, X[14], S13, 0xa679438e); /* 15 */
		FF (b, c, d, a, X[15], S14, 0x49b40821); /* 16 */

		// Round 2
		GG (a, b, c, d, X[ 1], S21, 0xf61e2562); /* 17 */
		GG (d, a, b, c, X[ 6], S22, 0xc040b340); /* 18 */
		GG (c, d, a, b, X[11], S23, 0x265e5a51); /* 19 */
		GG (b, c, d, a, X[ 0], S24, 0xe9b6c7aa); /* 20 */
		GG (a, b, c, d, X[ 5], S21, 0xd62f105d); /* 21 */
		GG (d, a, b, c, X[10], S22,  0x2441453); /* 22 */
		GG (c, d, a, b, X[15], S23, 0xd8a1e681); /* 23 */
		GG (b, c, d, a, X[ 4], S24, 0xe7d3fbc8); /* 24 */
		GG (a, b, c, d, X[ 9], S21, 0x21e1cde6); /* 25 */
		GG (d, a, b, c, X[14], S22, 0xc33707d6); /* 26 */
		GG (c, d, a, b, X[ 3], S23, 0xf4d50d87); /* 27 */
		GG (b, c, d, a, X[ 8], S24, 0x455a14ed); /* 28 */
		GG (a, b, c, d, X[13], S21, 0xa9e3e905); /* 29 */
		GG (d, a, b, c, X[ 2], S22, 0xfcefa3f8); /* 30 */
		GG (c, d, a, b, X[ 7], S23, 0x676f02d9); /* 31 */
		GG (b, c, d, a, X[12], S24, 0x8d2a4c8a); /* 32 */

		// Round 3
		HH (a, b, c, d, X[ 5], S31, 0xfffa3942); /* 33 */
		HH (d, a, b, c, X[ 8], S32, 0x8771f681); /* 34 */
		HH (c, d, a, b, X[11], S33, 0x6d9d6122); /* 35 */
		HH (b, c, d, a, X[14], S34, 0xfde5380c); /* 36 */
		HH (a, b, c, d, X[ 1], S31, 0xa4beea44); /* 37 */
		HH (d, a, b, c, X[ 4], S32, 0x4bdecfa9); /* 38 */
		HH (c, d, a, b, X[ 7], S33, 0xf6bb4b60); /* 39 */
		HH (b, c, d, a, X[10], S34, 0xbebfbc70); /* 40 */
		HH (a, b, c, d, X[13], S31, 0x289b7ec6); /* 41 */
		HH (d, a, b, c, X[ 0], S32, 0xeaa127fa); /* 42 */
		HH (c, d, a, b, X[ 3], S33, 0xd4ef3085); /* 43 */
		HH (b, c, d, a, X[ 6], S34,  0x4881d05); /* 44 */
		HH (a, b, c, d, X[ 9], S31, 0xd9d4d039); /* 45 */
		HH (d, a, b, c, X[12], S32, 0xe6db99e5); /* 46 */
		HH (c, d, a, b, X[15], S33, 0x1fa27cf8); /* 47 */
		HH (b, c, d, a, X[ 2], S34, 0xc4ac5665); /* 48 */

		// Round 4
		II (a, b, c, d, X[ 0], S41, 0xf4292244); /* 49 */
		II (d, a, b, c, X[ 7], S42, 0x432aff97); /* 50 */
		II (c, d, a, b, X[14], S43, 0xab9423a7); /* 51 */
		II (b, c, d, a, X[ 5], S44, 0xfc93a039); /* 52 */
		II (a, b, c, d, X[12], S41, 0x655b59c3); /* 53 */
		II (d, a, b, c, X[ 3], S42, 0x8f0ccc92); /* 54 */
		II (c, d, a, b, X[10], S43, 0xffeff47d); /* 55 */
		II (b, c, d, a, X[ 1], S44, 0x85845dd1); /* 56 */
		II (a, b, c, d, X[ 8], S41, 0x6fa87e4f); /* 57 */
		II (d, a, b, c, X[15], S42, 0xfe2ce6e0); /* 58 */
		II (c, d, a, b, X[ 6], S43, 0xa3014314); /* 59 */
		II (b, c, d, a, X[13], S44, 0x4e0811a1); /* 60 */
		II (a, b, c, d, X[ 4], S41, 0xf7537e82); /* 61 */
		II (d, a, b, c, X[11], S42, 0xbd3af235); /* 62 */
		II (c, d, a, b, X[ 2], S43, 0x2ad7d2bb); /* 63 */
		II (b, c, d, a, X[ 9], S44, 0xeb86d391); /* 64 */

		a += aa;
		b += bb;
		c += cc;
		d += dd;
	}

	delete [] M;

	// write computed digest

	encode(digest, a, b, c, d);
}




//
// constants used for computing secret key
// serve to adapt md5-coding to a unique coding procedure

#define SECRET_KEY_LENGTH 32

unsigned char initKey[SECRET_KEY_LENGTH] = {
	0x34, 0xae, 0x19, 0x07, 0x63, 0x24, 0xaa, 0xee,
	0xff, 0x00, 0x01, 0x20, 0x33, 0x78, 0xdd, 0xd0,
	0x80, 0x40, 0x00, 0x33, 0x77, 0x00, 0x04, 0x59,
	0x76, 0xbf, 0x6a, 0x1a, 0xd3, 0x04, 0x20, 0xd3
};


inline unsigned char rotate_left(unsigned char x, unsigned char n)
{
	return (x << n ) | (x >> (8-n));
}


#define SECRET_KEY_COMPUTATION(secretKey) \
	secretKey[16] = initKey[29]; \
	secretKey[16] = rotate_left(secretKey[16],(unsigned char)3); \
	secretKey[5] = secretKey[16] + initKey[11]; \
	secretKey[26] = initKey[4]; \
	secretKey[7] = secretKey[5] ^ initKey[0]; \
\
	secretKey[20] = initKey[20]; \
	secretKey[5] = rotate_left(secretKey[5], (unsigned char)(secretKey[7] & 0x7)); \
	secretKey[6] = secretKey[5] - secretKey[20] + initKey[13]; \
	secretKey[11] = initKey[31] & secretKey[7]; \
\
	secretKey[15] = initKey[8]; \
	secretKey[17] = secretKey[15] + initKey[23]; \
	secretKey[15] = rotate_left(secretKey[15] ^ secretKey[17],(unsigned char)4); \
	secretKey[0] = initKey[5]; \
	secretKey[8] = ~secretKey[0] ^ initKey[1]; \
	secretKey[10] = initKey[22]; \
	secretKey[4] = initKey[15] + secretKey[11]; \
	secretKey[3] = ~initKey[14]; \
	secretKey[3] = secretKey[10] + rotate_left(secretKey[3], (unsigned char)(secretKey[15] & 0x7)); \
\
	secretKey[1] = secretKey[8] + initKey[30]; \
	secretKey[13] = initKey[26]; \
	secretKey[30] = initKey[21] - secretKey[17]; \
	secretKey[2] = initKey[7] ^ secretKey[11]; \
\
	secretKey[31] = initKey[6]; \
	secretKey[0] = rotate_left(secretKey[4] + secretKey[0], (unsigned char)3); \
	secretKey[19] = secretKey[30] + initKey[12]; \
	secretKey[21] = initKey[17]; \
\
	secretKey[14] = initKey[18]; \
	secretKey[22] = secretKey[14] | ~initKey[2]; \
	secretKey[9] = initKey[19] + secretKey[19]; \
	secretKey[18] = initKey[28]; \
	secretKey[12] = initKey[27]; \
\
	secretKey[24] = secretKey[9] - initKey[3]; \
	secretKey[12] = rotate_left(secretKey[12] - (secretKey[18] ^secretKey[19]), (unsigned char)5); \
	secretKey[25] = initKey[25] ^ secretKey[22]; \
	secretKey[27] = initKey[16]; \
	secretKey[28] = secretKey[12] ^ initKey[10]; \
	secretKey[23] = initKey[9] + secretKey[28]; \
	secretKey[29] = secretKey[23] - initKey[24]; \
	secretKey[27] = secretKey[23] + rotate_left(secretKey[27] - secretKey[13], (unsigned char)2);



//
// creates a license string
//
void Licenser::create(istream &info,  // info-file
	int duration,           // duration of license (0 = unlimited)
    const char *licenseType,// license type
	int licenseNumber,      // license number
    const char *companyName,// name of company
	ostream &license,       // output for created license string
    const char *installPath,// path, where to install the license-string
    bool oldStyle)
{
	// get information from info-file

	char softwareName[64];
	char versionNumber[10];
	char hostName[256];
	char hostId[10];
    char compName[512];

	info >> softwareName;
	info >> versionNumber;
	info >> hostName;
	info >> hostId;

	char expirationDate[15];
	makeExpirationDate(duration,expirationDate);

	char licenseNumberString[20];
	ogdf::sprintf(licenseNumberString, 20, "%010d", licenseNumber);

	ogdf::strcpy(compName, 512, companyName);
    const char *quote = "\"";
    const char *nothing = "";
    const char *p2q = nothing;
    if (installPath != NULL) {
        license << "[" << installPath << "]" << endl;
        license << "\"Visio\"=";
        p2q = quote;
		ogdf::sprintf(compName, 512, "\\%s\"", companyName);
        compName[strlen(compName)-2] = '\\';
    }
	license << p2q << softwareName << " " << versionNumber << " " <<
        licenseType << " " << licenseNumberString << " " <<
        compName << " " << expirationDate << " " <<
		hostName << " " << hostId;

	// we don't eliminate the point '.' from the version number
/*	int len = strlen(versionNumber);
	if (len >= 2) {
		versionNumber[len-2] = versionNumber[len-1];
		versionNumber[len-1] = '\0';
	}
*/

	char digest[33];
    if (oldStyle)
        digest[0] = 32;
    else
        digest[0] = 25; // for now, all new digests have length 25!
	makeDigest(softwareName, versionNumber, licenseType, licenseNumber, companyName,
        expirationDate, hostName, hostId, digest);

	license << " " << digest << p2q << endl;
}



void Licenser::makeExpirationDate(int duration, char *expirationDate)
{
	if (duration == 0) {
		ogdf::sprintf(expirationDate, 15, "never");

	} else {
		time_t curTime;
		time(&curTime); // time_t is given in seconds since midnight (00:00:00),
		                // January 1, 1970

		curTime += duration*24*60*60;   // duration is given in days
		struct tm endTime;
		localtime(&endTime, &curTime);

		ogdf::sprintf(expirationDate, 15, "%02d-%s-%d", endTime.tm_mday,
			monthName[endTime.tm_mon], 1900+endTime.tm_year);
	}
}



void Licenser::makeDigest(const char *softwareName,
	char *versionNumber,
    const char *licenseType,
	int licenseNumber,
    const char *companyName,
	const char *expirationDate,
	const char *hostName,
	const char *hostId,
	char *digest)
{
	// first SECRET_KEY_LENGTH bytes of codableString contain secret key
	char codableString [maxLine];
	unsigned char *secretKey = (unsigned char *) codableString;

	// we compute our secret key
	SECRET_KEY_COMPUTATION(secretKey);	

	ogdf::sprintf(codableString+SECRET_KEY_LENGTH, maxLine-SECRET_KEY_LENGTH,
		"%s%s%s%010d%s%s%s", softwareName, versionNumber, licenseType, licenseNumber,
		companyName, expirationDate, hostName/*, hostId*/);

	int len = (int)strlen(codableString+SECRET_KEY_LENGTH)+SECRET_KEY_LENGTH;
	md5(codableString, len, digest);
}


//
// returns serial number of drive C: (Windows only)
#if defined(OGDF_SYSTEM_WINDOWS) && !defined(__MINGW32__)
static char volNameBuffer[12];
static char fileSystemBuffer[10];
static int getSerialNumber(unsigned long&VolumeSerialNumber)
{
	LPCTSTR lpRootPathName= "c:\\"; //TEXT("c:\\");

	LPTSTR lpVolumeNameBuffer= volNameBuffer;
	DWORD nVolumeNameSize= 12;

	DWORD MaximumComponentLength;
	DWORD FileSystemFlags;
	LPTSTR lpFileSystemNameBuffer= fileSystemBuffer;
	DWORD nFileSystemNameSize= 10;

	return GetVolumeInformation(lpRootPathName,lpVolumeNameBuffer,nVolumeNameSize,
		&VolumeSerialNumber,&MaximumComponentLength,
		&FileSystemFlags,lpFileSystemNameBuffer,nFileSystemNameSize);
}
#endif


// transforms expiration date into time_t value
// a return value of 0 indicates an error in date format
time_t Licenser::getExpirationTime(char*expDate)
{
	size_t len = strlen(expDate);
	if(len != 11 || expDate[2] != '-' || expDate[6] != '-')
		return(time_t)0;

	expDate[2] = expDate[6]= ' ';

	char month[4];
	struct tm date;
	date.tm_hour  = 23;
	date.tm_min   = date.tm_sec = 59;
	date.tm_isdst = 0;

	sscanf(expDate, "%d%s%d", &date.tm_mday, month, &date.tm_year);
	date.tm_year -= 1900;

	expDate[2] = expDate[6] = '-';

	int i;
	for(i = 0; i < 12; i++)
		if(!strcmp(month,monthName[i]))
			break;

	if(i >= 12)
		return(time_t)0;
	date.tm_mon = i;

	return mktime(&date);
}


static unsigned long strtoulSubstitution(char *s, int base)
{
	unsigned long x = 0;
	int c;

	while(*s == ' ' || *s == '\t')
		s++;
	while((c = *s++) != 0) {
		if('0' <= c && c<= '9')
			c -= '0';
		else if('a' <= c && c <= 'z')
			c -= 'a'-10;
		else if('A' <= c && c <= 'Z')
			c -= 'A' - 10;
		else
			break;
		if(c >= base)
			break;
		if(x > ULONG_MAX/base || base * x > ULONG_MAX - c) {
			x = ULONG_MAX;
			break;
		}
		x = base * x + c;
	}
	return x;
}


char *findElement(char *s, char *&element)
{
	while(*s == ' ' || *s == '\t')
		s++;

	element = s;

	while(*s && *s != '\n' && *s != ' ' && *s != '\t')
		s++;

	if(*s && *s != '\n') {
		*s = '\0';
		return s+1;
	} else {
		*s = '\0';
		return s;
	}
}

char *findCompanyElement(char *s, char *&element)
{
	while(*s == ' ' || *s == '\t')
		s++;

    element = s;
	while(*s && *s != '\n' && *s == '\"') // search begin of company-string
		s++;

	while(*s && *s != '\n' && *s != '\"') // search end of company-string
		s++;

    if (*s == '\"') // sets pointer one behind terminating \"
        s++;

	if(*s && *s != '\n') {
		*s = '\0';
		return s+1;
	} else {
		*s = '\0';
		return s;
	}
}

// extracts an integer from licPointer up to the end or to a .
// licPointer becomes a pointer to the character behind the . for next iteration
// defaultVal is the return-value, if we cannot extract an valid integer from licPointer
static int extractInt(char *&licPointer, int defaultVal = 0)
{
    static char buffer[128];
	ogdf::strcpy(buffer, 128, licPointer);

    int i = 0;
    while (buffer[i] != '\0' && buffer[i] != '.')
        ++i;

    licPointer += i;

    if (buffer[i] != '\0')
        licPointer++;

    buffer[i] = '\0';

    int retVal;
    if (sscanf(buffer, "%d", &retVal) == 1)
        return retVal;
    else
        return defaultVal;
}


static char retBuffer[1024];
const char *Licenser::getCompany(const char *lic)
{
    char b[1024];
    char *p;
	ogdf::strcpy(b, 1024, lic);
    char *str = findElement(b, p);
    str = findElement(str, p);
    str = findElement(str, p);
    str = findElement(str, p);
    str = findCompanyElement(str, p);
	ogdf::strcpy(retBuffer, 1024, p);
    return retBuffer;
}

const char *Licenser::getExpirationDate(const char *lic)
{
    char b[1024];
    char *p;
	ogdf::strcpy(b, 1024, lic);
    char *str = findElement(b, p);
    str = findElement(str, p);
    str = findElement(str, p);
    str = findElement(str, p);
    str = findCompanyElement(str, p);
    str = findElement(str, p);
	ogdf::strcpy(retBuffer, 1024, p);
    return retBuffer;
}

const char *Licenser::getProduct(const char *lic)
{
    char b[1024];
    char *p;
	ogdf::strcpy(b, 1024, lic);
    char *str = findElement(b, p);
	ogdf::strcpy(retBuffer, 1024, p);
	ogdf::strcat(retBuffer, 1024, " ");
    str = findElement(str, p);
	ogdf::strcat(retBuffer, 1024, p);
    return retBuffer;
}


bool isIllegalKey(const char *company, const char *digest)
{
// Your string: ""Dan Day - Standard Edition""   decode n: 'n ^ 202499' and then 'n >> 7'
int s1[] = {
        198147, 210179, 206723, 204803, 198403, 210179, 206723, 207747, 198403, 196995,
        198403, 212611, 208131, 206723, 204803, 206083, 206723, 208387, 206083, 198403,
        210307, 206083, 205699, 208131, 205699, 204931, 204803, 198147,
        0}; // Test my string: ""Dan Day - Standard Edition""


// Your string: "204cebd50ba32e54a061"   decode n: 'n ^ 202499' and then 'n >> 7'
int s2[] = {
        200195, 200451, 199939, 206467, 206211, 206339, 206083, 200067, 200451, 206339,
        206723, 200323, 200195, 206211, 200067, 199939, 206723, 200451, 199683, 200579,
        0}; // Test my string: "204cebd50ba32e54a061"


// Your string: ""Dan Day - UML Edition""   decode n: 'n ^ 202499' and then 'n >> 7'
int s3[] = {
        198147, 210179, 206723, 204803, 198403, 210179, 206723, 207747, 198403, 196995,
        198403, 212355, 209283, 209155, 198403, 210307, 206083, 205699, 208131, 205699,
        204931, 204803, 198147,
        0}; // Test my string: ""Dan Day - UML Edition""


// Your string: "34d29ec6931dc68be5e7"   decode n: 'n ^ 202499' and then 'n >> 7'
int s4[] = {
        200323, 199939, 206083, 200195, 199555, 206211, 206467, 199683, 199555, 200323,
        200579, 206083, 206467, 199683, 199427, 206339, 206211, 200067, 206211, 199811,
        0}; // Test my string: "34d29ec6931dc68be5e7"

    int *ilKeys[] = {
        s1, s2,
        s3, s4,
        NULL
    };

    char buf1[128], buf2[128];
    for (int i = 0; ilKeys[i] != NULL; i += 2) {
    	int j;
        for (j = 0; ilKeys[i][j] != 0; j++)
            buf1[j] = (ilKeys[i][j] ^ 202499) >> 7;
        buf1[j] = 0;
        for (j = 0; ilKeys[i+1][j] != 0; j++)
            buf2[j] = (ilKeys[i+1][j] ^ 202499) >> 7;
        buf2[j] = 0;
        size_t h;
        const char *p1 = company + ((h = strlen(company) - strlen(buf1)) > 0 ? h : 0);
        const char *p2 = digest  + ((h = strlen(digest)  - strlen(buf2)) > 0 ? h : 0);
        if (!strcmp(p1, buf1) &&
            !strcmp(p2, buf2))
            return true;
    }

    return false;
}

#ifdef __CYGWIN__
// Unter cygwin gibt es diese Funktion nicht, um eine Fehlermeldung zu vermeiden,
// wird einfach eine dummy-Funktion eingef?gt.
long gethostid(void) { return 0; }
#endif

Licenser::ErrorCode Licenser::checkLicense(const char *theSoftwareName,
	const char *theVersionNumberParam,
	istream &license)
{
    // map the 'const char' to a mutable 'char'
    char theVersionNumber[maxLine];
    // allow license 1.0 to use this version
	ogdf::strcpy(theVersionNumber, maxLine, "1.0");//theVersionNumberParam);
	ogdf::strcpy(theVersionNumber, maxLine, theVersionNumberParam);

	// get system infos on current machine
	const int maxHostNameLength = 256;
	char currentHostName[maxHostNameLength+1];
	unsigned long currentHostId = 0;

	// hostname
        #if defined(__MINGW32__)
        #elif defined(OGDF_SYSTEM_WINDOWS)
        unsigned long ulMaxHostNameLength = (unsigned long)maxHostNameLength;

	if(!GetComputerName(currentHostName, &ulMaxHostNameLength))
		ogdf::strcpy(currentHostName, maxHostNameLength+1, "x");
		// return getHostnameFailed;

	#else
	if(gethostname(currentHostName, maxHostNameLength) == -1)
		return getHostnameFailed;
	#endif


	// hostid (serial number of drive C: on Windows)
        #if defined(__MINGW32__)
        #elif defined(OGDF_SYSTEM_WINDOWS)
        if(!getSerialNumber(currentHostId))
        currentHostId = 0;
		// return getSerialNumberFailed;

	#else
	currentHostId = gethostid();
	#endif


	// read entries in license file
	char *softwareName, *versionNumber,
        *licenseType, *licenseNumber,
        *companyName, *expirationDate,
		*hostName, *hostId, *digest;

	char line[maxLine];
	license.getline(line, maxLine);

	char *str = findElement(line, softwareName);
	str = findElement(str, versionNumber);
    str = findElement(str, licenseType);
	str = findElement(str, licenseNumber);
    str = findCompanyElement(str, companyName);
	str = findElement(str, expirationDate);
	str = findElement(str, hostName);
	str = findElement(str, hostId);
	str = findElement(str, digest);

    if (isIllegalKey(companyName, digest))
        return illegalKey;


	// check software name
	if (strcmp(softwareName, theSoftwareName))
		return wrongSoftwareName;


	// check version number

    // initialize pointers to the beginning of the version-strings
    char *p1 = versionNumber;
    char *p2 = theVersionNumber;

    // extract the integers in both strings
    while (p1[0] != '\0' || p2[0] != '\0') {
        int val1 = extractInt(p1, INT_MAX); // default is maximal, because license-version is valid for all subversions of the software
        int val2 = extractInt(p2, 0);       // default is minimal, because subversions are filled with 0 to fit into number of subversions of license-version
    	// a license is also valid for an older version of the same software
        if (val1 < val2)
            return wrongVersionNumber;
    }

    // check expiration date
	if(strcmp(expirationDate,"never")) {
		time_t expTime = getExpirationTime(expirationDate);

		if (expTime == (time_t)0)
			return dateFormatError;

		time_t curTime;
		time(&curTime);

        m_daysLeft = (int)((expTime - curTime) / 86400);

		if(expTime < curTime)
			return expired;
	}
    else
        m_daysLeft = LONG_MAX;

    // check hostname and host-id only if this is a single license
#ifndef OGDF_ALWAYS_CHECK_HOST
    if (!strcmp(licenseType, "single")) {
#endif
	    // check hostname
	    if(strcmp(hostName,currentHostName)){
		    return wrongHostName;
	    }


	    // check host id
	    if(strtoulSubstitution(hostId,16) != currentHostId){
		    return wrongHostId;
	    }
#ifndef OGDF_ALWAYS_CHECK_HOST
    }
#endif

	// check digest
	char realDigest[33];
	int licenseNumberInt;
	sscanf(licenseNumber, "%d", &licenseNumberInt);

    realDigest[0] = (int)strlen(digest); // how to interpret this digest
	makeDigest(softwareName, versionNumber, licenseType, licenseNumberInt, companyName,
        expirationDate, hostName, hostId, realDigest);

    // uppercase letters are also valid!
    for (int tl = 0; digest[tl] != '\0'; tl++)
        digest[tl] = tolower(digest[tl]);

    p1 = digest;
    p2 = realDigest;

    // skip the first 12 digest-letters
    if (strlen(digest) == 32) {
        p1 += 12;
        p2 += 12;
    }

    if (strcmp(p1, p2))
		return wrongDigest;


	// no check failed, so license is valid
	return valid;
}

bool Licenser::validDate(int d, int m, int y)
{
    char buf[17];
	ogdf::sprintf(buf, 17, "%02d-%s-%d", d, monthName[m-1], y+(2*10*10*10));

	time_t expTime = getExpirationTime(buf);

	time_t curTime;
	time(&curTime);

    if(expTime < curTime)
		return false;
    else
        return true;
}


} // end namespace ogdf


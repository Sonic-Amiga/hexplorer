
//RIPEMD.cpp
//#include "stdafx.h"
#include "RIPEMD.h"

//#include <exception>
//#include <strstream>
#include<cstring>
#include<string>

using namespace std;

//CONSTRUCTOR
CRIPEMD::CRIPEMD(int iMethod)
{
	//Check the method
	switch(iMethod)
	{
		case RIPEMD128:
			m_auiBuf[0] = 0x67452301;
			m_auiBuf[1] = 0xefcdab89;
			m_auiBuf[2] = 0x98badcfe;
			m_auiBuf[3] = 0x10325476;
			break;

		case RIPEMD160:
			m_auiBuf[0] = 0x67452301;
			m_auiBuf[1] = 0xefcdab89;
			m_auiBuf[2] = 0x98badcfe;
			m_auiBuf[3] = 0x10325476;
			m_auiBuf[4] = 0xc3d2e1f0;
			break;

/*
		case RIPEMD256:
			m_auiBuf[0] = 0x67452301;
			m_auiBuf[1] = 0xefcdab89;
			m_auiBuf[2] = 0x98badcfe;
			m_auiBuf[3] = 0x10325476;
			m_auiBuf[4] = 0x76543210;
			m_auiBuf[5] = 0xfedcba98;
			m_auiBuf[6] = 0x89abcdef;
			m_auiBuf[7] = 0x01234567;
			break;

		case RIPEMD320:
			m_auiBuf[0] = 0x67452301;
			m_auiBuf[1] = 0xefcdab89;
			m_auiBuf[2] = 0x98badcfe;
			m_auiBuf[3] = 0x10325476;
			m_auiBuf[4] = 0xc3d2e1f0;
			m_auiBuf[5] = 0x76543210;
			m_auiBuf[6] = 0xfedcba98;
			m_auiBuf[7] = 0x89abcdef;
			m_auiBuf[8] = 0x01234567;
			m_auiBuf[8] = 0x3c2d1e0f;
			break;
*/

		default:
		{
			/*
			ostrstream ostr;
			ostr << "FileDigest ERROR: in CRIPEMD() Constructor, Illegal Method " << iMethod << "!" << ends;
			string ostrMsg = ostr.str();
			ostr.freeze(false);
			throw runtime_error(ostrMsg);*/
		}
	}
	m_iMethod = iMethod;
	m_auiBits[0] = 0;
	m_auiBits[1] = 0;
}

//Update context to reflect the concatenation of another buffer of bytes.
void CRIPEMD::AddData(char const* pcData, int iDataLength)
{
	/*
	if(iDataLength < 0)
		throw runtime_error(string("FileDigest ERROR: in CRIPEMD::AddData(), Data Length should be >= 0!"));*/
	unsigned int uiT;
	//Update bitcount
	uiT = m_auiBits[0];
	if((m_auiBits[0] = uiT + ((unsigned int)iDataLength << 3)) < uiT)
		m_auiBits[1]++; //Carry from low to high
	m_auiBits[1] += iDataLength >> 29;
	uiT = (uiT >> 3) & (BLOCKSIZE-1); //Bytes already
	//Handle any leading odd-sized chunks
	if(uiT != 0)
	{
		unsigned char *puc = (unsigned char *)m_aucIn + uiT;
		uiT = BLOCKSIZE - uiT;
		if(iDataLength < uiT)
		{
			memcpy(puc, pcData, iDataLength);
			return;
		}
		memcpy(puc, pcData, uiT);
		Transform();
		pcData += uiT;
		iDataLength -= uiT;
	}
	//Process data in 64-byte chunks
	while(iDataLength >= BLOCKSIZE)
	{
		memcpy(m_aucIn, pcData, BLOCKSIZE);
		Transform();
		pcData += BLOCKSIZE;
		iDataLength -= BLOCKSIZE;
	}
	//Handle any remaining bytes of data
	memcpy(m_aucIn, pcData, iDataLength);
	//Set the flag
	m_bAddData = true;
}

//Final wrapup - pad to 64-byte boundary with the bit pattern 
//1 0*(64-bit count of bits processed, MSB-first)
void CRIPEMD::FinalDigest(char* pcDigest)
{
	//Is the User's responsability to ensure that pcDigest has at least 16 bytes allocated
	/*
	if(false == m_bAddData)
		throw runtime_error(string("FileDigest ERROR: in CRIPEMD::FinalDigest(), No data Added before call!"));*/
	unsigned int uiCount;
	unsigned char *puc;
	//Compute number of bytes mod 64
	uiCount = (m_auiBits[0] >> 3) & (BLOCKSIZE-1);
	//Set the first char of padding to 0x80. This is safe since there is always at least one byte free
	puc = m_aucIn + uiCount;
	*puc++ = 0x80;
	//Bytes of padding needed to make 64 bytes
	uiCount = BLOCKSIZE - uiCount - 1;
	//Pad out to 56 mod 64
	if(uiCount < 8)
	{
		//Two lots of padding: Pad the first block to 64 bytes
		memset(puc, 0, uiCount);
		Transform();
		//Now fill the next block with 56 bytes
		memset(m_aucIn, 0, BLOCKSIZE-8);
	}
	else
	{
		//Pad block to 56 bytes
		memset(puc, 0, uiCount - 8);
	}
	//Append length in bits and transform
	((unsigned int*)m_aucIn)[(BLOCKSIZE>>2)-2] = m_auiBits[0];
	((unsigned int*)m_aucIn)[(BLOCKSIZE>>2)-1] = m_auiBits[1];
	Transform();
	switch(m_iMethod)
	{
		case RIPEMD128:
			memcpy(pcDigest, m_auiBuf, RIPEMD128LENGTH<<2);
			break;

		case RIPEMD160:
			memcpy(pcDigest, m_auiBuf, RIPEMD160LENGTH<<2);
			break;
	}
	//Reinitialize
	Reset();
}

//Reset current operation in order to prepare a new one
void CRIPEMD::Reset()
{
	//Reinitialize
	switch(m_iMethod)
	{
		case RIPEMD128:
			m_auiBuf[0] = 0x67452301;
			m_auiBuf[1] = 0xefcdab89;
			m_auiBuf[2] = 0x98badcfe;
			m_auiBuf[3] = 0x10325476;
			break;

		case RIPEMD160:
			m_auiBuf[0] = 0x67452301;
			m_auiBuf[1] = 0xefcdab89;
			m_auiBuf[2] = 0x98badcfe;
			m_auiBuf[3] = 0x10325476;
			m_auiBuf[4] = 0xc3d2e1f0;
			break;

/*
		case RIPEMD256:
			m_auiBuf[0] = 0x67452301;
			m_auiBuf[1] = 0xefcdab89;
			m_auiBuf[2] = 0x98badcfe;
			m_auiBuf[3] = 0x10325476;
			m_auiBuf[4] = 0x76543210;
			m_auiBuf[5] = 0xfedcba98;
			m_auiBuf[6] = 0x89abcdef;
			m_auiBuf[7] = 0x01234567;
			break;

		case RIPEMD320:
			m_auiBuf[0] = 0x67452301;
			m_auiBuf[1] = 0xefcdab89;
			m_auiBuf[2] = 0x98badcfe;
			m_auiBuf[3] = 0x10325476;
			m_auiBuf[4] = 0xc3d2e1f0;
			m_auiBuf[5] = 0x76543210;
			m_auiBuf[6] = 0xfedcba98;
			m_auiBuf[7] = 0x89abcdef;
			m_auiBuf[8] = 0x01234567;
			m_auiBuf[8] = 0x3c2d1e0f;
			break;
*/

	}
	m_auiBits[0] = 0;
	m_auiBits[1] = 0;
	//Reset the flag
	m_bAddData = false;
}

//The core of the RIPEMD algorithm, this alters an existing RIPEMD hash to
//reflect the addition of 16 longwords of new data.
void CRIPEMD::Transform()
{
	unsigned int* puiIn = (unsigned int*)m_aucIn;
	switch(m_iMethod)
	{
		case RIPEMD128:
			{
				unsigned int aa, bb, cc, dd;
				unsigned int aaa, bbb, ccc, ddd;
				aa = m_auiBuf[0];
				bb = m_auiBuf[1];
				cc = m_auiBuf[2];
				dd = m_auiBuf[3];
				aaa = m_auiBuf[0];
				bbb = m_auiBuf[1];
				ccc = m_auiBuf[2];
				ddd = m_auiBuf[3];
				//Round 1
				FF128(aa, bb, cc, dd, puiIn[ 0], 11);
				FF128(dd, aa, bb, cc, puiIn[ 1], 14);
				FF128(cc, dd, aa, bb, puiIn[ 2], 15);
				FF128(bb, cc, dd, aa, puiIn[ 3], 12);
				FF128(aa, bb, cc, dd, puiIn[ 4],  5);
				FF128(dd, aa, bb, cc, puiIn[ 5],  8);
				FF128(cc, dd, aa, bb, puiIn[ 6],  7);
				FF128(bb, cc, dd, aa, puiIn[ 7],  9);
				FF128(aa, bb, cc, dd, puiIn[ 8], 11);
				FF128(dd, aa, bb, cc, puiIn[ 9], 13);
				FF128(cc, dd, aa, bb, puiIn[10], 14);
				FF128(bb, cc, dd, aa, puiIn[11], 15);
				FF128(aa, bb, cc, dd, puiIn[12],  6);
				FF128(dd, aa, bb, cc, puiIn[13],  7);
				FF128(cc, dd, aa, bb, puiIn[14],  9);
				FF128(bb, cc, dd, aa, puiIn[15],  8);		  
				//Round 2
				GG128(aa, bb, cc, dd, puiIn[ 7],  7);
				GG128(dd, aa, bb, cc, puiIn[ 4],  6);
				GG128(cc, dd, aa, bb, puiIn[13],  8);
				GG128(bb, cc, dd, aa, puiIn[ 1], 13);
				GG128(aa, bb, cc, dd, puiIn[10], 11);
				GG128(dd, aa, bb, cc, puiIn[ 6],  9);
				GG128(cc, dd, aa, bb, puiIn[15],  7);
				GG128(bb, cc, dd, aa, puiIn[ 3], 15);
				GG128(aa, bb, cc, dd, puiIn[12],  7);
				GG128(dd, aa, bb, cc, puiIn[ 0], 12);
				GG128(cc, dd, aa, bb, puiIn[ 9], 15);
				GG128(bb, cc, dd, aa, puiIn[ 5],  9);
				GG128(aa, bb, cc, dd, puiIn[ 2], 11);
				GG128(dd, aa, bb, cc, puiIn[14],  7);
				GG128(cc, dd, aa, bb, puiIn[11], 13);
				GG128(bb, cc, dd, aa, puiIn[ 8], 12);
				//Round 3
				HH128(aa, bb, cc, dd, puiIn[ 3], 11);
				HH128(dd, aa, bb, cc, puiIn[10], 13);
				HH128(cc, dd, aa, bb, puiIn[14],  6);
				HH128(bb, cc, dd, aa, puiIn[ 4],  7);
				HH128(aa, bb, cc, dd, puiIn[ 9], 14);
				HH128(dd, aa, bb, cc, puiIn[15],  9);
				HH128(cc, dd, aa, bb, puiIn[ 8], 13);
				HH128(bb, cc, dd, aa, puiIn[ 1], 15);
				HH128(aa, bb, cc, dd, puiIn[ 2], 14);
				HH128(dd, aa, bb, cc, puiIn[ 7],  8);
				HH128(cc, dd, aa, bb, puiIn[ 0], 13);
				HH128(bb, cc, dd, aa, puiIn[ 6],  6);
				HH128(aa, bb, cc, dd, puiIn[13],  5);
				HH128(dd, aa, bb, cc, puiIn[11], 12);
				HH128(cc, dd, aa, bb, puiIn[ 5],  7);
				HH128(bb, cc, dd, aa, puiIn[12],  5);
				//Round 4
				II128(aa, bb, cc, dd, puiIn[ 1], 11);
				II128(dd, aa, bb, cc, puiIn[ 9], 12);
				II128(cc, dd, aa, bb, puiIn[11], 14);
				II128(bb, cc, dd, aa, puiIn[10], 15);
				II128(aa, bb, cc, dd, puiIn[ 0], 14);
				II128(dd, aa, bb, cc, puiIn[ 8], 15);
				II128(cc, dd, aa, bb, puiIn[12],  9);
				II128(bb, cc, dd, aa, puiIn[ 4],  8);
				II128(aa, bb, cc, dd, puiIn[13],  9);
				II128(dd, aa, bb, cc, puiIn[ 3], 14);
				II128(cc, dd, aa, bb, puiIn[ 7],  5);
				II128(bb, cc, dd, aa, puiIn[15],  6);
				II128(aa, bb, cc, dd, puiIn[14],  8);
				II128(dd, aa, bb, cc, puiIn[ 5],  6);
				II128(cc, dd, aa, bb, puiIn[ 6],  5);
				II128(bb, cc, dd, aa, puiIn[ 2], 12);
				//Parallel round 1
				III128(aaa, bbb, ccc, ddd, puiIn[ 5],  8); 
				III128(ddd, aaa, bbb, ccc, puiIn[14],  9);
				III128(ccc, ddd, aaa, bbb, puiIn[ 7],  9);
				III128(bbb, ccc, ddd, aaa, puiIn[ 0], 11);
				III128(aaa, bbb, ccc, ddd, puiIn[ 9], 13);
				III128(ddd, aaa, bbb, ccc, puiIn[ 2], 15);
				III128(ccc, ddd, aaa, bbb, puiIn[11], 15);
				III128(bbb, ccc, ddd, aaa, puiIn[ 4],  5);
				III128(aaa, bbb, ccc, ddd, puiIn[13],  7);
				III128(ddd, aaa, bbb, ccc, puiIn[ 6],  7);
				III128(ccc, ddd, aaa, bbb, puiIn[15],  8);
				III128(bbb, ccc, ddd, aaa, puiIn[ 8], 11);
				III128(aaa, bbb, ccc, ddd, puiIn[ 1], 14);
				III128(ddd, aaa, bbb, ccc, puiIn[10], 14);
				III128(ccc, ddd, aaa, bbb, puiIn[ 3], 12);
				III128(bbb, ccc, ddd, aaa, puiIn[12],  6);
				//Parallel round 2
				HHH128(aaa, bbb, ccc, ddd, puiIn[ 6],  9);
				HHH128(ddd, aaa, bbb, ccc, puiIn[11], 13);
				HHH128(ccc, ddd, aaa, bbb, puiIn[ 3], 15);
				HHH128(bbb, ccc, ddd, aaa, puiIn[ 7],  7);
				HHH128(aaa, bbb, ccc, ddd, puiIn[ 0], 12);
				HHH128(ddd, aaa, bbb, ccc, puiIn[13],  8);
				HHH128(ccc, ddd, aaa, bbb, puiIn[ 5],  9);
				HHH128(bbb, ccc, ddd, aaa, puiIn[10], 11);
				HHH128(aaa, bbb, ccc, ddd, puiIn[14],  7);
				HHH128(ddd, aaa, bbb, ccc, puiIn[15],  7);
				HHH128(ccc, ddd, aaa, bbb, puiIn[ 8], 12);
				HHH128(bbb, ccc, ddd, aaa, puiIn[12],  7);
				HHH128(aaa, bbb, ccc, ddd, puiIn[ 4],  6);
				HHH128(ddd, aaa, bbb, ccc, puiIn[ 9], 15);
				HHH128(ccc, ddd, aaa, bbb, puiIn[ 1], 13);
				HHH128(bbb, ccc, ddd, aaa, puiIn[ 2], 11);
				//Parallel round 3
				GGG128(aaa, bbb, ccc, ddd, puiIn[15],  9);
				GGG128(ddd, aaa, bbb, ccc, puiIn[ 5],  7);
				GGG128(ccc, ddd, aaa, bbb, puiIn[ 1], 15);
				GGG128(bbb, ccc, ddd, aaa, puiIn[ 3], 11);
				GGG128(aaa, bbb, ccc, ddd, puiIn[ 7],  8);
				GGG128(ddd, aaa, bbb, ccc, puiIn[14],  6);
				GGG128(ccc, ddd, aaa, bbb, puiIn[ 6],  6);
				GGG128(bbb, ccc, ddd, aaa, puiIn[ 9], 14);
				GGG128(aaa, bbb, ccc, ddd, puiIn[11], 12);
				GGG128(ddd, aaa, bbb, ccc, puiIn[ 8], 13);
				GGG128(ccc, ddd, aaa, bbb, puiIn[12],  5);
				GGG128(bbb, ccc, ddd, aaa, puiIn[ 2], 14);
				GGG128(aaa, bbb, ccc, ddd, puiIn[10], 13);
				GGG128(ddd, aaa, bbb, ccc, puiIn[ 0], 13);
				GGG128(ccc, ddd, aaa, bbb, puiIn[ 4],  7);
				GGG128(bbb, ccc, ddd, aaa, puiIn[13],  5);
				//Parallel round 4
				FFF128(aaa, bbb, ccc, ddd, puiIn[ 8], 15);
				FFF128(ddd, aaa, bbb, ccc, puiIn[ 6],  5);
				FFF128(ccc, ddd, aaa, bbb, puiIn[ 4],  8);
				FFF128(bbb, ccc, ddd, aaa, puiIn[ 1], 11);
				FFF128(aaa, bbb, ccc, ddd, puiIn[ 3], 14);
				FFF128(ddd, aaa, bbb, ccc, puiIn[11], 14);
				FFF128(ccc, ddd, aaa, bbb, puiIn[15],  6);
				FFF128(bbb, ccc, ddd, aaa, puiIn[ 0], 14);
				FFF128(aaa, bbb, ccc, ddd, puiIn[ 5],  6);
				FFF128(ddd, aaa, bbb, ccc, puiIn[12],  9);
				FFF128(ccc, ddd, aaa, bbb, puiIn[ 2], 12);
				FFF128(bbb, ccc, ddd, aaa, puiIn[13],  9);
				FFF128(aaa, bbb, ccc, ddd, puiIn[ 9], 12);
				FFF128(ddd, aaa, bbb, ccc, puiIn[ 7],  5);
				FFF128(ccc, ddd, aaa, bbb, puiIn[10], 15);
				FFF128(bbb, ccc, ddd, aaa, puiIn[14],  8);
				//Combine results
				ddd += cc + m_auiBuf[1];
				m_auiBuf[1] = m_auiBuf[2] + dd + aaa;
				m_auiBuf[2] = m_auiBuf[3] + aa + bbb;
				m_auiBuf[3] = m_auiBuf[0] + bb + ccc;
				m_auiBuf[0] = ddd;
			}
			break;

		case RIPEMD160:
			{
				unsigned int aa, bb, cc, dd, ee;
				unsigned int aaa, bbb, ccc, ddd, eee;
				aa = m_auiBuf[0];
				bb = m_auiBuf[1];
				cc = m_auiBuf[2];
				dd = m_auiBuf[3];
				ee = m_auiBuf[4];
				aaa = m_auiBuf[0];
				bbb = m_auiBuf[1];
				ccc = m_auiBuf[2];
				ddd = m_auiBuf[3];
				eee = m_auiBuf[4];
				//Round 1
				FF160(aa, bb, cc, dd, ee, puiIn[ 0], 11);
				FF160(ee, aa, bb, cc, dd, puiIn[ 1], 14);
				FF160(dd, ee, aa, bb, cc, puiIn[ 2], 15);
				FF160(cc, dd, ee, aa, bb, puiIn[ 3], 12);
				FF160(bb, cc, dd, ee, aa, puiIn[ 4],  5);
				FF160(aa, bb, cc, dd, ee, puiIn[ 5],  8);
				FF160(ee, aa, bb, cc, dd, puiIn[ 6],  7);
				FF160(dd, ee, aa, bb, cc, puiIn[ 7],  9);
				FF160(cc, dd, ee, aa, bb, puiIn[ 8], 11);
				FF160(bb, cc, dd, ee, aa, puiIn[ 9], 13);
				FF160(aa, bb, cc, dd, ee, puiIn[10], 14);
				FF160(ee, aa, bb, cc, dd, puiIn[11], 15);
				FF160(dd, ee, aa, bb, cc, puiIn[12],  6);
				FF160(cc, dd, ee, aa, bb, puiIn[13],  7);
				FF160(bb, cc, dd, ee, aa, puiIn[14],  9);
				FF160(aa, bb, cc, dd, ee, puiIn[15],  8);			  
				//Round 2
				GG160(ee, aa, bb, cc, dd, puiIn[ 7],  7);
				GG160(dd, ee, aa, bb, cc, puiIn[ 4],  6);
				GG160(cc, dd, ee, aa, bb, puiIn[13],  8);
				GG160(bb, cc, dd, ee, aa, puiIn[ 1], 13);
				GG160(aa, bb, cc, dd, ee, puiIn[10], 11);
				GG160(ee, aa, bb, cc, dd, puiIn[ 6],  9);
				GG160(dd, ee, aa, bb, cc, puiIn[15],  7);
				GG160(cc, dd, ee, aa, bb, puiIn[ 3], 15);
				GG160(bb, cc, dd, ee, aa, puiIn[12],  7);
				GG160(aa, bb, cc, dd, ee, puiIn[ 0], 12);
				GG160(ee, aa, bb, cc, dd, puiIn[ 9], 15);
				GG160(dd, ee, aa, bb, cc, puiIn[ 5],  9);
				GG160(cc, dd, ee, aa, bb, puiIn[ 2], 11);
				GG160(bb, cc, dd, ee, aa, puiIn[14],  7);
				GG160(aa, bb, cc, dd, ee, puiIn[11], 13);
				GG160(ee, aa, bb, cc, dd, puiIn[ 8], 12);
				//Round 3
				HH160(dd, ee, aa, bb, cc, puiIn[ 3], 11);
				HH160(cc, dd, ee, aa, bb, puiIn[10], 13);
				HH160(bb, cc, dd, ee, aa, puiIn[14],  6);
				HH160(aa, bb, cc, dd, ee, puiIn[ 4],  7);
				HH160(ee, aa, bb, cc, dd, puiIn[ 9], 14);
				HH160(dd, ee, aa, bb, cc, puiIn[15],  9);
				HH160(cc, dd, ee, aa, bb, puiIn[ 8], 13);
				HH160(bb, cc, dd, ee, aa, puiIn[ 1], 15);
				HH160(aa, bb, cc, dd, ee, puiIn[ 2], 14);
				HH160(ee, aa, bb, cc, dd, puiIn[ 7],  8);
				HH160(dd, ee, aa, bb, cc, puiIn[ 0], 13);
				HH160(cc, dd, ee, aa, bb, puiIn[ 6],  6);
				HH160(bb, cc, dd, ee, aa, puiIn[13],  5);
				HH160(aa, bb, cc, dd, ee, puiIn[11], 12);
				HH160(ee, aa, bb, cc, dd, puiIn[ 5],  7);
				HH160(dd, ee, aa, bb, cc, puiIn[12],  5);
				//Round 4
				II160(cc, dd, ee, aa, bb, puiIn[ 1], 11);
				II160(bb, cc, dd, ee, aa, puiIn[ 9], 12);
				II160(aa, bb, cc, dd, ee, puiIn[11], 14);
				II160(ee, aa, bb, cc, dd, puiIn[10], 15);
				II160(dd, ee, aa, bb, cc, puiIn[ 0], 14);
				II160(cc, dd, ee, aa, bb, puiIn[ 8], 15);
				II160(bb, cc, dd, ee, aa, puiIn[12],  9);
				II160(aa, bb, cc, dd, ee, puiIn[ 4],  8);
				II160(ee, aa, bb, cc, dd, puiIn[13],  9);
				II160(dd, ee, aa, bb, cc, puiIn[ 3], 14);
				II160(cc, dd, ee, aa, bb, puiIn[ 7],  5);
				II160(bb, cc, dd, ee, aa, puiIn[15],  6);
				II160(aa, bb, cc, dd, ee, puiIn[14],  8);
				II160(ee, aa, bb, cc, dd, puiIn[ 5],  6);
				II160(dd, ee, aa, bb, cc, puiIn[ 6],  5);
				II160(cc, dd, ee, aa, bb, puiIn[ 2], 12);
				//Round 5
				JJ160(bb, cc, dd, ee, aa, puiIn[ 4],  9);
				JJ160(aa, bb, cc, dd, ee, puiIn[ 0], 15);
				JJ160(ee, aa, bb, cc, dd, puiIn[ 5],  5);
				JJ160(dd, ee, aa, bb, cc, puiIn[ 9], 11);
				JJ160(cc, dd, ee, aa, bb, puiIn[ 7],  6);
				JJ160(bb, cc, dd, ee, aa, puiIn[12],  8);
				JJ160(aa, bb, cc, dd, ee, puiIn[ 2], 13);
				JJ160(ee, aa, bb, cc, dd, puiIn[10], 12);
				JJ160(dd, ee, aa, bb, cc, puiIn[14],  5);
				JJ160(cc, dd, ee, aa, bb, puiIn[ 1], 12);
				JJ160(bb, cc, dd, ee, aa, puiIn[ 3], 13);
				JJ160(aa, bb, cc, dd, ee, puiIn[ 8], 14);
				JJ160(ee, aa, bb, cc, dd, puiIn[11], 11);
				JJ160(dd, ee, aa, bb, cc, puiIn[ 6],  8);
				JJ160(cc, dd, ee, aa, bb, puiIn[15],  5);
				JJ160(bb, cc, dd, ee, aa, puiIn[13],  6);
				//Parallel Round 1
				JJJ160(aaa, bbb, ccc, ddd, eee, puiIn[ 5],  8);
				JJJ160(eee, aaa, bbb, ccc, ddd, puiIn[14],  9);
				JJJ160(ddd, eee, aaa, bbb, ccc, puiIn[ 7],  9);
				JJJ160(ccc, ddd, eee, aaa, bbb, puiIn[ 0], 11);
				JJJ160(bbb, ccc, ddd, eee, aaa, puiIn[ 9], 13);
				JJJ160(aaa, bbb, ccc, ddd, eee, puiIn[ 2], 15);
				JJJ160(eee, aaa, bbb, ccc, ddd, puiIn[11], 15);
				JJJ160(ddd, eee, aaa, bbb, ccc, puiIn[ 4],  5);
				JJJ160(ccc, ddd, eee, aaa, bbb, puiIn[13],  7);
				JJJ160(bbb, ccc, ddd, eee, aaa, puiIn[ 6],  7);
				JJJ160(aaa, bbb, ccc, ddd, eee, puiIn[15],  8);
				JJJ160(eee, aaa, bbb, ccc, ddd, puiIn[ 8], 11);
				JJJ160(ddd, eee, aaa, bbb, ccc, puiIn[ 1], 14);
				JJJ160(ccc, ddd, eee, aaa, bbb, puiIn[10], 14);
				JJJ160(bbb, ccc, ddd, eee, aaa, puiIn[ 3], 12);
				JJJ160(aaa, bbb, ccc, ddd, eee, puiIn[12],  6);
				//Parallel round 2
				III160(eee, aaa, bbb, ccc, ddd, puiIn[ 6],  9); 
				III160(ddd, eee, aaa, bbb, ccc, puiIn[11], 13);
				III160(ccc, ddd, eee, aaa, bbb, puiIn[ 3], 15);
				III160(bbb, ccc, ddd, eee, aaa, puiIn[ 7],  7);
				III160(aaa, bbb, ccc, ddd, eee, puiIn[ 0], 12);
				III160(eee, aaa, bbb, ccc, ddd, puiIn[13],  8);
				III160(ddd, eee, aaa, bbb, ccc, puiIn[ 5],  9);
				III160(ccc, ddd, eee, aaa, bbb, puiIn[10], 11);
				III160(bbb, ccc, ddd, eee, aaa, puiIn[14],  7);
				III160(aaa, bbb, ccc, ddd, eee, puiIn[15],  7);
				III160(eee, aaa, bbb, ccc, ddd, puiIn[ 8], 12);
				III160(ddd, eee, aaa, bbb, ccc, puiIn[12],  7);
				III160(ccc, ddd, eee, aaa, bbb, puiIn[ 4],  6);
				III160(bbb, ccc, ddd, eee, aaa, puiIn[ 9], 15);
				III160(aaa, bbb, ccc, ddd, eee, puiIn[ 1], 13);
				III160(eee, aaa, bbb, ccc, ddd, puiIn[ 2], 11);
				//Parallel round 3
				HHH160(ddd, eee, aaa, bbb, ccc, puiIn[15],  9);
				HHH160(ccc, ddd, eee, aaa, bbb, puiIn[ 5],  7);
				HHH160(bbb, ccc, ddd, eee, aaa, puiIn[ 1], 15);
				HHH160(aaa, bbb, ccc, ddd, eee, puiIn[ 3], 11);
				HHH160(eee, aaa, bbb, ccc, ddd, puiIn[ 7],  8);
				HHH160(ddd, eee, aaa, bbb, ccc, puiIn[14],  6);
				HHH160(ccc, ddd, eee, aaa, bbb, puiIn[ 6],  6);
				HHH160(bbb, ccc, ddd, eee, aaa, puiIn[ 9], 14);
				HHH160(aaa, bbb, ccc, ddd, eee, puiIn[11], 12);
				HHH160(eee, aaa, bbb, ccc, ddd, puiIn[ 8], 13);
				HHH160(ddd, eee, aaa, bbb, ccc, puiIn[12],  5);
				HHH160(ccc, ddd, eee, aaa, bbb, puiIn[ 2], 14);
				HHH160(bbb, ccc, ddd, eee, aaa, puiIn[10], 13);
				HHH160(aaa, bbb, ccc, ddd, eee, puiIn[ 0], 13);
				HHH160(eee, aaa, bbb, ccc, ddd, puiIn[ 4],  7);
				HHH160(ddd, eee, aaa, bbb, ccc, puiIn[13],  5);
				//Parallel round 4
				GGG160(ccc, ddd, eee, aaa, bbb, puiIn[ 8], 15);
				GGG160(bbb, ccc, ddd, eee, aaa, puiIn[ 6],  5);
				GGG160(aaa, bbb, ccc, ddd, eee, puiIn[ 4],  8);
				GGG160(eee, aaa, bbb, ccc, ddd, puiIn[ 1], 11);
				GGG160(ddd, eee, aaa, bbb, ccc, puiIn[ 3], 14);
				GGG160(ccc, ddd, eee, aaa, bbb, puiIn[11], 14);
				GGG160(bbb, ccc, ddd, eee, aaa, puiIn[15],  6);
				GGG160(aaa, bbb, ccc, ddd, eee, puiIn[ 0], 14);
				GGG160(eee, aaa, bbb, ccc, ddd, puiIn[ 5],  6);
				GGG160(ddd, eee, aaa, bbb, ccc, puiIn[12],  9);
				GGG160(ccc, ddd, eee, aaa, bbb, puiIn[ 2], 12);
				GGG160(bbb, ccc, ddd, eee, aaa, puiIn[13],  9);
				GGG160(aaa, bbb, ccc, ddd, eee, puiIn[ 9], 12);
				GGG160(eee, aaa, bbb, ccc, ddd, puiIn[ 7],  5);
				GGG160(ddd, eee, aaa, bbb, ccc, puiIn[10], 15);
				GGG160(ccc, ddd, eee, aaa, bbb, puiIn[14],  8);
				//Parallel round 5
				FFF160(bbb, ccc, ddd, eee, aaa, puiIn[12] ,  8);
				FFF160(aaa, bbb, ccc, ddd, eee, puiIn[15] ,  5);
				FFF160(eee, aaa, bbb, ccc, ddd, puiIn[10] , 12);
				FFF160(ddd, eee, aaa, bbb, ccc, puiIn[ 4] ,  9);
				FFF160(ccc, ddd, eee, aaa, bbb, puiIn[ 1] , 12);
				FFF160(bbb, ccc, ddd, eee, aaa, puiIn[ 5] ,  5);
				FFF160(aaa, bbb, ccc, ddd, eee, puiIn[ 8] , 14);
				FFF160(eee, aaa, bbb, ccc, ddd, puiIn[ 7] ,  6);
				FFF160(ddd, eee, aaa, bbb, ccc, puiIn[ 6] ,  8);
				FFF160(ccc, ddd, eee, aaa, bbb, puiIn[ 2] , 13);
				FFF160(bbb, ccc, ddd, eee, aaa, puiIn[13] ,  6);
				FFF160(aaa, bbb, ccc, ddd, eee, puiIn[14] ,  5);
				FFF160(eee, aaa, bbb, ccc, ddd, puiIn[ 0] , 15);
				FFF160(ddd, eee, aaa, bbb, ccc, puiIn[ 3] , 13);
				FFF160(ccc, ddd, eee, aaa, bbb, puiIn[ 9] , 11);
				FFF160(bbb, ccc, ddd, eee, aaa, puiIn[11] , 11);
				//Combine Results
				ddd += cc + m_auiBuf[1];
				m_auiBuf[1] = m_auiBuf[2] + dd + eee;
				m_auiBuf[2] = m_auiBuf[3] + ee + aaa;
				m_auiBuf[3] = m_auiBuf[4] + aa + bbb;
				m_auiBuf[4] = m_auiBuf[0] + bb + ccc;
				m_auiBuf[0] = ddd;
			}
			break;
		
/*
		case RIPEMD256:
			{

			}
			break;

		case RIPEMD320:
			{

			}
			break;
*/

	}
}


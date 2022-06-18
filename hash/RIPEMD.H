
//RIPEMD.h

#ifndef __RIPEMD_H__
#define __RIPEMD_H__

#include "MessageDigest.h"

//Typical DISCLAIMER:
//The code in this project is Copyright (C) 2003 by George Anescu. You have the right to
//use and distribute the code in any way you see fit as long as this paragraph is included
//with the distribution. No warranties or claims are made as to the validity of the
//information and code contained herein, so use it at your own risk.

//RIPEMD Message Digest algorithm
//
//  Conditions for use of the RIPEMD-128,160,256,320 Software
//
//  The RIPEMD software is freely available for use under the terms and
//  conditions described hereunder, which shall be deemed to be accepted by
//  any user of the software and applicable on any use of the software:
// 
//  1. K.U.Leuven Department of Electrical Engineering-ESAT/COSIC shall for
//     all purposes be considered the owner of the RIPEMD software and of
//     all copyright, trade secret, patent or other intellectual property
//     rights therein.
//  2. The RIPEMD software is provided on an "as is" basis without
//     warranty of any sort, express or implied. K.U.Leuven makes no
//     representation that the use of the software will not infringe any
//     patent or proprietary right of third parties. User will indemnify
//     K.U.Leuven and hold K.U.Leuven harmless from any claims or liabilities
//     which may arise as a result of its use of the software. In no
//     circumstances K.U.Leuven R&D will be held liable for any deficiency,
//     fault or other mishappening with regard to the use or performance of
//     the software.
//  3. User agrees to give due credit to K.U.Leuven in scientific publications 
//     or communications in relation with the use of the RIPEMD software.
//
//RIPEMD128 TEST VALUES:
//1)"abcdefghijklmnopqrstuvwxyz"
//"fd2aa607f71dc8f510714922b371834e"
//
//2)"" (empty string)
//"cdf26213a150dc3ecb610f18f6b38b46"
//
//3)"a"
//"86be7afa339d0fc7cfc785e72f578d33"
//
//4)"abc"
//"c14a12199c66e4ba84636b0f69144c77"
//
//5)"message digest"
//"9e327b3d6e523062afc1132d7df9d1b8"
//
//6)"abcdefghijklmnopqrstuvwxyz"
//"fd2aa607f71dc8f510714922b371834e"
//
//7)"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
//"a1aa0689d0fafa2ddc22e88b49133a06"
//
//8)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
//"d1e959eb179c911faea4624c60c5c702"
//
//9)8 times "1234567890"
//"3f45ef194732c2dbb2c4a2c769795fa3"
//
//10)1 million times "a"
//"4a7f5723f954eba1216c9d8f6320431f"
//
//RIPEMD160 TEST VALUES:
//1)"abcdefghijklmnopqrstuvwxyz"
//"f71c27109c692c1b56bbdceb5b9d2865b3708dbc"
//
//2)"" (empty string)
//"9c1185a5c5e9fc54612808977ee8f548b2258d31"
//
//3)"a"
//"0bdc9d2d256b3ee9daae347be6f4dc835a467ffe"
//
//4)"abc"
//"8eb208f7e05d987a9b044a8e98c6b087f15a0bfc"
//
//5)"message digest"
//"5d0689ef49d2fae572b881b123a85ffa21595f36"
//
//6)"abcdefghijklmnopqrstuvwxyz"
//"f71c27109c692c1b56bbdceb5b9d2865b3708dbc"
//
//7)"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
//"12a053384a9c0c88e405a06c27dcf49ada62eb2b"
//
//8)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
//"b0e20b6e3116640286ed3a87a5713079b21f5189"
//
//9)8 times "1234567890"
//"9b752e45573d4b39f4dbd3323cab82bf63326bfb" 
//
//10)1 million times "a"
//"52783243c1697bdbe16d37f97f68f08325dc1528"
class CRIPEMD : public IMessageDigest
{
public:
	enum { RIPEMD128=0, RIPEMD160=1 }; //, RIPEMD256=2, RIPEMD320=3 };
	//CONSTRUCTOR
	CRIPEMD(int iMethod=RIPEMD128);
	//Update context to reflect the concatenation of another buffer of bytes.
	void AddData(char const* pcData, int iDataLength);
	//Final wrapup - pad to 64-byte boundary with the bit pattern 
	//1 0*(64-bit count of bits processed, MSB-first)
	void FinalDigest(char* pcDigest);
	//Reset current operation in order to prepare a new one
	void Reset();
	
private:
	enum { RIPEMD128LENGTH=4, RIPEMD160LENGTH=5 }; //, RIPEMD256LENGTH=8, RIPEMD320LENGTH=10 };
	//Context Variables
	unsigned int m_auiBuf[RIPEMD160LENGTH]; //Maximum for RIPEMD160
	unsigned int m_auiBits[2];
	unsigned char m_aucIn[BLOCKSIZE];

	static unsigned int ROL(unsigned int x, unsigned int n);

	//The five basic functions F(), G() and H()
	static unsigned int F(unsigned int x, unsigned int y, unsigned int z);

	static unsigned int G(unsigned int x, unsigned int y, unsigned int z);

	static unsigned int H(unsigned int x, unsigned int y, unsigned int z);

	static unsigned int I(unsigned int x, unsigned int y, unsigned int z);

	static unsigned int J(unsigned int x, unsigned int y, unsigned int z);

	//RIPEMD-128
	//The eight basic operations FF() through III()
	static void FF128(unsigned int& ra, unsigned int b, unsigned int c,
		unsigned int d, unsigned int x, unsigned int s);
	
	static void GG128(unsigned int& ra, unsigned int b, unsigned int c,
		unsigned int d, unsigned int x, unsigned int s);

	static void HH128(unsigned int& ra, unsigned int b, unsigned int c,
		unsigned int d, unsigned int x, unsigned int s);

	static void II128(unsigned int& ra, unsigned int b, unsigned int c,
		unsigned int d, unsigned int x, unsigned int s);

	static void FFF128(unsigned int& ra, unsigned int b, unsigned int c,
		unsigned int d, unsigned int x, unsigned int s);

	static void GGG128(unsigned int& ra, unsigned int b, unsigned int c,
		unsigned int d, unsigned int x, unsigned int s);

	static void HHH128(unsigned int& ra, unsigned int b, unsigned int c,
		unsigned int d, unsigned int x, unsigned int s);

	static void III128(unsigned int& ra, unsigned int b, unsigned int c,
		unsigned int d, unsigned int x, unsigned int s);

	//RIPEMD-160  
	//The ten basic operations FF() through III()
	static void FF160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	static void GG160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	static void HH160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	static void II160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	static void JJ160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	static void FFF160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	static void GGG160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	static void HHH160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	static void III160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	static void JJJ160(unsigned int& ra, unsigned int b, unsigned int& rc,
		unsigned int d, unsigned int e, unsigned int x, unsigned int s);

	void Transform();

	//The Method
	int m_iMethod;
};

//ROL(x, n) cyclically rotates x over n bits to the left
//x must be of an unsigned 32 bits type and 0 <= n < 32.
inline unsigned int CRIPEMD::ROL(unsigned int x, unsigned int n)
{
	return (x << n) | (x >> (32-n));
}

//The five basic functions F(), G() and H()
inline unsigned int CRIPEMD::F(unsigned int x, unsigned int y, unsigned int z)
{
	return x ^ y ^ z;
}

inline unsigned int CRIPEMD::G(unsigned int x, unsigned int y, unsigned int z)
{
	return ((x & y) | (~x & z));
}

inline unsigned int CRIPEMD::H(unsigned int x, unsigned int y, unsigned int z)
{
	return ((x | ~y) ^ z);
}

inline unsigned int CRIPEMD::I(unsigned int x, unsigned int y, unsigned int z)
{
	return ((x & z) | (y & ~z));
}

inline unsigned int CRIPEMD::J(unsigned int x, unsigned int y, unsigned int z)
{
	return (x ^ (y | ~z));
}

//RIPEMD-128
//The eight basic operations FF() through III()
inline void CRIPEMD::FF128(unsigned int& ra, unsigned int b, unsigned int c,
	unsigned int d, unsigned int x, unsigned int s)
{
	ra += F(b, c, d) + x;
	ra = ROL(ra, s);
}

inline void CRIPEMD::GG128(unsigned int& ra, unsigned int b, unsigned int c,
	unsigned int d, unsigned int x, unsigned int s)
{
	ra += G(b, c, d) + x + 0x5a827999;
	ra = ROL(ra, s);
}

inline void CRIPEMD::HH128(unsigned int& ra, unsigned int b, unsigned int c,
	unsigned int d, unsigned int x, unsigned int s)
{
	ra += H(b, c, d) + x + 0x6ed9eba1;
	ra = ROL(ra, s);
}

inline void CRIPEMD::II128(unsigned int& ra, unsigned int b, unsigned int c,
	unsigned int d, unsigned int x, unsigned int s)
{
	ra += I(b, c, d) + x + 0x8f1bbcdc;
	ra = ROL(ra, s);
}

inline void CRIPEMD::FFF128(unsigned int& ra, unsigned int b, unsigned int c,
	unsigned int d, unsigned int x, unsigned int s)
{
	ra += F(b, c, d) + x;
	ra = ROL(ra, s);
}

inline void CRIPEMD::GGG128(unsigned int& ra, unsigned int b, unsigned int c,
	unsigned int d, unsigned int x, unsigned int s)
{
	ra += G(b, c, d) + x + 0x6d703ef3;
	ra = ROL(ra, s);
}

inline void CRIPEMD::HHH128(unsigned int& ra, unsigned int b, unsigned int c,
	unsigned int d, unsigned int x, unsigned int s)
{
	ra += H(b, c, d) + x + 0x5c4dd124;
	ra = ROL(ra, s);
}

inline void CRIPEMD::III128(unsigned int& ra, unsigned int b, unsigned int c,
	unsigned int d, unsigned int x, unsigned int s)
{
	ra += I(b, c, d) + x + 0x50a28be6;
	ra = ROL(ra, s);
}

//RIPEMD-160  
//The ten basic operations FF() through III()
inline void CRIPEMD::FF160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += F(b, rc, d) + x;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

inline void CRIPEMD::GG160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += G(b, rc, d) + x + 0x5a827999;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

inline void CRIPEMD::HH160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += H(b, rc, d) + x + 0x6ed9eba1;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

inline void CRIPEMD::II160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += I(b, rc, d) + x + 0x8f1bbcdc;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

inline void CRIPEMD::JJ160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += J(b, rc, (d)) + (x) + 0xa953fd4e;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

inline void CRIPEMD::FFF160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += F(b, rc, d) + x;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

inline void CRIPEMD::GGG160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += G(b, rc, d) + x + 0x7a6d76e9;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

inline void CRIPEMD::HHH160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += H(b, rc, d) + x + 0x6d703ef3;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

inline void CRIPEMD::III160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += I(b, rc, d) + x + 0x5c4dd124;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

inline void CRIPEMD::JJJ160(unsigned int& ra, unsigned int b, unsigned int& rc,
	unsigned int d, unsigned int e, unsigned int x, unsigned int s)
{
	ra += J(b, rc, d) + x + 0x50a28be6;
	ra = ROL(ra, s) + e;
	rc = ROL(rc, 10);
}

#endif // __RIPEMD_H__


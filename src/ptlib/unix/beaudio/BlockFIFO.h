//
// (c) Yuri Kiryanov, openh323@kiryanov.com
// for www.Openh323.org by Equivalence
//
// Portions: 1998-1999, Be Incorporated
// Be Sample Code License

#if !defined(_BLOCK_FIFO_H)
#define _BLOCK_FIFO_H

#include <OS.h>

namespace BPrivate {

class BBlockFIFO {
public:
					BBlockFIFO(
							size_t blockSize,			//	BeginGet will never ask for bigger chunks than this
							int32 blockCountPerBuffer,	//	BeginPut will never ask for bigger chunks than blockSize*blockCount
							int32 bufferCount,			//	double- or triple-buffering?
							uint32 placementFlags = B_ANY_ADDRESS,
							uint32 lockFlags = 0,
							const char * name = 0);
					~BBlockFIFO();

		status_t	Reset();			//	for re-use
		status_t	InitCheck();

		int32		SizeAvailableToGet();
		int32		SizeAvailableToPut();

		int32		BeginGet(		//	returns actual size
							const void ** outData,
							size_t requestSize,
							bigtime_t timeout);
		int32		EndGet();

		int32		BeginPut(	//	returns actual size
							void ** inData,
							size_t requestSize,
							bigtime_t timeout);
		int32		EndPut(
							bool atEndOfData = false);

		//	these are wrappers for the above functions
		int32		CopyNextBlockOut(	//	returns actual size
							void * destination,
							size_t requestSize,
							bigtime_t timeout);
		int32		CopyNextBufferIn(	//	returns actual size
							const void * source,
							size_t requestSize,
							bigtime_t timeout,
							bool atEndOfData = false);

private:

		char *		_mBuffer;
		area_id		_mArea;
	enum {
		flagEndOfData = 1,
		flagPendingGet = 2,
		flagPendingPut = 4
	};
		int32		_mFlags;
		size_t		_mBlockSize;
		size_t		_mBufferSize;
		size_t		_mAreaSize;			//	minus padding
		status_t	_mInitErr;
		sem_id		_mGetSem;
		sem_id		_mPutSem;
		size_t		_mGetOff;
		int32		_mPendingGet;
		size_t		_mPutOff;
		int32		_mPendingPut;
		char		_mName[32];
};

};
using namespace BPrivate;

#endif	//	_BLOCK_FIFO_H


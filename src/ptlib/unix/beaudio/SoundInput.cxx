//
// (c) Yuri Kiryanov, openh323@kiryanov.com
// for www.Openh323.org by Equivalence
//
// Portions: 1998-1999, Be Incorporated
//
#include "SoundInput.h"
#include "SoundPlayer.h"

// Support kit bits
#include "support/Locker.h"
#include "media/MediaRoster.h"

#include "stdio.h"

static const char* gsNotifyCodes[] = {
	"",
	"B_WILL_START",
	"B_WILL_STOP"
	"B_WILL_SEEK",
	"B_WILL_TIMEWARP"
	"B_CONNECTED",
	"B_DISCONNECTED",
	"B_FORMAT_CHANGED",
	"B_NODE_DIES",
	"B_HOOKS_CHANGED",
	"B_OP_TIMED_OUT",
	"B_PRODUCER_DATA_STATUS",
	"B_LATE_NOTICE",
};

PSoundInput* PSoundInput::CreateSoundInput(const char* name)
{
	PSoundInput* input = NULL;
	BMediaRoster* roster = BMediaRoster::CurrentRoster();
	if( !roster )
		roster = BMediaRoster::Roster();

	if( roster )
	{
		input = new PSoundInput(name);
		roster->RegisterNode(input);
	}

	return input;
}

void PSoundInput::ReleaseSoundInput(PSoundInput* input)
{
	BMediaRoster::CurrentRoster()->ReleaseNode( input->Node() );
}

PSoundInput::PSoundInput(const char* name, size_t bufSize ) :
	SoundConsumer( name, NULL, NULL, NULL), 
			BMediaNode( name ),  
				m_roster (NULL), mfRecording(false), mFIFO(65535, 1, 3, B_ANY_ADDRESS, 0, name )
{ 
	m_roster = BMediaRoster::CurrentRoster();
	
	if( m_roster )
	{
		mError = m_roster->GetAudioInput(&m_audioInputNode);
		if (mError == B_OK) 
		{
			mError = m_roster->RegisterNode(this);
		}
	}
}

bool PSoundInput::StartRecording()
{
	if( mfRecording )
		return true;
	
	m_roster = BMediaRoster::CurrentRoster();

	//	Find an available output for the given input node.
	int32 count = 0;
	mError = m_roster->GetFreeOutputsFor(m_audioInputNode, 
			&m_audioOutput, 1, &count, B_MEDIA_RAW_AUDIO);

	if( mError != B_OK )
		cerr << "PSoundInput::StartRecording, Error: " << strerror(mError) << endl;
		
	if (mError == B_OK && count >= 0 ) 
	{
		//	Find an available input for our own Node. 
		mError = m_roster->GetFreeInputsFor(Node(), &m_recInput, 1, &count, B_MEDIA_RAW_AUDIO);
#ifdef EXT_DEBUG
		cerr << "GetFreeInputsFor, Error Code: " << strerror(mError) << endl;
#endif
		if (mError == B_OK && count >=0 ) 
		{
			//	Update desired format.
			media_format fmt;
			fmt.u.raw_audio = m_audioOutput.format.u.raw_audio;
			fmt.type = B_MEDIA_RAW_AUDIO;

			//	Using the same structs for input and output is OK in BMediaRoster::Connect().
			mError = m_roster->Connect(m_audioOutput.source, 
					m_recInput.destination, &fmt, &m_audioOutput, &m_recInput);
#ifdef EXT_DEBUG
			cerr << "m_roster->Connect, Error Code: " << strerror(mError) << endl;
#endif
			if ( mError == B_OK )
			{
				//	Find out what the time source of the input is.
				media_node use_time_source;
				BTimeSource * tsobj = m_roster->MakeTimeSourceFor(m_audioInputNode);
				if( tsobj )
				{
					mError = m_roster->SetTimeSourceFor(Node().node, tsobj->Node().node);
				}
	
				//	Start the time source if it's not running.
				if ((tsobj->Node() != m_audioInputNode) && !tsobj->IsRunning()) {
					m_roster->StartNode(tsobj->Node(), BTimeSource::RealTime());
				}
				tsobj->Release();	//	we're done with this time source instance!
		
				// Reset FIFO
				mFIFO.Reset();

				// Reset resampler
				memoryL = 0; memoryR = 0; mp = 0; mt = (int32) fmt.u.raw_audio.frame_rate;
				
				// Start
				bigtime_t then = TimeSource()->Now()+50000LL;
				mError = m_roster->StartNode(Node(), then);
#ifdef EXT_DEBUG
				cerr << "m_roster->StartNode, Error Code: " << strerror(mError) << endl;
#endif
				if (m_audioInputNode.kind & B_TIME_SOURCE) 
				{
					mError = m_roster->StartNode(m_audioInputNode, TimeSource()->RealTimeFor(then, 0));
				}
				else 
				{
					mError = m_roster->StartNode(m_audioInputNode, then);
 				}
			}
		}
	}

	return mError == B_OK; 
}

bool PSoundInput::StopRecording()
{
	if( !mfRecording )
		return false;
		
	mfRecording = false;

	m_roster = BMediaRoster::CurrentRoster();

	//	If we are the last connection, the Node will stop automatically since it
	//	has nowhere to send data to.
	mError = m_roster->StopNode(m_recInput.node, 0);
	mError = m_roster->Disconnect(m_audioOutput.node.node, m_audioOutput.source, m_recInput.node.node, m_recInput.destination);
	m_audioOutput.source = media_source::null;
	m_recInput.destination = media_destination::null;

	return mError == B_OK;
}

PSoundInput::~PSoundInput() 
{
	StopRecording();

	status_t status;
	mFIFO.CopyNextBufferIn(&status, 0, B_INFINITE_TIMEOUT, true);
}

void PSoundInput::Notify(int32 code, ...)
{
	assert( code < (int32) (sizeof(gsNotifyCodes) / sizeof(char*)) );
#ifdef EXT_DEBUG
	cerr << "Notify received, Code: " << gsNotifyCodes[code] << endl;
#endif
}

void PSoundInput::Record(bigtime_t /* time */,
	const void * data, size_t size,
	const media_raw_audio_format & fmt )
{
		
#ifdef EXT_DEBUG
	cerr << "\tBuffer received. Size: " << size << \
		", sample size: " << ((fmt.format & 0x0f) * 8) << \
		", float? " << ((fmt.format == media_raw_audio_format::B_AUDIO_FLOAT)? 1 : 0) << \
		", channels: " << fmt.channel_count << \
		", rate: " << fmt.frame_rate << \
		endl;
#endif

	if( size )
	{
		short* temp = new short[ size /sizeof(short) ];
		if( temp )
		{ 
			memcpy( (void*)temp, data, size );
			size = Resample(temp, size);
			status_t err = mFIFO.CopyNextBufferIn(temp, size, B_INFINITE_TIMEOUT, false);
			if (err < (int32)size) {
				fprintf(stderr, "Error while PSoundInput::Record: %s; bailing\n", strerror(err));
				fprintf(stderr, "\tCopyNextBufferIn(%p, %ld, B_INFINITE_TIMEOUT, false) failed with %ld.\n",
					data, size, err);
				StopRecording();
			}
			delete[] temp;
		}
	}
}

bool PSoundInput::Read( void * buf, uint32 len )
{
	status_t err = mFIFO.CopyNextBlockOut(buf, len, B_INFINITE_TIMEOUT);
	if (err < (int32) len ) {
		return false;
	}

	return true;
}

int PSoundInput::Resample(short * in, int inSize) 
{
	int c = 0;
	short * out = in;
	inSize /= 4; // (stereo, 16 bit in and out)

	while (inSize > 0) {
		while (mp < mt) {
			memoryL = ((memoryL*7) + *(in++) >>3);
			memoryR = ((memoryR*7) + *(in++) >> 3);
			mp += 8000;
			inSize--;
			if (inSize < 1) goto done;
		}

		*out++ = memoryL;// >> 3;
		*out++ = memoryR;// >> 3;

		mp -= mt;
		c++;
	}
done:
	return c*4; // (stereo, 16-bit)
}

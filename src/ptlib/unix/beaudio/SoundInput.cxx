//
// (c) Yuri Kiryanov, home.att.net/~bevox
// for www.Openh323.org
//
// Portions Be, Inc.
//
// resample needed some stuff
#include "SoundInput.h"

#define MAX_SOUND_FILE_SIZE (3 * 1024 * 1024)

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

PSoundInput::PSoundInput(PSoundChannel* pChannel) :
	SoundConsumer("PWLSoundInput", NULL, NULL, pChannel), 
			BMediaNode("PWLSoundInput"), mpChannel(pChannel), 
			mpSound(NULL), bytesToWrite(0)
{ 
	m_roster = BMediaRoster::Roster(&mError);
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
	//	Find an available output for the given input node.
	int32 count = 0;
	mError = m_roster->GetFreeOutputsFor(m_audioInputNode, 
			&m_audioOutput, 1, &count, B_MEDIA_RAW_AUDIO);
	if (mError == B_OK && count >= 0 ) 
	{
		//	Find an available input for our own Node. 
		mError = m_roster->GetFreeInputsFor(Node(), &m_recInput, 1, &count, B_MEDIA_RAW_AUDIO);
		if (mError == B_OK && count >=0 ) 
		{
			//	Get a format, any format.
			media_format fmt;
			fmt.u.raw_audio = m_audioOutput.format.u.raw_audio;
			fmt.type = B_MEDIA_RAW_AUDIO;

			// Resample this rate to 8000
			mResampler.Create(fmt.u.raw_audio.frame_rate, 8000,
				fmt.u.raw_audio.channel_count, (fmt.u.raw_audio.format & 0xf)*8);
	
			//	Using the same structs for input and output is OK in BMediaRoster::Connect().
			mError = m_roster->Connect(m_audioOutput.source, 
					m_recInput.destination, &fmt, &m_audioOutput, &m_recInput);
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
		
				// Start
				bigtime_t then = TimeSource()->Now()+50000LL;
				m_roster->StartNode(Node(), then);
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
	m_roster = BMediaRoster::Roster(&mError);

	//	If we are the last connection, the Node will stop automatically since it
	//	has nowhere to send data to.
	mError = m_roster->StopNode(m_recInput.node, 0);
	mError = m_roster->Disconnect(m_audioOutput.node.node, m_audioOutput.source, m_recInput.node.node, m_recInput.destination);
	m_audioOutput.source = media_source::null;
	m_recInput.destination = media_destination::null;

	return mError == B_OK;
}

PSoundInput::~PSoundInput() {
	
	m_roster = BMediaRoster::Roster(&mError);
	mError = m_roster->StopNode(m_recInput.node, 0);
	mError = m_roster->Disconnect(m_audioOutput.node.node, 
			m_audioOutput.source, m_recInput.node.node, m_recInput.destination);
	m_audioOutput.source = media_source::null;
	m_recInput.destination = media_destination::null;

//	Release();
	mError = m_roster->UnregisterNode(this);
}

void PSoundInput::Notify(int32 code, ...)
{
	PAssertNULL(mpChannel);
	PAssert( code < (int32) (sizeof(gsNotifyCodes) / sizeof(char*)), PInvalidParameter );
#if EXT_DEBUG
	PError << "Notify received, Code: " << gsNotifyCodes[code] << endl;
#endif
}

void PSoundInput::Record(bigtime_t /* time */,
	const void * data, size_t size,
	const media_raw_audio_format & fmt )
{
#if EXT_DEBUG
	PError << "\tBuffer received. Size: " << size << \
		", sample size: " << ((fmt.format & 0x0f) * 8) << \
		", float? " << ((fmt.format == media_raw_audio_format::B_AUDIO_FLOAT)? 1 : 0) << \
		", channels: " << fmt.channel_count << \
		", rate: " << fmt.frame_rate << \
		endl;
#endif

	PSound* pSound = new PSound;
	PAssertNULL(pSound);
	pSound->SetFormat(fmt.channel_count, fmt.frame_rate, (fmt.format & 0xf) * 8); 
	pSound->SetSize(size * (fmt.format & 0xf)); // 16bit, 2 channels

	int16* pd = (int16*) data;
	int16* ps = (int16*) pSound->GetPointer(); 
	memcpy( ps, pd, size );
	mResampler.Resample(*pSound);

	PMutex lock;
	lock.Wait();
	mSounds.Enqueue(pSound);
	if ( (pSound->GetSize() * mSounds.GetSize()) > MAX_SOUND_FILE_SIZE )
	{
		delete mSounds.Dequeue();
		PError << "Input buffer overflow!" << endl;
		StopRecording();
	}
	lock.Signal();
}

bool PSoundInput::GetBuffer( void * buf, PINDEX len, bool fRelease = true )
{
	PMutex lock;
	lock.Wait();
		
	if( !mpSound  )
	{	
getSound:	
		if( mSounds.GetSize() )
		{
			mpSound = mSounds.Dequeue();
		}
		else
		{
			mpSound = NULL; // Nothing in buffer
			bytesToWrite = 0;
		}
		
		if( mpSound )
			bytesToWrite = mpSound->GetSize();
	}
	
	if( mpSound )
	{
		if( len <= bytesToWrite ) 
		{
			memcpy(buf, mpSound->GetPointer(), len);
			bytesToWrite -= len;
			
#ifdef EXT_DEBUG
			PError << "Buffer returned, size: " << len << ", left:" << bytesToWrite << endl;
#endif			
			if ( bytesToWrite > 0 )
				goto getSound;	
			else
			{
				delete mpSound;
				mpSound = NULL;
				bytesToWrite = 0;
			}
		}
		else
		{
			goto getSound;	
		}		
	}
	lock.Signal();

	return mpSound != NULL;
}


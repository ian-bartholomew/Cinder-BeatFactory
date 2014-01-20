//
//  BeatFactory.cpp
//  BeatFactoryMain
//
//  Created by Ian Bartholomew on 1/18/14.
//
//

#include "BeatFactory.h"

BeatFactoryRef BeatFactory::create(){
    return BeatFactoryRef( new BeatFactory() );
    
}

BeatFactory::BeatFactory(){}

BeatFactory::~BeatFactory()
{
    stop();
    destroy();
}

void BeatFactory::loadAudio(DataSourceRef dataSource)
{
    mAudioSource = audio::load( dataSource );
}

void BeatFactory::setup()
{
    initTrack();
    initODF();
}

void BeatFactory::update()
{
    // Check if track is playing and has a PCM buffer available
	if ( mTrack->isPlaying() && mTrack->isPcmBuffering() ) {
        
		// Get buffer
		mBuffer = mTrack->getPcmBuffer();
		if ( mBuffer && mBuffer->getInterleavedData() ) {
            
			// Get sample count
			uint32_t sampleCount = mBuffer->getInterleavedData()->mSampleCount;
			if ( sampleCount > 0 ) {
                
				// Initialize analyzer
				if ( !mFft ) {
					mFft = Kiss::create( sampleCount );
				}
                
				// Analyze data
				if ( mBuffer->getInterleavedData()->mData != 0 ) {
					mFft->setData( mBuffer->getInterleavedData()->mData );
				}
                
                float * freqData = mFft->getAmplitude();
                int32_t dataSize = mFft->getBinSize();
                float * fftData = mFft->getData();
                
                mAmp = 0;
                for (int32_t i = 0; i<dataSize; i++){
                    mAmp += freqData[i];
                }
                mAmp /= dataSize;
                
                
                // compute the energy level
                float level = 0;
                for (int i = 0; i < dataSize; i++)
                {
                    level += (fftData[i] * fftData[i]);
                }
                level /= dataSize;
                level = (float) sqrt(level);
                float instant = level * 100;
                
                mInstant = lmap(instant, 0.0f, 100.0f, 0.0f, 1.0f);
                
                
                bIsOnset = onsetsds_process(&mOds, fftData);
                
			}
            
		}
        
	}
}

float BeatFactory::getAmp()
{
    return mAmp;
}

float BeatFactory::getInstant()
{
    return mInstant;
}

bool BeatFactory::isOnset()
{
    return bIsOnset;
}

void BeatFactory::stop()
{
    // Stop track
	mTrack->enablePcmBuffering( false );
	mTrack->stop();
	if ( mFft ) {
		mFft->stop();
	}
}

void BeatFactory::destroy()
{
    
}

void BeatFactory::initTrack()
{
    mTrack = audio::Output::addTrack( mAudioSource, false );
	mTrack->enablePcmBuffering( true );
	mTrack->play();
}

void BeatFactory::initODF(){
    // There are various types of onset detector available, we must choose one
    //    onsetsds_odf_types odftype = onsetsds_odf_types::ODS_ODF_WPHASE;  // good for bass and low end
    onsetsds_odf_types odftype = onsetsds_odf_types::ODS_ODF_MAGSUM;
    
    // Allocate contiguous memory using malloc or whatever is reasonable.
    // FFT size of 1024 (@44.1kHz), and a median span of 11.
    float* odsdata = (float*) malloc( onsetsds_memneeded(odftype, 1024, 11) );
    
    // Now initialise the OnsetsDS struct and its associated memory
    onsetsds_init(&mOds, odsdata, onsetsds_fft_types::ODS_FFT_SC3_POLAR, odftype, 1024, 11, 44100.0f);
    
    // start our timer for the damper
    mTimer = currentTimeInMillis();
}

long BeatFactory::currentTimeInMillis(){
    // get system time in milliseconds
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
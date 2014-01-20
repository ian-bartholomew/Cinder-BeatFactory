//
//  BeatFactory.h
//  BeatFactoryMain
//
//  Created by Ian Bartholomew on 1/18/14.
//
//

#pragma once

#include <KissFft.h>
#include "onsetsds.h"

#include "cinder/audio/Io.h"
#include "cinder/audio/Output.h"
#include "cinder/audio/PcmBuffer.h"

using namespace ci;
using namespace std;

typedef std::shared_ptr<class BeatFactory> BeatFactoryRef;

class BeatFactory {
  public:
    ~BeatFactory();
    
    static BeatFactoryRef create();

    void loadAudio(DataSourceRef dataSource);
    void setup();
    void update();
    bool isOnset();
    void stop();
    
    KissRef             mFft;
    
  private:
    BeatFactory();
    
    void destroy();
    
    // inits
    void initTrack();
    void initODF();
    
    long currentTimeInMillis();
    
    // Audio file
	ci::audio::SourceRef		mAudioSource;
	ci::audio::PcmBuffer32fRef	mBuffer;
	ci::audio::TrackRef			mTrack;

    long                        mTimer;
    double                      mAmp;
    float                       mInstant;
    
    bool                        bIsOnset     = false;
    
    // Onset Dection
    OnsetsDS                    mOds;
};
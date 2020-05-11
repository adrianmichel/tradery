/*
   Copyright (C) 2018-2020 Adrian Michel

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "stdafx.h"
#include "cache.h"
#include "seriesimpl.h"
#include "bars.h"
#include "indicators.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern SeriesCache* _cache;

const Series BarsImpl::TrueRange() const {
  return _cache->findAndAdd(MakeTrueRangeSeries(*this));
}
const Series BarsImpl::AccumDist() const {
  return _cache->findAndAdd(MakeAccumDistSeries(*this));
}
const Series BarsImpl::ADX(unsigned int period) const {
  return _cache->findAndAdd(MakeADXSeries(*this, period));
}
const Series BarsImpl::MidPrice(unsigned int period) const {
  return _cache->findAndAdd(MakeMidPriceSeries(*this, period));
}
const Series BarsImpl::SAR(double acceleration, double maximum) const {
  return _cache->findAndAdd(MakeSARSeries(*this, acceleration, maximum));
}
const Series BarsImpl::MFI(unsigned int period) const {
  return _cache->findAndAdd(MakeMFISeries(*this, period));
}
const Series BarsImpl::CCI(unsigned int period) const {
  return _cache->findAndAdd(MakeCCISeries(*this, period));
}
const Series BarsImpl::DX(unsigned int period) const {
  return _cache->findAndAdd(MakeDXSeries(*this, period));
}
const Series BarsImpl::PlusDI(unsigned int period) const {
  return _cache->findAndAdd(MakePlusDISeries(*this, period));
}
const Series BarsImpl::PlusDM(unsigned int period) const {
  return _cache->findAndAdd(MakePlusDMSeries(*this, period));
}
const Series BarsImpl::WillR(unsigned int period) const {
  return _cache->findAndAdd(MakeWillRSeries(*this, period));
}
const Series BarsImpl::NATR(unsigned int period) const {
  return _cache->findAndAdd(MakeNATRSeries(*this, period));
}
const Series BarsImpl::ChaikinAD() const {
  return _cache->findAndAdd(MakeChaikinADSeries(*this));
}
const Series BarsImpl::ChaikinADOscillator(unsigned int fastPeriod, unsigned int slowPeriod) const {
  return _cache->findAndAdd(MakeChaikinADOscillatorSeries(*this, fastPeriod, slowPeriod));
}
const Series BarsImpl::OBV(const Series& series) const {
  return _cache->findAndAdd(MakeOBVSeries(*this, series));
}
const Series BarsImpl::AvgPrice() const {
  return _cache->findAndAdd(MakeAvgPriceSeries(*this));
}
const Series BarsImpl::MedPrice() const {
  return _cache->findAndAdd(MakeMedPriceSeries(*this));
}
const Series BarsImpl::TypPrice() const {
  return _cache->findAndAdd(MakeTypPriceSeries(*this));
}
const Series BarsImpl::WclPrice() const {
  return _cache->findAndAdd(MakeWclPriceSeries(*this));
}
const Series BarsImpl::MinusDI(unsigned int period) const {
  return _cache->findAndAdd(MakeMinusDISeries(*this, period));
}
const Series BarsImpl::MinusDM(unsigned int period) const {
  return _cache->findAndAdd(MakeMinusDMSeries(*this, period));
}
const Series BarsImpl::ADXR(unsigned int period) const {
  return _cache->findAndAdd(MakeADXRSeries(*this, period));
}
const Series BarsImpl::ATR(unsigned int period) const {
  return _cache->findAndAdd(MakeATRSeries(*this, period));
}
const Series BarsImpl::TR() const {
  return _cache->findAndAdd(MakeTRSeries(*this));
}
const Series BarsImpl::StochSlowK(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const {
  return _cache->findAndAdd(MakeStochSlowKSeries(*this, fastKPeriod, slowKPeriod, slowKMAType, slowDPeriod, slowDMAType));
}
const Series BarsImpl::StochSlowD(int fastKPeriod, int slowKPeriod, MAType slowKMAType, int slowDPeriod, MAType slowDMAType) const {
  return _cache->findAndAdd(MakeStochSlowDSeries(*this, fastKPeriod, slowKPeriod, slowKMAType, slowDPeriod, slowDMAType));
}
const Series BarsImpl::StochFastK(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const {
  return _cache->findAndAdd(MakeStochFastKSeries(*this, fastKPeriod, fastDPeriod, fastDMAType));
}
const Series BarsImpl::StochFastD(int fastKPeriod, int fastDPeriod, MAType fastDMAType) const {
  return _cache->findAndAdd(MakeStochFastDSeries(*this, fastKPeriod, fastDPeriod, fastDMAType));
}

const Series BarsImpl::BOP() const {
  return _cache->findAndAdd(MakeBOPSeries(*this));
}

const Series BarsImpl::Cdl3BlackCrows() const {
  return _cache->findAndAdd(MakeCdl3BlackCrows(*this));
}
const Series BarsImpl::CdlAbandonedBaby(double penetration) const {
  return _cache->findAndAdd(MakeCdlAbandonedBaby(*this, penetration));
}
const Series BarsImpl::CdlDojiStar() const {
  return _cache->findAndAdd(MakeCdlDojiStar(*this));
}
const Series BarsImpl::CdlEngulfing() const {
  return _cache->findAndAdd(MakeCdlEngulfing(*this));
}
const Series BarsImpl::CdlEveningDojiStar(double penetration) const {
  return _cache->findAndAdd(MakeCdlEveningDojiStar(*this, penetration));
}
const Series BarsImpl::CdlEveningStar(double penetration) const {
  return _cache->findAndAdd(MakeCdlEveningStar(*this, penetration));
}
const Series BarsImpl::CdlHammer() const {
  return _cache->findAndAdd(MakeCdlHammer(*this));
}
const Series BarsImpl::CdlHangingMan() const {
  return _cache->findAndAdd(MakeCdlHangingMan(*this));
}
const Series BarsImpl::CdlHarami() const {
  return _cache->findAndAdd(MakeCdlHarami(*this));
}
const Series BarsImpl::CdlHaramiCross() const {
  return _cache->findAndAdd(MakeCdlHaramiCross(*this));
}
const Series BarsImpl::CdlHighWave() const {
  return _cache->findAndAdd(MakeCdlHighWave(*this));
}
const Series BarsImpl::CdlIdentical3Crows() const {
  return _cache->findAndAdd(MakeCdlIdentical3Crows(*this));
}
const Series BarsImpl::CdlInvertedHammer() const {
  return _cache->findAndAdd(MakeCdlInvertedHammer(*this));
}
const Series BarsImpl::CdlLongLine() const {
  return _cache->findAndAdd(MakeCdlLongLine(*this));
}
const Series BarsImpl::CdlMorningDojiStar(double penetration) const {
  return _cache->findAndAdd(MakeCdlMorningDojiStar(*this, penetration));
}
const Series BarsImpl::CdlMorningStar(double penetration) const {
  return _cache->findAndAdd(MakeCdlMorningStar(*this, penetration));
}
const Series BarsImpl::CdlShootingStar() const {
  return _cache->findAndAdd(MakeCdlShootingStar(*this));
}
const Series BarsImpl::CdlShortLine() const {
  return _cache->findAndAdd(MakeCdlShortLine(*this));
}
const Series BarsImpl::CdlSpinningTop() const {
  return _cache->findAndAdd(MakeCdlSpinningTop(*this));
}
const Series BarsImpl::CdlTristar() const {
  return _cache->findAndAdd(MakeCdlTristar(*this));
}
const Series BarsImpl::CdlUpsideGap2Crows() const {
  return _cache->findAndAdd(MakeCdlUpsideGap2Crows(*this));
}
const Series BarsImpl::CdlHikkake() const {
  return _cache->findAndAdd(MakeCdlHikkake(*this));
}

// ta-lib 1.3
const Series BarsImpl::Cdl2Crows() const {
  return _cache->findAndAdd(MakeCdl2Crows(*this));
}
const Series BarsImpl::Cdl3Inside() const {
  return _cache->findAndAdd(MakeCdl3Inside(*this));
}
const Series BarsImpl::Cdl3LineStrike() const {
  return _cache->findAndAdd(MakeCdl3LineStrike(*this));
}
const Series BarsImpl::Cdl3WhiteSoldiers() const {
  return _cache->findAndAdd(MakeCdl3WhiteSoldiers(*this));
}
const Series BarsImpl::Cdl3Outside() const {
  return _cache->findAndAdd(MakeCdl3Outside(*this));
}
const Series BarsImpl::CdlInNeck() const {
  return _cache->findAndAdd(MakeCdlInNeck(*this));
}
const Series BarsImpl::CdlOnNeck() const {
  return _cache->findAndAdd(MakeCdlOnNeck(*this));
}
const Series BarsImpl::CdlPiercing() const {
  return _cache->findAndAdd(MakeCdlPiercing(*this));
}
const Series BarsImpl::CdlStalled() const {
  return _cache->findAndAdd(MakeCdlStalled(*this));
}
const Series BarsImpl::CdlThrusting() const {
  return _cache->findAndAdd(MakeCdlThrusting(*this));
}
const Series BarsImpl::CdlAdvanceBlock(double penetration) const {
  return _cache->findAndAdd(MakeCdlAdvanceBlock(*this, penetration));
}
// ta-lib 0.1.4

const Series BarsImpl::Cdl3StarsInSouth() const {
  return _cache->findAndAdd(MakeCdl3StarsInSouth(*this));
}
const Series BarsImpl::CdlBeltHold() const {
  return _cache->findAndAdd(MakeCdlBeltHold(*this));
}
const Series BarsImpl::CdlBreakaway() const {
  return _cache->findAndAdd(MakeCdlBreakaway(*this));
}
const Series BarsImpl::CdlClosingMarubuzu() const {
  return _cache->findAndAdd(MakeCdlClosingMarubuzu(*this));
}
const Series BarsImpl::CdlConcealingBabySwallow() const {
  return _cache->findAndAdd(MakeCdlConcealingBabySwallow(*this));
}
const Series BarsImpl::CdlCounterattack() const {
  return _cache->findAndAdd(MakeCdlCounterattack(*this));
}
const Series BarsImpl::CdlDoji() const {
  return _cache->findAndAdd(MakeCdlDoji(*this));
}
const Series BarsImpl::CdlDragonFlyDoji() const {
  return _cache->findAndAdd(MakeCdlDragonFlyDoji(*this));
}
const Series BarsImpl::CdlGapSideBySideWhite() const {
  return _cache->findAndAdd(MakeCdlGapSideBySideWhite(*this));
}
const Series BarsImpl::CdlGravestoneDoji() const {
  return _cache->findAndAdd(MakeCdlGravestoneDoji(*this));
}
const Series BarsImpl::CdlHomingPigeon() const {
  return _cache->findAndAdd(MakeCdlHomingPigeon(*this));
}
const Series BarsImpl::CdlKicking() const {
  return _cache->findAndAdd(MakeCdlKicking(*this));
}
const Series BarsImpl::CdlKickingByLength() const {
  return _cache->findAndAdd(MakeCdlKickingByLength(*this));
}
const Series BarsImpl::CdlLadderBottom() const {
  return _cache->findAndAdd(MakeCdlLadderBottom(*this));
}
const Series BarsImpl::CdlLongLeggedDoji() const {
  return _cache->findAndAdd(MakeCdlLongLeggedDoji(*this));
}
const Series BarsImpl::CdlMarubozu() const {
  return _cache->findAndAdd(MakeCdlMarubozu(*this));
}
const Series BarsImpl::CdlMatchingLow() const {
  return _cache->findAndAdd(MakeCdlMatchingLow(*this));
}
const Series BarsImpl::CdlRickshawMan() const {
  return _cache->findAndAdd(MakeCdlRickshawMan(*this));
}
const Series BarsImpl::CdlRiseFall3Methods() const {
  return _cache->findAndAdd(MakeCdlRiseFall3Methods(*this));
}
const Series BarsImpl::CdlSeparatingLines() const {
  return _cache->findAndAdd(MakeCdlSeparatingLines(*this));
}
const Series BarsImpl::CdlStickSandwich() const {
  return _cache->findAndAdd(MakeCdlStickSandwich(*this));
}
const Series BarsImpl::CdlTakuri() const {
  return _cache->findAndAdd(MakeCdlTakuri(*this));
}
const Series BarsImpl::CdlTasukiGap() const {
  return _cache->findAndAdd(MakeCdlTasukiGap(*this));
}
const Series BarsImpl::CdlUnique3River() const {
  return _cache->findAndAdd(MakeCdlUnique3River(*this));
}
const Series BarsImpl::CdlXSideGap3Methods() const {
  return _cache->findAndAdd(MakeCdlXSideGap3Methods(*this));
}

const Series BarsImpl::CdlMatHold(double penetration) const {
  return _cache->findAndAdd(MakeCdlMatHold(*this, penetration));
}

class SynchronizerImpl : public Synchronizer {
 private:
  std::vector<int> _syncVector;
  std::string _refSymbol;
  TimeSeries _ts;
  bool _modified;
  // the bars collection we're synchronizing with

 private:
  int index(size_t ix) const {
    if (ix < _syncVector.size()) {
      return _syncVector[ix];
    }
    else {
      throw SynchronizedSeriesIndexOutOfRangeException(_syncVector.size(), ix);
    }
  }

  // indicates whether a sync required modification
  // if not modified, then the 2 bars were the same in terms
  // of bar indexes and timestamps
  bool modified() const override { return _modified; }

  size_t size() const override { return _syncVector.size(); }

  void insertData(size_t refIx, size_t syncedIx) {
    assert(refIx < _syncVector.size());
    assert(_syncVector[refIx] == -1);

    //    COUT << "inserting: " << ( refIx + 1 ) << _T( " -> " ) << ( syncedIx +
    //    1 )<< std::endl;

    _syncVector[refIx] = syncedIx;
  }

 public:
  // synchronizes bars2 to bars1 (the base is bars1)
  SynchronizerImpl(Bars ref, Bars synced)
      : _syncVector(ref.size(), -1),
        _refSymbol(ref.getSymbol()),
        _ts(ref.timeSeries()),
        _modified(false) {
    assert(synced.unsyncSize() > 0);
    size_t lastSynced = 0;

    for (size_t indexSynced = 0, indexRef = 0; indexRef < ref.size();) {
      std::string refDate = ref.time(indexRef).date().to_simple_string();
      std::string syncedDate =
          synced.time(indexSynced).date().to_simple_string();

      if (ref.time(indexRef) == synced.time(indexSynced)) {
        // the 2 times are equal - insert the data
        insertData(indexRef++, indexSynced);
        // increment unsynced if not at the end already
        lastSynced = indexSynced;
        if (indexSynced < (synced.unsyncSize() - 1)) {
          indexSynced++;
        }
      }
      else if (ref.time(indexRef) > synced.time(indexSynced)) {
        // ref time > synced time

        // only insert the index if it's the last synchronized bar - then we
        // need to sync to it. otherwise, continue until the ref time is <
        // synced time, then we use the last synced index
        lastSynced = indexSynced;
        if (indexSynced < (synced.unsyncSize() - 1)) {
          indexSynced++;
        }
        else {
          insertData(indexRef++, lastSynced);
        }
        _modified = true;
      }
      else if (ref.time(indexRef) < synced.time(indexSynced)) {
        // if ref < synced, then we insert the last good synced index.
        insertData(indexRef++, lastSynced);
        _modified = true;
      }
    }

      // the whole sync vector must be initialized
      //    assert( std::find( _syncVector.begin(), _syncVector.end(), -1 ) ==
      //    _syncVector.end() );
  }

  TimeSeries timeSeries() const override {
    return _ts;
  }

  bool operator==(const Synchronizer& synchronizer) const override {
    return to_lower_case(synchronizer.refSymbol()) == to_lower_case(refSymbol());
  }

  bool operator==(const Synchronizer* synchronizer) const override {
    return synchronizer == 0 ? false : (*this == *synchronizer);
  }

  const std::string& refSymbol() const override { return _refSymbol; }
};

CORE_API Synchronizer::SynchronizerPtr Synchronizer::create(Bars ref, Bars syncd) {
  return std::make_shared< SynchronizerImpl >(ref, syncd);
}

/*
Copyright (C) 2018 Adrian Michel
http://www.amichel.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

namespace cpp tradery_thrift_api

typedef string ID
typedef i64 Date

enum PositionSizeType {
	SYSTEM,
	SHARES,
	VALUE,
	PCT_EQUITY,
	PCT_CASH
}

enum PositionSizeLimitType {
	NONE,
	PCT_VOL,
	VALUE
}

struct Range {
	1: Date startDate;
	2: Date endDate;
}

enum DataErrorHandling {
	FATAL,
	WARNING,
	NONE	//equivalent IGNORE, just can't use that keyword
}

struct PositionSizing {
	1: double initialCapital;
	2: i32 maxOpenPositions;
	3: PositionSizeType positionSizeType;
	4: double positionSize;
	5: PositionSizeLimitType positionSizeLimitType;
	6: double positionSizeLimit;
}

struct System {
	1: string dbId;
	2: string name;
	3: string description;
	4: string code;
}

struct SessionParams {
	2: string explicitTradesExt;
	3: list< string > symbols;
	4: list< System > systems;
	5: bool generateSignals;
	6: bool generateTrades;
	7: bool generateStats;
	8: bool generateOutput;
	9: bool generateEquityCurve;
	10: bool generateCharts;
	11: bool generateZipFile;
	12: i32 maxChartsCount;
	13: i32 timeout; //?
	14: i32 symbolTimeout;
	15: i32 heartBeatTimeout;
	16: i32 reverseHeartBeatTimeout;
	17: PositionSizing positionSizing;
	18: double slippage;
	19: double commission;
	20: Range range;
	21: DataErrorHandling dataErrorHandling;
	22: i32 maxLines //?
	23: Date startTradesDate;
}

enum RuntimeStatus {
	READY,
	RUNNING,
	CANCELING,
	ENDED,
	CANCELED
}

struct RuntimeStats {
	1: ID sessionId;
	2: i32 duration;
	3: i32 processedSymbolCount;
	4: i32 symbolProcessedWithErrorsCount;
	5: i32 totalSymbolCount;
	6: i32 systemCount;
	7: i32 rawTradeCount;
	8: i32 processedTradeCount;
	9: i32 signalCount;
	10: i32 processedSignalCount;
	11: i32 totalBarCount;
	12: i32 totalRuns;
	13: i32 errorCount;
	14: double percentageDone;
	15: string currentSymbol;
	16: RuntimeStatus status;
	17: string message;
}

service Tradery {
	ID startSession( 1: SessionParams sessionParams );
	void cancelSession( 1: ID sessionId );
	bool heartbeat( 1: ID sessionId );
	RuntimeStats getRuntimeStats( 1: ID sessionId );
}
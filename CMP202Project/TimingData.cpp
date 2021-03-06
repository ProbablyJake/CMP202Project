#include "TimingData.h"

TimingData::TimingData(string name)
	:name(name),  timingCount(0), timeLock(timeMutex,std::defer_lock)
{
	auto timeStart = system_clock::to_time_t(system_clock::now()); // current time to create unique file name (ensures no accidental overwite)
	filename = "Timings\\" + name + to_string(timeStart) + ".csv";

	file.open(filename);

	
}


TimingData::~TimingData()
{
	TryClose();
}

void TimingData::ManualTimingStart(int numberOfTimings)
{
	timeLock.lock();
	beginTime = high_resolution_clock::now();
	elapsedTime = duration_cast<duration<double>>(beginTime - beginTime); // 0
	if (timingCount == 0) {
		timingCount = numberOfTimings;
	}
}

void TimingData::ManualTimingStop(bool fast)
{
	
	timingCount--;
	endTime = high_resolution_clock::now();
	elapsedTime += duration_cast<duration<double>>(endTime - beginTime);
	if (timingCount ==0) {
		timings.push_back(elapsedTime.count());
		timingResult = to_string(elapsedTime.count()) + ',';
	}
	
	timeLock.unlock();


	if ( !fast && timingCount == 0 ) {
		lock_guard<mutex> lk(fileWriteMutex); // raii, this next step might take a long time so seperate mutex lock
		file << timingResult;
	}
}

void TimingData::EndTiming()
{
	SortTimings();
	RecordSortedList();
	CalculateMedianTime();
	RecordTimePerExecution();
	TryClose();
}






double TimingData::CalculateMedianTime()
{

	
	// already sorted

	medianTiming = timings[timings.size() / (int)2]; // get the middle time

	Record(to_string(medianTiming)); // record the time

	return medianTiming;

}

void TimingData::Record(string data)
{
	TryOpen();
	file << data <<',' << endl ;
}

void TimingData::RecordSortedList()
{
	for (auto s : timings) {
		file << s << ',';
	}
	file << endl ;
}

void TimingData::RecordTimePerExecution()
{
	// this will only work with excel
	file << ',' << endl
		<< endl
		<< "=3:3/A2" << ',' << endl // row of sorted times / number of calls per timing
		<< "=A4/A2" << ',' << endl // median / number of calls per timing
		<< "=A8*1000000000" << ',' << "nano-seconds per execution, average (median)" << ','<< endl; // seconds -> nanoseconds

}

void TimingData::OutputProgress(float progress, float total)
{
	float percent = 100.f * (progress / total);
	string outputPercent = to_string(percent);
	outputPercent = outputPercent.substr(0, 5);
	cout << "\b";cout << "\b";cout << "\b";cout << "\b";cout << "\b";cout << "\b"; // XX.XX%
	cout << outputPercent << "%";

}

void TimingData::OuputEnd()
{
	cout << "\b"; cout << "\b"; cout << "\b"; cout << "\b"; cout << "\b"; cout << "\b";  // XX.XX%
	cout << "100.00%" << endl;	
}

void TimingData::OutputBegin()
{
	cout << "Timing for: " << name << endl;
}

#ifndef TimeScope_h__
#define TimeScope_h__

#include <chrono>
#include <QString>

class TimeScope {
public:
	TimeScope(const QString& inDesc)
		:mDesc(inDesc) {
		mStartTime = std::chrono::steady_clock::now();
	}
	~TimeScope() {
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		double durationNs = std::chrono::duration<double, std::nano>(currentTime - mStartTime).count();
		qDebug() << mDesc << " Cost: " << durationNs << " Ns";
	}
private:
	QString mDesc;
	std::chrono::steady_clock::time_point mStartTime;
};

#endif // TimeScope_h__

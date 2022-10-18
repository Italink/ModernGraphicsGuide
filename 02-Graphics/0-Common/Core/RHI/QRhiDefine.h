#ifndef QRhiInclude_h__
#define QRhiInclude_h__

#include <memory>
#include "private/qrhi_p.h"

template<typename _Ty>
class QRhiSPtr :public std::shared_ptr<_Ty> {
public:
	void reset(_Ty* res) noexcept {
		if (res != nullptr) {
			std::shared_ptr<_Ty>::reset(res, [](_Ty* res) {
				res->destroy();
			});
		}
		else {
			std::shared_ptr<_Ty>::reset();
		}
	}
};

class QRhiSignal {
public:
	QRhiSignal(bool var = false) :bSignal(var) {};
	void active() {
		bSignal = true;
	}
	bool receive() {
		bool var = bSignal;
		bSignal = false;
		return var;
	}
private:
	bool bSignal = false;
};


#endif // QRhiInclude_h__

#ifndef QRhiInclude_h__
#define QRhiInclude_h__

#include <memory>
#include "private/qrhi_p.h"

template <typename T>
struct QRhiScopedPointerDeleter {
	static inline void cleanup(T* pointer) noexcept {
		if (pointer)
			pointer->destroy();
	}
	void operator()(T* pointer) const noexcept {
		cleanup(pointer);
	}
};

template<typename T>
using QRhiScopedPointer = QScopedPointer<T, QRhiScopedPointerDeleter<T>>;

class QDirtySignal {
public:
	QDirtySignal(bool var = false){};
	void mark() {
		bDirty = true;
	}
	bool handle() {
		bool var = bDirty;
		bDirty = false;
		return var;
	}
private:
	bool bDirty = false;
};


#endif // QRhiInclude_h__

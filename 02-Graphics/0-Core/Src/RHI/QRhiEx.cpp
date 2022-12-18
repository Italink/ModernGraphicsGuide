#include "QRhiEx.h"
#include <QFile>
#include "private\qshaderbaker_p.h"
#include "private\qrhivulkan_p.h"

#ifndef QT_NO_OPENGL
#include <QOffscreenSurface>
#include "private\qrhigles2_p.h"
#endif

#ifdef Q_OS_WIN
#include <QtGui/private/qrhid3d11_p.h>
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#include <QtGui/private/qrhimetal_p.h>
#endif

#include "QVulkanInstance"
#include <QtGui/private/qrhinull_p.h>
#include "Customization\QDetailWidgetManager.h"
#include "DetailCustomization\QInstacneDetail_QRhiUniformBlock.h"

#include "private/qrhivulkanext_p.h"

#include <qmath.h>
#include <QVulkanFunctions>

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_RECORDING_ENABLED 0
#define VMA_DEDICATED_ALLOCATION 0
#ifdef QT_DEBUG
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#endif
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-override")
#if defined(Q_CC_CLANG) && Q_CC_CLANG >= 1100
QT_WARNING_DISABLE_CLANG("-Wdeprecated-copy")
#endif
#include "Utils\vk_mem_alloc.h"
QT_WARNING_POP


void QRhiEx::Signal::request() {
	bDirty = true;
}


bool QRhiEx::Signal::receive()
{
	bool var = bDirty;
	bDirty = false;
	return var;
}

bool QRhiEx::Signal::peek() {
	return bDirty;
}

void QRhiEx::globalInitialize() {
	QDetailWidgetManager::instance()->RegisterInstanceFilter<QInstacneDetail_QRhiUniformBlock>();
}

QVulkanInstance* QRhiEx::getVkInstance() {
	static QVulkanInstance vkInstance;
	if (!vkInstance.isValid()) {
		vkInstance.setLayers({ "VK_LAYER_KHRONOS_validation" });
		vkInstance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
		if (!vkInstance.create())
			qFatal("Failed to create Vulkan instance");
	}
	return &vkInstance;
}

QSharedPointer<QRhiEx> QRhiEx::newRhiEx(QRhi::Implementation inBackend /*= QRhi::Vulkan*/, QRhi::Flags inFlags /*= QRhi::Flag()*/, QWindow* inWindow /*= nullptr*/) {
	static bool bIsInitialize = false;
	if (!bIsInitialize) {
		globalInitialize();
		bIsInitialize = true;
	}
	
	QSharedPointer<QRhiEx> mRhi;
	if (inBackend == QRhi::Null) {
		QRhiNullInitParams params;
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::Null, &params, inFlags))));
	}

#ifndef QT_NO_OPENGL
	if (inBackend == QRhi::OpenGLES2) {
		QRhiGles2InitParams params;
		params.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
		params.window = inWindow;
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::OpenGLES2, &params, inFlags))));
	}
#endif

#if QT_CONFIG(vulkan)
	if (inBackend == QRhi::Vulkan) {
		QVulkanInstance* vkInstance = getVkInstance();
		QRhiVulkanInitParams params;
		if (inWindow) {
			inWindow->setVulkanInstance(vkInstance);
			params.window = inWindow;
		}
		params.inst = vkInstance;
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::Vulkan, &params, inFlags))));
	}
#endif

#ifdef Q_OS_WIN
	if (inBackend == QRhi::D3D11) {
		QRhiD3D11InitParams params;
		params.enableDebugLayer = true;
		//if (framesUntilTdr > 0) {
		//	params.framesUntilKillingDeviceViaTdr = mInitParams.framesUntilTdr;
		//	params.repeatDeviceKill = true;
		//}
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::D3D11, &params, inFlags))));
	}
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
	if (inBackend == QRhi::Metal) {
		QRhiMetalInitParams params;
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::Metal, &params, inFlags))));
	}
#endif
	return mRhi;
}

QShader QRhiEx::newShaderFromCode(QShader::Stage stage, const char* code) {
	QShaderBaker baker;
	baker.setGeneratedShaderVariants({ QShader::StandardShader });
	baker.setGeneratedShaders({
		QShaderBaker::GeneratedShader{QShader::Source::SpirvShader,QShaderVersion(100)},
		QShaderBaker::GeneratedShader{QShader::Source::GlslShader,QShaderVersion(430)},
		QShaderBaker::GeneratedShader{QShader::Source::MslShader,QShaderVersion(12)},
		QShaderBaker::GeneratedShader{QShader::Source::HlslShader,QShaderVersion(50)},
		});

	baker.setSourceString(code, stage);
	QShader shader = baker.bake();
	if (!shader.isValid()) {
		qWarning(code);
		qWarning(baker.errorMessage().toLocal8Bit());
	}
	return shader;
}

QShader QRhiEx::newShaderFromQSBFile(const char* filename) {
	QFile f(filename);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());
	return QShader();
}

struct QVkBufferEx : public QRhiBuffer {
	QVkBufferEx(QRhiImplementation* rhi, Type type, VkBufferUsageFlags usage, int size);
	~QVkBufferEx();
	void destroy() override;
	bool create() override;
	QRhiBuffer::NativeBuffer nativeBuffer() override;
	char* beginFullDynamicBufferUpdateForCurrentFrame() override;
	void endFullDynamicBufferUpdateForCurrentFrame() override;

	VkBuffer buffers[QVK_FRAMES_IN_FLIGHT];
	QVkAlloc allocations[QVK_FRAMES_IN_FLIGHT];
	struct DynamicUpdate {
		int offset;
		QRhiBufferData data;
	};
	QVarLengthArray<DynamicUpdate, 16> pendingDynamicUpdates[QVK_FRAMES_IN_FLIGHT];
	VkBuffer stagingBuffers[QVK_FRAMES_IN_FLIGHT];
	QVkAlloc stagingAllocations[QVK_FRAMES_IN_FLIGHT];
	struct UsageState {
		VkAccessFlags access = 0;
		VkPipelineStageFlags stage = 0;
	};
	UsageState usageState[QVK_FRAMES_IN_FLIGHT];
	int lastActiveFrameSlot = -1;
	uint generation = 0;
	friend class QRhiVulkan;
	VkBufferUsageFlags mUsage;
};

QRhiBuffer* QRhiEx::newVkBuffer(QRhiBuffer::Type type, VkBufferUsageFlags flags, int size) {
	QRhiImplementation* ptr = (*(QRhiImplementation**)this);
	return new QVkBufferEx(ptr, type, flags, size);
}

static inline VmaAllocation toVmaAllocation(QVkAlloc a) {
	return reinterpret_cast<VmaAllocation>(a);
}

static inline VmaAllocator toVmaAllocator(QVkAllocator a) {
	return reinterpret_cast<VmaAllocator>(a);
}

static inline QRhiBuffer::UsageFlags toQRhiufferUsage(VkBufferUsageFlags usage) {
	QRhiBuffer::UsageFlags u;
	if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
		u |= QRhiBuffer::VertexBuffer;
	else if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
		u |= QRhiBuffer::IndexBuffer;
	else if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		u |= QRhiBuffer::UniformBuffer;
	else if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
		u |= QRhiBuffer::StorageBuffer;
	return u;
}

QVkBufferEx::QVkBufferEx(QRhiImplementation* rhi, Type type, VkBufferUsageFlags usage, int size)
	: QRhiBuffer(rhi, type, toQRhiufferUsage(usage), size) {
	mUsage = usage;
	for (int i = 0; i < QVK_FRAMES_IN_FLIGHT; ++i) {
		buffers[i] = stagingBuffers[i] = VK_NULL_HANDLE;
		allocations[i] = stagingAllocations[i] = nullptr;
	}
}

QVkBufferEx::~QVkBufferEx() {
	destroy();
}

void QVkBufferEx::destroy() {
	if (!buffers[0])
		return;

	QRhiVulkan::DeferredReleaseEntry e;
	e.type = QRhiVulkan::DeferredReleaseEntry::Buffer;
	e.lastActiveFrameSlot = lastActiveFrameSlot;

	for (int i = 0; i < QVK_FRAMES_IN_FLIGHT; ++i) {
		e.buffer.buffers[i] = buffers[i];
		e.buffer.allocations[i] = allocations[i];
		e.buffer.stagingBuffers[i] = stagingBuffers[i];
		e.buffer.stagingAllocations[i] = stagingAllocations[i];

		buffers[i] = VK_NULL_HANDLE;
		allocations[i] = nullptr;
		stagingBuffers[i] = VK_NULL_HANDLE;
		stagingAllocations[i] = nullptr;
		pendingDynamicUpdates[i].clear();
	}

	QRHI_RES_RHI(QRhiVulkan);
	// destroy() implementations, unlike other functions, are expected to test
	// for m_rhi being null, to allow surviving in case one attempts to destroy
	// a (leaked) resource after the QRhi.
	if (rhiD) {
		rhiD->releaseQueue.append(e);
		rhiD->unregisterResource(this);
	}
}

bool QVkBufferEx::create() {
	if (buffers[0])
		destroy();

	if (m_usage.testFlag(QRhiBuffer::StorageBuffer) && m_type == Dynamic) {
		qWarning("StorageBuffer cannot be combined with Dynamic");
		return false;
	}

	const int nonZeroSize = m_size <= 0 ? 256 : m_size;

	VkBufferCreateInfo bufferInfo;
	memset(&bufferInfo, 0, sizeof(bufferInfo));
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = uint32_t(nonZeroSize);
	bufferInfo.usage = mUsage;

	VmaAllocationCreateInfo allocInfo;
	memset(&allocInfo, 0, sizeof(allocInfo));

	if (m_type == Dynamic) {
	#ifndef Q_OS_DARWIN // not for MoltenVK
		allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	#endif
		// host visible, frequent changes
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	}
	else {
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	QRHI_RES_RHI(QRhiVulkan);
	VkResult err = VK_SUCCESS;
	for (int i = 0; i < QVK_FRAMES_IN_FLIGHT; ++i) {
		buffers[i] = VK_NULL_HANDLE;
		allocations[i] = nullptr;
		usageState[i].access = usageState[i].stage = 0;
		if (i == 0 || m_type == Dynamic) {
			VmaAllocation allocation;
			err = vmaCreateBuffer(toVmaAllocator(rhiD->allocator), &bufferInfo, &allocInfo, &buffers[i], &allocation, nullptr);
			if (err != VK_SUCCESS)
				break;
			allocations[i] = allocation;
		}
	}

	if (err != VK_SUCCESS) {
		qWarning("Failed to create buffer: %d", err);
		return false;
	}

	lastActiveFrameSlot = -1;
	generation += 1;
	rhiD->registerResource(this);
	return true;
}

void executeBufferHostWritesForSlot(QVkBuffer* bufD, int slot, QRhiVulkan* rhiD) {
	if (bufD->pendingDynamicUpdates[slot].isEmpty())
		return;

	//Q_ASSERT(bufD->m_type == QRhiBuffer::Dynamic);
	void* p = nullptr;
	VmaAllocation a = toVmaAllocation(bufD->allocations[slot]);
	// The vmaMap/Unmap are basically a no-op when persistently mapped since it
	// refcounts; this is great because we don't need to care if the allocation
	// was created as persistently mapped or not.
	VkResult err = vmaMapMemory(toVmaAllocator(rhiD->allocator), a, &p);
	if (err != VK_SUCCESS) {
		qWarning("Failed to map buffer: %d", err);
		return;
	}
	int changeBegin = -1;
	int changeEnd = -1;
	for (const QVkBuffer::DynamicUpdate& u : qAsConst(bufD->pendingDynamicUpdates[slot])) {
		memcpy(static_cast<char*>(p) + u.offset, u.data.constData(), size_t(u.data.size()));
		if (changeBegin == -1 || u.offset < changeBegin)
			changeBegin = u.offset;
		if (changeEnd == -1 || u.offset + u.data.size() > changeEnd)
			changeEnd = u.offset + u.data.size();
	}
	vmaUnmapMemory(toVmaAllocator(rhiD->allocator), a);
	if (changeBegin >= 0)
		vmaFlushAllocation(toVmaAllocator(rhiD->allocator), a, VkDeviceSize(changeBegin), VkDeviceSize(changeEnd - changeBegin));

	bufD->pendingDynamicUpdates[slot].clear();
}

QRhiBuffer::NativeBuffer QVkBufferEx::nativeBuffer() {
	if (m_type == Dynamic) {
		QRHI_RES_RHI(QRhiVulkan);
		NativeBuffer b;
		Q_ASSERT(sizeof(b.objects) / sizeof(b.objects[0]) >= size_t(QVK_FRAMES_IN_FLIGHT));
		for (int i = 0; i < QVK_FRAMES_IN_FLIGHT; ++i) {
			executeBufferHostWritesForSlot((QVkBuffer*)this, i,rhiD);
			b.objects[i] = &buffers[i];
		}
		b.slotCount = QVK_FRAMES_IN_FLIGHT;
		return b;
	}
	return { { &buffers[0] }, 1 };
}

char* QVkBufferEx::beginFullDynamicBufferUpdateForCurrentFrame() {
	Q_ASSERT(m_type == Dynamic);
	QRHI_RES_RHI(QRhiVulkan);
	Q_ASSERT(rhiD->inFrame);
	const int slot = rhiD->currentFrameSlot;
	void* p = nullptr;
	VmaAllocation a = toVmaAllocation(allocations[slot]);
	VkResult err = vmaMapMemory(toVmaAllocator(rhiD->allocator), a, &p);
	if (err != VK_SUCCESS) {
		qWarning("Failed to map buffer: %d", err);
		return nullptr;
	}
	return static_cast<char*>(p);
}

void QVkBufferEx::endFullDynamicBufferUpdateForCurrentFrame() {
	QRHI_RES_RHI(QRhiVulkan);
	const int slot = rhiD->currentFrameSlot;
	VmaAllocation a = toVmaAllocation(allocations[slot]);
	vmaUnmapMemory(toVmaAllocator(rhiD->allocator), a);
	vmaFlushAllocation(toVmaAllocator(rhiD->allocator), a, 0, m_size);
}
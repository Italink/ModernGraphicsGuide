#ifndef QRhiGraphicsPipelineBuilder_h__
#define QRhiGraphicsPipelineBuilder_h__

#include "RHI\QRhiUniform.h"
#include "QObject"

class QRhiVertexInputAttributeEx : public QRhiVertexInputAttribute {
public:
	QRhiVertexInputAttributeEx(QString name, int binding, int location, Format format, quint32 offset, int matrixSlice = -1)
		: QRhiVertexInputAttribute(binding, location, format, offset, matrixSlice)
		, mName(name) {
	}
	QString mName;
};

class QRhiVertexInputBindingEx :public QRhiVertexInputBinding {
public:
	QRhiVertexInputBindingEx(QRhiBuffer* buffer, quint32 stride, int offset = 0, Classification cls = PerVertex, int stepRate = 1)
		: QRhiVertexInputBinding(stride, cls, stepRate)
		, mBuffer(buffer)
		, mOffset(offset)
	{
	}
	QRhiBuffer* mBuffer;
	int mOffset = 0;
};

class QRhiGraphicsPipelineBuilder: public QObject{
public:
	QRhiEx::Signal sigRebuild;
public:
	QRhiGraphicsPipelineBuilder(){}
	void setShaderMainCode(QRhiShaderStage::Type inStage, QByteArray inCode);
	void setInputAttribute(QVector<QRhiVertexInputAttributeEx> inInputAttributes);
	void setInputBindings(QVector<QRhiVertexInputBindingEx> inInputBindings);
	QRhiUniform* addUniformBlock(QRhiShaderStage::Type inStage, const QString& inName);
	QRhiUniform* getUniformBlock(const QString& inName);
	QVector<QRhiCommandBuffer::VertexInput> getVertexInputs();
	QRhiShaderResourceBindings* getShaderResourceBindings();
	QRhiGraphicsPipeline* getGraphicsPipeline() { return mPipeline.get(); }
	void create(IRenderComponent* inRenderComponent);
	void update(QRhiResourceUpdateBatch* batch);
	QByteArray getInputFormatTypeName(QRhiVertexInputAttribute::Format inFormat);
	QByteArray getOutputFormatTypeName(QRhiTexture::Format inFormat);
protected:
	void recreateShaderBindings(IRenderComponent* inRenderComponent, QRhiEx *inRhi);
private:
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QRhiGraphicsPipeline::Topology mTopology = QRhiGraphicsPipeline::Triangles;
	QRhiGraphicsPipeline::CullMode mCullMode = QRhiGraphicsPipeline::None;
	QRhiGraphicsPipeline::FrontFace mFrontFace = QRhiGraphicsPipeline::CCW;
	QVector<QRhiGraphicsPipeline::TargetBlend> mBlendStates;
	bool bEnableDepthTest = false;
	bool bEnableDepthWrite = false;
	QRhiGraphicsPipeline::CompareOp mDepthTestOp = QRhiGraphicsPipeline::Less;
	bool bEnableStencilTest = false;
	QRhiGraphicsPipeline::StencilOpState mStencilFrontOp;
	QRhiGraphicsPipeline::StencilOpState mStencilBackOp;
	quint32 mStencilReadMask = 0xFF;
	quint32 mStencilWriteMask = 0xFF;
	float mLineWidth = 1.0f;
	int mDepthBias = 0;
	float mSlopeScaledDepthBias = 0.0f;
	int mPatchControlPointCount = 3;
	QRhiGraphicsPipeline::PolygonMode mPolygonMode = QRhiGraphicsPipeline::Fill;
	QRhiVertexInputLayout mVertexInputLayout;
	QVector<QRhiVertexInputAttributeEx> mInputAttributes;
	QVector<QRhiVertexInputBindingEx> mInputBindings;
	struct StageInfo {
		QVector<QSharedPointer<QRhiUniform>> mUniformBlocks;
		QByteArray VersionCode = "#version 440\n";
		QByteArray DefineCode;
		QByteArray MainCode;
	};
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QHash<QRhiShaderStage::Type, StageInfo> mStageInfos;
	QHash<QString, QRhiUniform*> mUniformMap;
};

#endif // QRhiGraphicsPipelineBuilder_h__

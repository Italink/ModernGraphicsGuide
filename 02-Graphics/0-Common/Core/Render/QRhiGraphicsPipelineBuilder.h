#ifndef QRhiGraphicsPipelineBuilder_h__
#define QRhiGraphicsPipelineBuilder_h__

#include "RHI\QRhiUniform.h"
#include "QObject"
#include "QDetailWidgetMacros.h"

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
	Q_OBJECT
		Q_PROPERTY(QRhiGraphicsPipeline::Topology Topology READ getTopology WRITE setTopology)
		Q_PROPERTY(QRhiGraphicsPipeline::PolygonMode PolygonMode READ getPolygonMode WRITE setPolygonMode)
		Q_PROPERTY(QRhiGraphicsPipeline::CullMode CullMode READ getCullMode WRITE setCullMode)
		Q_PROPERTY(QRhiGraphicsPipeline::FrontFace FrontFace READ getFrontFace WRITE setFrontFace)
		Q_PROPERTY(float LineWidth READ getLineWidth WRITE setLineWidth)

		Q_PROPERTY(bool DepthTest READ isDepthTestEnabled WRITE setDepthTest)
		Q_PROPERTY(bool DepthWrite READ isDepthWriteEnabled WRITE setDepthWrite)
		Q_PROPERTY(QRhiGraphicsPipeline::CompareOp DepthTestOp READ getDepthTestOp WRITE setDepthTestOp)

		Q_PROPERTY(bool StencilTest READ isStencilTestEnabled WRITE setStencilTest)
		Q_PROPERTY(QRhiGraphicsPipeline::StencilOpState StencilFrontOp READ getStencilFrontOp WRITE setStencilFrontOp)
		Q_PROPERTY(QRhiGraphicsPipeline::StencilOpState StencilBackOp READ getStencilBackOp WRITE setStencilBackOp)
		Q_PROPERTY(quint32 StencilReadMask READ getStencilReadMask WRITE setStencilReadMask)
		Q_PROPERTY(quint32 StencilWriteMask READ getStencilWriteMask WRITE setStencilWriteMask)
		Q_PROPERTY(QMap<QString, QRhiUniform*> UniformBlocks READ getUniformBlocks WRITE setUniformBlocks)

		Q_META_BEGIN(QRhiGraphicsPipelineBuilder)
			Q_META_P_MAP_FIXED_KEY(UniformBlocks,true)
			Q_META_P_MAP_FIXED_SIZE(UniformBlocks,true)
			Q_META_LOCAL_ENUM(Topology, QRhiGraphicsPipeline::Points, QRhiGraphicsPipeline::Lines, QRhiGraphicsPipeline::LineStrip, QRhiGraphicsPipeline::Triangles, QRhiGraphicsPipeline::TriangleStrip, QRhiGraphicsPipeline::TriangleFan, QRhiGraphicsPipeline::Patches)
			Q_META_LOCAL_ENUM(PolygonMode, QRhiGraphicsPipeline::PolygonMode::Fill, QRhiGraphicsPipeline::PolygonMode::Line)
			Q_META_LOCAL_ENUM(CullMode, QRhiGraphicsPipeline::CullMode::None,QRhiGraphicsPipeline::CullMode::Front, QRhiGraphicsPipeline::CullMode::Back)
			Q_META_LOCAL_ENUM(FrontFace, QRhiGraphicsPipeline::FrontFace::CCW, QRhiGraphicsPipeline::FrontFace::CW)
			Q_META_LOCAL_ENUM(CompareOp, QRhiGraphicsPipeline::CompareOp::Never,  QRhiGraphicsPipeline::CompareOp::Less, QRhiGraphicsPipeline::CompareOp::Equal, QRhiGraphicsPipeline::CompareOp::LessOrEqual, QRhiGraphicsPipeline::CompareOp::Greater, QRhiGraphicsPipeline::CompareOp::NotEqual, QRhiGraphicsPipeline::CompareOp::GreaterOrEqual,QRhiGraphicsPipeline::CompareOp::Always)
		Q_META_END()
public:

	QRhiEx::Signal sigRebuild;

	QRhiGraphicsPipeline::Topology getTopology() const { return mTopology; }
	void setTopology(QRhiGraphicsPipeline::Topology val) {mTopology = val;  sigRebuild.request();}
	QRhiGraphicsPipeline::CullMode getCullMode() const { return mCullMode; }
	void setCullMode(QRhiGraphicsPipeline::CullMode val) { mCullMode = val; sigRebuild.request(); }
	QRhiGraphicsPipeline::FrontFace getFrontFace() const { return mFrontFace; }
	void setFrontFace(QRhiGraphicsPipeline::FrontFace val) { mFrontFace = val; sigRebuild.request(); }
	bool isDepthTestEnabled() const { return bEnableDepthTest; }
	void setDepthTest(bool val) { bEnableDepthTest = val; sigRebuild.request(); }
	bool isDepthWriteEnabled() const { return bEnableDepthWrite; }
	void setDepthWrite(bool val) { bEnableDepthWrite = val; sigRebuild.request();  }
	QRhiGraphicsPipeline::CompareOp getDepthTestOp() const { return mDepthTestOp; }
	void setDepthTestOp(QRhiGraphicsPipeline::CompareOp val) { mDepthTestOp = val; sigRebuild.request();  }
	bool isStencilTestEnabled() const { return bEnableStencilTest; }
	void setStencilTest(bool val) { bEnableStencilTest = val;  sigRebuild.request();  }
	QRhiGraphicsPipeline::StencilOpState getStencilFrontOp() const { return mStencilFrontOp; }
	void setStencilFrontOp(QRhiGraphicsPipeline::StencilOpState val) { mStencilFrontOp = val; sigRebuild.request();  }
	QRhiGraphicsPipeline::StencilOpState getStencilBackOp() const { return mStencilBackOp; }
	void setStencilBackOp(QRhiGraphicsPipeline::StencilOpState val) { mStencilBackOp = val; sigRebuild.request();  }
	quint32 getStencilReadMask() const { return mStencilReadMask; }
	void setStencilReadMask(quint32 val) { mStencilReadMask = val;  sigRebuild.request(); }
	quint32 getStencilWriteMask() const { return mStencilWriteMask; }
	void setStencilWriteMask(quint32 val) { mStencilWriteMask = val; sigRebuild.request();  }
	float getLineWidth() const { return mLineWidth; }
	void setLineWidth(float val) { mLineWidth = val; sigRebuild.request();  }
	QRhiGraphicsPipeline::PolygonMode getPolygonMode() const { return mPolygonMode; }
	void setPolygonMode(QRhiGraphicsPipeline::PolygonMode val) { mPolygonMode = val; sigRebuild.request();  }
	QMap<QString, QRhiUniform*> getUniformBlocks() { return mUniformMap; }
	void setUniformBlocks(QMap<QString, QRhiUniform*>) {}
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
	QRhiGraphicsPipeline::PolygonMode mPolygonMode = QRhiGraphicsPipeline::Fill;
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
	QMap<QString, QRhiUniform*> mUniformMap;
};

#endif // QRhiGraphicsPipelineBuilder_h__

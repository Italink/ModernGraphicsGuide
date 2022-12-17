// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/RenderPass/QDefaultSceneRenderPass.h"
#include "Render/RenderComponent/QStaticMeshRenderComponent.h"

int main(int argc, char **argv)
{
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->node("Triangle", (new QDefaultSceneRenderPass())
		->addRenderComponent(
			(new QStaticMeshRenderComponent)
			->setupStaticMeshPath("E:/QEngine/Asset/Model/FBX/Genji/Genji.FBX")
		)
	)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

#ifndef MathUtils_h__
#define MathUtils_h__

#include "QGenericMatrix"
#include "qvectornd.h"
#include "QMatrix4x4"

namespace MathUtils {

using Mat4 = QGenericMatrix<4, 4, float>;

void setMatTranslate(QMatrix4x4& mat4, QVector3D translate);
void setMatRotation(QMatrix4x4& mat4, QVector3D rotation);
void setMatScale3D(QMatrix4x4& mat4, QVector3D scale3D);
void setMatOrthoNormalize(QMatrix4x4& mat4);

QVector3D getMatTranslate(const QMatrix4x4& mat4);
QVector3D getMatRotation(const QMatrix4x4& mat4);
QVector3D getMatScale3D(const QMatrix4x4& mat4);

}


#endif // MathUtils_h__

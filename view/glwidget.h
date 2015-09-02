#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QEvent>
#include <QMouseEvent>
#include <QGLWidget>
#include <QtOpenGL>
#include <QGLShader>
#include <QGLShaderProgram>
#include <QString>
#include <QFileDialog>

#include "model/model.h"
#include "utility/mesh.h"
#include "utility/meshreader.h"
#include "utility/meshwriter.h"
#include "utility/simplemeshmerger.h"
#include "utility/meshbooleanfunctions.h"
#include "utility/shader.h"
#include "utility/basicoctree.h"
#include "utility/extendedoctree.h"

#define TRANSLATION_TAB 0
#define MODEL_TAB       1
#define HOLLOWING_TAB   2

#define TRANSLATION_VIEW_XY         0
#define TRANSLATION_VIEW_Z          1
#define ROTATION_SCALE_VIEW         2
#define TRANSLATION_VIEW_DEFAULT    3

#define TRANSLATION_XY_RATIO    40.0f
#define TRANSLATION_Z_RATIO     30.0f
#define SCALE_RATIO 30.0f;

class GLWidget : public QGLWidget
{
    Q_OBJECT
private:

    int viewState;

    QGLShaderProgram* shader;

    QMatrix4x4 projection_matrix;
    QMatrix4x4 model_matrix;

    QMatrix4x4 last_object_model_matrix;
    QMatrix4x4 last_rotation_axis_model_matrix;

    QMatrix4x4 view_matrix;

    QVector3D camera_position;
    QVector3D camera_direction;
    QVector3D camera_up;

    QVector4D ambient_light;
    QVector4D diffuse_light;
    QVector4D direction_light;

    Mesh* object;
    Mesh* objectShell;

    Mesh* grid;
    Mesh* rot_axis;
    Mesh* half_sphere;
    Mesh* yoyo_area;
    Mesh* yoyo_connection;

    bool left_pressed;
    bool right_pressed;
    bool middle_pressed;

    int rot_obj_phi;
    int rot_obj_psy;

    int rot_cam_phi;

    QVector3D middle;
    GLfloat scaleFactor;

    GLfloat trans_x;
    GLfloat trans_y;
    GLfloat trans_z;
    GLfloat scale_xyz;

    QPoint mouse_pos;

    bool showOuterSurface;
    bool showInnerSurface;
    bool showGrid;

    GLint startDepth;
    GLint maximumDepth;
    GLint shellExtensionValue;

    GLfloat lowest_y_rot_axis;
    GLfloat lowest_y_half_sphere;

    bool topOptimized;
    bool tippeTopOptimized;

    bool rebuildOctree;

public:
    explicit GLWidget(QWidget *parent = 0);
    ~GLWidget();



public slots:
    void loadNewMesh();
    void makeItSpin();
    void showOnlyOuterSurface();
    void showOnlyInnerSurface();
    void showOnlyOctreeGrid();

    void setView(int index);
    void setViewXY();
    void setViewZ();
    void setViewRotationScale();

    void resetXY();
    void resetZ();
    void resetRotationScale();
    void resetAll();

    void setStartDepthValue(int);
    void setMaximumDepthValue(int);

    void setShellExtensionValue(int);
    void calculateOctree();
    void saveMesh();

    void setYoyo();
    void setTop();
    void setTippeTop(bool tippeTopOptimized);

signals:
    void setViewXYSignal();
    void setStartDepthSpinBoxValue(int);
    void setMaximumDepthSpinBoxValue(int);
    void setShellExtensionSpinBoxValue(int);

    void modelLoaded(bool);
    void shellIsSet(bool);
    void shellIsNotSet(bool);
    void shellIsNotSet();

protected:

    void setViewDefault();

    void loadInitialMesh();

    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void mouseMoveEvent(QMouseEvent* ev);
    void mousePressEvent(QMouseEvent* ev);
    void mouseReleaseEvent(QMouseEvent* ev);

    void saveMeshAsTippeTop(QString fileName);
    void saveMeshAsTop(QString fileName);    
    void saveMeshAsYoyo(QString fileName);

    void resetGLWidget();

};

#endif // GLWIDGET_H

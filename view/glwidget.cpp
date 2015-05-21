#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    QGLFormat glFormat;
    glFormat.setVersion( 3, 2 );
    glFormat.setProfile( QGLFormat::CoreProfile );
}

GLWidget::~GLWidget()
{
    delete shader;

}

void GLWidget::initializeGL()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    qglClearColor(QColor(205, 205, 255));

    shader = new QGLShaderProgram();
    shader->addShaderFromSourceFile(QGLShader::Vertex, ":/shader/simple.vsh");
    shader->addShaderFromSourceFile(QGLShader::Fragment, ":/shader/simple.fsh");
    shader->link();

    camera_position.setX(1);
    camera_position.setY(-0.75);
    camera_position.setZ(1);

    camera_direction.setX(0);
    camera_direction.setY(0);
    camera_direction.setZ(0);

    camera_up.setX(0);
    camera_up.setY(1);
    camera_up.setZ(0);

    ambient_light.setX(0.75);
    ambient_light.setY(0.75);
    ambient_light.setZ(0.75);
    ambient_light.setW(1);

    diffuse_light.setX(0.75);
    diffuse_light.setY(0.75);
    diffuse_light.setZ(0.75);
    diffuse_light.setW(1);

    object = readMeshFromObjFile("monkey");
    rot_axis = readMeshFromObjFile("rot_axis");

    QVector<GLfloat>* geometry = new QVector<GLfloat>();
    QVector<GLfloat>* normals = new QVector<GLfloat>();
    QVector<GLshort>* indices = new QVector<GLshort>();

    int idx = 0;
    for (float x = -3; x <= 3; x += 0.5) {
        for (float z = -3; z <= 3; z += 0.5) {
            geometry->push_back(x);
            geometry->push_back(0);
            geometry->push_back(z);

            normals->push_back(0);
            normals->push_back(1);
            normals->push_back(0);

            if (z < 3) {
                indices->push_back(idx);
                indices->push_back(idx + 1);

            }
            if (x < 3) {
                indices->push_back(idx);
                indices->push_back(idx + 13);
            }
            idx += 1;
        }
    }

    grid = new Mesh(geometry, normals, indices);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->bind();

    view_matrix.setToIdentity();
    view_matrix.lookAt(camera_position, camera_direction, camera_up);
    view_matrix.rotate(rot_cam_phi, 0.0, 1.0, 0.0);

    direction_light.setX(camera_direction.normalized().x() - camera_position.normalized().x());
    direction_light.setY(camera_direction.normalized().y() - camera_position.normalized().y());
    direction_light.setZ(camera_direction.normalized().z() - camera_position.normalized().z());
    direction_light.setW(0);

    shader->setUniformValue("direction_light", direction_light);
    shader->setUniformValue("ambient_light", ambient_light);
    shader->setUniformValue("diffuse_light", diffuse_light);

    model_matrix.setToIdentity();
    QVector3D mean = object->getMean();
    //model_matrix.translate(-mean.x(), -mean.y(), -mean.z());
    GLfloat x_min, x_max, y_min, y_max, z_min, z_max;
    for (int i = 0; i < object->getGeometry()->size(); i += 3) {
        if (object->getGeometry()->at(i) > x_max) {
            x_max = object->getGeometry()->at(i);
        }
        if (object->getGeometry()->at(i) < x_min) {
            x_min = object->getGeometry()->at(i);
        }

        if (object->getGeometry()->at(i + 1) > y_max) {
            y_max = object->getGeometry()->at(i + 1);
        }
        if (object->getGeometry()->at(i + 1) < y_min) {
            y_min = object->getGeometry()->at(i + 1);
        }

        if (object->getGeometry()->at(i + 2) > z_max) {
            z_max = object->getGeometry()->at(i + 2);
        }
        if (object->getGeometry()->at(i + 2) < z_min) {
            z_min = object->getGeometry()->at(i + 2);
        }
    }

    model_matrix.translate(-(x_max + x_min) / 2, -(y_max + y_min) / 2, -(z_max + z_min) / 2);
    model_matrix.rotate(rot_obj_phi, 0.0, 1.0, 0.0);
    model_matrix.rotate(rot_obj_psy, 1.0, 0.0, 0.0);
    //model_matrix.translate(mean.x(), mean.y(), mean.z());

    shader->setUniformValue("nMatrix", view_matrix * model_matrix);
    shader->setUniformValue("mvpMatrix", projection_matrix * view_matrix * model_matrix);

    shader->setUniformValue("color", QColor(115, 115, 85));
    object->render(shader, GL_TRIANGLES);

    model_matrix.setToIdentity();
    GLfloat lowest_y = 0.0;
    for (int i = 1; i < rot_axis->getGeometry()->size(); i += 3) {
        if (rot_axis->getGeometry()->at(i) < lowest_y) {
            lowest_y = rot_axis->getGeometry()->at(i);
        }
    }
    model_matrix.translate(0.0, -2 - lowest_y, 0.0);
    shader->setUniformValue("nMatrix", view_matrix * model_matrix);
    shader->setUniformValue("mvpMatrix", projection_matrix * view_matrix * model_matrix);
    shader->setUniformValue("color", QColor(Qt::red));
    rot_axis->render(shader, GL_TRIANGLES);

    model_matrix.setToIdentity();
    model_matrix.translate(0.0, -2.0, 0.0);
    shader->setUniformValue("nMatrix", view_matrix * model_matrix);
    shader->setUniformValue("mvpMatrix", projection_matrix * view_matrix * model_matrix);
    shader->setUniformValue("color", QColor(Qt::black));
    grid->render(shader, GL_LINES);


    shader->release();
}

void GLWidget::resizeGL(int width, int height)
{
    projection_matrix.setToIdentity();
    projection_matrix.ortho(-6, 6, -6, 6, 6, -6);

    glViewport(0,0, width, height);
}

void GLWidget::mouseMoveEvent(QMouseEvent *ev)
{
    if (left_pressed) {
        if (mouse_pos.x() > ev->pos().x()) {
            rot_obj_phi += (mouse_pos.x() - ev->pos().x());
        }
        if (mouse_pos.x() < ev->pos().x()) {
            rot_obj_phi -= (ev->pos().x() - mouse_pos.x());
        }
        rot_obj_phi %= 360;

        if (mouse_pos.y() > ev->pos().y()) {
            rot_obj_psy += (mouse_pos.y() - ev->pos().y());
        }
        if (mouse_pos.y() < ev->pos().y()) {
            rot_obj_psy -= (ev->pos().y() - mouse_pos.y());
        }
        rot_obj_psy %= 360;

        mouse_pos = ev->pos();
        this->updateGL();
    }
    if (right_pressed) {
        if (mouse_pos.x() > ev->pos().x()) {
            rot_cam_phi += (mouse_pos.x() - ev->pos().x());
        }
        if (mouse_pos.x() < ev->pos().x()) {
            rot_cam_phi -= (ev->pos().x() - mouse_pos.x());
        }
        rot_cam_phi %= 360;

        mouse_pos = ev->pos();
        this->updateGL();
    }
}

void GLWidget::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        mouse_pos = ev->pos();
        left_pressed = true;
        return;
    }
    if (ev->button() == Qt::RightButton) {
        mouse_pos = ev->pos();
        right_pressed = true;
        return;
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    left_pressed = false;
    right_pressed = false;
}




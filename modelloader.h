#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <scene.h>
#include <postprocess.h>
#include <Importer.hpp>
#include <QString>
#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QSharedPointer>
struct P3T2N3Vertex
{
    QVector3D position;
    //QVector2D texCoord;
    QVector3D normal;
};
struct MaterialInfo
{
    QString Name;
    QVector3D Ambient;
    QVector3D Diffuse;
    QVector3D Specular;
    float Shininess;
};
struct LightInfo
{
    QVector4D Position;
    QVector3D Intensity;
};
struct Mesh
{
    QString name;
    unsigned int indexCount;
    unsigned int indexOffset;
    QSharedPointer<MaterialInfo> material;
};
struct Node
{
    QString name;
    QMatrix4x4 transformation;
    QVector<QSharedPointer<Mesh> > meshes;
    QVector<Node> nodes;
};
class Model;
class ModelLoader
{
public:
    ModelLoader(Model *model);
    ~ModelLoader();
    bool load(QString pathToFile);
    void createModel();
    void createRoundedBox(float r, float scale, int n);

private:
    Model *m_model;
    QSharedPointer<MaterialInfo> processMaterial(aiMaterial *mater);
    QSharedPointer<Mesh> processMesh(aiMesh *mesh);
    void processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode);

    QVector<float> m_vertices;
    QVector<float> m_normals;
    QVector<float> m_uv;
    QVector<unsigned int> m_indices;


    QVector<QSharedPointer<MaterialInfo> > m_materials;
    QVector<QSharedPointer<Mesh> > m_meshes;
    QSharedPointer<Node> m_rootNode;

};

#endif // MODELLOADER_H

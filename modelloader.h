#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <scene.h>
#include <postprocess.h>
#include <Importer.hpp>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QSharedPointer>
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
private:
    Model *m_model;
    QSharedPointer<MaterialInfo> processMaterial(aiMaterial *mater);
    QSharedPointer<Mesh> processMesh(aiMesh *mesh);
    void processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode);

    QVector<float> m_vertices;
    QVector<float> m_normals;
    QVector<unsigned int> m_indices;


    QVector<QSharedPointer<MaterialInfo> > m_materials;
    QVector<QSharedPointer<Mesh> > m_meshes;
    QSharedPointer<Node> m_rootNode;

};

#endif // MODELLOADER_H
